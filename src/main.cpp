#include <iostream>
#include <memory>

#include "Simulator.hpp"


constexpr size_t kExpected_Args_Cnt = 2;

const std::string kColor_Blue{"\033[34m"};
const std::string kColor_Reset{"\033[0m"};

void Print_Info() {
	std::cout << kColor_Blue;
	std::cout << "| [fsinodes] Inodes File System (23-01-29)" << std::endl;
	std::cout << "| Seminar Work of KIV/ZOS - \"Zaklady operacnich systemu\"" << std::endl;
	std::cout << "| Stanislav Kafara, skafara@students.zcu.cz" << std::endl;
	std::cout << "| University of West Bohemia, Pilsen";
	std::cout << kColor_Reset << std::endl << std::endl;
}

int main(int argc, char **argv) {
	if (argc != kExpected_Args_Cnt) {
		std::cerr << "[fsinodes]ERR: Invalid arguments count" << std::endl;
		return EXIT_FAILURE;
	}

	Print_Info();

	std::unique_ptr<I_FSOps> fs = std::make_unique<FileSystem>(argv[1], std::cout);
	Simulator::Run(std::cin, *fs);

	return EXIT_SUCCESS;
}

