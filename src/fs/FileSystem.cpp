#include <iostream>
#include <sstream>
#include <fstream>
#include <utility>

#include "FileSystem.hpp"
#include "../util/FSCmdParser.hpp"
#include "components/util/Iterator_DirItems.hpp"


const std::string &Get_FSMessage_String(FSMessages message) {
	return FSMessages_Strings.at(message);
}

FileSystem::FileSystem(const std::string &fs_path, std::ostream &out_stream) :
		_fs_path(fs_path), _out_stream(out_stream), _work_dir_path(kRoot_Dir_Path), _work_dir_inode_idx(kRoot_Dir_Inode_Idx) {
	if (std::filesystem::exists(_fs_path)) {
		_fs_container = std::make_shared<MMappedFile>(_fs_path);
		Init_Structures();
	}
}

void FileSystem::OP_format(uint32_t size) {
	_work_dir_path = kRoot_Dir_Path;
	_work_dir_inode_idx = kRoot_Dir_Inode_Idx;
	_fs_container = std::make_shared<MMappedFile>(_fs_path);

	const t_Superblock sb_formatted = Get_Formatted_Superblock(size);
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
		.Set_Refs_Cnt(1)
		.Set_File_Size(DataBlock::kSize)
		.Unset_Directs_Indirects()
		.Set_Direct(0, kRoot_Dir_DBlock_Idx);

	_bm_data->Set(kRoot_Dir_DBlock_Idx, true);
	DataBlock root_dir_dblock = _data->Get(kRoot_Dir_DBlock_Idx);
	Iterator_DirItems it_root_dir_items{root_dir_inode, _data};
	it_root_dir_items.Append_Dir_Item(kRoot_Dir_Inode_Idx, kDot);
	it_root_dir_items.Append_Dir_Item(kRoot_Dir_Inode_Idx, kDot_Dot);

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

	const std::filesystem::path _path{path};

	const uint32_t inode_idx = Resolve_Path(path);
	Inode inode = _inodes->Get(inode_idx);
	if (!inode.Get_Is_Dir()) {
		throw PathNotFoundException{};
	}

	if (_path.is_absolute()) {
		_work_dir_path = Get_Cannonical_Path(_path);
	} else {
		_work_dir_path = Get_Cannonical_Path(_work_dir_path / _path);
	}
	_work_dir_inode_idx = inode_idx;

	Print_Message(FSMessages::kOk);
}

void FileSystem::OP_pwd() const {
	Assert_Is_Formatted();

	_out_stream << _work_dir_path.string() << std::endl;
}

void FileSystem::OP_cp(const std::string &path1, const std::string &path2) {
	Assert_Is_Formatted();

	const uint32_t src_inode_idx = Resolve_Path(path1);
	Inode src_inode = _inodes->Get(src_inode_idx);
	if (src_inode.Get_Is_Dir()) {
		throw FSException{FSMessages::kFileNotFound};
	}

	uint32_t dst_dir_inode_idx;
	std::string dst_filename;
	try {
		const uint32_t dst_filename_inode_idx = Resolve_Path(path2);
		if (_inodes->Get(dst_filename_inode_idx).Get_Is_Dir()) {
			dst_dir_inode_idx = dst_filename_inode_idx;
			dst_filename = std::filesystem::path{path1}.filename();
		}
		else {
			throw FSException{FSMessages::kPathNotFound};
		}
	}
	catch (const PathNotFoundException &) {
		dst_dir_inode_idx = Resolve_Parent(path2);
		dst_filename = std::filesystem::path{path2}.filename();
	}

	Inode dst_dir_inode = _inodes->Get(dst_dir_inode_idx);
	if (!dst_dir_inode.Get_Is_Dir()) {
		throw PathNotFoundException{};
	}

	const uint32_t dst_inode_idx = Acquire_Inode();
	Inode dst_inode = _inodes->Get(dst_inode_idx);
	dst_inode
		.Set_Is_Dir(false)
		.Set_Refs_Cnt(1)
		.Set_File_Size(src_inode.Get_File_Size())
		.Unset_Directs_Indirects();

	Iterator_DataBlocks it_src{src_inode, _data};
	Iterator_DataBlocks it_dst{dst_inode, _data};
	t_Byte_Buf buf;
	buf.reserve(DataBlock::kSize);
	for (uint32_t left = src_inode.Get_File_Size(); left > 0; ++it_src) {
		it_dst.Append_Data_Block(Data_Block_Acquirer);
		if (left >= DataBlock::kSize) {
			(*it_src).Get_Content(buf);
			(*it_dst).Set_Content(buf);
			left -= DataBlock::kSize;
		}
		else {
			(*it_src).Get_Content(buf, left);
			(*it_dst).Set_Content(buf, left);
			left = 0;
		}

		buf.clear();
	}

	Iterator_DirItems it_dst_dir_items{dst_dir_inode, _data};
	it_dst_dir_items.Append_Dir_Item(dst_inode_idx, dst_filename);

	Print_Message(FSMessages::kOk);
}

