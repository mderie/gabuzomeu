
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <map>

#include "bnflite.h"

typedef unsigned char byte;

// We need two passes even though the language is interpreted (this is due
// to the potential forward jumps to code region not yet spotted.
bool firstPass = true;
int lastCell = -1;
int instructionCounter = 0;
int instructionPointer = 0;
std::map<std::string, int> labels;
std::vector<std::string> instructions;

enum class CellKind { Body, Head, Tail }; // the Body is optional :)
enum class CellId { ga, bu, zo, meu };

// Base class ! All of them are... interpret time technically speaking :)
class RuntimeException
{};

//TODO: ...
class InvalidNumberException : public RuntimeException
{
public:
    InvalidNumberException(const std::string value) {}
};

class InvalidCellKindException : public RuntimeException
{};

class LabelNotFoundException : public RuntimeException
{};

class NoMoreInputException : public RuntimeException
{};

class AlienException : public RuntimeException
{};

// Expect an upper cased string without the leading #
// Raise an exception in case of conversion failure
byte EvalBaseFourNumber(const std::string& value)
{
    if (value.size() < 2)
    {
        throw InvalidNumberException(value);
    }

    int power = 0;
    int result = 0;
    size_t pos = value.size();

    while (pos > 1)
    {
        if (value.substr(pos - 2, 2) == "GA")
        {
            // result += 0 * pow(4, power);
            pos -= 2;
        }
        else if (value.substr(pos - 2, 2) == "BU")
        {
            //result += 1 * pow(4, power);
            result++;
            pos -= 2;
        }
        else if (value.substr(pos - 2, 2) == "ZO")
        {
            result += 2 * ((int)pow(4, power));
            pos -= 2;
        }
        else if (value.substr(pos - 3, 3) == "MEU")
        {
            result += 3 * ((int)pow(4, power));
            pos -= 3;
        }
        else
        {
            throw InvalidNumberException(value);
        }
        power++;
    }

    if (pos == 1)
    {
        throw InvalidNumberException(value); // Never use new for exception object, catch with references... C# bad habit :(
    }

    return (byte)result;
}

byte EvalCellName(const std::string& value)
{
    if (value == "GA")
    {
        return (byte) CellId::ga;
    }
    else if (value == "BU")
    {
        return (byte) CellId::bu;
    }
    else if (value == "ZO")
    {
        return (byte) CellId::zo;
    }
    else if (value == "MEU")
    {
        return (byte) CellId::meu;
    }

    throw AlienException();
}

struct Cell
{
    CellKind kind;
    int content; //TODO: Use byte ? Anyway holds either the index of anoter bird or just a value
};

struct Bird
{
    //Cell ga, bu, zo, meu;
    Cell cells[4];

    Bird()
    {
        for (auto &i : cells)
        {
            i.kind = CellKind::Body;
            i.content = 0;
        }
    }
};

std::vector<Bird> birds;

void ShowUsage()
{
    std::cout << "Usage :" << std::endl;
    std::cout << "gabuzomeu program_file_name.gbzm \"optional_input_string\"" << std::endl;
    std::cout << "gabuzomeu \"program_source_code\" \"optional_input_string\"" << std::endl;
    exit(-1);
}

// Kind of visitors, called for each token

static bool DoLast(const char* lexem, size_t len)
{
    printf("Last = : %.*s;\n", len, lexem);
    return true;
}

static bool DoJump(const char* lexem, size_t len)
{
    printf("Jump = : %.*s;\n", len, lexem);
    return true;
}

static bool DoDump(const char* lexem, size_t len)
{
    printf("Dump = : %.*s;\n", len, lexem);
    return true;
}

static bool DoPump(const char* lexem, size_t len)
{
    printf("Pump = : %.*s;\n", len, lexem);
    return true;
}

static bool DoFree(const char* lexem, size_t len)
{
    printf("Free = : %.*s;\n", len, lexem);
    return true;
}

static bool DoBird(const char* lexem, size_t len)
{
    printf("Bird = : %.*s;\n", len, lexem);
    return true;
}

static bool DoMove(const char* lexem, size_t len)
{
    printf("Move = : %.*s;\n", len, lexem);
    return true;
}

static bool DoHead(const char* lexem, size_t len)
{
    printf("Head = : %.*s;\n", len, lexem);
    return true;
}

