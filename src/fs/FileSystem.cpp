#include <iostream>
#include <sstream>
#include <fstream>
#include <utility>
#include <queue>

#include "FileSystem.hpp"
#include "../util/FSCmdParser.hpp"
#include "sections/data/util/Iterator_DirItems.hpp"


const std::string &Get_FSMessage_String(FSMessages message) {
	return FSMessages_Strings.at(message);
}

FileSystem::FileSystem(const std::string &fs_path, std::ostream &out_stream) :
		_fs_path(fs_path), _out_stream(out_stream), _work_dir(kRoot_Dir_Inode_Idx, kRoot_Dir_Path) {
	if (std::filesystem::exists(_fs_path)) {
		_fs_container = std::make_shared<MMappedFile>(_fs_path);
		Init_Structures();
	}
}

void FileSystem::OP_format(size_t size) {
	_work_dir = {kRoot_Dir_Inode_Idx, kRoot_Dir_Path};
	_fs_container = std::make_shared<MMappedFile>(_fs_path);

	const Superblock::t_Superblock sb_formatted = Get_Formatted_Superblock(size);
	try {
		_fs_container->Resize(sb_formatted.Disk_Size);
		_fs_container->Clear();
	}
	catch (const std::ios_base::failure &) {
		throw FSException{Get_FSMessage_String(FSMessages::kCannotCreateFile)};
	}
	_superblock = std::make_unique<Superblock>(_fs_container, kSuperblock_Offset);
	_superblock->Set(sb_formatted);
	Init_Structures();

	_bm_inodes->Set(kRoot_Dir_Inode_Idx, true);
	Inode root_dir_inode = _inodes->Get(kRoot_Dir_Inode_Idx);
	root_dir_inode
		.Set_Is_Dir(true)
		.Set_Is_Symlink(false)
		.Set_Refs_Cnt(1)
		.Set_File_Size(DataBlock::kSize)
		.Unset_References()
		.Set_Direct(0, kRoot_Dir_DBlock_Idx);

	_bm_data->Set(kRoot_Dir_DBlock_Idx, true);
	DataBlock root_dir_dblock = _data->Get(kRoot_Dir_DBlock_Idx);
	Iterator_DirItems it_root_dir_items{root_dir_inode, _data};
	it_root_dir_items.Append(kRoot_Dir_Inode_Idx, kDot);
	it_root_dir_items.Append(kRoot_Dir_Inode_Idx, kDot_Dot);

	Print_Message(FSMessages::kOk);
}

void FileSystem::OP_load(const std::string &path) {
	if (!std::filesystem::exists(path)) {
		throw FSException{FSMessages::kFileNotFound};
	}

	std::ifstream ifstream{path};
	for (std::string line; std::getline(ifstream, line);) {
		try {
			const auto op = FSCmdParser::Parse(line);
			op(*this);
		}
		catch (const FSException &e) {
			std::cerr << e.what() << std::endl;
		}
	}

	Print_Message(FSMessages::kOk);
}

void FileSystem::OP_cd(const std::string &path) {
	Assert_Is_Formatted();

	const auto result = Resolve_Final_Path(path);
	Inode inode = _inodes->Get(result.first);
	if (!inode.Get_Is_Dir()) {
		throw PathNotFoundException{};
	}

	_work_dir = result;

	Print_Message(FSMessages::kOk);
}

void FileSystem::OP_pwd() const {
	Assert_Is_Formatted();

	_out_stream << _work_dir.second.string() << std::endl;
}

