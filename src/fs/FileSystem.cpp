#include <iostream>
#include <sstream>
#include <utility>

#include "FileSystem.hpp"
#include "../util/FSCmdParser.hpp"
#include "components/util/Iterator_DirItems.hpp"


FileSystem::FileSystem(std::string fs_path, std::ostream &out_stream) :
		_fs_path(std::move(fs_path)), _out_stream(out_stream) {
	if (std::filesystem::exists(_fs_path)) {
		_fs_container = std::make_shared<MMappedFile>(_fs_path);
		_superblock = std::make_unique<Superblock>(_fs_container, 0);
		Init_Components();

		_work_dir_path = "/"; // TODO magic
		_work_dir_inode_idx = 0; // TODO magic?
	}
}

void FileSystem::OP_format(uint32_t size) {
	if (_fs_container == nullptr) {
		_fs_container = std::make_shared<MMappedFile>(_fs_path);
		_superblock = std::make_unique<Superblock>(_fs_container, 0);
	}

	t_Superblock sb_formatted = Get_Formatted_Superblock(size);
	_fs_container->Resize(sb_formatted.Disk_Size);
	_superblock->Set(sb_formatted);
	Init_Components();

	_work_dir_path = "/"; // TODO magic
	_work_dir_inode_idx = 0; // TODO magic?

	_bm_inodes->Set(0, true);
	Inode root_dir_inode = _inodes->Get(0);
	root_dir_inode
		.Set_Is_Dir(true)
		.Set_Refs_Cnt(1)
		.Unset_Directs_Indirects()
		.Set_Direct(0, 0);

	_bm_data->Set(0, true);
	DataBlock dblock = _data->Get(0);
	dblock.Set_Dir_Item(0, 0, ".");
	dblock.Set_Dir_Item(1, 0, "..");

	_out_stream << "OK" << std::endl;
}

void FileSystem::OP_load(std::istream &cmd_istream) {
	for (std::string line; std::getline(cmd_istream, line);) {
		const auto op = FSCmdParser::Parse(line);
		op(*this);
	}
}

void FileSystem::OP_cd(const std::string &path) {
	const std::filesystem::path _path{path};

	try {
		uint32_t inode_idx = Resolve_Path(path);

		if (_path.is_absolute()) {
			_work_dir_path = Get_Cannonical_Path(_path);
		} else {
			_work_dir_path = Get_Cannonical_Path(_work_dir_path / _path);
		}
		_work_dir_inode_idx = inode_idx;

		_out_stream << "OK" << std::endl;
	}
	catch (int e) {
		_out_stream << "err: " << e << std::endl;
	}
}

void FileSystem::OP_pwd() const {
	_out_stream << _work_dir_path.string() << std::endl;
}

void FileSystem::OP_cp(const std::string &path1, const std::string &path2) {
	uint32_t src_inode_idx = Resolve_Path(path1);
	Inode src_inode = _inodes->Get(src_inode_idx);


	uint32_t dst_dir_inode_idx{};

	std::filesystem::path _path2{path2};
	std::string dst_filename{};
	try {
		uint32_t dst_filename_inode_idx = Resolve_Path(path2);
		if (_inodes->Get(dst_filename_inode_idx).Get_Is_Dir()) {
			dst_dir_inode_idx = dst_filename_inode_idx;
			dst_filename = std::filesystem::path{path1}.filename();
		}
		else {
			throw 5;
		}
	} catch (int i) {
		if (i == 5 || i == -1) {
			throw i;
		} // -2
		dst_dir_inode_idx = Resolve_Parent(path2);
		dst_filename = _path2.filename();
	}

	uint32_t dst_inode_idx = Acquire_Inode();
	Inode dst_inode = _inodes->Get(dst_inode_idx);
	dst_inode.Set_Is_Dir(false).Set_Refs_Cnt(1).Set_File_Size(src_inode.Get_File_Size())
		.Unset_Directs_Indirects();
		//.Set_Direct(0, src_inode.Get_Direct(0))
		//.Set_Direct(1, src_inode.Get_Direct(1))
		//.Set_Direct(2, src_inode.Get_Direct(2))
		//.Set_Direct(3, src_inode.Get_Direct(3))
		//.Set_Direct(4, src_inode.Get_Direct(4))
		//.Set_Indirect1(src_inode.Get_Indirect1())
		//.Set_Indirect2(src_inode.Get_Indirect2());
	// clone

	Inode dst_dir_inode = _inodes->Get(dst_dir_inode_idx);
	Iterator_DirItems it_dst_dir_items{dst_dir_inode, _data};
	it_dst_dir_items.Append_Dir_Item(dst_inode_idx, dst_filename);

	Iterator_DataBlocks it_src{src_inode, _data};
	Iterator_DataBlocks it_dst{dst_inode, _data};
	t_Byte_Buf buf{};
	buf.reserve(1024); // TODO
	//
	for (uint32_t left = src_inode.Get_File_Size(); left > 0; ++it_src) {
		if (left >= 1024) {
			(*it_src).Get_Content(buf, 1024);
			left -= 1024;
		}
		else {
			(*it_src).Get_Content(buf, left);
			left = 0;
		}
		it_dst.Append_Data_Block(Data_Block_Acquirer);
		(*it_dst).Set_Content(buf, 1024);
		buf.clear();
	}
	//

	_out_stream << "OK" << std::endl;
}

