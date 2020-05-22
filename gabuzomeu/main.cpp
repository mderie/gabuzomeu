
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <map>

#include "bnflite.h"

typedef unsigned char byte; //TODO: Get rid of this ?

//////////////////////////////////////////////////////////////////////////
// Global variables part one (TODO: put all this into a class, one day ?-)
//////////////////////////////////////////////////////////////////////////

const char* tailexpr = nullptr; // Remember, it must be read in reverse : a pointer to a const char
bool numericOutput = false;
// We need two passes even though the language is interpreted (this is due
// to the potential forward jumps to code region not yet spotted.
// Actually the second pass is used only for the expression evaluation
bool firstPass = true;
bool performJump = false;
bool performSub = false;
bool performDiv = false;
//int lastCell = -1;
std::string lastCell;
std::string gotoLabel;
std::vector<byte> input;
std::vector<byte> output;
int charPointer = 0;

int calcResult = 0;
int exprResult = 1;
//BUG : ISSUE WITH RECURSION EVALUATION ==> CHECK GOOGLE
int compResult = 0;
//int compResult1 = 1;
//int factResult0 = 0;
//int factResult1 = 0;
//int instructionCounter = 0;
int instructionPointer = 0; // AKA IP
int birdPointer = 0; // AKA current bird index in Birds (AKA BP)
std::map<std::string, int> labels;

// Unable to get the number of elementsinside an enum
// https://stackoverflow.com/questions/712463/number-of-elements-in-an-enum
enum class OpCode { last, jump, pump, dump, free, bird, move, calc, head, tail, zero, else_, last_item }; // Care of keyword !
const std::string OpCodes[(int) (OpCode::last_item)] = { "LAST", "JUMP", "PUMP", "DUMP", "FREE", "BIRD", "MOVE", "CALC", "HEAD", "TAIL", "ZERO", "ELSE" };

struct Instruction
{
    OpCode opCode;
    std::string operand1;
    std::string operand2;
    Instruction(OpCode opc, const std::string &op1, const std::string &op2) : opCode(opc), operand1(op1), operand2(op2) {}
};

std::vector<Instruction> instructions;

enum class CellKind { Body, Head, Tail }; // the Body is optional :)
enum class CellId { ga, bu, zo, meu, last_item };
const std::string Cells[(int) (CellId::last_item)] = { "GA", "BU", "ZO", "MEU" };

/////////////
// Exceptions
/////////////

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

/////////////
// Converters
/////////////

// Expect an upper cased string without the leading #
// Raise an exception in case of conversion failure
// Called either from the input string parsing either from the interpreter
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
            result += 2 * ((int) pow(4, power));
            pos -= 2;
        }
        else if (value.substr(pos - 3, 3) == "MEU")
        {
            result += 3 * ((int) pow(4, power));
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

    throw AlienException(); // Since the lexical analysis should have done its job
}

struct Cell
{
    CellKind kind;
    int content; //TODO: Use byte ? Anyway holds either the index of anoter bird or just a value (so rename it value ?-)
};

