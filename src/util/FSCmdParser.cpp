#include <sstream>
#include <regex>

#include "FSCmdParser.hpp"


const std::unordered_map<std::string, const t_FSCmdParser> FSCmdParser::kCmds_Parsers{
	{"format", Parse_OP_format},
	{"load", Parse_OP_load},
	{"cd", Parse_OP_cd},
	{"pwd", Parse_OP_pwd},

	{"cp", Parse_OP_cp},
	{"mv", Parse_OP_mv},
	{"rm", Parse_OP_rm},

	{"mkdir", Parse_OP_mkdir},
	{"rmdir", Parse_OP_rmdir},

	{"ls", Parse_OP_ls},

	{"cat", Parse_OP_cat},

	{"info", Parse_OP_info},

	{"incp", Parse_OP_incp},
	{"outcp", Parse_OP_outcp}
};

t_FSOp FSCmdParser::Parse(const std::string &line) {
	std::istringstream cmd_isstream{line};
	std::string cmd;

	std::getline(cmd_isstream, cmd, ' '); // get cmd name
	for (const auto &cmd_parser : kCmds_Parsers) {
		if (cmd_parser.first == cmd) { // cmd name
			const t_FSOp op = cmd_parser.second(cmd_isstream); // parse cmd, args, get fs op

			std::string _;
			if (!cmd_isstream || cmd_isstream >> _) { // invalid args or args left
				throw std::invalid_argument{"Invalid Command Format"};
			}

			return op; // parsed fs op
		}
	}

	throw std::invalid_argument{"Command '" + cmd + "' does not exist."};
}

t_FSOp FSCmdParser::Parse_OP_format(std::istream &args) {
	const std::regex size_pattern("([1-9]\\d*)(KB|MB|GB)");
	std::smatch match;

	std::string size_input;
	args >> size_input;

	if (!std::regex_match(size_input, match, size_pattern)) {
		throw std::invalid_argument{"'" + size_input + "' is not a valid size."};
	}

	const uint32_t number = static_cast<uint32_t>(std::stoul(match[1].str()));
	const std::string &unit = match[2].str();
	uint32_t multiplier;
	if (unit == "KB") {
		multiplier = 1024;
	} else if (unit == "MB") {
		multiplier = 1048576;
	} else {
		multiplier = 1073741824;
	}

	const uint32_t size = number * multiplier;
	return [size](I_FSOps &fs) {
		fs.OP_format(size);
	};
}

t_FSOp FSCmdParser::Parse_OP_load(std::istream &args) {
	std::string path{};
	args >> path;

	return [path](I_FSOps &fs) {
		fs.OP_load(path);
	};
}

t_FSOp FSCmdParser::Parse_OP_cd(std::istream &args) {
	std::string path;
	args >> path;

	return [path](I_FSOps &fs) {
		fs.OP_cd(path);
	};
}

t_FSOp FSCmdParser::Parse_OP_pwd(std::istream &) {
	return [](I_FSOps &fs) {
		fs.OP_pwd();
	};
}

t_FSOp FSCmdParser::Parse_OP_cp(std::istream &args) {
	std::string path1, path2;
	args >> path1 >> path2;

	return [path1, path2](I_FSOps &fs) {
		fs.OP_cp(path1, path2);
	};
}

t_FSOp FSCmdParser::Parse_OP_mv(std::istream &args) {
	std::string path1, path2;
	args >> path1 >> path2;

	return [path1, path2](I_FSOps &fs) {
		fs.OP_mv(path1, path2);
	};
}

t_FSOp FSCmdParser::Parse_OP_rm(std::istream &args) {
	std::string path;
	args >> path;
	Assert_Path_Not_Dot_Ddot(path);

	return [path](I_FSOps &fs) {
		fs.OP_rm(path);
	};
}

t_FSOp FSCmdParser::Parse_OP_mkdir(std::istream &args) {
	std::string path;
	args >> path;
	Assert_Path_Not_Dot_Ddot(path);

	return [path](I_FSOps &fs) {
		fs.OP_mkdir(path);
	};
}

t_FSOp FSCmdParser::Parse_OP_rmdir(std::istream &args) {
	std::string path;
	args >> path;
	Assert_Path_Not_Dot_Ddot(path);

	return [path](I_FSOps &fs) {
		fs.OP_rmdir(path);
	};
}

t_FSOp FSCmdParser::Parse_OP_ls(std::istream &args) {
	std::string path{kDot};
	if (!args.eof()) {
		if (!(args >> path)) {
			args.clear();
		}
	}

	return [path](I_FSOps &fs) {
		fs.OP_ls(path);
	};
}

t_FSOp FSCmdParser::Parse_OP_cat(std::istream &args) {
	std::string path;
	args >> path;

	return [path](I_FSOps &fs) {
		fs.OP_cat(path);
	};
}

t_FSOp FSCmdParser::Parse_OP_info(std::istream &args) {
	std::string path;
	args >> path;

	return [path](I_FSOps &fs) {
		fs.OP_info(path);
	};
}

t_FSOp FSCmdParser::Parse_OP_incp(std::istream &args) {
	std::string path1, path2;
	args >> path1 >> path2;

	return [path1, path2](I_FSOps &fs) {
		fs.OP_incp(path1, path2);
	};
}

t_FSOp FSCmdParser::Parse_OP_outcp(std::istream &args) {
	std::string path1, path2;
	args >> path1 >> path2;

	return [path1, path2](I_FSOps &fs) {
		fs.OP_outcp(path1, path2);
	};
}

void FSCmdParser::Assert_Path_Not_Dot_Ddot(const std::filesystem::path &path) {
	const std::filesystem::path filename = path.filename();
	if (filename == kDot || filename == kDot_Dot) {
		throw std::invalid_argument{"./.. Not Allowed"};
	}
}