void FileSystem::OP_mv(const std::string &path1, const std::string &path2) {
	uint32_t src_inode_idx = Resolve_Path(path1);
	uint32_t src_dir_inode_idx = Resolve_Parent(path1);
	Inode src_dir_inode = _inodes->Get(src_dir_inode_idx);


	uint32_t dst_dir_inode_idx{};

	std::filesystem::path _path2{path2};
	std::string dst_filename{};
	try {
		uint32_t dst_filename_inode_idx = Resolve_Path(path2);
		if (_inodes->Get(dst_filename_inode_idx).Get_Is_Dir()) {
			dst_dir_inode_idx = dst_filename_inode_idx;
			dst_filename = std::filesystem::path{path1}.filename();
		}
		else {
			throw 5;
		}
	} catch (int i) {
		if (i == 5 || i == -1) {
			throw i;
		} // -2
		dst_dir_inode_idx = Resolve_Parent(path2);
		dst_filename = _path2.filename();
	}
	//

	Iterator_DirItems it_src_dir_items{src_dir_inode, _data};
	++it_src_dir_items;
	++it_src_dir_items; // TODO
	it_src_dir_items.Remove_Dir_Item();

	Inode dst_dir_inode = _inodes->Get(dst_dir_inode_idx);
	Iterator_DirItems it_dst_dir_items{dst_dir_inode, _data};
	it_dst_dir_items.Append_Dir_Item(src_inode_idx, dst_filename);

	_out_stream << "MV" << std::endl;
}

void FileSystem::OP_rm(const std::string &path) {
	uint32_t inode_idx = Resolve_Path(path);
	Inode inode = _inodes->Get(inode_idx);



	_out_stream << "RM" << std::endl;
}

void FileSystem::OP_mkdir(const std::string &path) {
	const std::string filename = std::filesystem::path{path}.filename();
	uint32_t parent_inode_idx = Resolve_Parent(path);
	Inode parent_inode = _inodes->Get(parent_inode_idx);

	Iterator_DirItems it{parent_inode, _data};
	// allocate inode
	uint32_t dir_inode_idx = 0;
	for (; dir_inode_idx < 8 * (_superblock->Get_BMap_Data_Start_Addr() - _superblock->Get_BMap_Inodes_Start_Addr()); ++dir_inode_idx) {
		if (_bm_inodes->Is_Set(dir_inode_idx)) {
			continue;
		}

		break;
	}
	if (dir_inode_idx == 0) {
		throw -1;
	}

	// allocate (acquire) dblock
	uint32_t dir_dblock_idx = 0;
	for (; dir_dblock_idx < 8 * (_superblock->Get_Inodes_Start_Addr() - _superblock->Get_BMap_Data_Start_Addr()); ++dir_dblock_idx) {
		if (_bm_data->Is_Set(dir_dblock_idx)) {
			continue;
		}

		break;
	}
	if (dir_dblock_idx == 0) {
		throw -1;
	}
	_bm_inodes->Set(dir_inode_idx, true);
	_bm_data->Set(dir_dblock_idx, true);

	Inode dir_inode = _inodes->Get(dir_inode_idx);
	dir_inode.Set_Is_Dir(true).Set_Refs_Cnt(1).Set_File_Size(1024).Unset_Directs_Indirects().Set_Direct(0, dir_dblock_idx);
	DataBlock dblock1 = _data->Get(dir_dblock_idx);
	dblock1.Set_Dir_Item(0, dir_inode_idx, ".");
	dblock1.Set_Dir_Item(1, parent_inode_idx, "..");
	it.Append_Dir_Item(dir_inode_idx, filename);

	_out_stream << "OK" << std::endl;
}

