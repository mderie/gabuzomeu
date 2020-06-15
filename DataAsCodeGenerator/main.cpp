
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

	bool stark = false;

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
		if (stark)
		{
			ofs.write("CALCGA,", 7);
		}
		else
		{
			ofs.write("CALC GA, ", 9);			
		}		

		Quine = true;  // Remove the trailing #
		s = NumberToNibble(Base::Four, line[i]);
		ofs.write(s.c_str(), s.size() - 1);

		if (i % 2)
		{
			if (stark)
			{
				ofs.write("BIRDZOMOVEZO", 12);
			}
			else
			{
				ofs.write("BIRD ZO", 7);
				ofs << std::endl;
				ofs.write("MOVE ZO", 7);
				ofs << std::endl;
			}			
		}
		else
		{
			if (stark)
			{
				ofs.write("BIRDBUMOVEBU", 12);
			}
			else
			{
				ofs << std::endl;
				ofs.write("BIRD BU", 7);
				ofs << std::endl;
				ofs.write("MOVE BU", 7);
				ofs << std::endl;
			}
		}
	}

	ofs.close();
	std::cout << "Output file created = " << outputFileName << std::endl;

	return 0;
}
