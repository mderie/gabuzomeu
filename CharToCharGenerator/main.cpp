
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

	bool stark = false;

	std::string input(argv[1]);
	std::ofstream ofs(input);
	std::string s;
	for (int i = 0; i < input.size(); i++)
	{
		if (stark)
		{
			ofs.write("CALCMEU,", 8);

		}
		else
		{
			ofs.write("CALC MEU, ", 10);
			ofs << std::endl;
		}
		
		s = NumberToNibble(Base::Four, InfInt(input[i]));
		std::cout << "input[" << i << "] = " << input[i] << std::endl;
		ofs.write(s.c_str(), s.size());
		if (stark)
		{
			ofs.write("DUMPMEU", 7);
		}
		else
		{
			ofs.write("DUMP MEU", 8);
			ofs << std::endl; // Multiplatform issue, we can't guess the size of the new line character(s)...
		}		
	}
	std::cout << "Output file created = " << input << std::endl;

	ofs.close();
	return 0;
}