void FileSystem::OP_cp(const std::string &path1, const std::string &path2) {
	Assert_Is_Formatted();

	const uint32_t src_inode_idx = Resolve_Final_Path_Inode(path1);
	Inode src_inode = _inodes->Get(src_inode_idx);
	if (src_inode.Get_Is_Dir()) {
		throw FSException{FSMessages::kFileNotFound};
	}

	uint32_t dst_dir_inode_idx;
	std::string dst_filename;
	try {
		const uint32_t dst_filename_inode_idx = Resolve_Final_Path_Inode(path2);
		if (_inodes->Get(dst_filename_inode_idx).Get_Is_Dir()) {
			dst_dir_inode_idx = dst_filename_inode_idx;
			dst_filename = std::filesystem::path{path1}.filename();
		}
		else {
			throw FSException{FSMessages::kPathNotFound};
		}
	}
	catch (const PathNotFoundException &) {
		dst_dir_inode_idx = Resolve_Final_Parent_Inode(path2);
		dst_filename = std::filesystem::path{path2}.filename();
	}
	Assert_Valid_Filename_Length(dst_filename);

	Inode dst_dir_inode = _inodes->Get(dst_dir_inode_idx);
	if (!dst_dir_inode.Get_Is_Dir()) {
		throw PathNotFoundException{};
	}

	const uint32_t dblocks_cnt = Get_Necessary_Data_Blocks_Cnt(src_inode.Get_File_Size());

	Assert_Has_Resources(*_bm_inodes, 1);
	Assert_Has_Resources(*_bm_data, dblocks_cnt);

	const uint32_t dst_inode_idx = Acquire_Inode();
	Inode dst_inode = _inodes->Get(dst_inode_idx);
	dst_inode
		.Set_Is_Dir(false)
		.Set_Is_Symlink(false)
		.Set_Refs_Cnt(1)
		.Set_File_Size(src_inode.Get_File_Size())
		.Unset_References();

	std::queue<uint32_t> dblocks_idx_pool;
	_bm_data->Process_Free(dblocks_cnt, [&dblocks_idx_pool] (uint32_t dblock_idx) {
		dblocks_idx_pool.push(dblock_idx);
	});

	Iterator_DataBlocks it_src_dblocks{src_inode, _data};
	Iterator_DataBlocks it_dst_dblocks{dst_inode, _data};
	I_ReadableWritable::t_Byte_Buf buf;
	buf.reserve(DataBlock::kSize);
	for (uint32_t left = src_inode.Get_File_Size(); left > 0; ++it_src_dblocks) {
		it_dst_dblocks.Append([this, &dblocks_idx_pool] () {
			const uint32_t dblock_idx = dblocks_idx_pool.front();
			dblocks_idx_pool.pop();
			return Acquire_Data_Block(dblock_idx);
		});
		if (left >= DataBlock::kSize) {
			(*it_src_dblocks).Read_Content(buf);
			(*it_dst_dblocks).Write_Content(buf);
			left -= DataBlock::kSize;
		}
		else {
			(*it_src_dblocks).Read_Content(buf, left);
			(*it_dst_dblocks).Write_Content(buf, left);
			left = 0;
		}

		buf.clear();
	}

	Iterator_DirItems it_dst_dir_items{dst_dir_inode, _data};
	it_dst_dir_items.Append(dst_inode_idx, dst_filename);

	Print_Message(FSMessages::kOk);
}

void FileSystem::OP_mv(const std::string &path1, const std::string &path2) {
	Assert_Is_Formatted();

	const uint32_t src_inode_idx = Resolve_Path_Inode(path1);
	const Inode src_inode = _inodes->Get(src_inode_idx);
	if (src_inode.Get_Is_Dir()) {
		throw FSException{FSMessages::kFileNotFound};
	}

	const uint32_t src_dir_inode_idx = Resolve_Final_Parent_Inode(path1);
	Inode src_dir_inode = _inodes->Get(src_dir_inode_idx);

	uint32_t dst_dir_inode_idx;
	std::string dst_filename;
	try {
		const uint32_t dst_filename_inode_idx = Resolve_Final_Path_Inode(path2);
		if (_inodes->Get(dst_filename_inode_idx).Get_Is_Dir()) {
			dst_dir_inode_idx = dst_filename_inode_idx;
			dst_filename = std::filesystem::path{path1}.filename();
		}
		else {
			throw FSException{FSMessages::kPathNotFound};
		}
	}
	catch (const PathNotFoundException &) {
		dst_dir_inode_idx = Resolve_Final_Parent_Inode(path2);
		dst_filename = std::filesystem::path{path2}.filename();
	}
	Assert_Valid_Filename_Length(dst_filename);

	Inode dst_dir_inode = _inodes->Get(dst_dir_inode_idx);
	if (!dst_dir_inode.Get_Is_Dir()) {
		throw PathNotFoundException{};
	}

	Iterator_DirItems it_dst_dir_items{dst_dir_inode, _data};
	it_dst_dir_items.Append(src_inode_idx, dst_filename);

	Iterator_DirItems it_src_dir_items{src_dir_inode, _data};
	for (; (*it_src_dir_items).Inode_Idx != src_inode_idx; ++it_src_dir_items);
	it_src_dir_items.Remove();

	Print_Message(FSMessages::kOk);
}

