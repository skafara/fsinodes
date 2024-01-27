#pragma once

#include <iostream>

#include "fs/FileSystem.hpp"


class Simulator {
public:
	//Simulator(std::istream &cmd_istream, std::unique_ptr<I_FSOps> fs);

	static void Run(std::istream &cmd_istream, I_FSOps &fs);

private:
	/*std::istream &_cmd_istream;
	const std::unique_ptr<I_FSOps> _fs;*/
};
