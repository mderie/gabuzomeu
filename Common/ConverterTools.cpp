
#include "ConverterTools.hpp"
#include "RuntimeExceptions.hpp"

bool big = false; // Big number
bool Quine = false; // No trailing # at for the nibble
bool write = false; // Force printing the non printable character (ASCII code < 32)

// Use a special version of Base64 ? https://en.wikipedia.org/wiki/Base64 A-Z, a-z, 0-9, + /
// Where we replace '+' & '/' by ':' & ',' could help us achieving Quine exploration !

// This should be a template :)
// No generic mapping from string "enum" array and enum value :(
byte GetCellId(const std::string& value)
{
    const auto &it = std::find(std::begin(CellIds), std::end(CellIds), value);
    if (it == std::end(CellIds))
    {
        throw AlienException(value); // Since the lexical analysis should have done its job
    }
    return (byte) (it - std::begin(CellIds));
}

// Not an half byte here but a base 4 number in GABUZOMEU style, so surrounded by # :)
std::string NumberToNibble4(const InfInt &n)
{
    std::string result;
    size_t index;
    InfInt shift = n;
    while (shift > 0)
    {
        index = (shift % 4).toInt(); // Get the two rightmost bits
        //std::cout << "shift = " << shift.toString() << " & index = " << index << std::endl;
        result = CellIds[index] + result;
        shift /= 4; // No bit operator support :(
    }
    return result;
}

std::string NumberToNibble16(const InfInt &n)
{
    std::string result;
    size_t index;
    InfInt shift = n;
    while (shift > 0)
    {
        index = (shift % 16).toInt();
        if (index < 10)
        {
            result.insert(0, 1, ('0' + (char) index));
        }
        else
        {
            result.insert(0, 1, ('A' + (char) (index - 10)));
        }
        shift /= 16;
    }
    return result;
}

std::string NumberToNibble64(const InfInt &n)
{
    std::string result;
    size_t index;
    InfInt shift = n;
    while (shift > 0)
    {
        index = (shift % 64).toInt();
        //std::cout << "index = " << index << " & shift = " << shift << std::endl;
        if (index < 26)
        {
            result.insert(0, 1, ('A' + (char) index));
        }
        else if (index < 52)
        {
            result.insert(0, 1, ('a' + (char) (index - 26)));
        }
        else if (index < 62)
        {
            result.insert(0, 1, ('0' + (char) (index - 52)));
        }
        else if (index == 62)
        {
            result.insert(0, 1, '+');
        }
        else if (index == 63)
        {
            result.insert(0, 1, '/');
        }
        shift /= 64;
    }
    return result;
}

std::string NumberToNibble(const Base &base, const InfInt &n)
{
    if (base == Base::default_)
    {
        throw BaseNotSupported(std::to_string((byte)Base::default_));
    }

    std::string result;

    switch (base)
    {
    case Base::Four: { result = NumberToNibble4(n); break; }
    case Base::SixTeen: { result = NumberToNibble16(n); break; }
    case Base::SixtyFour: { result = NumberToNibble64(n); break; }
    default: // Trad. algo for 2, 10 bases (no translation needed for 256 :)
    {
        size_t index;
        InfInt shift = n;
        while (shift > 0)
        {
            index = (shift % (byte) base).toInt();
            result = std::to_string(index) + result;
            shift = shift / (byte) base;
        }
    }
    } // switch

    return "#" + result + (Quine ? "" : "#");
}

// We assume here that value is > 0
InfInt pow(const InfInt &base, const InfInt &expo)
{
    if ((base == 0) and (expo == 0))
    {
        throw InvalidNumberException("NAN");
    }
    if (base == 0)
    {
        return 0;
    }
    if (expo == 0)
    {
        return 1;
    }

    InfInt result = base;
    for (InfInt i = 2; i <= expo; i += 1) // Some operators are missing... IE: pow !-)
    {
        result *= base;
    }
    return result;
}

/*
bool IsPrime(const InfInt& value)
{
    bool result = false;

    return result;
}

std::string PowerOfPrime(const InfInt &value)
{
    std::string result;

    return result;
}
*/

// Expect an upper cased string without any #
// Raise an exception in case of conversion failure
// Called either from the input string parsing either indirectly from the interpreter
//TODO: [future] Make use the CellIds array ?
InfInt Nibble4ToNumber(const std::string &s)
{
    if (s.size() < 2)
    {
        throw InvalidNumberException(s); // (value);
    }

    InfInt power = 0;
    InfInt result = 0;
    size_t pos = s.size();

    while (pos > 1)
    {
        if (s.substr(pos - 2, 2) == "GA")
        {
            // result += 0 * ((int) pow(4, power));
            pos -= 2;
        }
        else if (s.substr(pos - 2, 2) == "BU")
        {
            result += InfInt(1) * pow(4, power);
            pos -= 2;
        }
        else if (s.substr(pos - 2, 2) == "ZO")
        {
            result += InfInt(2) * pow(4, power);
            pos -= 2;
        }
        else if (s.substr(pos - 3, 3) == "MEU")
        {
            result += InfInt(3) * pow(4, power);
            pos -= 3;
        }
        else
        {
            throw InvalidNumberException(s);
        }
        power += 1;
        //std::cout << "result = " << result.toString() << std::endl;
    }

    if (pos == 1)
    {
        // Never use new for exception object, catch with references... C# bad habit :(
        throw InvalidNumberException(s);
    }

    return result;
}