void FileSystem::OP_rm(const std::string &path) {
	Assert_Is_Formatted();

	try {
		const uint32_t inode_idx = Resolve_Path_Inode(path);
		Inode inode = _inodes->Get(inode_idx);
		if (inode.Get_Is_Dir()) {
			throw PathNotFoundException{};
		}

		const uint32_t dir_inode_idx = Resolve_Final_Parent_Inode(path);
		Inode dir_inode = _inodes->Get(dir_inode_idx);

		// release dblocks
		Iterator_DataBlocks::Release_All(inode, _data, [this] (uint32_t dblock_idx) {
			_data->Get(dblock_idx).Empty_Content();
			_bm_data->Set(dblock_idx, false);
		});

		// release inode
		_bm_inodes->Set(inode_idx, false);

		Iterator_DirItems it_dir_items{dir_inode, _data};
		for (; (*it_dir_items).Inode_Idx != inode_idx; ++it_dir_items);
		it_dir_items.Remove();

		Print_Message(FSMessages::kOk);
	}
	catch (const PathNotFoundException &) {
		throw FSException{FSMessages::kFileNotFound};
	}
}

void FileSystem::OP_mkdir(const std::string &path) {
	Assert_Is_Formatted();

	try {
		Resolve_Final_Path_Inode(path);
		throw FSException{FSMessages::kExist};
	}
	catch (const PathNotFoundException &) {
		//
	}

	const std::string dir_name = std::filesystem::path{path}.filename();
	Assert_Valid_Filename_Length(dir_name);

	const uint32_t parent_inode_idx = Resolve_Final_Parent_Inode(path);
	Inode parent_inode = _inodes->Get(parent_inode_idx);

	if (!parent_inode.Get_Is_Dir()) {
		throw PathNotFoundException{};
	}

	Assert_Has_Resources(*_bm_inodes, 1);
	Assert_Has_Resources(*_bm_data, 1);

	const uint32_t dir_inode_idx = Acquire_Inode();
	const uint32_t dir_dblock_idx = Acquire_Data_Block();

	Inode dir_inode = _inodes->Get(dir_inode_idx);
	dir_inode
		.Set_Is_Dir(true)
		.Set_Is_Symlink(false)
		.Set_Refs_Cnt(1)
		.Set_File_Size(DataBlock::kSize)
		.Unset_References()
		.Set_Direct(0, dir_dblock_idx);

	Iterator_DirItems it_dir_items{dir_inode, _data};
	it_dir_items.Append(dir_inode_idx, kDot);
	it_dir_items.Append(parent_inode_idx, kDot_Dot);

	Iterator_DirItems it_parent_dir{parent_inode, _data};
	it_parent_dir.Append(dir_inode_idx, dir_name);

	Print_Message(FSMessages::kOk);
}

