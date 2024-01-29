#pragma once

#include <istream>


/**
 * FS Operations
 */
class I_FSOps {
public:
	/**
	 * Transparently destructs
	 */
	virtual ~I_FSOps() = default;

public:
	/**
	 * Formats the FS
	 * @param size FS size
	 */
	virtual void OP_format(size_t size) = 0;
	/**
	 * Loads and executes commands from file
	 * @param path Path to file
	 */
	virtual void OP_load(const std::string &path) = 0;

	/**
	 * Changes working directory to provided path
	 * @param path Path
	 */
	virtual void OP_cd(const std::string &path) = 0;
	/**
	 * Prints working directory
	 */
	virtual void OP_pwd() const = 0;

	/**
	 * Copies file
	 * @param path1 Path Source
	 * @param path2 Path Destination
	 */
	virtual void OP_cp(const std::string &path1, const std::string &path2) = 0;
	/**
	 * Moves file
	 * @param path1 Path Source
	 * @param path2 Path Destination
	 */
	virtual void OP_mv(const std::string &path1, const std::string &path2) = 0;
	/**
	 * Removes file
	 * @param path Path
	 */
	virtual void OP_rm(const std::string &path) = 0;

	/**
	 * Creates a directory
	 * @param path Path
	 */
	virtual void OP_mkdir(const std::string &path) = 0;
	/**
	 * Removes directory
	 * @param path Path
	 */
	virtual void OP_rmdir(const std::string &path) = 0;

	/**
	 * Lists directory
	 * @param path Path
	 */
	virtual void OP_ls(const std::string &path) const = 0;

	/**
	 * Prints file content
	 * @param path Path
	 */
	virtual void OP_cat(const std::string &path) const = 0;

	/**
	 * Prints file/directory info
	 * @param path Path
	 */
	virtual void OP_info(const std::string &path) const = 0;

	/**
	 * Copies a file into FS
	 * @param path1 Path Source
	 * @param path2 Path Destination
	 */
	virtual void OP_incp(const std::string &path1, const std::string &path2) = 0;
	/**
	 * Copies a file from FS
	 * @param path1 Path Source
	 * @param path2 Path Destination
	 */
	virtual void OP_outcp(const std::string &path1, const std::string &path2) = 0;

	/**
	 * Symlinks a file
	 * @param path1 Path Source
	 * @param path2 Path Destination
	 */
	virtual void OP_slink(const std::string &path1, const std::string &path2) = 0;

};
