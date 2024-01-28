#include "Simulator.hpp"
#include "util/FSCmdParser.hpp"


void Simulator::Run(std::istream &cmd_istream, I_FSOps &fs) {
	for (std::string line; std::getline(cmd_istream, line);) {
		if (line.empty()) {
			continue;
		}
		if (line == "exit") {
			return;
		}

		try {
			const auto op = FSCmdParser::Parse(line);
			op(fs);
		}
		catch (const FSException &e) {
			std::cerr << e.what() << std::endl;
		}
		catch (const std::exception &e) {
			std::cerr << "[fsinodes]ERR: " << e.what() << std::endl;
		}
	}
}