static bool DoTail(const char* lexem, size_t len)
{
    printf("Tail = : %.*s;\n", len, lexem);
    return true;
}

static bool DoZero(const char* lexem, size_t len)
{
    printf("Zero = : %.*s;\n", len, lexem);
    return true;
}

static bool DoElse(const char* lexem, size_t len)
{
    printf("Else = : %.*s;\n", len, lexem);
    return true;
}

static bool DoCalc(const char* lexem, size_t len)
{
    printf("Calc = : %.*s;\n", len, lexem);
    return true;
}

// Needed for instructions that have two operands
// We could also introduce DoEpression...
static bool DoCell(const char* lexem, size_t len)
{
    printf("Cell = : %.*s;\n", len, lexem);
    lastCell = EvalCellName(std::string(lexem, len));
    return true;
}

static bool DoInstruction(const char* lexem, size_t len)
{
    // This blocks the parser...
    // return false;

    if (!firstPass)
    {        
        return true; // Just continue diving !
    }

    //printf("Instruction = : %.*s;\n", len, lexem);
    instructions.emplace_back(std::string(lexem, len)); // Store it !
    return true;
}

// Called twice in a row each time... Why ?
static bool DoLabel(const char* lexem, size_t len)
{
    if (!firstPass) 
    {
        return true;
    }

    printf("Label = : %.*s;\n", len, lexem);
    std::string key = std::string(lexem, len);
        
    if (labels.find(key) == labels.end())
    {
        labels[key] = instructionCounter; // Store it !
    }
    else
    {
        std::cout << "Duplicate label : " << key << std::endl;
        return false;
    }

    return true;
}

static bool DoLine(const char* lexem, size_t len)
{
    //printf("Line = : %.*s;\n", len, lexem);
    return true;
}

static bool DoLineList(const char* lexem, size_t len)
{
    //printf("LineList = : %.*s;\n", len, lexem);
    return true;
}

static bool DoProgram(const char* lexem, size_t len)
{
    // For the .* inside the %s check the explanation here http://www.cplusplus.com/reference/cstdio/printf/
    //printf("Program = : %.*s;\n", len, lexem);
    return true;
}

void InitP1()
{
    firstPass = true;
    instructions.clear();
    labels.clear();
    instructionCounter = 0; //TODO: Or -1 ?
    instructionPointer = 0; // Idem
    lastCell = -1;
    birds.clear();
    birds.emplace_back(Bird());
}

void InitP2()
{
    firstPass = false;
    instructions.clear();
    labels.clear();
    instructionCounter = 0; //TODO: Or -1 ?
    instructionPointer = 0; // Idem
    lastCell = -1;
    birds.clear();
    birds.emplace_back(Bird());
}