void FileSystem::OP_rmdir(const std::string &path) {
	Assert_Is_Formatted();

	try {
		const std::string dir_name = std::filesystem::path{path}.filename();
		const uint32_t parent_inode_idx = Resolve_Final_Parent_Inode(path);
		Inode parent_inode = _inodes->Get(parent_inode_idx);

		bool found = false;
		Iterator_DirItems it_parent_dir_items{parent_inode, _data};
		for (; it_parent_dir_items != Iterator_DirItems::kDepleted; ++it_parent_dir_items) {
			if ((*it_parent_dir_items).Item_Name == dir_name) {
				found = true;
				break;
			}
		}
		if (!found) {
			throw PathNotFoundException{};
		}

		const uint32_t dir_inode_idx = (*it_parent_dir_items).Inode_Idx;
		Inode dir_inode = _inodes->Get(dir_inode_idx);
		if (!dir_inode.Get_Is_Dir()) {
			throw PathNotFoundException{};
		}

		Iterator_DirItems it_dir_items{dir_inode, _data};
		++it_dir_items;
		++it_dir_items;
		if (it_dir_items != Iterator_DirItems::kDepleted) {
			throw FSException{FSMessages::kNotEmpty};
		}

		it_parent_dir_items.Remove();

		// release
		_bm_inodes->Set(dir_inode_idx, false);
		Iterator_DataBlocks::Release_All(dir_inode, _data, [this] (uint32_t dblock_idx) {
			_data->Get(dblock_idx).Empty_Content();
			_bm_data->Set(dblock_idx, false);
		});

		Print_Message(FSMessages::kOk);
	}
	catch (const PathNotFoundException &) {
		throw FSException{FSMessages::kFileNotFound};
	}
}

void FileSystem::OP_ls(const std::string &path) const {
	Assert_Is_Formatted();

	Inode inode = _inodes->Get(Resolve_Final_Path_Inode(path));
	if (!inode.Get_Is_Dir()) {
		throw PathNotFoundException{};
	}

	for (Iterator_DirItems it{inode, _data}; it != Iterator_DirItems::kDepleted; ++it) {
		DataBlock::t_DirItem dir_item = *it;

		if (_inodes->Get(dir_item.Inode_Idx).Get_Is_Dir()) {
			_out_stream << "+";
		} else {
			_out_stream << "-";
		}
		_out_stream << dir_item.Item_Name << std::endl;
	}
}

void FileSystem::OP_cat(const std::string &path) const {
	Assert_Is_Formatted();

	try {
		const uint32_t inode_idx = Resolve_Final_Path_Inode(path);
		Inode inode = _inodes->Get(inode_idx);
		if (inode.Get_Is_Dir()) {
			throw PathNotFoundException{};
		}

		I_ReadableWritable::t_Byte_Buf buf;
		buf.reserve(DataBlock::kSize);
		Iterator_DataBlocks it_dblocks{inode, _data};
		for (uint32_t left = inode.Get_File_Size(); left > 0; ++it_dblocks) {
			const DataBlock dblock = *it_dblocks;
			if (left >= DataBlock::kSize) {
				dblock.Read_Content(buf);
				left -= DataBlock::kSize;
			} else {
				dblock.Read_Content(buf, left);
				left = 0;
			}

			for (const std::byte byte : buf) {
				_out_stream << static_cast<char>(byte);
			}
			buf.clear();
		}
	}
	catch (const PathNotFoundException &) {
		throw FSException{FSMessages::kFileNotFound};
	}
}

