
// From source code to data generation (used for Quine)

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
		std::cout << "Usage : DataAsCodeGenerator any_file" << std::endl;
		return -1;
	}

	std::ifstream ifs(argv[1]);
	std::string line;
	while (ifs.good())
	{
		std::getline(ifs, line);
	}
	ifs.close();

	std::string outputFileName = std::string(argv[1]) + std::string(".out");
	std::ofstream ofs(outputFileName.c_str());
	std::string s;
	for (int i = 0; i < line.size(); i++)
	{
		ofs.write("CALC GA,", 8);
		ofs.write(" n", 1);
		s = NumberToNibble(Base::Four, line[i]);
		ofs.write(s.c_str(), s.size() - 1);
		ofs.write(" ", 1);
		if (i % 2)
		{
			ofs.write("BIRD ZO MOVE ZO", 15);
		}
		else
		{
			ofs.write("BIRD BU MOVE BU", 15);
		}
		ofs.write(" ", 1);
	}
		
	ofs.close();
	std::cout << "Output file created = " << outputFileName << std::endl;

	return 0;
}
