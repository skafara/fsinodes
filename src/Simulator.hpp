#pragma once

#include <iostream>

#include "fs/FileSystem.hpp"


/**
 * Filesystem Simulator
 */
class Simulator {
public:
	/**
	 * Runs the simulation on provided filesystem
	 * @param cmd_istream Input stream of filesystem commands
	 * @param fs Filesystem
	 */
	static void Run(std::istream &cmd_istream, I_FSOps &fs);

};
