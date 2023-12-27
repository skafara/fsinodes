#include "Simulator.hpp"
#include "util/FSCmdParser.hpp"


Simulator::Simulator(std::istream &cmd_istream, std::unique_ptr<I_FSOps> fs) :
	_cmd_istream(cmd_istream),
	_fs(std::move(fs)) {
	//
}

void Simulator::Run() {
	for (std::string line; std::getline(_cmd_istream, line);) {
		if (line.empty()) {
			continue;
		}
		if (line == "exit") {
			return;
		}

		try {
			const auto op = FSCmdParser::Parse(line);
			op(*_fs);
		}
		catch (const std::exception &e) {
			std::cerr << e.what() << std::endl;
		}
	}
}