void FileSystem::OP_rmdir(const std::string &path) {
	const std::string filename = std::filesystem::path{path}.filename();
	uint32_t parent_inode_idx = Resolve_Parent(path);
	Inode parent_inode = _inodes->Get(parent_inode_idx);

	Iterator_DirItems it{parent_inode, _data};
	for (; (*it).Item_Name != filename; ++it);
	it.Remove_Dir_Item();

	_bm_inodes->Set((*it).Inode_Idx, false);
	_bm_data->Set(_inodes->Get((*it).Inode_Idx).Get_Direct(0), false); // foreach inode-directindirect
}

void FileSystem::OP_ls(const std::string &path) const {
	Inode dir_inode = _inodes->Get(Resolve_Path(path));
	for (Iterator_DirItems it{dir_inode, _data}; it != it.end(); ++it) {
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
	const uint32_t inode_idx = Resolve_Path(path);
	Inode inode = _inodes->Get(inode_idx);

	t_Byte_Buf file_contents{}; // file contents iterator?, reserve
	file_contents.reserve(inode.Get_File_Size());

	Iterator_DataBlocks it{inode, _data};
	for (uint32_t left = inode.Get_File_Size(); left > 0; ++it) {
		DataBlock dblock = *it;
		if (left >= 1024) {
			dblock.Get_Content(file_contents);
			left -= 1024;
		} else {
			dblock.Get_Content(file_contents, left);
			left = 0;
		}
	}

	for (const std::byte byte : file_contents) {
		_out_stream << static_cast<char>(byte);
	}
}

void FileSystem::OP_info(const std::string &path) const {
	uint32_t inode_idx = Resolve_Path(path);
	Inode inode = _inodes->Get(inode_idx);

	_out_stream << std::filesystem::path{path}.filename().string();
	_out_stream << " - " << inode.Get_File_Size();
	_out_stream << " - i-node " << inode_idx;

	_out_stream << " - Direct";
	for (uint32_t i = 0; i < 5; ++i) { // TODO
		const uint32_t direct = inode.Get_Direct(i);
		_out_stream << " [" << i << "]";
		if (direct != -1) { // TODO
			_out_stream << direct;
		} else {
			_out_stream << "nil"; // TODO
		}
	}

	const uint32_t indirect1 = inode.Get_Indirect1();
	_out_stream << " - Indirect1 ";
	if (indirect1 != -1) { // TODO
		_out_stream << indirect1;
	} else {
		_out_stream << "nil";
	}

	const uint32_t indirect2 = inode.Get_Indirect2();
	_out_stream << " - Indirect2 ";
	if (indirect2 != -1) { // TODO
		_out_stream << indirect2;
	} else {
		_out_stream << "nil";
	}
	_out_stream << std::endl;
}

#include <fstream>
void FileSystem::OP_incp(const std::string &path1, const std::string &path2) {
	std::ifstream ifstream{path1, std::ios::in | std::ios::binary};
	const uint32_t file_size = std::filesystem::file_size(path1);
	const uint32_t dblocks_cnt = std::ceil(file_size / static_cast<double>(1024)); // TODO dopocet s indirect1 a 2 a test jestli jich je dost a test jestli mam volny inode

	uint32_t dir_inode_idx{};

	std::filesystem::path _path2{path2};
	std::string dst_filename{};
	try {
		uint32_t filename_inode_idx = Resolve_Path(path2);
		if (_inodes->Get(filename_inode_idx).Get_Is_Dir()) {
			dir_inode_idx = filename_inode_idx;
			dst_filename = std::filesystem::path{path1}.filename();
		}
		else {
			throw 5;
		}
	} catch (int i) {
		if (i == 5 || i == -1) {
			throw i;
		} // -2
		uint32_t filename_inode_idx = Resolve_Parent(path2);
		dir_inode_idx = filename_inode_idx;
		dst_filename = _path2.filename();
	}

	uint32_t inode_idx = Acquire_Inode();
	Inode inode = _inodes->Get(inode_idx);
	inode.Set_Is_Dir(false).Set_Refs_Cnt(1).Set_File_Size(file_size).Unset_Directs_Indirects();

	Inode dir_inode = _inodes->Get(dir_inode_idx);
	Iterator_DirItems it_dir_items{dir_inode, _data};
	it_dir_items.Append_Dir_Item(inode_idx, dst_filename);

	Iterator_DataBlocks it{inode, _data};
	t_Byte_Buf buf{};
	buf.resize(1024); // TODO
	for (uint32_t i = 0; ifstream.read(reinterpret_cast<char *>(buf.data()), 1024).gcount() > 0; ++i) {
		it.Append_Data_Block(Data_Block_Acquirer);
		(*it).Set_Content(buf, ifstream.gcount());
	}

	_out_stream << "OK" << std::endl;
}

void FileSystem::OP_outcp(const std::string &path1, const std::string &path2) {
	std::ofstream ofstream{path2, std::ios::out | std::ios::binary}; // TODO check if exists or open
	std::cout << ofstream.is_open() << std::endl;

	uint32_t inode_idx = Resolve_Path(path1);
	Inode inode = _inodes->Get(inode_idx);

	Iterator_DataBlocks it{inode, _data};
	t_Byte_Buf file_contents{};
	file_contents.reserve(inode.Get_File_Size());
	for (uint32_t left = inode.Get_File_Size(); left > 0; ++it) {
		DataBlock dblock = *it;
		if (left >= 1024) {
			dblock.Get_Content(file_contents);
			left -= 1024;
		} else {
			dblock.Get_Content(file_contents, left);
			left = 0;
		}
	}

	for (const std::byte byte : file_contents) {
		ofstream << static_cast<char>(byte);
	}

	_out_stream << "OK" << std::endl;
}

uint32_t FileSystem::Resolve_Path(const std::string &path) const {
	const std::filesystem::path _path{path};

	auto begin = _path.begin();
	uint32_t inode_idx = _work_dir_inode_idx;
	if (begin->string() == "/") { // TODO magic
		begin++;
		inode_idx = 0;
	}

	bool is_dir = true;
	for (auto it_path = begin; it_path != _path.end(); ++it_path) {
		if (!is_dir) {
			throw -1;
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
			throw -2;
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

	constexpr uint32_t dblock_size = 1024; // TODO magic
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
			vfs_size, dblock_size, dblocks_cnt,
			sb_size, bm_data_start_addr,
			inodes_start_addr, data_start_addr
	};
}

void FileSystem::Init_Components() {
	_bm_inodes = std::make_unique<Bitmap>(_fs_container, _superblock->Get_BMap_Inodes_Start_Addr());
	_bm_data = std::make_unique<Bitmap>(_fs_container, _superblock->Get_BMap_Data_Start_Addr());
	_inodes = std::make_unique<Inodes>(_fs_container, _superblock->Get_Inodes_Start_Addr());
	_data = std::make_shared<Data>(_fs_container, _superblock->Get_Data_Start_Addr());
}

std::filesystem::path FileSystem::Get_Cannonical_Path(const std::filesystem::path &path) {
	std::vector<std::string> components;
	std::istringstream path_isstream{path};

	std::string filename;
	while (std::getline(path_isstream, filename, '/')) { // TODO magic delim
		if (filename == "." || filename.empty()) { // TODO magic dot
			continue;
		}
		if (filename == "..") { // TODO magic dotdot
			if (components.empty()) {
				continue;
			}

			components.pop_back();
			continue;
		}

		components.push_back(filename);
	}

	std::filesystem::path result{"/"}; // TODO magic
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