struct Bird
{
    //Cell ga, bu, zo, meu;
    Cell cells[(int) CellId::last_item];

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

//////////////////////////////////////////
// Kind of visitors, called for each token
//////////////////////////////////////////

static bool OnLast(const char *lexem, size_t len)
{
    printf("Last = : %.*s;\n", len, lexem);
    instructions.emplace_back(Instruction(OpCode::last, std::string(lexem, len), ""));
    return true;
}

static bool OnJump(const char *lexem, size_t len)
{
    printf("Jump = : %.*s;\n", len, lexem);
    instructions.emplace_back(Instruction(OpCode::jump, std::string(lexem, len), ""));
    return true;
}

static bool OnDump(const char *lexem, size_t len)
{
    printf("Dump = : %.*s;\n", len, lexem);
    instructions.emplace_back(Instruction(OpCode::dump, std::string(lexem, len), ""));
    return true;
}

static bool OnPump(const char *lexem, size_t len)
{
    printf("Pump = : %.*s;\n", len, lexem);
    instructions.emplace_back(Instruction(OpCode::pump, std::string(lexem, len), ""));
    return true;
}

static bool OnFree(const char *lexem, size_t len)
{
    printf("Free = : %.*s;\n", len, lexem);
    instructions.emplace_back(Instruction(OpCode::free, std::string(lexem, len), ""));
    return true;
}

static bool OnBird(const char *lexem, size_t len)
{
    printf("Bird = : %.*s;\n", len, lexem);
    instructions.emplace_back(Instruction(OpCode::bird, std::string(lexem, len), ""));
    return true;
}

static bool OnMove(const char *lexem, size_t len)
{
    printf("Move = : %.*s;\n", len, lexem);
    instructions.emplace_back(Instruction(OpCode::move, std::string(lexem, len), ""));
    return true;
}

static bool OnHead(const char *lexem, size_t len)
{
    printf("Head = : %.*s;\n", len, lexem);
    instructions.emplace_back(Instruction(OpCode::head, lastCell, std::string(lexem, len)));
    return true;
}

static bool OnTail(const char *lexem, size_t len)
{
    printf("Tail = : %.*s;\n", len, lexem);
    instructions.emplace_back(Instruction(OpCode::tail, lastCell, std::string(lexem, len)));
    return true;
}

static bool OnZero(const char *lexem, size_t len)
{
    printf("Zero = : %.*s;\n", len, lexem);
    instructions.emplace_back(Instruction(OpCode::zero, lastCell, std::string(lexem, len)));
    return true;
}

static bool OnElse(const char *lexem, size_t len)
{
    printf("Else = : %.*s;\n", len, lexem);
    instructions.emplace_back(Instruction(OpCode::else_, lastCell, std::string(lexem, len)));
    return true;
}

static bool OnAddSub(const char *lexem, size_t len)
{
    printf("AddSub = : %.*s;\n", len, lexem);
    performSub = (*lexem == '-');
    return true;
}

static bool OnMulDiv(const char *lexem, size_t len)
{
    printf("MulDiv = : %.*s;\n", len, lexem);
    performDiv = (*lexem == '/');
    return true;
}

static bool OnCalc(const char *lexem, size_t len)
{
    printf("Calc = : %.*s;\n", len, lexem);
    if (firstPass)
    {
        instructions.emplace_back(Instruction(OpCode::calc, lastCell, std::string(lexem, len)));
    }
    else
    {
        //TODO: ?
    }
    return true;
}

static bool OnExpression(const char* lexem, size_t len)
{
    printf("Expression = : %.*s;\n", len, lexem);

    if (!firstPass)
    {
        if (performSub)
        {
            calcResult -= exprResult;
        }
        else
        {
            calcResult += exprResult;
        }
        std::cout << "calcResult = " << calcResult << std::endl;
    }

    return true;
}

static bool OnComponent(const char* lexem, size_t len)
{
    printf("Component = : %.*s;\n", len, lexem);

    if (!firstPass)
    {
        if (performDiv)
        {
            exprResult /= compResult;
        }
        else
        {
            exprResult *= compResult;
        }
        std::cout << "exprResult = " << exprResult << std::endl;
    }

    return true;
}

static bool OnFactor(const char* lexem, size_t len)
{
    printf("Factor = : %.*s;\n", len, lexem);

    if (!firstPass)
    {
        if (*lexem == '#')
        {
            compResult = EvalBaseFourNumber(std::string(lexem + 1, len - 1));
        }
        else
        {
            byte cellId = EvalCellName(instructions[instructionPointer].operand1);
            if ((birds[birdPointer].cells[cellId].kind == CellKind::Head) or (birds[birdPointer].cells[cellId].kind == CellKind::Tail))
            {
                throw InvalidCellKindException();
            }

            compResult = birds[birdPointer].cells[cellId].content;
        }
        std::cout << "compResult = " << compResult << std::endl;
    }

    return true;
}

// Needed for instructions that have two operands
// We could also introduce DoEpression...
static bool OnCell(const char* lexem, size_t len)
{
    printf("Cell = : %.*s;\n", len, lexem);
    //lastCell = EvalCellName(std::string(lexem, len));
    lastCell = std::string(lexem, len);
    return true;
}

static bool OnInstruction(const char* lexem, size_t len)
{
    // This blocks the parser...
    // return false;

    if (!firstPass)
    {        
        return true; // Just continue diving !
    }

    //printf("Instruction = : %.*s;\n", len, lexem);
    //instructionCounter++;
    return true;
}

// Called twice in a row each time... Why ?
static bool OnLabel(const char* lexem, size_t len)
{
    if (!firstPass) 
    {
        return true;
    }

    printf("Label = : %.*s;\n", len, lexem);
    std::string key = std::string(lexem, len);
        
    if (labels.find(key) == labels.end())
    {
        labels[key] = instructions.size(); // instructionCounter; // Store it !
    }
    else
    {
        std::cout << "Duplicate label : " << key << std::endl;
        return false;
    }

    return true;
}

static bool OnLine(const char* lexem, size_t len)
{
    //printf("Line = : %.*s;\n", len, lexem);
    return true;
}

static bool OnLineList(const char* lexem, size_t len)
{
    //printf("LineList = : %.*s;\n", len, lexem);
    return true;
}

static bool OnProgram(const char* lexem, size_t len)
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
    //instructionCounter = 0; //TODO: Or -1 ?
    //instructionPointer = 0; // Idem
    //birdPointer = 0;
    //lastCell = -1;
    //birds.clear();
    //birds.emplace_back(Bird());
}