void FileSystem::OP_info(const std::string &path) const {
	Assert_Is_Formatted();

	try {
		const uint32_t inode_idx = Resolve_Path_Inode(path);
		Inode inode = _inodes->Get(inode_idx);

		const std::string nil = "nil";

		_out_stream << std::filesystem::path{path}.filename().string();
		_out_stream << " - " << inode.Get_File_Size();
		_out_stream << " - i-node " << inode_idx;

		_out_stream << " - Direct";
		for (uint32_t i = 0; i < Inode::kDirect_Refs_Cnt; ++i) {
			const uint32_t direct = inode.Get_Direct(i);
			_out_stream << " [" << i << "]";
			if (direct != Inode::kDirect_Ref_Unset) {
				_out_stream << direct;
			}
			else {
				_out_stream << nil;
			}
		}

		const uint32_t indirect1 = inode.Get_Indirect1();
		_out_stream << " - Indirect1 ";
		if (indirect1 != Inode::kIndirect_Ref_Unset) {
			_out_stream << indirect1;
		}
		else {
			_out_stream << nil;
		}

		const uint32_t indirect2 = inode.Get_Indirect2();
		_out_stream << " - Indirect2 ";
		if (indirect2 != Inode::kIndirect_Ref_Unset) {
			_out_stream << indirect2;
		}
		else {
			_out_stream << nil;
		}
		_out_stream << std::endl;
	}
	catch (const PathNotFoundException &) {
		throw FSException{FSMessages::kFileNotFound};
	}
}

void FileSystem::OP_incp(const std::string &path1, const std::string &path2) {
	Assert_Is_Formatted();

	if (!std::filesystem::exists(path1)) {
		Print_Message(FSMessages::kFileNotFound);
		return;
	}

	std::ifstream ifstream{path1, std::ios::in | std::ios::binary};
	const uint32_t file_size = std::filesystem::file_size(path1);
	const uint32_t dblocks_cnt = Get_Necessary_Data_Blocks_Cnt(file_size);

	uint32_t dir_inode_idx;
	std::string dst_filename;
	try {
		const uint32_t filename_inode_idx = Resolve_Final_Path_Inode(path2);
		if (_inodes->Get(filename_inode_idx).Get_Is_Dir()) {
			dir_inode_idx = filename_inode_idx;
			dst_filename = std::filesystem::path{path1}.filename();
		}
		else {
			throw FSException{FSMessages::kPathNotFound};
		}
	}
	catch (const PathNotFoundException &) {
		dir_inode_idx = Resolve_Final_Parent_Inode(path2);
		dst_filename = std::filesystem::path{path2}.filename();
	}
	Assert_Valid_Filename_Length(dst_filename);

	Inode dir_inode = _inodes->Get(dir_inode_idx);
	if (!dir_inode.Get_Is_Dir()) {
		throw PathNotFoundException{};
	}

	Assert_Has_Resources(*_bm_inodes, 1);
	Assert_Has_Resources(*_bm_data, dblocks_cnt);

	const uint32_t inode_idx = Acquire_Inode();
	Inode inode = _inodes->Get(inode_idx);
	inode
		.Set_Is_Dir(false)
		.Set_Is_Symlink(false)
		.Set_Refs_Cnt(1)
		.Set_File_Size(file_size)
		.Unset_References();

	Iterator_DataBlocks it_dblocks{inode, _data};
	I_ReadableWritable::t_Byte_Buf buf;
	buf.resize(DataBlock::kSize);

	std::queue<uint32_t> dblocks_idx_pool;
	_bm_data->Process_Free(dblocks_cnt, [&dblocks_idx_pool] (uint32_t dblock_idx) {
		dblocks_idx_pool.push(dblock_idx);
	});

	for (; ifstream.read(reinterpret_cast<char *>(buf.data()), DataBlock::kSize).gcount() > 0; ) {
		it_dblocks.Append([this, &dblocks_idx_pool] () {
			const uint32_t dblock_idx = dblocks_idx_pool.front();
			dblocks_idx_pool.pop();
			return Acquire_Data_Block(dblock_idx);
		});
		(*it_dblocks).Write_Content(buf, ifstream.gcount());
	}

	Iterator_DirItems it_dir_items{dir_inode, _data};
	it_dir_items.Append(inode_idx, dst_filename);

	Print_Message(FSMessages::kOk);
}