// Three Specialisations since those bases use letters
InfInt Nibble64ToNumber(const std::string &s)
{
    InfInt result = 0;
    InfInt power = 0;

    for (size_t i = s.size() - 1; i >= 0; i--)
    {
        if ((s[i] >= 'A') and (s[i] <= 'Z'))
        {
            result += InfInt((s[i] - 'A')) * pow(64, power);
        }
        else if ((s[i] >= 'a') and (s[i] <= 'z'))
        {
            result += InfInt((s[i] - 'a') + 26) * pow(64, power);
        }
        else if ((s[i] >= '0') and (s[i] <= '9'))
        {
            result += InfInt((s[i] - '0') + 52) * pow(64, power);
        }
        else if (s[i] == '+')
        {
            result += InfInt(62) * pow(64, power);
        }
        else if (s[i] == '/')
        {
            result += InfInt(63) * pow(64, power);
        }
        else
        {
            throw InvalidNumberException(s + " in base 64");
        }
        power += 1;
    }

    return result;
}

InfInt Nibble16ToNumber(const std::string &s)
{
    InfInt result = 0; // The = 0 is hopefuly optional !
    InfInt power = 0;

    for (size_t i = s.size() - 1; i >= 0; i--)
    {
        if ((s[i] >= '0') and (s[i] <= '9'))
        {
            result += InfInt((s[i] - '0')) * pow(InfInt(64), power);
        }
        else if ((s[i] >= 'A') and (s[i] <= 'Z'))
        {
            result += InfInt((s[i] - 'A') + 10) * pow(InfInt(64), power);
        }
        else
        {
            throw InvalidNumberException(s + " in base 16");
        }
        power += 1;
    }

    return result;
}

InfInt NibbleToNumber(const Base &base, const std::string &s)
{
    InfInt result;
    //std::cout << "NibbleToNumber ==> base = " << base << " & s = " << s << std::endl;
    switch (base)
    {
    case Base::Four: { result = Nibble4ToNumber(s); break; }
    case Base::SixTeen: { result = Nibble16ToNumber(s); break; }
    case Base::SixtyFour: { result = Nibble64ToNumber(s); break; }
    default: // Trad. algo for 2, 10 bases (no translation needed for 256 :)
    {
        InfInt power = 0;
        for (int i = s.size() - 1; i >= 0; i--)
        {
            //std::cout << "i = " << i << ", s[i] = " << s[i] << ", (((size_t) s[i]) - (size_t) '0') = " << (((size_t)s[i]) - (size_t)'0') << " & (size_t) base = " << (size_t)base << std::endl;
            if ((((size_t) s[i]) - (size_t) '0') < (size_t) base)
            {
                result += InfInt((((size_t)s[i]) - (size_t)'0')) * pow(InfInt((size_t) base), power);
            }
            else
            {
                throw InvalidNumberException(s + " in base " + std::to_string((size_t) base));
            }
            power += 1;
        }
    }
    } // switch (I hate switch syntax... The only switch I lile is the Nintendo one :)

    /*
    std::cout << "pow(InfInt(0), InfInt(3) = " << pow(InfInt(0), InfInt(3)) << std::endl;
    std::cout << "pow(InfInt(4), InfInt(0) = " << pow(InfInt(4), InfInt(0)) << std::endl;
    std::cout << "pow(InfInt(4), InfInt(1) = " << pow(InfInt(4), InfInt(1)) << std::endl;
    std::cout << "pow(InfInt(4), InfInt(2) = " << pow(InfInt(4), InfInt(2)) << std::endl;
    std::cout << "pow(InfInt(4), InfInt(3) = " << pow(InfInt(4), InfInt(3)) << std::endl;
    return 0;
    */

    //std::cout << "NibbleToNumber ==> result = " << result << " & big = " << big << std::endl;
    if ((result > 255) and !big)
    {
        //std::cout << "throw" << std::endl;
        //throw 0;
        throw InvalidNumberException("Number is too big, consider using the -b | --big parameter");
    }

    return result;
}