std::string RunInterpreter(const std::vector<std::string>& lines, const std::string& input)
{
    std::string output;
    std::string line;

    // charset
    // Needed ?
    // bnf::Token value(1, 255);

    // Keywords (it seems that we can't hardcode them as string into the production rules :(
    bnf::Lexem l_last("LAST");
    bnf::Lexem l_jump("JUMP");
    bnf::Lexem l_dump("DUMP");
    bnf::Lexem l_pump("PUMP");
    bnf::Lexem l_free("FREE");
    bnf::Lexem l_bird("BIRD");
    bnf::Lexem l_move("MOVE");
    bnf::Lexem l_calc("CALC");
    bnf::Lexem l_head("HEAD");
    bnf::Lexem l_tail("TAIL");
    bnf::Lexem l_zero("ZERO");
    bnf::Lexem l_else("ELSE");

    // Tokens
    // It seems that the lib consider token as building blocks for lexemes... So character set operators
    bnf::Token t_alpha('A', 'Z');

    /*
    bnf::Token t_label = ""; //TODO: Add support for "_"
    t_label.Add('A', 'Z');
    bnf::Token t_litteral = "";
    t_litteral.Add("GA");
    t_litteral.Add("BU");
    t_litteral.Add("ZO");
    t_litteral.Add("MEU");
    */

    // Lexemes (always terminals !)
    // It seems that Lexeme are useless with this lib... Nope it is the opposite... It is the token that allow us to play at character level
    // Care : t_alpha+ is translated into 1 * t_alpha and t_alpha* is translated into * t_alpha (or bnf::Series(0, t_alpha);)
    bnf::Lexem l_label = 1 * t_alpha; // Or bnf::Series(1, t_alpha);
    //l_test = l_test + bnf::Lexem();

    // BUG HERE for the LEXEM !!! So don't use string litteral at all
    // See Lexem exp = "Ee" + !Token("+-") + I_DIGIT ;

    bnf::Lexem l_cell = bnf::Lexem("GA") | bnf::Lexem("BU") | bnf::Lexem("ZO") | bnf::Lexem("MEU");
    //bnf::Lexem l_litteral = 1 * ("#" + t_litteral);

    // Lexemes more like Rules
    bnf::Lexem l_litteral = "#" + 1 * l_cell;
    bnf::Lexem l_colon_label = ":" + l_label;

    // Rules
    // Base of production, one can associate / bind actions to them

    bnf::Rule r_last = (l_last + l_label) + DoLast;
    bnf::Rule r_jump = (l_jump + l_label) + DoJump;
    bnf::Rule r_dump = (l_dump + l_cell) + DoDump;
    bnf::Rule r_pump = (l_pump + l_cell) + DoPump;
    bnf::Rule r_free = (l_free + l_cell) + DoFree;
    bnf::Rule r_bird = (l_bird + l_cell) + DoBird;
    bnf::Rule r_move = (l_move + l_cell) + DoMove;
    bnf::Rule r_head = (l_head + l_cell + DoCell + "," + l_label) + DoHead;
    bnf::Rule r_tail = (l_tail + l_cell + DoCell + "," + l_label) + DoTail;
    bnf::Rule r_zero = (l_zero + l_cell + DoCell + "," + l_label) + DoZero;
    bnf::Rule r_else = (l_else + l_cell + DoCell + "," + l_label) + DoElse;

    bnf::Rule r_expression; // Must be "alone"... Recursivity issue
    bnf::Rule r_calc = (l_calc + l_cell + DoCell + "," + r_expression) + DoCalc;
    bnf::Rule r_factor = l_litteral | l_cell | "(" + r_expression + ")";
    bnf::Rule r_component = r_factor + *("*" | "/" + r_factor);
    r_expression = r_component + *("+" | "-" + r_component); // Recursion !

    bnf::Rule r_instruction = (r_last | r_jump | r_dump | r_pump | r_free | r_bird | r_move | r_calc | r_head | r_tail | r_zero | r_else) + DoInstruction;
    bnf::Rule r_line = (l_colon_label + DoLabel | r_instruction) + DoLine;

    // This was ok but raises duplicates !
    //bnf::Rule r_line_list; // Idem
    //r_line_list = (r_line | r_line + r_line_list) + DoLineList; // Care...

    bnf::Rule r_line_list = (* r_line) + DoLineList;
    bnf::Rule r_program = r_line_list + DoProgram;

    /*
    for (auto &it : lines)
    {
        transform(it.begin(), it.end(), line.begin(), std::toupper); // Upper case once for all :)

        pos = line.find("//"); //TODO: Or simply ";" ?
        if (pos != std::string::npos)
        {
            line = line.substr(0, pos);
        }

        if (line.size() == 0)
        {
            continue;
        }

        //TODO: ...
        //std::stringstream tokens =
        //for ( : tokens)

    }
    */

    const char* tail = 0; // Must be read in reverse : a pointer to a const char
    const char justHelloWorld[] = "CALC GA, #BUZOZOGA DUMP GA CALC BU, #BUZOBUBU DUMP BU CALC ZO, #BUZOMEUGA DUMP ZO DUMP ZO CALC MEU, #BUZOMEUMEU DUMP MEU CALC GA, #ZOGAGA DUMP GA CALC BU, #BUMEUBUMEU DUMP BU DUMP MEU CALC BU, #BUMEUGAZO DUMP BU DUMP ZO CALC GA, #BUZOBUGA DUMP GA";
    //const char helloWorld[] = "CALC GA, #BUZOZOGA\nDUMP GA\nCALC BU, #BUZOBUBU\nDUMP BU\nCALC ZO, #BUZOMEUGA\nDUMP ZO\nDUMP ZO\nCALC MEU, #BUZOMEUMEU\nDUMP MEU\nCALC GA, #ZOGAGA\nDUMP GA\nCALC BU, #BUMEUBUMEU\nDUMP BU\nDUMP MEU\nCALC BU, #BUMEUGAZO\nDUMP BU\nDUMP ZO\nCALC GA, #BUZOBUGA\nDUMP GA";
    //const char helloWorld[] = "CALCGA,#BUZOZOGADUMPGACALCBU,#BUZOBUBUDUMPBUCALCZO,#BUZOMEUGADUMPZODUMPZOCALCMEU,#BUZOMEUMEUDUMPMEUCALCGA,#ZOGAGADUMPGACALCBU,#BUMEUBUMEUDUMPBUDUMPMEUCALCBU,#BUMEUGAZODUMPBUDUMPZOCALCGA,#BUZOBUGADUMPGA";
    const char justHelloZob[] = "CALC GA, #BUZOZOGA DUMP GA CALC BU, #BUZOBUBU DUMP BU CALC ZOB"; // KO so OK :)
    const char justCalc[] = "CALC GA, #BUZOZOGA"; // OK
    const char justMove[] = "MOVEGA"; // OK
    const char justJump[] = "JUMPTOTO"; // OK
    const char justDoubleJump[] = "JUMPTOTOJUMPTITI"; // OK
    const char justBirdWithSpace[] = "BIRD GA"; // OK
    const char justDoubleMove[] = "MOVEGAMOVEBU"; // OK
    const char justBogusMove[] = "MOVEGO"; // KO so OK :)
    const char justDoubleMoveWithSpaces[] = "MOVE GA MOVE MEU"; // OK
    const char justNothing[] = ""; // OK
    const char justLabel[] = ":TOTO"; // OK
    const char justCell[] = "GA"; // KO so OK :)
    const char justGarbage[] = "TGA"; // KO so OK :)
    const char justSomeLabels[] = ":TOTO :TITI MOVE GA :AHU"; // OK
    const char justSameLabels[] = ":TOTO :TITI MOVE GA :TOTO"; // KO so OK :)

    InitP1();

    int tst = bnf::Analyze(r_program, justHelloWorld, &tail);
    if (tst > 0)
    {
        std::cout << "OK" << std::endl;
    }
    else
    {
        printf("Failed, stopped at=%.40s\n status = 0x%0X,  flg = %s%s%s%s%s%s%s%s\n",
            tail ? tail : "", tst,
            tst & bnf::eOk ? "eOk" : "Not",
            tst & bnf::eRest ? ", eRest" : "",
            tst & bnf::eOver ? ", eOver" : "",
            tst & bnf::eEof ? ", eEof" : "",
            tst & bnf::eBadRule ? ", eBadRule" : "",
            tst & bnf::eBadLexem ? ", eBadLexem" : "",
            tst & bnf::eSyntax ? ", eSyntax" : "",
            tst & bnf::eError ? ", eError" : "");

        exit(0);
    }

    std::cout << "instructions.size() = " << instructions.size() << " & labels.size() = " << labels.size() << std::endl;

    tail = nullptr; //TODO: Put this in init ?
    InitP2();
    //tst = bnf::Analyze(r_program, justHelloWorld, &tail);

    // Disjoin Rule recursion to safe rule removal
    r_expression = bnf::Null();
    //r_line_list = bnf::Null();

    exit(0); //TODO: Remove this when done

    return output;
}

int main(int argc, char *argv[])
{
    //TODO: Remove when done
    std::vector<std::string> dummies;
    std::string dummy;
    RunInterpreter(dummies, dummy);

    if (argc != 3)
    {
        ShowUsage();
    }

    std::string input = std::string(argv[2]);
    if ((input.size() > 1) and (input.front() == '"') and (input.back() == '"'))
    {
        input = input.substr(1, input.size() - 2);
    }
    else
    {
        ShowUsage();
    }

    std::string program = std::string(argv[1]);
    std::vector<std::string> lines;
    if ((program.size() > 1) and (program.front() == '"') and (program.back() == '"'))
    {
        lines.emplace_back(program.substr(1, program.size() - 2));
    }
    else
    {
        try
        {
            std::ifstream file(program);
            std::string s;
            while (std::getline(file, s))
            {
                lines.emplace_back(s);
            }
        }
        catch (...)
        {
            ShowUsage();
        }
    }

    std::cout << RunInterpreter(lines, input) << std::endl;
}