void FileSystem::OP_outcp(const std::string &path1, const std::string &path2) {
	Assert_Is_Formatted();

	try {
		const uint32_t inode_idx = Resolve_Final_Path_Inode(path1);
		Inode inode = _inodes->Get(inode_idx);
		if (inode.Get_Is_Dir()) {
			throw PathNotFoundException{};
		}

		std::filesystem::path dst_path = path2;
		if (std::filesystem::exists(dst_path) && std::filesystem::is_directory(dst_path)) {
			dst_path /= std::filesystem::path{path1}.filename();
		}
		std::ofstream ofstream{dst_path, std::ios::out | std::ios::binary};
		if (!ofstream.is_open()) {
			throw FSException{FSMessages::kPathNotFound};
		}

		I_ReadableWritable::t_Byte_Buf buf;
		buf.reserve(DataBlock::kSize);
		Iterator_DataBlocks it_dblocks{inode, _data};
		for (uint32_t left = inode.Get_File_Size(); left > 0; ++it_dblocks) {
			const DataBlock dblock = *it_dblocks;
			if (left >= DataBlock::kSize) {
				dblock.Read_Content(buf);
				left -= DataBlock::kSize;
			}
			else {
				dblock.Read_Content(buf, left);
				left = 0;
			}

			for (const std::byte byte : buf) {
				ofstream << static_cast<char>(byte);
			}
			buf.clear();
		}

		Print_Message(FSMessages::kOk);
	}
	catch (const PathNotFoundException &) {
		throw FSException{FSMessages::kFileNotFound};
	}
}

void FileSystem::OP_slink(const std::string &path1, const std::string &path2) {
	Assert_Is_Formatted();
	Assert_Valid_Filename_Length(path2);
	Assert_Has_Resources(*_bm_inodes, 1);
	Assert_Has_Resources(*_bm_data, 1);

	bool src_is_dir = false;
	try {
		if (_inodes->Get(Resolve_Final_Path_Inode(path1)).Get_Is_Dir()) {
			src_is_dir = true;
		}
	}
	catch (const PathNotFoundException &) {
		throw FSException{FSMessages::kFileNotFound};
	}

	try {
		Resolve_Final_Path_Inode(path2);
		throw FSException{FSMessages::kPathNotFound};
	}
	catch (const PathNotFoundException &) {
		//
	}

	const uint32_t dst_dir_inode_idx = Resolve_Final_Parent_Inode(path2);
	const std::string dst_filename = std::filesystem::path{path2}.filename();

	const uint32_t dst_inode_idx = Acquire_Inode();
	Inode dst_inode = _inodes->Get(dst_inode_idx);
	dst_inode
		.Set_Is_Dir(false)
		.Set_Is_Symlink(true)
		.Set_Refs_Cnt(1)
		.Set_File_Size(DataBlock::kSize)
		.Unset_References();
	if (src_is_dir) {
		dst_inode.Set_Is_Dir(true);
	}

	Iterator_DataBlocks it_dst_dblocks{dst_inode, _data};
	it_dst_dblocks.Append([this] () {
		return Acquire_Data_Block();
	});
	(*it_dst_dblocks).Write_Path(path1);

	Inode dst_dir_inode = _inodes->Get(dst_dir_inode_idx);
	Iterator_DirItems it_dir_items{dst_dir_inode, _data};
	it_dir_items.Append(dst_inode_idx, dst_filename);

	Print_Message(FSMessages::kOk);
}

