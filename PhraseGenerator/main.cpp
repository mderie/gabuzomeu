
// From open text to gabuzomeu data sets and strings (used for 99 bottles of beer)

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>

// Home made
#include "..\Common\RuntimeExceptions.hpp"
#include "..\Common\ConverterTools.hpp"

std::vector<std::string> charLinks;
//std::string buildList;
std::vector<std::string> chainLinks;
int cellIdToFirstLink; // constrain from the outside word !
int cellIdToNextChar; // Care of the alternance !
int cellIdToNextLink; // Care of the alternance !

int NextCellId(int value)
{
	return (value < 3 ? ++value : 0);
}
int PrevCellId(int value)
{
	return (value > 0 ? --value : 3);
}

void RunString(const std::string& value)
{
	std::string result;

	bool flip = false;
	for (const auto& it : value)
	{
		result = "CALC " + value[it];
		flip = !flip;
	}

	charLinks.emplace_back(result);
}

void RunInit()
{
	//TODO
}

void RunList()
{
	//TODO
}

void RunPlays()
{
	//TODO
}

int main(int argc, char* argv[])
{
	if (argc != 4)
	{
		std::cout << "Usage PhraseGenerator input_file_name output_file_name start_cell_id (between 0 and 3)" << std::endl;
		return -1;
	}

	std::string input = argv[1];
	std::string output = argv[2];
	std::string tmp = argv[3];

	if ((tmp.size() > 1) or (tmp[0] < '0') or (tmp[0] > '3'))
	{
		std::cout << "Bad cell id value !" << std::endl;
		return -2;
	}
	int startCellId = std::stoi(tmp);
	cellIdToNextChar = NextCellId(startCellId);
	cellIdToNextLink = NextCellId(startCellId);

	if (!std::filesystem::exists(input))
	{
		std::cout << "input_file_name doesn't exist !" << std::endl;
		return -3;
	}

	if (std::filesystem::exists(output))
	{
		std::cout << "output_file_name already exists !" << std::endl;
		return -4;
	}

	std::string s;
	std::ifstream ifs(input);
	while (ifs.good())
	{
		std::getline(ifs, s);
		RunString(s);
	}
	ifs.close();

	std::ofstream ofs(output);
	RunInit();
	RunList();
	for (int i = 0; i < charLinks.size(); i++)
	{
		RunPlays();
	}
	ofs.close();

	return 0;
}
