
// From source code to gabuzomeu big number (used for Quine)

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

int main(int argc, char *argv[])
{
	std::string source;
	std::string data;

	//std::cout << "debug 0" << std::endl;

	try
	{
		cxxopts::Options options("NumberGenerator", "");
		options.add_options()
			("s,source", "", cxxopts::value<std::string>()->default_value(""))
			("d,data", "", cxxopts::value<std::string>()->default_value(""));

		//std::cout << "debug 1" << std::endl;

		auto parameters = options.parse(argc, argv);
		//std::cout << "debug 2" << std::endl;
		source = parameters["source"].as<std::string>();
		//std::cout << "debug 21" << std::endl;
		data = parameters["data"].as<std::string>();
	}
	catch (const std::exception &e)
	{
		std::cout << "source = " << source << ", data = " << data << std::endl;
		return -1;
	}

	//std::cout << "debug 22" << std::endl;

	if ((source != "" and data != "") or (source == "" and data == ""))
	{
		std::cout << "Usage : NumberGenerator --source=\"filename\" or --data=\"any_value\"" << std::endl;
		return -2;
	}

	//std::cout << "debug 3" << std::endl;

	std::string input;
	//std::cout << "source = " << source << std::endl;
	if (source == "")
	{
		input = data;
	}
	else
	{
		if (!std::filesystem::exists(source))
		{
			//std::cout << "Pff";
			throw FileNotFoundException();
		}

		input.resize(std::filesystem::file_size(source), '\0');
		std::ifstream ifs(source, std::ios::binary);
		ifs.read((char*) input.data(), std::filesystem::file_size(source));
		ifs.close();				
	}

	/*
	std::cout << "input = " << input << std::endl;	
	if ((input.size() < 3) or (input[0] != '"') or (input[input.size() - 1] != '"'))
	{
		std::cout << "Invalid data string" << std::endl;
		return -2;
	}
	input = input.substr(1, input.size() - 1);
	*/

	/*
	std::string output = ;
	for (const auto& c : input)
	{
		output += NumberToNibble(InfInt(c));
	}
	*/
	//std::vector<byte> output(argv[1], argv[1] + strlen(argv[1]));

	//std::cout << "input = " << input << std::endl;
	std::vector<byte> output(input.begin(), input.end());
	std::cout << "CALC MEU, " << NumberToNibble(ByteStreamToNumber(output)) << std::endl << "DUMP MEU";

    return 0;
}
