
// From source code to data generation using multiple birds with alternate arrow (used for Quine)

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
	if (argc < 2)
	{
		std::cout << "Usage : DataAsCodeGenerator --file=\"any\" [-s | --stark]" << std::endl;
		return -1;
	}

	cxxopts::Options options("DataAsCodeGenerator", "");
	options.add_options()
		("f,file", "", cxxopts::value<std::string>()->default_value(""))
		("s,stark", "", cxxopts::value<bool>()->default_value("false"));

	auto parameters = options.parse(argc, argv);
	std::string file = parameters["file"].as<std::string>();
	bool stark = parameters["stark"].as<bool>(); // Unreadable mode !
	Quine = true;  // Remove the trailing #

	if (file == "")
	{
		std::cout << "Usage : DataAsCodeGenerator --file=\"any\" [-s | --stark]" << std::endl;
		return -2;
	}

	std::ifstream ifs(file);
	std::vector<std::string> lines;
	std::string line;
	while (ifs.good())
	{
		std::getline(ifs, line);

		if (line == "")
		{
			continue;
		}

		if (size_t pos = line.find(';') ; pos != std::string::npos) // Thanks to C++ 17
		{
			line = line.substr(0, pos);
		}

		if (line[0] == ':')
		{
			// We can't shortcut the labels here... It must be done directly in the code (else we wan't be able to rebuild itself from the data
		}

		//if (stark) {}
		lines.emplace_back(line);
	}
	ifs.close();

	std::string outputFileName = std::string(file) + std::string(".out");
	std::ofstream ofs(outputFileName.c_str());
	std::string s;
	for (const auto &line : lines) // It seems that we can't recycle the existing variable... Let's cheat then !-)
	{
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
			
			s = NumberToNibble(Base::Four, line[i]);
			ofs.write(s.c_str(), s.size());

			if (!stark)
			{
				ofs << std::endl;
			}			

			if (i % 2)
			{
				if (stark)
				{
					ofs.write("BIRDZOLIFTZO", 12);
				}
				else
				{
					ofs.write("BIRD ZO", 7);
					ofs << std::endl;
					ofs.write("LIFT ZO", 7);
					ofs << std::endl;
				}			
			}
			else
			{
				if (stark)
				{
					ofs.write("BIRDBULIFTBU", 12);
				}
				else
				{
					ofs.write("BIRD BU", 7);
					ofs << std::endl;
					ofs.write("LIFT BU", 7);
					ofs << std::endl;
				}
			}
		}

		/*
		if (!stark)
		{
			s = NumberToNibble(Base::Four, line[i]);
			ofs.write("BIRD BU", 7);
			ofs << std::endl;
			ofs.write("LIFT BU", 7);
			ofs << std::endl;
		}
		*/
	}

	ofs.close();
	std::cout << "Output file created = " << outputFileName << std::endl;

	return 0;
}