// Care : we expect numbers here surrounded by # Since the input data can be anything
// We don't know the base yet (so we use 256 by default) nor the number length
std::vector<BSII> CompositeStringToNumbers(const std::string &s)
{
    std::vector<BSII> result;
    if (s == "")
    {
        return result;
    }

    std::string tmp;
    int state = 0; // (in string or in number)
    size_t pos = 0;
    size_t len = s.size();
    //std::cout << "pos = " << pos << " , len = " << len << " & s = " << s << std::endl;

    while (pos < len)
    {
        //std::cout << "pos = " << pos << " & s[pos] = " << s[pos] << std::endl;
        switch (s[pos])
        {
        case '#':
        {
            if (state == 0)
            {
                state = 1;
                //std::cout << "state = 1" << std::endl;
            }
            else // then (state == 1)
            {
                //std::cout << "tmp0 = " << tmp << std::endl;
                result.emplace_back(BSII(tmp)); // emplace_back(StringToNibble(tmp));
                tmp = "";
                state = 0;
            }

            break;
        }

        default:
        {
            if (state == 0)
            {
                //std::cout << "s[pos] = " << s[pos] << std::endl;
                result.push_back(BSII(Base::default_, (InfInt) s[pos])); // emplace_back((byte) s[pos]);
            }
            else // Then (state == 1)
            {
                tmp += s[pos];
                //std::cout << "tmp1 = " << tmp << std::endl;
            }
        }
        } // switch

        pos++;
    } // while

    if (result.size() == 0)
    {
        throw InvalidInputException("");
    }
    return result;
}

std::string NumbersToCompositeString(const std::vector<BSII> &v)
{
    std::string result;

    //std::cout << "NumbersToCompositeString ==> initial result = " << result << " & v.size() = " << v.size() << std::endl;

    for (const auto &it : v)
    {
        if ((it.ii > 255) and !big)
        {
            throw InvalidNumberException("Number is too big, consider using the -b | --big parameter");
        }

        if ((Base) it.b == Base::default_)
        {
            //std::cout << "NumbersToCompositeString it = " << it.ii << std::endl;
            if ((it.ii < 32) and (!write)) // ASCII limitation (avoid non printable characters)
            {
                //std::cout << " < 32 " << std::endl;
                result += NumberToNibble(Base::Four, (byte) it.ii.toInt());
            }
            else if (it.ii > 255)
            {
                //std::cout << " big " << std::endl;
                std::vector<byte> bytes = NumberToByteStream(it.ii);
                for (const auto& it2 : bytes)
                {
                    if ((it2 < 32) and (!write))
                    {
                        //std::cout << " < 32 (2)" << std::endl;
                        result += NumberToNibble(Base::Four, it2);
                    }
                    else
                    {
                        //std::cout << " std (2) " << std::endl;
                        result += (char) it2;
                    }
                }
            }
            else
            {
                //std::cout << " std " << std::endl;
                result += (char) it.ii.toInt();
            }
        }
        else
        {
            result += NumberToNibble(it.b, it.ii);
        }
        //std::cout << "NumbersToCompositeString loop result = " << result << std::endl;
    }
    
    //std::cout << "NumbersToCompositeString ==> final result = " << result << std::endl;
    return result;
}

//TODO: [future] Can be shorter with do ... while !
std::vector<byte> NumberToByteStream(const InfInt &n)
{
    std::vector<byte> result;
    byte mod = (byte) (n % 256).toInt();
    //std::cout << "n = " << n << " & mod =  " << (int) mod << std::endl;
    InfInt div = n / 256;
    while (div > 0)
    {
        //std::cout << "div =  " << div.toString() << std::endl;
        result.emplace(result.begin(), mod);
        mod = (byte) (div % 256).toInt();
        //std::cout << "mod = " << (int) mod << std::endl;
        div = div / 256;
    }
    result.emplace(result.begin(), mod);
    return result;
}

std::vector<byte> NumbersToByteStream(const std::vector<BSII> &v, bool internal)
{
    std::vector<byte> result;
    std::vector<byte> tmp;
    std::string s;
    //std::cout << "NumbersToByteStream v.size() = " << v.size() << std::endl;
    for (const auto &it : v)
    {
        if ((it.b == Base::default_) or (internal))
        {
            //std::cout << "NumbersToByteStream it = " << it << std::endl;
            tmp = NumberToByteStream(it.ii); // This is not a recursion :)
            result.insert(std::end(result), std::begin(tmp), std::end(tmp));
        }
        else
        {
            s = NumberToNibble(it.b, it.ii);
            result.insert(std::end(result), std::begin(s), std::end(s)); // BUG HERE !!!
        }
    }
    return result;
}

InfInt ByteStreamToNumber(const std::vector<byte> &v)
{
    if (v.size() == 0)
    {
        throw EmptyContainer("ByteStreamToNumber");
    }

    InfInt result = 0;
    InfInt power = v.size() - 1;
    for (const auto& it : v)
    {
        //std::cout << "it = " << (int) it << " & power = " << power << std::endl;
        result += InfInt(it) * pow(256, power--);
        //std::cout << "result = " << result << std::endl;
    }

    return result;
}

InfInt LexerNibbleToNumber(const std::string& s)
{
    return NibbleToNumber(Base::Four, s); // +"#");
}
