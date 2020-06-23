
// From string to char generator (doublon ?)
// Transformed to source code file to compact version !

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include <regex>

// Third party
#include "cxxopts.hpp"
#include "infint.hpp"

// Home made
#include "RuntimeExceptions.hpp"
#include "ConverterTools.hpp"
#include "StringTools.hpp"

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cout << "Usage : CharToCharGenerator \"any_source_code_file\"" << std::endl;
		return -1;
	}

	std::string input(argv[1]);
	std::ifstream ifs(input);
	std::ofstream ofs(input + ".out");
	std::string line;
	std::string result;

	while (ifs.good())
	{
		std::getline(ifs, line);

		if (line == "")
		{
			continue;
		}

		if (size_t pos = line.find(';'); pos != std::string::npos) // Thanks to C++ 17
		{
			line = line.substr(0, pos);
		}

		if (line[0] == ':')
		{
			// We can't shortcut the labels here... It must be done directly in the code (else we wan't be able to rebuild itself from the data
			line = " " + line + " "; // Add spaces
		}
		else
		{
			line = std::regex_replace(line, std::regex(" "), ""); // Remove spaces :)
		}

		if (islower(line[line.size() - 1]))
		{
			line += " ";
		}

		result += line;
	}

	ifs.close();
	
	result = std::regex_replace(result, std::regex("  "), " "); // Remove double spaces :)
	// Same effect :)
	// FindAndReplaceInternal(result, "  ", " ");

	ofs.write(result.c_str(), result.size());
	ofs.close();
	std::cout << "Output file created = " << input + ".out" << std::endl;

	return 0;
}
