
// From string to char generator (used for Quine)

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>

// Third party
#include "..\Common\cxxopts.hpp"
#include "..\Common\infint.hpp"

// Home made
#include "..\Common\RuntimeExceptions.hpp"
#include "..\Common\ConverterTools.hpp"
#include "..\..\Common\StringTools.hpp"

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cout << "Usage : CharToCharGenerator \"any_string\"" << std::endl;
		return -1;
	}

	std::string input(argv[1]);
	std::ofstream ofs(input);
	std::string s;
	for (int i = 0; i < input.size(); i++)
	{
		ofs.write("CALC MEU, ", 10);
		s = NumberToNibble(Base::Four, InfInt(input[i]));
		std::cout << "input[" << i << "] = " << input[i] << std::endl;
		ofs.write(s.c_str(), s.size());
		ofs.write("\nDUMP MEU\n", 10);
	}
	std::cout << "Output file created = " << input << std::endl;

	ofs.close();
	return 0;
}
