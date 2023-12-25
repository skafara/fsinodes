#pragma once

#include "../fs/FileSystem.hpp"


using t_FSOp = std::function<void (I_FSOps &)>;
using t_FSCmdParser = std::function<t_FSOp (std::istream &)>;

class FSCmdParser {
public:
	static t_FSOp Parse(const std::string &line);

private:
	static const std::unordered_map<std::string, const t_FSCmdParser> kCmds_Parsers;

	static t_FSOp Parse_OP_format(std::istream &args);
	static t_FSOp Parse_OP_load(std::istream &args);

	static t_FSOp Parse_OP_cd(std::istream &args);
	static t_FSOp Parse_OP_pwd(std::istream &args);

	static t_FSOp Parse_OP_cp(std::istream &args);
	static t_FSOp Parse_OP_mv(std::istream &args);
	static t_FSOp Parse_OP_rm(std::istream &args);

	static t_FSOp Parse_OP_mkdir(std::istream &args);
	static t_FSOp Parse_OP_rmdir(std::istream &args);

	static t_FSOp Parse_OP_ls(std::istream &args);

	static t_FSOp Parse_OP_cat(std::istream &args);

	static t_FSOp Parse_OP_info(std::istream &args);

	static t_FSOp Parse_OP_incp(std::istream &args);
	static t_FSOp Parse_OP_outcp(std::istream &args);
};
