#pragma once

#include <istream>


class I_FSOps {
public:
	virtual ~I_FSOps() = default;

public:
	virtual void OP_format(uint32_t size) = 0;
	virtual void OP_load(const std::string &path) = 0;

	virtual void OP_cd(const std::string &path) = 0;
	virtual void OP_pwd() const = 0;

	virtual void OP_cp(const std::string &path1, const std::string &path2) = 0;
	virtual void OP_mv(const std::string &path1, const std::string &path2) = 0;
	virtual void OP_rm(const std::string &path) = 0;

	virtual void OP_mkdir(const std::string &path) = 0;
	virtual void OP_rmdir(const std::string &path) = 0;

	virtual void OP_ls(const std::string &path) const = 0;

	virtual void OP_cat(const std::string &path) const = 0;

	virtual void OP_info(const std::string &path) const = 0;

	virtual void OP_incp(const std::string &path1, const std::string &path2) = 0;
	virtual void OP_outcp(const std::string &path1, const std::string &path2) = 0;
};