Superblock::t_Superblock FileSystem::Get_Formatted_Superblock(size_t fs_size) {
	constexpr uint32_t kSb_Size = sizeof(Superblock::t_Superblock);
	constexpr uint32_t kInode_Size = sizeof(Inode::t_Inode);

	constexpr uint32_t kDBlock_Size = DataBlock::kSize;
	constexpr uint32_t kExp_File_DBlocks = 10;

	uint32_t inodes_cnt = (fs_size - kSb_Size) / (kInode_Size + kExp_File_DBlocks * kDBlock_Size + (kExp_File_DBlocks + 1.0) / 8.0);
	inodes_cnt += (8 - (inodes_cnt % 8)) % 8;

	uint32_t dblocks_cnt = (fs_size - kSb_Size - inodes_cnt - inodes_cnt / 8.0) / (1.0 / 8.0 + kDBlock_Size);
	dblocks_cnt -= dblocks_cnt % 8;

	const uint32_t vfs_size = kSb_Size + inodes_cnt * kInode_Size + inodes_cnt / 8.0 + dblocks_cnt * kDBlock_Size + dblocks_cnt / 8.0;

	const uint32_t bm_data_start_addr = kSb_Size + inodes_cnt / 8;
	const uint32_t inodes_start_addr = bm_data_start_addr + dblocks_cnt / 8;
	const uint32_t data_start_addr = inodes_start_addr + inodes_cnt * kInode_Size;

	return {
			"skafara", "fsinodes",
			vfs_size, kDBlock_Size, inodes_cnt, dblocks_cnt,
			kSb_Size, bm_data_start_addr,
			inodes_start_addr, data_start_addr
	};
}

void FileSystem::Init_Structures() {
	_superblock = std::make_unique<Superblock>(_fs_container, kSuperblock_Offset);
	_bm_inodes = std::make_unique<Bitmap>(_fs_container, _superblock->Get_BMap_Inodes_Start_Addr(), _superblock->Get_Inodes_Cnt() / 8 * sizeof(std::byte));
	_bm_data = std::make_unique<Bitmap>(_fs_container, _superblock->Get_BMap_Data_Start_Addr(), _superblock->Get_Data_Blocks_Cnt() / 8 * sizeof(std::byte));
	_inodes = std::make_unique<Inodes>(_fs_container, _superblock->Get_Inodes_Start_Addr());
	_data = std::make_shared<Data>(_fs_container, _superblock->Get_Data_Start_Addr());
}

std::filesystem::path FileSystem::Get_Cannonical_Path(const std::filesystem::path &path) {
	std::vector<std::string> components;
	std::istringstream path_isstream{path};

	std::string filename;
	while (std::getline(path_isstream, filename, kPath_Delimiter)) {
		if (filename == kDot || filename.empty()) {
			continue;
		}
		if (filename == kDot_Dot) {
			if (components.empty()) {
				continue;
			}

			components.pop_back();
			continue;
		}

		components.push_back(filename);
	}

	std::filesystem::path result{kRoot_Dir_Path};
	for (const std::string& component : components) {
		result /= component;
	}

	return result;
}

uint32_t FileSystem::Acquire_Inode() {
	uint32_t inode_idx;
	_bm_inodes->Process_Free(1, [this, &inode_idx] (uint32_t idx) {
		inode_idx = idx;
		_bm_inodes->Set(idx, true);
	});

	return inode_idx;
}

uint32_t FileSystem::Acquire_Data_Block() {
	uint32_t dblock_idx;
	_bm_data->Process_Free(1, [this, &dblock_idx] (uint32_t idx) {
		dblock_idx = idx;
		_bm_data->Set(idx, true);
	});

	return dblock_idx;
}

uint32_t FileSystem::Acquire_Data_Block(uint32_t dblock_idx) {
	_bm_data->Set(dblock_idx, true);
	return dblock_idx;
}

void FileSystem::Print_Message(FSMessages message) const {
	_out_stream << Get_FSMessage_String(message) << std::endl;
}

bool FileSystem::Is_Formatted() const {
	return _fs_container != nullptr;
}

void FileSystem::Assert_Is_Formatted() const {
	if (!Is_Formatted()) {
		throw FSException{"Filesystem Not Formatted"};
	}
}

