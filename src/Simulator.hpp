#pragma once

#include <iostream>

#include "fs/FileSystem.hpp"


class Simulator {
public:
	Simulator(std::istream &cmd_istream, std::unique_ptr<I_FSOps> fs);

	void Run();

private:
	std::istream &_cmd_istream;
	const std::unique_ptr<I_FSOps> _fs;
};
