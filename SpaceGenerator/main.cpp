
// From source code to space (used for Quine)

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

	return 0;
}
