
#include "ConverterTools.hpp"

bool big = false;

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
std::string NumberToNibble(const InfInt &n)
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
    return "#" + result; // + "#";
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

// Expect an upper cased string without the leading #
// Raise an exception in case of conversion failure
// Called either from the input string parsing either from the interpreter
//TODO: [future] Make use the CellIds array ?
InfInt NibbleToNumber(const std::string& s)
{
    //std::cout << "NibbleToNumber input = " << s << std::endl;
    if (s.size() < 2)
    {
        throw InvalidNumberException(s); // (value);
    }

    /*
    std::cout << "pow(InfInt(0), InfInt(3) = " << pow(InfInt(0), InfInt(3)) << std::endl;
    std::cout << "pow(InfInt(4), InfInt(0) = " << pow(InfInt(4), InfInt(0)) << std::endl;
    std::cout << "pow(InfInt(4), InfInt(1) = " << pow(InfInt(4), InfInt(1)) << std::endl;
    std::cout << "pow(InfInt(4), InfInt(2) = " << pow(InfInt(4), InfInt(2)) << std::endl;
    std::cout << "pow(InfInt(4), InfInt(3) = " << pow(InfInt(4), InfInt(3)) << std::endl;
    return 0;
    */

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

    //std::cout << "big = " << big << std::endl;
    if ((result > 255) and !big)
    {
        //std::cout << "throw" << std::endl;
        //throw 0;
        throw OverflowException(s);
    }

    return result;
}

// Care : we expect numbers here surrounded by # Since the input data can be anything
std::vector<InfInt> CompositeStringToNumbers(const std::string& s)
{
    std::vector<InfInt> result;
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
                result.push_back(NibbleToNumber(tmp)); // emplace_back(StringToNibble(tmp));
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
                result.push_back((InfInt)s[pos]); // emplace_back((byte) s[pos]);                
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

    return result;
}

std::string NumbersToCompositeString(const std::vector<InfInt> &v)
{
    std::string result;

    //std::cout << "NumbersToCompositeString ==> initial result = " << result << std::endl;

    for (const auto &it : v)
    {
        //std::cout << "NumbersToCompositeString it = " << it << std::endl;
        if (it < 32) // ASCII limitation (avoid non printable characters)
        {
            //std::cout << " < 32 " << std::endl;
            result += NumberToNibble((byte) it.toInt());
        }
        else if (it > 255)
        {
            if (big)
            {                
                //std::cout << " big " << std::endl;
                std::vector<byte> bytes = NumberToByteStream(it);
                for (const auto &it2 : bytes)
                {
                    if (it2 < 32)
                    {
                        //std::cout << " < 32 (2)" << std::endl;
                        result += NumberToNibble(it2);
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
                throw InvalidNumberException("NumbersToCompositeString");
            }
        }
        else
        {
            //std::cout << " std " << std::endl;
            result += (char) it.toInt();
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

std::vector<byte> NumbersToByteStream(const std::vector<InfInt>& v)
{
    std::vector<byte> result;
    std::vector<byte> tmp;
    //std::cout << "NumbersToByteStream v.size() = " << v.size() << std::endl;
    for (const auto &it : v)
    {
        //std::cout << "NumbersToByteStream it = " << it << std::endl;
        tmp = NumberToByteStream(it); // This is not a recursion :)
        result.insert(std::end(result), std::begin(tmp), std::end(tmp));
    }
    return result;
}

InfInt ByteStreamToNumber(const std::vector<byte>& v)
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
