#pragma once

#include <string>
#include <filesystem>

#include "I_FSOps.hpp"
#include "../io/MMappedFile.hpp"
#include "components/Superblock.hpp"
#include "components/util/Bitmap.hpp"
#include "components/Inodes.hpp"
#include "components/Data.hpp"


enum class FSMessages {
	kOk,
	kPathNotFound,
	kFileNotFound,
	kNotEmpty,
	kCannotCreateFile,
	kExist
};

const std::unordered_map<FSMessages, const std::string> FSMessages_Strings{
	{FSMessages::kOk, "OK"},
	{FSMessages::kPathNotFound, "PATH NOT FOUND"},
	{FSMessages::kFileNotFound, "FILE NOT FOUND"},
	{FSMessages::kNotEmpty, "NOT EMPTY"},
	{FSMessages::kCannotCreateFile, "CANNOT CREATE FILE"},
	{FSMessages::kExist, "EXIST"}
};

const std::string &Get_FSMessage_String(FSMessages message);

class FSException : public std::runtime_error {
public:
	explicit FSException(const std::string &msg) : std::runtime_error(msg) {
		//
	}
	explicit FSException(FSMessages msg) : std::runtime_error(Get_FSMessage_String(msg)) {
		//
	}
};

class PathNotFoundException : public FSException {
public:
	explicit PathNotFoundException() : FSException(FSMessages::kPathNotFound) {
		//
	}
};

/*class NotFormattedException : public FSException {
public:
	explicit NotFormattedException() : FSException() {
		//
	}
};*/

using t_DataBlockAcquirer = std::function<uint32_t ()>;

class FileSystem : public I_FSOps {
public:
	FileSystem(const std::string &fs_path, std::ostream &out_stream);

	FileSystem(const FileSystem &) = delete;
	FileSystem &operator=(const FileSystem &) = delete;

	void OP_format(uint32_t size) override;
	void OP_load(const std::string &path) override;

	void OP_cd(const std::string &path) override;
	void OP_pwd() const override;

	void OP_cp(const std::string &path1, const std::string &path2) override;
	void OP_mv(const std::string &path1, const std::string &path2) override;
	void OP_rm(const std::string &path) override;

	void OP_mkdir(const std::string &path) override;
	void OP_rmdir(const std::string &path) override;

	void OP_ls(const std::string &path) const override;

	void OP_cat(const std::string &path) const override;

	void OP_info(const std::string &path) const override;

	void OP_incp(const std::string &path1, const std::string &path2) override;
	void OP_outcp(const std::string &path1, const std::string &path2) override;

private:
	static constexpr uint32_t kSuperblock_Offset = 0;

	inline static const std::string kRoot_Dir_Path = "/";
	static constexpr uint32_t kRoot_Dir_Inode_Idx = 0;
	static constexpr uint32_t kRoot_Dir_DBlock_Idx = 0;

	static constexpr char kPath_Delimiter = '/';
	inline static const std::string kDot = ".";
	inline static const std::string kDot_Dot = "..";

	const std::string _fs_path;
	std::ostream &_out_stream;
	std::shared_ptr<I_FSContainer> _fs_container;

	std::unique_ptr<Superblock> _superblock;
	std::unique_ptr<Bitmap> _bm_inodes;
	std::unique_ptr<Bitmap> _bm_data;
	std::unique_ptr<Inodes> _inodes;
	std::shared_ptr<Data> _data;

	std::filesystem::path _work_dir_path;
	uint32_t _work_dir_inode_idx;

	static t_Superblock Get_Formatted_Superblock(size_t fs_size);
	static std::filesystem::path Get_Cannonical_Path(const std::filesystem::path &path);

	uint32_t Resolve_Path(const std::string &path) const;
	uint32_t Resolve_Parent(const std::string &path) const;

	//std::vector<uint32_t> Reserve_Data_Blocks(uint32_t cnt);
	uint32_t Acquire_Data_Block();
	t_DataBlockAcquirer Data_Block_Acquirer = [this] { return Acquire_Data_Block(); };

	uint32_t Acquire_Inode();

	void Init_Structures();

	void Print_Message(FSMessages message) const;

	bool Is_Formatted() const;

	void Assert_Is_Formatted() const;
};