void FileSystem::OP_mv(const std::string &path1, const std::string &path2) {
	Assert_Is_Formatted();

	const uint32_t src_inode_idx = Resolve_Path(path1);
	const Inode src_inode = _inodes->Get(src_inode_idx);
	if (src_inode.Get_Is_Dir()) {
		throw FSException{FSMessages::kFileNotFound};
	}

	const uint32_t src_dir_inode_idx = Resolve_Parent(path1);
	Inode src_dir_inode = _inodes->Get(src_dir_inode_idx);

	uint32_t dst_dir_inode_idx;
	std::string dst_filename;
	try {
		const uint32_t dst_filename_inode_idx = Resolve_Path(path2);
		if (_inodes->Get(dst_filename_inode_idx).Get_Is_Dir()) {
			dst_dir_inode_idx = dst_filename_inode_idx;
			dst_filename = std::filesystem::path{path1}.filename();
		}
		else {
			throw FSException{FSMessages::kPathNotFound};
		}
	}
	catch (const PathNotFoundException &) {
		dst_dir_inode_idx = Resolve_Parent(path2);
		dst_filename = std::filesystem::path{path2}.filename();
	}

	Inode dst_dir_inode = _inodes->Get(dst_dir_inode_idx);
	if (!dst_dir_inode.Get_Is_Dir()) {
		throw PathNotFoundException{};
	}

	Iterator_DirItems it_dst_dir_items{dst_dir_inode, _data};
	it_dst_dir_items.Append_Dir_Item(src_inode_idx, dst_filename);

	Iterator_DirItems it_src_dir_items{src_dir_inode, _data};
	for (; (*it_src_dir_items).Inode_Idx != src_inode_idx; ++it_src_dir_items);
	it_src_dir_items.Remove_Dir_Item();

	Print_Message(FSMessages::kOk);
}

void FileSystem::OP_rm(const std::string &path) {
	Assert_Is_Formatted();
	uint32_t inode_idx = Resolve_Path(path);
	Inode inode = _inodes->Get(inode_idx);



	_out_stream << "RM" << std::endl;
}

void FileSystem::OP_mkdir(const std::string &path) {
	Assert_Is_Formatted();

	try {
		Resolve_Path(path);
		throw FSException{FSMessages::kExist};
	}
	catch (const PathNotFoundException &) {
		//
	}

	const std::string dir_name = std::filesystem::path{path}.filename();
	const uint32_t parent_inode_idx = Resolve_Parent(path);
	Inode parent_inode = _inodes->Get(parent_inode_idx);

	if (!parent_inode.Get_Is_Dir()) {
		throw PathNotFoundException{};
	}

	const uint32_t dir_inode_idx = Acquire_Inode();
	const uint32_t dir_dblock_idx = Acquire_Data_Block();

	Inode dir_inode = _inodes->Get(dir_inode_idx);
	dir_inode
		.Set_Is_Dir(true)
		.Set_Refs_Cnt(1)
		.Set_File_Size(DataBlock::kSize)
		.Unset_Directs_Indirects()
		.Set_Direct(0, dir_dblock_idx);

	Iterator_DirItems it_dir_items{dir_inode, _data};
	it_dir_items.Append_Dir_Item(dir_inode_idx, kDot);
	it_dir_items.Append_Dir_Item(parent_inode_idx, kDot_Dot);

	Iterator_DirItems it_parent_dir{parent_inode, _data};
	it_parent_dir.Append_Dir_Item(dir_inode_idx, dir_name);

	Print_Message(FSMessages::kOk);
}

