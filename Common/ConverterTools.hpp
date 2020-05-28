
#ifndef CONVERTER_TOOLS
#define CONVERTER_TOOLS

#include <string>
#include <vector>

#include "infint.hpp"
#include "RuntimeExceptions.hpp"

typedef unsigned char byte;
extern bool big; // No time, let's have a global...

enum class CellId { ga, bu, zo, meu, last_item };
const std::string CellIds[(size_t) (CellId::last_item)] = { "GA", "BU", "ZO", "MEU" };

std::string NumberToNibble(const InfInt &n);
InfInt NibbleToNumber(const std::string &s);
std::vector<InfInt> CompositeStringToNumbers(const std::string &s);
std::string NumbersToCompositeString(const std::vector<InfInt> &v);
std::vector<byte> NumberToByteStream(const InfInt &n);
std::vector<byte> NumbersToByteStream(const std::vector<InfInt>& v);
//TODO: Needed ?
//... ByteStreamToNibble(...));
//... ByteStreamToNumbers(...)
#endif // CONVERTER_TOOLS
