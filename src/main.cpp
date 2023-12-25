#include <iostream>
#include <memory>

#include "Simulator.hpp"


constexpr size_t kExpected_Args_Cnt = 2;

const std::string BLUE_COLOR{"\033[34m"};
const std::string RESET_COLOR{"\033[0m"};

int main(int argc, char **argv) {
	if (argc != kExpected_Args_Cnt) {
		std::cerr << "[fsinodes]ERR: Invalid arguments count" << std::endl;
		return EXIT_FAILURE;
	}

	std::cout << BLUE_COLOR;
	std::cout << "| [fsinodes] Inodes File System (23-12-14)" << std::endl;
	std::cout << "| Seminar Work of KIV/ZOS - \"Zaklady operacnich systemu\"" << std::endl;
	std::cout << "| Stanislav Kafara, skafara@students.zcu.cz" << std::endl;
	std::cout << "| University of West Bohemia, Pilsen";
	std::cout << RESET_COLOR << std::endl << std::endl;

	std::unique_ptr<I_FSOps> fs = std::make_unique<FileSystem>(argv[1], std::cout);
	Simulator sim{std::cin, std::move(fs)};
	sim.Run();

	return EXIT_SUCCESS;
}