void InitP2()
{
    firstPass = false;
    //instructions.clear();
    //labels.clear();
    //instructionCounter = 0; //TODO: Or -1 ?
    instructionPointer = 0; // Idem
    birdPointer = 0;
    lastCell = -1;
    birds.clear();
    birds.emplace_back(Bird());
}

//////////////////////////////////////
// Global variables part two (miam !-)
//////////////////////////////////////

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

bnf::Lexem l_addsub = bnf::Lexem("+") | bnf::Lexem("-");
bnf::Lexem l_muldiv = bnf::Lexem("*") | bnf::Lexem("/");

// Lexemes more like Rules
bnf::Lexem l_litteral = "#" + 1 * l_cell;
bnf::Lexem l_colon_label = ":" + l_label;

// Rules
// Base of production, one can associate / bind actions to them

bnf::Rule r_last = (l_last + l_label) + OnLast;
bnf::Rule r_jump = (l_jump + l_label) + OnJump;
bnf::Rule r_dump = (l_dump + l_cell) + OnDump;
bnf::Rule r_pump = (l_pump + l_cell) + OnPump;
bnf::Rule r_free = (l_free + l_cell) + OnFree;
bnf::Rule r_bird = (l_bird + l_cell) + OnBird;
bnf::Rule r_move = (l_move + l_cell) + OnMove;
bnf::Rule r_head = (l_head + l_cell + OnCell + "," + l_label) + OnHead;
bnf::Rule r_tail = (l_tail + l_cell + OnCell + "," + l_label) + OnTail;
bnf::Rule r_zero = (l_zero + l_cell + OnCell + "," + l_label) + OnZero;
bnf::Rule r_else = (l_else + l_cell + OnCell + "," + l_label) + OnElse;

bnf::Rule r_expression; // Must be "alone"... Recursivity issue
bnf::Rule r_calc = (l_calc + l_cell + OnCell + "," + r_expression) + OnCalc;
bnf::Rule r_factor = (l_litteral | l_cell | "(" + r_expression + ")") + OnFactor;
bnf::Rule r_component = (r_factor + *(l_muldiv + OnMulDiv + r_factor)) + OnComponent;

bnf::Rule r_instruction = (r_last | r_jump | r_dump | r_pump | r_free | r_bird | r_move | r_calc | r_head | r_tail | r_zero | r_else) + OnInstruction;
bnf::Rule r_line = (l_colon_label + OnLabel | r_instruction) + OnLine;

// This was ok but raises strange duplicate lines !
//bnf::Rule r_line_list; // Idem
//r_line_list = (r_line | r_line + r_line_list) + DoLineList; // Care...

bnf::Rule r_line_list = (*r_line) + OnLineList;
bnf::Rule r_program = r_line_list + OnProgram;

void DoLast()
{
    performJump = (charPointer >= input.size());
    gotoLabel = instructions[instructionPointer].operand1;
}

