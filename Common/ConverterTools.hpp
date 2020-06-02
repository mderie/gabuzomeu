
#ifndef CONVERTER_TOOLS
#define CONVERTER_TOOLS

#include <string>
#include <vector>

#include "infint.hpp"
#include "RuntimeExceptions.hpp"

typedef unsigned char byte;
extern bool big; // No time do better, let's have a global...

enum class CellId { ga, bu, zo, meu, last_item };
const std::string CellIds[(size_t) (CellId::last_item)] = { "GA", "BU", "ZO", "MEU" };
byte GetCellId(const std::string& value); // Return type could be size_t as well

InfInt pow(const InfInt &base, const InfInt &expo); // This one is clearly missing in InfInt lib (bit operators as well :(
/*
bool IsPrime(const InfInt& value);
std::string PowerOfPrime(const InfInt& value);
*/

std::string NumberToNibble(const InfInt &n);
InfInt NibbleToNumber(const std::string &s);
std::vector<InfInt> CompositeStringToNumbers(const std::string &s);
std::string NumbersToCompositeString(const std::vector<InfInt> &v);
std::vector<byte> NumberToByteStream(const InfInt &n);
std::vector<byte> NumbersToByteStream(const std::vector<InfInt> &v);
InfInt ByteStreamToNumber(const std::vector<byte> &v);
//TODO: Needed ?
//... ByteStreamToNumbers(...)
#endif // CONVERTER_TOOLS
