
// From source code to space ? Well, generate CALC / DUMP series on the default bird in order to print a given string (used for Quine)

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>

// Third party
#include "cxxopts.hpp"
#include "InfInt.hpp"

// Home made
#include "RuntimeExceptions.hpp"
#include "ConverterTools.hpp"
#include "StringTools.hpp"

int main(int argc, char* argv[])
{
	/*
	std::vector<std::string> tokens = { "CALC", "GA,", "#ZOBUMEUGA", "CALC", "BU,", "#ZOBUMEUGA" "CALC", "MEU,", "#BU", ":LOOP", "CALC", "MEU,", "MEU*GA", "CALC", "BU,", "BU-#BU", "ELSE", "BU,", "LOOP", "DUMP", "MEU" };
	std::string output = "space.dat";
	std::ofstream osf(output);
	std::vector<byte> v;
	for (int i = 1; i < 181; i++)
	{
		InfInt bn = pow(InfInt(i), InfInt(i));
		v = NumberToByteStream(bn);
		output = "i = " + std::to_string(i) + ", nibble = " + NumberToNibble(Base::default_, InfInt(i)) + ", v.size() = " + std::to_string(v.size()) + ", bn.numberOfDigits() = " + std::to_string(bn.numberOfDigits()) + " & bn.value = " + bn.toString() + "\n";
		osf.write(output.c_str(), output.size());
	}
	osf.close();
	*/

	// So we can uses 0 & 1 to the console ;-)
	/*
	for (size_t i = 0; i < 32; i++)
	{
		std::cout << "i = " << i << " and it gives the char '" << char(i) << "'" << std::endl;
	}
	*/

	if (argc < 3)
	{
		std::cout << "Usage : SpaceGenerator --cell=\"name\" --data=\"any\" [-s | --stark] [-Q | --Quine]" << std::endl;
		return -1;
	}

	std::string cell;
	std::string data;
	bool stark;

	try
	{
		cxxopts::Options options("SpaceGenerator", "");
		options.add_options()
			("c,cell", "", cxxopts::value<std::string>()->default_value(""))
			("d,data", "", cxxopts::value<std::string>()->default_value(""))
			("s,stark", "", cxxopts::value<bool>()->default_value("false"))
			("Q,Quine", "", cxxopts::value<bool>()->default_value("false"));

		auto parameters = options.parse(argc, argv);
		cell = parameters["cell"].as<std::string>();
		data = parameters["data"].as<std::string>();
		stark = parameters["stark"].as<bool>(); // Unreadable mode !
		Quine = parameters["Quine"].as<bool>(); // Care : global variable (beurk :)
	}
	catch (const std::exception& e)
	{
		std::cout << "Exception : " << e.what() << std::endl;
		return -2;
	}

	for (size_t i = 0; i < data.size(); i++)
	{
		if (stark)
		{
			std::cout << "CALC" << cell << ",";
		}
		else
		{
			std::cout << "CALC " << cell << ", ";
		}

		std::cout << NumberToNibble(Base::four, InfInt(data[i]));
		if (!stark)
		{
			std::cout << std::endl;
		}

		if (stark)
		{
			std::cout << "DUMP" << cell;
		}
		else
		{
			std::cout << "DUMP " << cell;
		}

		if (!stark)
		{
			std::cout << std::endl;
		}
	}

	return 0;
}