void DoJump()
{
    performJump = true;
    gotoLabel = instructions[instructionPointer].operand1;
}
void DoPump()
{
    byte cellId = EvalCellName(instructions[instructionPointer].operand1);
    if ((birds[birdPointer].cells[cellId].kind == CellKind::Head) or (birds[birdPointer].cells[cellId].kind == CellKind::Tail))
    {
        throw InvalidCellKindException();
    }

    if (charPointer >= input.size())
    {
        throw NoMoreInputException();
    }
    birds[birdPointer].cells[cellId].content = input[charPointer++];
}

void DoDump()
{
    byte cellId = EvalCellName(instructions[instructionPointer].operand1);
    if ((birds[birdPointer].cells[cellId].kind == CellKind::Head) or (birds[birdPointer].cells[cellId].kind == CellKind::Tail))
    {
        throw InvalidCellKindException();
    }

    output.emplace_back(birds[birdPointer].cells[cellId].content);
}

// Care : recursive ! Or not... We can leave everything as is and not recycle birds
// nor changing all the id's ! So some birds may be unreachable
void Free(int birdIndex)
{
}

void DoFree()
{
    byte cellId = EvalCellName(instructions[instructionPointer].operand1);
    if ((birds[birdPointer].cells[cellId].kind != CellKind::Head) and (birds[birdPointer].cells[cellId].kind != CellKind::Tail))
    {
        throw InvalidCellKindException();
    }
    Free(birds[birdPointer].cells[cellId].content);

    birds[birdPointer].cells[cellId].kind = CellKind::Body;
    birds[birdPointer].cells[cellId].content = 0;
}

void DoBird()
{
    byte cellId = EvalCellName(instructions[instructionPointer].operand1);
    if ((birds[birdPointer].cells[cellId].kind == CellKind::Head) or (birds[birdPointer].cells[cellId].kind == CellKind::Tail))
    {
        throw InvalidCellKindException();
    }

    birds[birdPointer].cells[cellId].kind = CellKind::Tail;
    birds[birdPointer].cells[cellId].content = birds.size();

    Bird bird;
    bird.cells[cellId].kind = CellKind::Head;
    bird.cells[cellId].content = birdPointer;
    birds.emplace_back(bird);
}

void DoMove()
{
    byte cellId = EvalCellName(instructions[instructionPointer].operand1);
    if ((birds[birdPointer].cells[cellId].kind == CellKind::Head) or (birds[birdPointer].cells[cellId].kind == CellKind::Tail))
    {
        birdPointer = birds[birdPointer].cells[cellId].content;
    }
    else
    {
        throw InvalidCellKindException();
    }
}

void DoCalc()
{
    byte cellId = EvalCellName(instructions[instructionPointer].operand1);
    if ((birds[birdPointer].cells[cellId].kind == CellKind::Head) or (birds[birdPointer].cells[cellId].kind == CellKind::Tail))
    {
        throw InvalidCellKindException();
    }

    tailexpr = nullptr;
    calcResult = 0;
    exprResult = 1;
    compResult = 0;
    //factResult = 0;
    bnf::Analyze(r_expression, instructions[instructionPointer].operand2.c_str(), &tailexpr);
    birds[birdPointer].cells[cellId].content = exprResult;
}

void DoHead()
{
    byte cellId = EvalCellName(instructions[instructionPointer].operand1);
    if (birds[birdPointer].cells[cellId].kind != CellKind::Head)
    {
        throw InvalidCellKindException();
    }

    performJump = true;
    gotoLabel = instructions[instructionPointer].operand2;
}

void DoTail()
{
    byte cellId = EvalCellName(instructions[instructionPointer].operand1);
    if (birds[birdPointer].cells[cellId].kind != CellKind::Tail)
    {
        throw InvalidCellKindException();
    }

    performJump = true;
    gotoLabel = instructions[instructionPointer].operand2;
}

void DoZero()
{
    byte cellId = EvalCellName(instructions[instructionPointer].operand1);
    if ((birds[birdPointer].cells[cellId].kind == CellKind::Head) or (birds[birdPointer].cells[cellId].kind == CellKind::Tail))
    {
        throw InvalidCellKindException();
    }

    performJump = (birds[birdPointer].cells[cellId].content == 0);
    gotoLabel = instructions[instructionPointer].operand2;
}