uint32_t FileSystem::Get_Necessary_Data_Blocks_Cnt(size_t filesize) {
	uint32_t cnt = 0;
	size_t acquired = 0;
	for (uint32_t i = 0; i < Inode::kDirect_Refs_Cnt && acquired < filesize; ++i, ++cnt, acquired += DataBlock::kSize);
	if (acquired < filesize) ++cnt;
	for (uint32_t i = 0; i < DataBlock::kSize / sizeof(uint32_t) && acquired < filesize; ++i, ++cnt, acquired += DataBlock::kSize);
	if (acquired < filesize) ++cnt;
	for (uint32_t i = 0; i < DataBlock::kSize / sizeof(uint32_t) && acquired < filesize; ++i, ++cnt) {
		for (uint32_t j = 0; j < DataBlock::kSize / sizeof(uint32_t) && acquired < filesize; ++j, ++cnt, acquired += DataBlock::kSize);
	}
	return cnt;
}

void FileSystem::Assert_Has_Resources(const Bitmap &bitmap, uint32_t cnt) {
	if (!bitmap.Has_Free(cnt)) {
		throw FSException{"Filesystem Resources Depleted"};
	}
}

void FileSystem::Assert_Valid_Filename_Length(const std::filesystem::path &path) {
	if (path.filename().string().length() > kValid_Filename_Max_Len) {
		throw std::invalid_argument{"Valid Filename Length Exceeded"};
	}
}

std::pair<uint32_t, std::filesystem::path>
FileSystem::Resolve_Path(const std::filesystem::path &path, const std::pair<uint32_t, std::filesystem::path> &start, bool is_ignore_end) const {
	std::filesystem::path result_path{start.second};
	std::filesystem::path path_{path};

	uint32_t inode_idx = start.first;
	bool is_dir = true;
	for (auto it_path = path_.begin(); it_path != path_.end();) {
		if (!is_dir) {
			throw PathNotFoundException{};
		}

		const std::string filename = it_path->string();
		if (filename.empty()) {
			++it_path;
			continue;
		}

		if (filename == kRoot_Dir_Path) {
			inode_idx = kRoot_Dir_Inode_Idx;
			result_path = kRoot_Dir_Path;
			++it_path;
			continue;
		}

		Inode inode = _inodes->Get(inode_idx);
		bool found_item = false;
		bool found_symlink = false;
		for (Iterator_DirItems it{inode, _data}; it != Iterator_DirItems::kDepleted; ++it) {
			DataBlock::t_DirItem dir_item = *it;
			if (dir_item.Item_Name == filename) {
				found_item = true;
				Inode found_inode = _inodes->Get(dir_item.Inode_Idx);

				if (found_inode.Get_Is_Symlink()) {
					found_symlink = true;

					auto path_left{++it_path};
					if (path_left == path_.end() && is_ignore_end) {
						return {dir_item.Inode_Idx, Get_Cannonical_Path(result_path / filename)};
					}

					std::filesystem::path new_path = _data->Get(found_inode.Get_Direct(0)).Read_Path();
					for (; path_left != path_.end(); ++path_left) {
						new_path /= (*path_left);
					}

					path_ = new_path;
					it_path = path_.begin();
					break;
				}

				if (!found_inode.Get_Is_Dir()) {
					is_dir = false;
				}
				inode_idx = dir_item.Inode_Idx;
				result_path /= filename;
				break;
			}
		}

		if (!found_item) {
			throw PathNotFoundException{};
		}

		if (found_symlink) {
			continue;
		}

		++it_path;
	}

	return {inode_idx, Get_Cannonical_Path(result_path)};
}

uint32_t FileSystem::Resolve_Path_Inode(const std::filesystem::path &path) const {
	return Resolve_Path(path, _work_dir, true).first;
}

std::pair<uint32_t, std::filesystem::path> FileSystem::Resolve_Final_Path(const std::filesystem::path &path) const {
	return Resolve_Path(path, _work_dir, false);
}

uint32_t FileSystem::Resolve_Final_Path_Inode(const std::filesystem::path &path) const {
	return Resolve_Final_Path(path).first;
}

uint32_t FileSystem::Resolve_Final_Parent_Inode(const std::filesystem::path &path) const {
	return Resolve_Final_Path_Inode(path.parent_path());
}
