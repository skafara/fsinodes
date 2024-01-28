#pragma once

#include <string>
#include <filesystem>

#include "I_FSOps.hpp"
#include "container/MMappedFile.hpp"
#include "sections/superblock/Superblock.hpp"
#include "sections/bitmaps/Bitmap.hpp"
#include "sections/inodes/Inodes.hpp"
#include "sections/data/Data.hpp"


/**
 * FS Messages
 */
enum class FSMessages {
	kOk,
	kPathNotFound,
	kFileNotFound,
	kNotEmpty,
	kCannotCreateFile,
	kExist
};

/**
 * FS Messages Descriptions
 */
const std::unordered_map<FSMessages, const std::string> FSMessages_Strings{
	{FSMessages::kOk, "OK"},
	{FSMessages::kPathNotFound, "PATH NOT FOUND"},
	{FSMessages::kFileNotFound, "FILE NOT FOUND"},
	{FSMessages::kNotEmpty, "NOT EMPTY"},
	{FSMessages::kCannotCreateFile, "CANNOT CREATE FILE"},
	{FSMessages::kExist, "EXIST"}
};

/**
 * Returns FS Message Description
 * @param message Message
 * @return Description
 */
const std::string &Get_FSMessage_String(FSMessages message);


/**
 * Filesystem Exception
 */
class FSException : public std::runtime_error {
public:
	/**
	 * Transparently constructs
	 * @param msg Message
	 */
	explicit FSException(const std::string &msg) : std::runtime_error(msg) {
		//
	}
	/**
	 * Transparently constructs
	 * @param msg FS Message
	 */
	explicit FSException(FSMessages msg) : std::runtime_error(Get_FSMessage_String(msg)) {
		//
	}
};

/**
 * Path Not Found Exception
 */
class PathNotFoundException : public FSException {
public:
	/**
	 * Transparently constructs
	 */
	explicit PathNotFoundException() : FSException(FSMessages::kPathNotFound) {
		//
	}
};


/**
 * Filesystem
 */
class FileSystem : public I_FSOps {
public:
	/**
	 * Constructs and loads in filesystem
	 * @param fs_path Path
	 * @param out_stream Output stream
	 */
	FileSystem(const std::string &fs_path, std::ostream &out_stream);

	FileSystem(const FileSystem &) = delete;
	FileSystem &operator=(const FileSystem &) = delete;

public:
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

	void OP_slink(const std::string &path1, const std::string &path2) override;

private:
	static constexpr uint32_t kSuperblock_Offset = 0;

	inline static const std::string kRoot_Dir_Path = "/";
	static constexpr uint32_t kRoot_Dir_Inode_Idx = 0;
	static constexpr uint32_t kRoot_Dir_DBlock_Idx = 0;

	static constexpr char kPath_Delimiter = '/';
	inline static const std::string kDot = ".";
	inline static const std::string kDot_Dot = "..";

	static constexpr size_t kValid_Filename_Max_Len = 11;

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

	static uint32_t Get_Necessary_Data_Blocks_Cnt(size_t filesize);

	static Superblock::t_Superblock Get_Formatted_Superblock(size_t fs_size);
	static std::filesystem::path Get_Cannonical_Path(const std::filesystem::path &path);

	void Init_Structures();
	bool Is_Formatted() const;

	uint32_t Resolve_Path(const std::string &path) const;
	uint32_t Resolve_Parent(const std::string &path) const;

	uint32_t Acquire_Inode();
	uint32_t Acquire_Data_Block();
	uint32_t Acquire_Data_Block(uint32_t dblock_idx);

	void Print_Message(FSMessages message) const;

	static void Assert_Valid_Filename_Length(const std::filesystem::path &path);
	void Assert_Is_Formatted() const;
	static void Assert_Has_Resources(const Bitmap &bitmap, uint32_t cnt);
};