void DoElse()
{
    byte cellId = EvalCellName(instructions[instructionPointer].operand1);
    if ((birds[birdPointer].cells[cellId].kind == CellKind::Head) or (birds[birdPointer].cells[cellId].kind == CellKind::Tail))
    {
        throw InvalidCellKindException();
    }

    performJump = (birds[birdPointer].cells[cellId].content != 0);
    gotoLabel = instructions[instructionPointer].operand2;
}

void RunInterpreter()
{
    while (instructionPointer < (int)instructions.size())
    {
        performJump = false;
        gotoLabel = "";

        switch (instructions[instructionPointer].opCode)
        {
        case OpCode::last: { DoLast(); break; }
        case OpCode::jump: { DoJump(); break; }
        case OpCode::pump: { DoPump(); break; }
        case OpCode::dump: { DoDump(); break; }
        case OpCode::free: { DoFree(); break; }
        case OpCode::bird: { DoBird(); break; }
        case OpCode::move: { DoMove(); break; }
        case OpCode::calc: { DoCalc(); break; }
        case OpCode::head: { DoHead(); break; }
        case OpCode::tail: { DoTail(); break; }
        case OpCode::zero: { DoZero(); break; }
        case OpCode::else_: { DoElse(); break; }

        default: { throw AlienException(); }
        } // switch

        if (performJump)
        {
            instructionPointer = labels[gotoLabel];
        }
        else
        {
            instructionPointer++;
        }
    } // while not end of program
}

std::string RunAnalyzers(const std::vector<std::string>& lines, const std::string& input)
{
    std::string output;
    std::string line;

    // charset
    // Needed ?
    // bnf::Token value(1, 255);

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
    const char justSimpleExpression[] = "CALC MEU, #ZO * #BU";
    const char justBiggerExpression[] = "CALC MEU, #MEU * #BU + #GA / #BU - (#ZO * #ZO)";

    InitP1();

    //TODO: Add two events to catch the operator lexem (+ & -) and (* & /)
    r_expression = (r_component + *(l_addsub + OnAddSub + r_component)) + OnExpression; // Recursion !
    int tst = bnf::Analyze(r_program, justSimpleExpression, &tailexpr);
    if (tst > 0)
    {
        std::cout << "OK" << std::endl;
    }
    else
    {
        printf("Failed, stopped at=%.40s\n status = 0x%0X,  flg = %s%s%s%s%s%s%s%s\n",
            tailexpr ? tailexpr : "", tst,
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

    //tail = nullptr; //TODO: Put this in init ?
    InitP2();
    //tst = bnf::Analyze(r_program, justHelloWorld, &tail);
    RunInterpreter(); // May call Analyze again...

    // Disjoin Rule recursion to safe rule removal
    r_expression = bnf::Null();
    //r_line_list = bnf::Null();

    exit(0); //TODO: Remove this when done

    return output;
}

void ShowUsage()
{
    std::cout << "Usage :" << std::endl;
    std::cout << "gabuzomeu program_file_name.gbzm \"optional_input_string\" [-n]" << std::endl;
    std::cout << "gabuzomeu \"program_source_code\" \"optional_input_string\" [-n]" << std::endl;
    std::cout << "The input string may be splitted into several pieces and #values may appear in between" << std::endl;
    std::cout << "And the optional n parameter is an indication if the output should be numeric instead of string" << std::endl;
    exit(-1);
}

///////////////////////
// The main (of) course
///////////////////////

int main(int argc, char *argv[])
{
    //TODO: Remove when done
    std::vector<std::string> dummies;
    std::string dummy;
    InitP1();
    RunAnalyzers(dummies, dummy);

    //TODO: Enhance this to support the forth optional one
    if (argc != 3)
    {
        ShowUsage();
    }

    std::string input = std::string(argv[2]);
    if ((input.size() > 1) and (input.front() == '"') and (input.back() == '"'))
    {
        input = input.substr(1, input.size() - 2); //TODO: Enhance this in order to support #values. We need to call EvalBaseFourNumber
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

    std::cout << RunAnalyzers(lines, input) << std::endl;
}
