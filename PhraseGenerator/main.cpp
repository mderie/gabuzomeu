
// From open text to gabuzomeu data sets and strings (used for 99 bottles of beer)

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>

// Third party
#include "../Common/cxxopts.hpp"
#include "../Common/infint.hpp"

// Home made
#include "../Common/RuntimeExceptions.hpp"
#include "../Common/ConverterTools.hpp"
#include "../../Common/StringTools.hpp"

std::vector<std::string> stringList;
std::vector<std::string> playStringList;
std::vector<std::string> itemList;
std::vector<std::string> playItemList;
int cellIdToFirstLink; // constrain from the outside word !
int cellIdToNextChar; // Care of the alternance !
int cellIdToNextLink; // Care of the alternance !
std::ofstream ofs;

int NextCellId(int value)
{
	return (value < 3 ? ++value : 0);
}
int PrevCellId(int value)
{
	return (value > 0 ? --value : 3);
}

void RunString(const std::string& s)
{
	std::string result;
	std::string play;
	std::string back;
		
	for (size_t i=1; i<=s.size(); i++)
	{
		result += " CALC " + CellIds[cellIdToNextChar] + ", " + NumberToNibble(Base::default_, InfInt(s[i]));
		play += "";
		back += "";

		cellIdToNextChar = NextCellId(cellIdToNextChar);
		//std::cout << "i = " << i << " & i % 3 = " << i % 3 << std::endl;
		if ((i % 3) == 0)
		{
			if (i < s.size())
			{
				result += " BIRD " + CellIds[cellIdToNextChar];
				cellIdToNextChar = NextCellId(cellIdToNextChar);

				play += "";
				back += "";
			}
		}
	}

	stringList.emplace_back(result);
	std::cout << "result = " << result << std::endl;

	playStringList.emplace_back(play);
	std::cout << "play = " << play << std::endl;

	playStringList.emplace_back(back);
	std::cout << "back = " << back << std::endl;
}

void RunInit()
{
	//TODO
}

void RunList()
{
	//TODO
}

void RunPlays(size_t index)
{
	//TODO
}

int main(int argc, char* argv[])
{
	if (argc != 4)
	{
		std::cout << "Usage : PhraseGenerator input_file_name output_file_name start_cell_id (between 0 and 3)" << std::endl;
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
	cellIdToNextLink = NextCellId(cellIdToNextChar);

	// Care : if one provides a filename between double quotes they will be removed by the platform !

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

	//TODO: Output to screen finally ? If someone wants it to a file then just use >
	ofs.open(output);
	//RunInit();
	RunList();
	/*
	for (int i = 0; i < charList.size(); i++)
	{
		cellIdToNextChar = NextCellId(startCellId);
		RunPlays(i);
	}
	*/
	ofs.close();

	return 0;
}
