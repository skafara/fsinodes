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
	void OP_format(size_t size) override;
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

	std::pair<uint32_t, std::filesystem::path> _work_dir;

	/**
	 * Computes data blocks count a file will need to be stored
	 * @param filesize File size
	 * @return Count
	 */
	static uint32_t Get_Necessary_Data_Blocks_Cnt(size_t filesize);

	/**
	 * Returns a superblock formatted relative to the FS size
	 * @param fs_size FS size
	 * @return Superblock
	 */
	static Superblock::t_Superblock Get_Formatted_Superblock(size_t fs_size);
	/**
	 * Returns an absolute without Dot and Ddot files
	 * @param path Path
	 * @return Path
	 */
	static std::filesystem::path Get_Cannonical_Path(const std::filesystem::path &path);

	/**
	 * Initializes FS sections
	 */
	void Init_Structures();
	/**
	 * Returns whether has been formatted
	 * @return Bool
	 */
	bool Is_Formatted() const;

	/**
	 * Resolves a path to the inode index and path
	 * @param path Path
	 * @param start Starting point (directory inode index and path)
	 * @param is_ignore_end Ignore expanding symlink if it is the last element of path
	 * @return Inode index and path
	 */
	std::pair<uint32_t, std::filesystem::path> Resolve_Path(const std::filesystem::path &path, const std::pair<uint32_t, std::filesystem::path> &start, bool is_ignore_end) const;
	/**
	 * Resolves a path from working directory
	 * Ignores expanding symlink at the end
	 * @param path Path
	 * @return Inode index
	 */
	uint32_t Resolve_Path_Inode(const std::filesystem::path &path) const;
	/**
	 * Resolves a path from working directory
	 * Expands symlinks at the end
	 * @param path Path
	 * @return Inode index and path
	 */
	std::pair<uint32_t, std::filesystem::path> Resolve_Final_Path(const std::filesystem::path &path) const;
	/**
	 * Resolves a path from working directory
	 * Expands symlinks at the end
	 * @param path Path
	 * @return Inode index
	 */
	uint32_t Resolve_Final_Path_Inode(const std::filesystem::path &path) const;
	/**
	 * Resolves a parent path from working directory
	 * Expands symlinks at the end
	 * @param path Path
	 * @return Inode index
	 */
	uint32_t Resolve_Final_Parent_Inode(const std::filesystem::path &path) const;

	/**
	 * Acquires an inode
	 * Sets it as spent
	 * @return Inode index
	 */
	uint32_t Acquire_Inode();
	/**
	 * Acquires a data block
	 * Sets it as spent
	 * @return Data block index
	 */
	uint32_t Acquire_Data_Block();
	/**
	 * Acquires a data block with given index
	 * Sets it as spent
	 * @param dblock_idx Data block index
	 * @return Data block index
	 */
	uint32_t Acquire_Data_Block(uint32_t dblock_idx);

	/**
	 * Prints FS message
	 * @param message Message
	 */
	void Print_Message(FSMessages message) const;

	/**
	 * Asserts file name length validity
	 * @param path Path
	 */
	static void Assert_Valid_Filename_Length(const std::filesystem::path &path);
	/**
	 * Asserts FS has been formatted
	 */
	void Assert_Is_Formatted() const;
	/**
	 * Asserts there is enough resources
	 * @param bitmap Bitmap
	 * @param cnt Count
	 */
	static void Assert_Has_Resources(const Bitmap &bitmap, uint32_t cnt);
};
