
#include "ConverterTools.hpp"

bool big = false;

// Not an half byte here but a base 4 number in GABUZOMEU style, so surrounded by # :)
std::string NumberToNibble(const InfInt &n)
{
    std::string result;
    size_t index;
    InfInt shift = n;
    while (shift > 0)
    {
        index = shift.toInt() & 3; // Get the two rightmost bits
        //std::cout << "value = " << (int)value << " & index = " << index << std::endl;
        result = CellIds[index] + result;
        shift /= 4; // No bit operator support
    }
    return "#" + result + "#";
}

// We assume here that value is > 0
InfInt pow(const InfInt &base, const InfInt &expo)
{
    if ((base == 0) and (expo == 0))
    {
        throw InvalidNumberException(); // NAN
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

// Expect an upper cased string without the leading #
// Raise an exception in case of conversion failure
// Called either from the input string parsing either from the interpreter
//TODO: [future] Make use the CellIds array ?
InfInt NibbleToNumber(const std::string& s)
{
    if (s.size() < 2)
    {
        throw InvalidNumberException(); // (value);
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
            throw InvalidNumberException(); //(value);
        }
        power += 1;
        //std::cout << "result = " << result.toString() << std::endl;
    }

    if (pos == 1)
    {
        // Never use new for exception object, catch with references... C# bad habit :(
        throw InvalidNumberException(); // (value); 
    }

    if (result > 255 & !big)
    {
        throw InvalidNumberException(); // (value);
    }

    return result;
}

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
    //std::cout << "v.size() = " << v.size() << " & v[0].toString() = " << v[0].toString() << std::endl;

    std::string result;
    for (const auto &it : v)
    {
        if (it < 32) // ASCII limitation (avoid non printable characters)
        {
            result += NumberToNibble((byte) it.toInt());
        }
        else if (it > 255)
        {
            if (big)
            {
                //std::cout << "NumbersToCompositeString" << std::endl;
                std::vector<byte> bytes = NumberToByteStream(it);
                for (const auto &it2 : bytes)
                {
                    if (it2 < 32)
                    {
                        result += NumberToNibble(it2);
                    }
                    else
                    {
                        result += (char) it2;
                    }
                }
            }
            else
            {
                throw InvalidNumberException(); // (it);
            }
        }
        else
        {
            result += (char) it.toInt();
        }
    }

    return result;
}

std::vector<byte> NumberToByteStream(const InfInt &n)
{
    std::vector<byte> result;
    byte mod = (byte) (n % 256).toInt();
    //std::cout << "mod =  " << (int) mod << std::endl;
    InfInt div = n / 256;
    while (div > 0)
    {
        //std::cout << "div =  " << div.toString() << std::endl;
        result.emplace(result.begin(), mod);
        mod = (byte) (div % 256).toInt();
        div = div / 256;
    }
    result.emplace(result.begin(), mod);
    return result;
}

std::vector<byte> NumbersToByteStream(const std::vector<InfInt>& v)
{
    std::vector<byte> result;
    std::vector<byte> tmp;
    for (const auto &it : v)
    {
        tmp = NumberToByteStream(it); // This is not a recursion :)
        result.insert(std::end(result), std::begin(tmp), std::end(tmp));
    }
    return result;
}