void FileSystem::OP_rmdir(const std::string &path) {
	Assert_Is_Formatted();

	try {
		const std::string dir_name = std::filesystem::path{path}.filename();
		const uint32_t parent_inode_idx = Resolve_Parent(path);
		Inode parent_inode = _inodes->Get(parent_inode_idx);

		bool found = false;
		Iterator_DirItems it_parent_dir_items{parent_inode, _data};
		for (; it_parent_dir_items != it_parent_dir_items.end(); ++it_parent_dir_items) {
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
		if (it_dir_items != it_dir_items.end()) {
			throw FSException{FSMessages::kNotEmpty};
		}

		it_parent_dir_items.Remove_Dir_Item();

		// release
		_bm_inodes->Set(dir_inode_idx, false);
		_bm_data->Set(_inodes->Get(dir_inode_idx).Get_Direct(0), false);

		Print_Message(FSMessages::kOk);
	}
	catch (const PathNotFoundException &) {
		throw FSException{FSMessages::kFileNotFound};
	}
}

void FileSystem::OP_ls(const std::string &path) const {
	Assert_Is_Formatted();

	Inode inode = _inodes->Get(Resolve_Path(path));
	if (!inode.Get_Is_Dir()) {
		throw PathNotFoundException{};
	}

	for (Iterator_DirItems it{inode, _data}; it != it.end(); ++it) {
		t_DirItem dir_item = *it;

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
		const uint32_t inode_idx = Resolve_Path(path);
		Inode inode = _inodes->Get(inode_idx);
		if (inode.Get_Is_Dir()) {
			throw PathNotFoundException{};
		}

		t_Byte_Buf buf;
		buf.reserve(DataBlock::kSize);
		Iterator_DataBlocks it_dblocks{inode, _data};
		for (uint32_t left = inode.Get_File_Size(); left > 0; ++it_dblocks) {
			const DataBlock dblock = *it_dblocks;
			if (left >= DataBlock::kSize) {
				dblock.Get_Content(buf);
				left -= DataBlock::kSize;
			} else {
				dblock.Get_Content(buf, left);
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
		const uint32_t inode_idx = Resolve_Path(path);
		Inode inode = _inodes->Get(inode_idx);

		const std::string nil = "nil";

		_out_stream << std::filesystem::path{path}.filename().string();
		_out_stream << " - " << inode.Get_File_Size();
		_out_stream << " - i-node " << inode_idx;

		_out_stream << " - Direct";
		for (uint32_t i = 0; i < kInode_Directs_Cnt; ++i) {
			const uint32_t direct = inode.Get_Direct(i);
			_out_stream << " [" << i << "]";
			if (direct != Inode::kRef_Unset) {
				_out_stream << direct;
			}
			else {
				_out_stream << nil;
			}
		}

		const uint32_t indirect1 = inode.Get_Indirect1();
		_out_stream << " - Indirect1 ";
		if (indirect1 != Inode::kRef_Unset) {
			_out_stream << indirect1;
		}
		else {
			_out_stream << nil;
		}

		const uint32_t indirect2 = inode.Get_Indirect2();
		_out_stream << " - Indirect2 ";
		if (indirect2 != Inode::kRef_Unset) {
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
	const uint32_t dblocks_cnt = std::ceil(file_size / static_cast<double>(1024)); // TODO dopocet s indirect1 a 2 a test jestli jich je dost a test jestli mam volny inode

	uint32_t dir_inode_idx;
	std::string dst_filename;
	try {
		const uint32_t filename_inode_idx = Resolve_Path(path2);
		if (_inodes->Get(filename_inode_idx).Get_Is_Dir()) {
			dir_inode_idx = filename_inode_idx;
			dst_filename = std::filesystem::path{path1}.filename();
		}
		else {
			throw FSException{FSMessages::kPathNotFound};
		}
	}
	catch (const PathNotFoundException &) {
		dir_inode_idx = Resolve_Parent(path2);
		dst_filename = std::filesystem::path{path2}.filename();
	}

	Inode dir_inode = _inodes->Get(dir_inode_idx);
	if (!dir_inode.Get_Is_Dir()) {
		throw PathNotFoundException{};
	}

	const uint32_t inode_idx = Acquire_Inode();
	Inode inode = _inodes->Get(inode_idx);
	inode.Set_Is_Dir(false).Set_Refs_Cnt(1).Set_File_Size(file_size).Unset_Directs_Indirects();

	Iterator_DataBlocks it_dblocks{inode, _data};
	t_Byte_Buf buf;
	buf.resize(DataBlock::kSize);
	for (; ifstream.read(reinterpret_cast<char *>(buf.data()), DataBlock::kSize).gcount() > 0; ) {
		it_dblocks.Append_Data_Block(Data_Block_Acquirer);
		(*it_dblocks).Set_Content(buf, ifstream.gcount());
	}

	Iterator_DirItems it_dir_items{dir_inode, _data};
	it_dir_items.Append_Dir_Item(inode_idx, dst_filename);

	Print_Message(FSMessages::kOk);
}

void FileSystem::OP_outcp(const std::string &path1, const std::string &path2) {
	Assert_Is_Formatted();

	try {
		const uint32_t inode_idx = Resolve_Path(path1);
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

		t_Byte_Buf buf;
		buf.reserve(DataBlock::kSize);
		Iterator_DataBlocks it_dblocks{inode, _data};
		for (uint32_t left = inode.Get_File_Size(); left > 0; ++it_dblocks) {
			const DataBlock dblock = *it_dblocks;
			if (left >= DataBlock::kSize) {
				dblock.Get_Content(buf);
				left -= DataBlock::kSize;
			} else {
				dblock.Get_Content(buf, left);
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

uint32_t FileSystem::Resolve_Path(const std::string &path) const {
	const std::filesystem::path _path{path};

	auto begin = _path.begin();
	uint32_t inode_idx = _work_dir_inode_idx;
	if (begin->string() == kRoot_Dir_Path) {
		begin++;
		inode_idx = kRoot_Dir_Inode_Idx;
	}

	bool is_dir = true;
	for (auto it_path = begin; it_path != _path.end(); ++it_path) {
		if (!is_dir) {
			throw PathNotFoundException{};
		}

		const std::string filename = it_path->string();
		if (filename.empty()) {
			continue;
		}

		Inode inode = _inodes->Get(inode_idx);
		bool found = false;
		for (Iterator_DirItems it{inode, _data}; it != it.end(); ++it) {
			t_DirItem dir_item = *it;

			if (dir_item.Item_Name == filename) {
				if (!_inodes->Get(dir_item.Inode_Idx).Get_Is_Dir()) {
					is_dir = false;
				}
				inode_idx = dir_item.Inode_Idx;
				found = true;
				break;
			}
		}

		if (!found) {
			throw PathNotFoundException{};
		}
	}

	return inode_idx;
}

uint32_t FileSystem::Resolve_Parent(const std::string &path) const {
	const std::filesystem::path _path{path};
	return Resolve_Path(_path.parent_path());
}

t_Superblock FileSystem::Get_Formatted_Superblock(size_t fs_size) {
	constexpr uint32_t sb_size = sizeof(t_Superblock);
	constexpr uint32_t inode_size = sizeof(t_Inode);

	constexpr uint32_t dblock_size = DataBlock::kSize;
	constexpr uint32_t exp_file_dblocks = 10;

	uint32_t inodes_cnt = (fs_size - sb_size) / (inode_size + exp_file_dblocks * dblock_size + (exp_file_dblocks + 1.0) / 8.0);
	inodes_cnt += (8 - (inodes_cnt % 8)) % 8;

	uint32_t dblocks_cnt = (fs_size - sb_size - inodes_cnt - inodes_cnt / 8.0) / (1.0 / 8.0 + dblock_size);
	dblocks_cnt -= dblocks_cnt % 8;

	const uint32_t vfs_size = sb_size + inodes_cnt * inode_size + inodes_cnt / 8.0 + dblocks_cnt * dblock_size + dblocks_cnt / 8.0;

	const uint32_t bm_data_start_addr = sb_size + inodes_cnt / 8;
	const uint32_t inodes_start_addr = bm_data_start_addr + dblocks_cnt / 8;
	const uint32_t data_start_addr = inodes_start_addr + inodes_cnt * inode_size;

	return {
			"skafara", "fsinodes",
			vfs_size, dblock_size, inodes_cnt, dblocks_cnt,
			sb_size, bm_data_start_addr,
			inodes_start_addr, data_start_addr
	};
}

void FileSystem::Init_Structures() {
	_superblock = std::make_unique<Superblock>(_fs_container, kSuperblock_Offset);
	_bm_inodes = std::make_unique<Bitmap>(_fs_container, _superblock->Get_BMap_Inodes_Start_Addr());
	_bm_data = std::make_unique<Bitmap>(_fs_container, _superblock->Get_BMap_Data_Start_Addr());
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

/*std::vector<uint32_t> FileSystem::Reserve_Data_Blocks(uint32_t cnt) {
	std::vector<uint32_t> dblocks_idxs{};
	for (uint32_t i = 0, j = 0; i < cnt && j < 8 * (_superblock->Get_Inodes_Start_Addr() - _superblock->Get_BMap_Data_Start_Addr()); ++j) {
		if (_bm_data->Is_Set(j)) {
			continue;
		}

		dblocks_idxs.push_back(j);
		++i;
	}

	if (dblocks_idxs.size() < cnt) {
		throw -1;
	}

	return dblocks_idxs;
}*/

uint32_t FileSystem::Acquire_Data_Block() {
	uint32_t dblock_idx = 0;
	//std::cout << 8 * (_superblock->Get_Inodes_Start_Addr() - _superblock->Get_BMap_Data_Start_Addr()) << std::endl;
	for (uint32_t i = 0; i < 1048328; ++i) {
		if (_bm_data->Is_Set(i)) {
			continue;
		}

		dblock_idx = i;
		break;
	}

	if (dblock_idx == 0) {
		throw -1;
	}

	_bm_data->Set(dblock_idx, true);
	return dblock_idx;
}

uint32_t FileSystem::Acquire_Inode() {
	uint32_t inode_idx = 0;
	for (uint32_t i = 0; i < 8 * (_superblock->Get_BMap_Data_Start_Addr() - _superblock->Get_BMap_Inodes_Start_Addr()); ++i) {
		if (_bm_inodes->Is_Set(i)) {
			continue;
		}

		inode_idx = i;
		break;
	}

	if (inode_idx == 0) {
		throw -1;
	}

	_bm_inodes->Set(inode_idx, true);
	return inode_idx;
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
