
#ifndef CONVERTER_TOOLS
#define CONVERTER_TOOLS

#include <string>
#include <vector>

#include "infint.hpp"
#include "RuntimeExceptions.hpp"

typedef unsigned char byte;
extern bool big; // No time do better, let's have a global...

// Change the immediate rule in order to enforce a trailing #
// We need this in order to not change the nibble I/O, so when working with other bases than the default one
// The situation is more on the reading from file or from the command line ie : #BUGA
// could be read as #BU in B4 followed by GA in B256 or as #BUGA in B4
// So in Quine mode we explicitely ask the parser to enforce a trailing # for the immediate in
// order to dump them as usual nibbles so with a trailing # as well...
// Update : let's do the opposite way : in Quine mode the all nibbles that goes to the output
// will be without the trailing #
extern bool Quine; // Respect for the person is bigger than the codding style hence the upper case
extern bool write;

enum class Base { Unknown = 0, Two = 2, Four = 4, Ten = 8, SixTeen = 16, SixtyFour = 64, default_ = 256 };

struct BSII
{
    Base b;
    std::string s;
    InfInt ii;

    // Hum... https://www.fluentcpp.com/2018/06/15/should-structs-have-constructors-in-cpp/
    // And https://en.cppreference.com/w/cpp/language/default_constructor
    // We can't use resize() if we don't provide an parameterless ctor...
    BSII(const Base &base, const InfInt &value) : b(base), ii(value) {}
    BSII(const std::string &value) : s(value) { b = Base::Unknown; }
    BSII() { b = Base::Unknown; ii = 0; s = ""; }
};

enum class CellId { ga, bu, zo, meu, last_item };
const std::string CellIds[(size_t) (CellId::last_item)] = { "GA", "BU", "ZO", "MEU" };
byte GetCellId(const std::string& value); // Return type could be size_t as well

InfInt pow(const InfInt &base, const InfInt &expo); // This one is clearly missing in InfInt lib (bit operators as well :(
/*
bool IsPrime(const InfInt& value);
std::string PowerOfPrime(const InfInt& value);
*/

// Let's extend the nibble definition to a number in any known (except 256 which is the default one) base surrounded by #
// Keep in mind that four base 4 numbers the alphabet is GA BU ZO & MEU
// The length of the number is not limited but the "big" parameter act as a threhold for the value check if it fits a byte or not
std::string NumberToNibble(const Base &base, const InfInt &n);
InfInt NibbleToNumber(const Base &base, const std::string &s); // TODO: replace size_t usage by Base enum ?
std::vector<BSII> CompositeStringToNumbers(const std::string &s);
std::string NumbersToCompositeString(const std::vector<BSII> &v);
std::vector<byte> NumberToByteStream(const InfInt &n);
std::vector<byte> NumbersToByteStream(const std::vector<BSII> &v, bool internal); // The internal flags tells that we are called from GBZM
InfInt ByteStreamToNumber(const std::vector<byte> &v);
//TODO: Needed ?
//... ByteStreamToNumbers(...)

// Special version called only from the source code analysis (Gabuzomeu knows only the #base4 internally)
InfInt LexerNibbleToNumber(const std::string& s);

#endif // CONVERTER_TOOLS
