
#include <iostream>
#include <cstdarg>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <stack>
#include <cmath>
#include <map>
#include <tuple>
#include <filesystem> // Finally present :)

// Arhg : the getopt POSIX standard for parsing the command line is missing !
// #include <unistd.h>

// Third party
#include "bnflite.hpp"
#include "cxxopts.hpp"
#include "InfInt.hpp"

// Home made
#include "RuntimeExceptions.hpp"
#include "ConverterTools.hpp"
#include "StringTools.hpp"

//TODO: Create Common/Language.hpp ?
#define INTERPRETER_VERSION 1.2 // Follow github versionning
#define LANGUAGE_VERSION 1.1 // 1.0 was without BASE & GBZM

//////////////////////////////////////////////////////////////////////////////////////
// Global variables part one (TODO: [future] put all this into a class, one day ?-) //
//////////////////////////////////////////////////////////////////////////////////////

bool analysis = false;
bool interpret = false;
bool noprompt = false;
size_t limit = 0; // No limitation !
size_t totalInstructionCounter = 0;

const char *tailExpr = nullptr; // Remember, it must be read in reverse : a pointer to a const char
const char *tailProg = nullptr;
// We need two passes even though the language is interpreted (this is due
// to the potential forward jumps to code region not yet spotted.
// Actually the second pass is used only for the expression evaluation
bool firstPass = true;
//bool performSub = false;
//bool performDiv = false;

std::string lastCell;
std::string targetCell;
//std::string gotoLabel;

// I/O vectors needed since the context is created lately in initP2
// Using tuple because the base of numbers in the various I/O operation may change
// We need to store them either as string (base = 0 meaning unknown) or as InfInt
// with the current base, the string is empty then !
//typedef std::tuple<byte, std::string, InfInt> BSBI; // Base - String - Big Integer use get<x> to access the xth element of the tuple :)
std::vector<BSII> inputs;
std::vector<BSII> outputs;

//int calcResult = 0;
InfInt exprResult = 0; // Always one at time !
//int termResult = 1;
//int termLeft = 0;
//int factResult = 0;
//int factLeft = 0;

// No need to put it in the context, we can't have expression referring to GBZM
std::stack<InfInt> operands; // Finally unavoidable for CALC expression, since it is a recursive evaluation !

/*
size_t instructionPointer = 0; // AKA IP
size_t birdPointer = 0;
*/
size_t totalBirdCounder = 1; // There is always one default bird
std::map<std::string, size_t> labels;

// Unable to get the number of elementsinside an enum
// https://stackoverflow.com/questions/712463/number-of-elements-in-an-enum
enum class OpCode { last, jump, pump, dump, free, bird, lift, calc, head, tail, zero, else_, gbzm, base, last_item }; // Care of the else keyword !
const std::string OpCodes[(int) (OpCode::last_item)] = { "LAST", "JUMP", "PUMP", "DUMP", "FREE", "BIRD", "LIFT", "CALC", "HEAD", "TAIL", "ZERO", "ELSE", "GBZM", "BASE" };

struct Instruction
{
    OpCode opCode;
    std::string operand1;
    std::string operand2;
    Instruction(OpCode opc, const std::string &op1, const std::string &op2) : opCode(opc), operand1(op1), operand2(op2) {}
};
std::vector<Instruction> instructions; // Not in the context :)

enum class CellKind { Body, Head, Tail }; // the Body is optional :)

/*
byte EvalCellName(const std::string &value)
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

    throw AlienException(value); // Since the lexical analysis should have done its job
}
*/

struct Cell
{
    CellKind kind;
    InfInt value; // Anyway holds either the index of anoter bird or just a number
};

struct Bird
{
    //Cell ga, bu, zo, meu;
    Cell cells[(int) CellId::last_item];

    Bird()
    {
        for (auto &it : cells)
        {
            it.kind = CellKind::Body;
            it.value = 0;
        }
    }
};
//std::vector<Bird> birds;

struct Context
{
    size_t instructionPointer; // AKA IP
    size_t birdPointer; // AKA current bird index in Birds (AKA BP)
    size_t pumpPointer; //TODO: AKA PP ?
    Base ioBase;
    std::string gotoLabel;
    std::vector<BSII> inputs;
    std::vector<BSII> outputs;
    std::vector<Bird> birds;

    Context()
    {
        Clear();
    }

    void Clear()
    {
        //exprResult = 0;
        ioBase = Base::default_;
        instructionPointer = 0;
        birdPointer = 0;
        pumpPointer = 0;
        inputs.clear();
        outputs.clear();
        birds.clear();
        birds.emplace_back(Bird());
    }
};
std::stack<Context> contexts;

///////////////////
// Trace filters //
///////////////////

//TODO: [future] We could easily redirect those to files
//TODO: Finalize anyway the basic filter implementation + Logger ?

void AnalysisFilter(const char* format, ...)
{
    if (!analysis)
    {
        return;
    }

    char s[512]; // Is there a safer way ?
    va_list args;
    va_start(args, format);
    vsnprintf(s, sizeof(s), format, args);    
    va_end(args);

    std::cout << s << std::endl;
}

void AnalysisFilter(const std::string &value)
{
    if (!analysis)
    {
        return;
    }
    std::cout << value << std::endl;
}

void InterpretFilter(const std::string &value)
{
    if (!interpret)
    {
        return;
    }
    std::cout << value << std::endl;
}

///////////////////////////////////////////
// Event handlers, called for each token //
///////////////////////////////////////////

static bool OnLast(const char *lexem, size_t len)
{
    AnalysisFilter("Last = : %.*s;\n", len, lexem);
    instructions.emplace_back(Instruction(OpCode::last, std::string(lexem, len), ""));
    return true;
}

static bool OnJump(const char *lexem, size_t len)
{
    AnalysisFilter("Jump = : %.*s;\n", len, lexem);
    instructions.emplace_back(Instruction(OpCode::jump, std::string(lexem, len), ""));
    return true;
}

static bool OnDump(const char *lexem, size_t len)
{
    AnalysisFilter("Dump = : %.*s;\n", len, lexem);
    instructions.emplace_back(Instruction(OpCode::dump, std::string(lexem, len), ""));
    return true;
}

static bool OnPump(const char *lexem, size_t len)
{
    //printf("Pump = : %.*s;\n", len, lexem);
    instructions.emplace_back(Instruction(OpCode::pump, std::string(lexem, len), ""));
    return true;
}

static bool OnFree(const char *lexem, size_t len)
{
    AnalysisFilter("Free = : %.*s;\n", len, lexem);
    //AnalysisFilter("FREE = " + std::string(len, lexem));
    instructions.emplace_back(Instruction(OpCode::free, std::string(lexem, len), ""));
    return true;
}

static bool OnBird(const char *lexem, size_t len)
{
    AnalysisFilter("Bird = : %.*s;\n", len, lexem);
    instructions.emplace_back(Instruction(OpCode::bird, std::string(lexem, len), ""));
    return true;
}

static bool OnLift(const char *lexem, size_t len)
{
    AnalysisFilter("Lift = : %.*s;\n", len, lexem);
    instructions.emplace_back(Instruction(OpCode::lift, std::string(lexem, len), ""));
    return true;
}

static bool OnHead(const char *lexem, size_t len)
{
    AnalysisFilter("Head = : %.*s;\n", len, lexem);
    instructions.emplace_back(Instruction(OpCode::head, lastCell, std::string(lexem, len)));
    return true;
}

static bool OnTail(const char *lexem, size_t len)
{
    AnalysisFilter("Tail = : %.*s;\n", len, lexem);
    instructions.emplace_back(Instruction(OpCode::tail, lastCell, std::string(lexem, len)));
    return true;
}

static bool OnZero(const char *lexem, size_t len)
{
    AnalysisFilter("Zero = : %.*s;\n", len, lexem);
    instructions.emplace_back(Instruction(OpCode::zero, lastCell, std::string(lexem, len)));
    return true;
}

static bool OnElse(const char *lexem, size_t len)
{
    AnalysisFilter("Else = : %.*s;\n", len, lexem);
    instructions.emplace_back(Instruction(OpCode::else_, lastCell, std::string(lexem, len)));
    return true;
}

static bool OnGbzm(const char* lexem, size_t len)
{
    AnalysisFilter("Gbzm = : %.*s;\n", len, lexem);
    instructions.emplace_back(Instruction(OpCode::gbzm, lastCell, std::string(lexem, len)));
    return true;
}

static bool OnBase(const char* lexem, size_t len)
{
    AnalysisFilter("Base = : %.*s;\n", len, lexem);
    instructions.emplace_back(Instruction(OpCode::base, std::string(lexem, len), ""));
    return true;
}

//////////////
// Catchers //
//////////////

static bool CaptureAdd(const char *lexem, size_t len)
{
    //AnalysisFilter("Add = : %.*s;\n", len, lexem);
    //performSub = false;
    //termLeft = termResult;
    //printf("termLeft = : %d;\n", termLeft);
    return true;
}

static bool CaptureSub(const char* lexem, size_t len)
{
    //AnalysisFilter("Sub = : %.*s;\n", len, lexem);
    //performSub = true;
    //termLeft = termResult;
    //printf("termLeft = : %d;\n", termLeft);
    return true;
}

static bool CaptureMul(const char *lexem, size_t len)
{
    //AnalysisFilter("Mul = : %.*s;\n", len, lexem);
    if (!firstPass)
    {
        //factLeft = factResult;
        //printf("factLeft = : %d;\n", factLeft);
        //performDiv = false;
    }
    return true;
}

static bool CaptureDiv(const char* lexem, size_t len)
{
    //printf("Div = : %.*s;\n", len, lexem);
    if (!firstPass)
    {
        //factLeft = factResult;
        //printf("factLeft = : %d;\n", factLeft);
        //performDiv = true;
    }
    return true;
}

static bool CaptureMod(const char* lexem, size_t len)
{
    //printf("Mod = : %.*s;\n", len, lexem);
    if (!firstPass)
    {
    }
    return true;
}

static bool OnCalc(const char *lexem, size_t len)
{
    AnalysisFilter("Calc = : %.*s;\n", len, lexem);
    if (firstPass)
    {
        instructions.emplace_back(Instruction(OpCode::calc, targetCell, std::string(lexem, len)));
        //std::cout << "OnCalc : targetCell = " << targetCell << " & lexem =  " << std::string(lexem, len) << std::endl;
    }
    else
    {
        //TODO: Anything to do ?
    }
    return true;
}

//////////////////////
// Kind of visitors //
//////////////////////

static bool VisitExpression(const char* lexem, size_t len)
{
    //printf("VisitExpression = : %.*s;\n", len, lexem);

    if (!firstPass)
    {
    }

    return true;
}

static bool CaptureExpression(const char* lexem, size_t len)
{
    //printf("CaptureExpression = : %.*s;\n", len, lexem);

    if (!firstPass)
    {
        /*
        if (performSub)
        {
            // exprResult -= termResult;
            // exprResult = termLeft - termResult;
        }
        else
        {
            // exprResult += termResult;
            //exprResult = termLeft + termResult;
            //...
        }
        std::cout << "exprResult = " << exprResult << std::endl;
        */

        // Don't !!!
        // factResult = exprResult;
        //factResult = calcResult;
        //std::cout << "factResult_expression = " << factResult << std::endl;

        // Needed ?
        //exprResult = operands.top(); operands.pop(); // Stack should be empty now :)
        //std::cout << "pushed CaptureExpression exprResult =  " << exprResult << std::endl;
    }

    return true;
}

static bool VisitTerm(const char* lexem, size_t len)
{
    //printf("VisitTerm = : %.*s;\n", len, lexem);

    return true;
}

static bool VisitPower(const char* lexem, size_t len)
{
    AnalysisFilter("VisitPower = : %.*s;\n", len, lexem);

    return true;
}

static bool CaptureTerm(const char* lexem, size_t len)
{
    //printf("CaptureTerm = : %.*s;\n", len, lexem);

    if (!firstPass)
    {
        /*
        if (performDiv)
        {
            // termResult /= factResult;
            //termResult = factLeft / factResult;
        }
        else
        {
            // termResult *= factResult;
            //termResult = factLeft * factResult;
        }
        */
    }

    return true;
}

static bool CapturePow(const char* lexem, size_t len)
{
    AnalysisFilter("CapturePow = : %.*s;\n", len, lexem);

    if (!firstPass)
    {
    }

    return true;
}

static bool CapturePowFactorFollow(const char* lexem, size_t len)
{
    AnalysisFilter("CapturePowFactorFollow = : %.*s;\n", len, lexem);

    if (!firstPass)
    {
        InfInt op2 = operands.top(); operands.pop();
        InfInt op1 = operands.top(); operands.pop();
        InfInt value = pow(op1, op2);
        operands.push(value);
    }

    return true;
}

static bool CaptureAddTermFollow(const char* lexem, size_t len)
{
    //printf("CaptureAddTermFollow = : %.*s;\n", len, lexem);

    if (!firstPass)
    {
        /*
        exprResult = operands.top();
        if (performSub)
        {
            exprResult -= termResult;
        }
        else
        {
            exprResult += termResult;
        }
        std::cout << "exprResult = " << exprResult << std::endl;
        */
        InfInt op2 = operands.top(); operands.pop();
        InfInt op1 = operands.top(); operands.pop();
        InfInt value = op1 + op2;
        operands.push(value);        
        //std::cout << "pushed(" << op1 << "+" << op2 << " = " << value << ")" << std::endl;
    }

    return true;
}

static bool CaptureSubTermFollow(const char* lexem, size_t len)
{
    //printf("CaptureSubTermFollow = : %.*s;\n", len, lexem);

    if (!firstPass)
    {
        /*
        exprResult = operands.top();
        if (performSub)
        {
            exprResult -= termResult;
        }
        else
        {
            exprResult += termResult;
        }
        std::cout << "exprResult = " << exprResult << std::endl;
        */
        InfInt op2 = operands.top(); operands.pop();
        InfInt op1 = operands.top(); operands.pop();
        InfInt value = op1 - op2;
        operands.push(value);
        //std::cout << "pushed(" << op1 << "-" << op2 << " = " << value << ")" << std::endl;
    }

    return true;
}

static bool CaptureLitteral(const char* lexem, size_t len)
{
    AnalysisFilter("Litteral = : %.*s;\n", len, lexem);

    if (!firstPass)
    {
        exprResult = LexerNibbleToNumber(std::string(lexem + 1, len - 1));
        //std::cout << "pushed CaptureLitteral exprResult = " << exprResult << std::endl;
        operands.push(exprResult);
    }

    return true;
}

static bool VisitFactor(const char* lexem, size_t len)
{
    //printf("VisitFactor = : %.*s;\n", len, lexem);
    return true;
}

static bool CaptureFactor(const char* lexem, size_t len)
{
    //printf("CaptureFactor = : %.*s;\n", len, lexem);
    if (!firstPass)
    {
        /*
        if (performDiv)
        {
            termResult /= factResult;
        }
        else
        {
            termResult *= factResult;
        }
        std::cout << "termResult = " << termResult << std::endl;
        */
    }
    return true;
}

static bool CaptureMulPowerFollow(const char* lexem, size_t len)
{
    //printf("CaptureMulPowerFollow = : %.*s;\n", len, lexem);
    if (!firstPass)
    {
        /*
        if (performDiv)
        {
            termResult /= factResult;
        }
        else
        {
            termResult *= factResult;
        }
        std::cout << "termResult = " << termResult << std::endl;
        */
        InfInt op2 = operands.top(); operands.pop();
        InfInt op1 = operands.top(); operands.pop();
        InfInt value = op1 * op2;
        operands.push(value);
        //std::cout << "pushed(" << op1 << "*" << op2 << " = " << value << ")" << std::endl;
    }
    return true;
}

static bool CaptureDivPowerFollow(const char* lexem, size_t len)
{
    //printf("CaptureDivPowerFollow = : %.*s;\n", len, lexem);
    if (!firstPass)
    {
        /*
        if (performDiv)
        {
            termResult /= factResult;
        }
        else
        {
            termResult *= factResult;
        }
        std::cout << "termResult = " << termResult << std::endl;
        */
        InfInt op2 = operands.top(); operands.pop();
        InfInt op1 = operands.top(); operands.pop();
        InfInt value = op1 / op2;
        operands.push(value);
        //std::cout << "pushed(" << op1 << "/" << op2 << " = " << value << ")" << std::endl;
    }
    return true;
}

static bool CaptureModPowerFollow(const char* lexem, size_t len)
{
    //printf("CaptureModPowerFollow = : %.*s;\n", len, lexem);
    if (!firstPass)
    {
        InfInt op2 = operands.top(); operands.pop();
        InfInt op1 = operands.top(); operands.pop();
        InfInt value = op1 % op2;
        operands.push(value);
        //std::cout << "pushed(" << op1 << "%" << op2 << " = " << value << ")" << std::endl;
    }
    return true;
}

static bool CaptureCell(const char* lexem, size_t len)
{
    //printf("Cell = : %.*s;\n", len, lexem);
    AnalysisFilter("Cell = : %.*s;\n", len, lexem);
    //lastCell = EvalCellName(std::string(lexem, len));
    lastCell = std::string(lexem, len);

    if (!firstPass)
    {
        byte cellId = GetCellId(lastCell);
        if ((contexts.top().birds[contexts.top().birdPointer].cells[cellId].kind == CellKind::Head) or (contexts.top().birds[contexts.top().birdPointer].cells[cellId].kind == CellKind::Tail))
        {
            throw InvalidCellKindException("CaptureCell " + CellIds[cellId]);
        }
        exprResult = contexts.top().birds[contexts.top().birdPointer].cells[cellId].value;
        //std::cout << "pushed CaptureCell exprResult = " << exprResult << " , cellId = " << (int) cellId << std::endl;
        operands.push(exprResult);
    }

    return true;
}

// Needed for CALC since cell appears as target AND MAY some appear as part of the expression !
static bool CaptureTargetCell(const char* lexem, size_t len)
{
    //printf("TargetCell = : %.*s;\n", len, lexem);
    AnalysisFilter("TargetCell = : %.*s;\n", len, lexem);
    //lastCell = EvalCellName(std::string(lexem, len));
    targetCell = std::string(lexem, len);

    if (!firstPass)
    {
        byte cellId = GetCellId(instructions[contexts.top().instructionPointer].operand1);
        if ((contexts.top().birds[contexts.top().birdPointer].cells[cellId].kind == CellKind::Head) or (contexts.top().birds[contexts.top().birdPointer].cells[cellId].kind == CellKind::Tail))
        {
            throw InvalidCellKindException("CaptureTargetCell " + CellIds[cellId]);
        }
        exprResult = contexts.top().birds[contexts.top().birdPointer].cells[cellId].value;
        //std::cout << "pushed TargetCell exprResult = " << exprResult << std::endl;
        operands.push(exprResult);
    }

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

static bool OnLabel(const char* lexem, size_t len)
{
    if (!firstPass) 
    {
        return true;
    }

    //printf("Label = : %.*s;\n", len, lexem);
    std::string key = std::string(lexem + 1, len -1); // Remove the colon
        
    if (labels.find(key) == labels.end())
    {
        labels[key] = instructions.size(); // instructionCounter; // Store it !
    }
    else
    {
        std::cout << "Duplicate label : " << key << std::endl;
        //TODO: [future] Raise exception ? But we are not at runtime...
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

////////////////////////
// Phase initializers //
////////////////////////

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
    // Too dangerous here ! Conflict in GBZM implementation
    //contexts.top().inputs = inputs;
    //lastCell = -1;
    //birds.emplace_back(Bird());

    //totalInstructionCounter = 0;
}

////////////////////////////////////////////////////////////////////
// Global variables part two (miam !-) : language defitions items //
////////////////////////////////////////////////////////////////////

// Keywords (it seems that we can't hardcode them as string into the production rules :(
bnf::Lexem l_last("LAST");
bnf::Lexem l_jump("JUMP");
bnf::Lexem l_dump("DUMP");
bnf::Lexem l_pump("PUMP");
bnf::Lexem l_free("FREE");
bnf::Lexem l_bird("BIRD");
bnf::Lexem l_lift("LIFT");
bnf::Lexem l_calc("CALC");
bnf::Lexem l_head("HEAD");
bnf::Lexem l_tail("TAIL");
bnf::Lexem l_zero("ZERO");
bnf::Lexem l_else("ELSE");
bnf::Lexem l_gbzm("GBZM"); // Judas !
bnf::Lexem l_base(OpCodes[(int) OpCode::base].c_str()); // OK as well but a bit long :)

// Tokens
// It seems that the lib consider token as building blocks for lexemes... So character set operators
bnf::Token t_alpha('A', 'Z'); // We can't add here support for "_" :(

/*
bnf::Token t_label = "";
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

//bnf::Lexem l_addsub = bnf::Lexem("+") | bnf::Lexem("-");
//bnf::Lexem l_muldiv = bnf::Lexem("*") | bnf::Lexem("/");
bnf::Lexem l_add = bnf::Lexem("+");
bnf::Lexem l_sub = bnf::Lexem("-"); // Care we don't support unary minus operator, in fact we assume positive integer only
bnf::Lexem l_mul = bnf::Lexem("*");
bnf::Lexem l_div = bnf::Lexem("/");
// Added in chronological order...
bnf::Lexem l_pow = bnf::Lexem("^");
bnf::Lexem l_mod = bnf::Lexem("%");
//TODO: Needed ? 
//bnf::Lexem l_bnot = bnf::Lexem("~");
//bnf::Lexem l_band = bnf::Lexem("&");
//bnf::Lexem l_bor = bnf::Lexem("|");

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
bnf::Rule r_lift = (l_lift + l_cell) + OnLift;
bnf::Rule r_head = (l_head + l_cell + CaptureCell + "," + l_label) + OnHead;
bnf::Rule r_tail = (l_tail + l_cell + CaptureCell + "," + l_label) + OnTail;
bnf::Rule r_zero = (l_zero + l_cell + CaptureCell + "," + l_label) + OnZero;
bnf::Rule r_else = (l_else + l_cell + CaptureCell + "," + l_label) + OnElse;
bnf::Rule r_gbzm = (l_gbzm + l_cell + CaptureCell + "," + l_cell) + OnGbzm;
bnf::Rule r_base = (l_base + (l_litteral | l_cell)) + OnBase; // Exception : we authorize the usage of an immediate value ! Else we should use dedicated keywords (BYTE or CHAR, HEXA, FOUR, DEC & BIT) ?

bnf::Rule r_expression; // Must be "alone"... Recursivity issue
bnf::Rule r_term;
bnf::Rule r_power;
bnf::Rule r_factor;
//bnf::Rule r_factor = (l_litteral | l_cell | "(" + r_expression + ")") + OnFactor; //TODO: Add minus support ? http://homepage.divms.uiowa.edu/~jones/compiler/spring13/notes/10.shtml

bnf::Rule r_calc = (l_calc + l_cell + CaptureTargetCell + "," + r_expression) + OnCalc;

bnf::Rule r_instruction = (r_last | r_jump | r_dump | r_pump | r_free | r_bird | r_lift | r_calc | r_head | r_tail | r_zero | r_else | r_gbzm | r_base) + OnInstruction;
bnf::Rule r_line = (l_colon_label + OnLabel | r_instruction) + OnLine;

// This was ok but raises strange duplicate lines !
//bnf::Rule r_line_list; // Idem
//r_line_list = (r_line | r_line + r_line_list) + DoLineList; // Care...

//bnf::Rule r_line_list = (*r_line) + OnLineList;
//bnf::Rule r_program = r_line_list + OnProgram;

// Short way :p
bnf::Rule r_program = (*r_line) + OnProgram;

////////////////////////
// Express yourself ! //
////////////////////////

void preludeExpression()
{
    tailExpr = nullptr;    
    exprResult = 0;
    //calcResult = 0;
    //termResult = 1; // Neutral for *
    //termLeft = 0;
    //factResult = 0;
    //factLeft = 0;

    // Not good
    //r_term = (r_factor + "*" + +OnMul + r_term | r_factor + "/" + OnDiv + r_term | r_factor) + OnTerm; // Right recursion !
    //r_expression = (r_term + "+" + OnAdd + r_expression | r_term + "-" + OnSub + r_expression | r_term) + OnExpression;  // Idem + longest first

    // Good ?-) Notice the usage of simple lexemes (bypass C++ operator issues)
    r_expression = (r_term + CaptureTerm + *(l_add + CaptureAdd + r_term + CaptureAddTermFollow | l_sub + CaptureSub + r_term + CaptureSubTermFollow)) + VisitExpression;
    //r_term_follow = "+" + OnAdd + r_term;
    r_term = (r_power + CapturePow + *(l_mul + CaptureMul + r_power + CaptureMulPowerFollow | l_div + CaptureDiv + r_power + CaptureDivPowerFollow | l_mod + CaptureMod + r_power + CaptureModPowerFollow)) + VisitTerm; // Right recursion & longest first !
    //r_fact_follow = ...
    r_power = (r_factor + CaptureFactor + *(l_pow + CapturePow + r_factor + CapturePowFactorFollow)) + VisitPower;
    r_factor = (l_litteral + CaptureLitteral | l_cell + CaptureCell | "(" + r_expression + CaptureExpression + ")") + VisitFactor; // Only one recursion
}

void postludeExpression()
{
    // Disjoin recursive rules for safe removal
    r_expression = bnf::Null();
    r_term = bnf::Null();
    r_power = bnf::Null();
    r_factor = bnf::Null();
}

////////////////////
// Do something ! // 
////////////////////

void DoLast()
{
    if (contexts.top().pumpPointer == contexts.top().inputs.size())
    {
        //std::cout << "charPointer = " << charPointer << " & inputs.size() = " << inputs.size() << std::endl;
        //std::cout << "instructionPointer = " << instructionPointer << " & instructions[instructionPointer].operand1 = " << instructions[instructionPointer].operand1 << std::endl;
        contexts.top().gotoLabel = instructions[contexts.top().instructionPointer].operand1;
    }
}

void DoJump()
{
    contexts.top().gotoLabel = instructions[contexts.top().instructionPointer].operand1;
}

void DoPump()
{
    byte cellId = GetCellId(instructions[contexts.top().instructionPointer].operand1);
    if ((contexts.top().birds[contexts.top().birdPointer].cells[cellId].kind == CellKind::Head) or (contexts.top().birds[contexts.top().birdPointer].cells[cellId].kind == CellKind::Tail))
    {
        throw InvalidCellKindException("DoPump " + CellIds[cellId]);
    }

    if (contexts.top().pumpPointer >= contexts.top().inputs.size())
    {
        throw NoMoreInputException("DoPump");
    }

    if (contexts.top().inputs[contexts.top().pumpPointer].b == Base::Unknown)
    {
        contexts.top().birds[contexts.top().birdPointer].cells[cellId].value = NibbleToNumber(contexts.top().ioBase, contexts.top().inputs[contexts.top().pumpPointer].s);
    }
    else
    {
        contexts.top().birds[contexts.top().birdPointer].cells[cellId].value = contexts.top().inputs[contexts.top().pumpPointer].ii;
    }
    contexts.top().pumpPointer++;
}

void DoDump()
{
    byte cellId = GetCellId(instructions[contexts.top().instructionPointer].operand1);
    //std::cout << "Debug 0 : cellId = " << (int) cellId << std::endl;
    if ((contexts.top().birds[contexts.top().birdPointer].cells[cellId].kind == CellKind::Head) or (contexts.top().birds[contexts.top().birdPointer].cells[cellId].kind == CellKind::Tail))
    {
        throw InvalidCellKindException("DoDump " + CellIds[cellId]);
    }

    //std::cout << "birdPointer = " << birdPointer << " & DUMP = " << birds[birdPointer].cells[cellId].value << std::endl;
    //std::cout << "size before = " << contexts.top().outputs.size() << std::endl;
    //std::cout << "DoDump ==> value = " << contexts.top().birds[contexts.top().birdPointer].cells[cellId].value << std::endl;
    contexts.top().outputs.emplace_back(BSII(contexts.top().ioBase, contexts.top().birds[contexts.top().birdPointer].cells[cellId].value));
    //std::cout << "size after = " << contexts.top().outputs.size() << std::endl;
}

// Care : recursive ! Or not... We can leave everything as is and not recycle birds
// nor changing all the id's ! So some birds may be unreachable !
void Free(int birdIndex)
{
}

//TODO: Verify if we are up or down regarding the link to destroy... Else it is a clean cut !
void DoFree()
{
    byte cellId = GetCellId(instructions[contexts.top().instructionPointer].operand1);
    if ((contexts.top().birds[contexts.top().birdPointer].cells[cellId].kind != CellKind::Head) and (contexts.top().birds[contexts.top().birdPointer].cells[cellId].kind != CellKind::Tail))
    {
        throw InvalidCellKindException("DoFree " + CellIds[cellId]);
    }
    Free(contexts.top().birds[contexts.top().birdPointer].cells[cellId].value.toInt());

    contexts.top().birds[contexts.top().birdPointer].cells[cellId].kind = CellKind::Body;
    contexts.top().birds[contexts.top().birdPointer].cells[cellId].value = 0;
}

void DoBird()
{
    byte cellId = GetCellId(instructions[contexts.top().instructionPointer].operand1);
    if ((contexts.top().birds[contexts.top().birdPointer].cells[cellId].kind == CellKind::Head) or (contexts.top().birds[contexts.top().birdPointer].cells[cellId].kind == CellKind::Tail))
    {
        throw InvalidCellKindException("DoBird " + CellIds[cellId]);
    }

    contexts.top().birds[contexts.top().birdPointer].cells[cellId].kind = CellKind::Tail;
    contexts.top().birds[contexts.top().birdPointer].cells[cellId].value = contexts.top().birds.size();

    Bird bird;
    bird.cells[cellId].kind = CellKind::Head;
    bird.cells[cellId].value = contexts.top().birdPointer;
    contexts.top().birds.emplace_back(bird);

    totalBirdCounder++;
}

void DoLift()
{
    byte cellId = GetCellId(instructions[contexts.top().instructionPointer].operand1);
    if ((contexts.top().birds[contexts.top().birdPointer].cells[cellId].kind == CellKind::Head) or (contexts.top().birds[contexts.top().birdPointer].cells[cellId].kind == CellKind::Tail))
    {        
        contexts.top().birdPointer = contexts.top().birds[contexts.top().birdPointer].cells[cellId].value.toInt();
        //std::cout << "DoLift ==> contexts.top().birdPointer = " << contexts.top().birdPointer << std::endl;
    }
    else
    {
        throw InvalidCellKindException("DoLift " + CellIds[cellId]);
    }
}

void DoCalc()
{
    byte cellId = GetCellId(instructions[contexts.top().instructionPointer].operand1);
    if ((contexts.top().birds[contexts.top().birdPointer].cells[cellId].kind == CellKind::Head) or (contexts.top().birds[contexts.top().birdPointer].cells[cellId].kind == CellKind::Tail))
    {
        throw InvalidCellKindException("DoCalc " + CellIds[cellId]);
    }

    //std::cout << "Debug 0" << std::endl;
    preludeExpression();
    //std::cout << "Debug 1 expr = " << instructions[instructionPointer].operand2 << std::endl;
    bnf::Analyze(r_expression, instructions[contexts.top().instructionPointer].operand2.c_str(), &tailExpr);
    
    //TODO: Introduce if (!...) ???

    //std::cout << "Debug 2" << std::endl;
    postludeExpression();
    //std::cout << "Debug 3" << std::endl;
    exprResult = operands.top(); operands.pop();
    //std::cout << "DoCalc ==> contexts.top().exprResult = " << exprResult << std::endl;
    contexts.top().birds[contexts.top().birdPointer].cells[cellId].value = exprResult;
    //std::cout << "DoCalc ==> birdPointer = " << birdPointer << ", cellId = " << (int) cellId << " & CALC = " << birds[birdPointer].cells[cellId].value << std::endl;
}

void DoHead()
{
    byte cellId = GetCellId(instructions[contexts.top().instructionPointer].operand1);
    if (contexts.top().birds[contexts.top().birdPointer].cells[cellId].kind == CellKind::Head)
    {
        contexts.top().gotoLabel = instructions[contexts.top().instructionPointer].operand2;
    }
}

void DoTail()
{
    byte cellId = GetCellId(instructions[contexts.top().instructionPointer].operand1);
    if (contexts.top().birds[contexts.top().birdPointer].cells[cellId].kind == CellKind::Tail)
    {
        contexts.top().gotoLabel = instructions[contexts.top().instructionPointer].operand2;
    }
}

void DoZero()
{
    byte cellId = GetCellId(instructions[contexts.top().instructionPointer].operand1);
    if ((contexts.top().birds[contexts.top().birdPointer].cells[cellId].kind == CellKind::Head) or (contexts.top().birds[contexts.top().birdPointer].cells[cellId].kind == CellKind::Tail))
    {
        throw InvalidCellKindException("DoZero " + CellIds[cellId]);
    }

    if (contexts.top().birds[contexts.top().birdPointer].cells[cellId].value == 0)
    {
        contexts.top().gotoLabel = instructions[contexts.top().instructionPointer].operand2;
    }
}

void DoElse()
{
    byte cellId = GetCellId(instructions[contexts.top().instructionPointer].operand1);
    if ((contexts.top().birds[contexts.top().birdPointer].cells[cellId].kind == CellKind::Head) or (contexts.top().birds[contexts.top().birdPointer].cells[cellId].kind == CellKind::Tail))
    {
        throw InvalidCellKindException("DoElse " + CellIds[cellId]);
    }

    if (contexts.top().birds[contexts.top().birdPointer].cells[cellId].value != 0)
    {
        contexts.top().gotoLabel = instructions[contexts.top().instructionPointer].operand2;
    }    
}

void DoGbzm()
{
    void RunInterpreter(); // Good old forward decl !-)

    byte cellId = GetCellId(instructions[contexts.top().instructionPointer].operand1);
    if ((contexts.top().birds[contexts.top().birdPointer].cells[cellId].kind == CellKind::Head) or (contexts.top().birds[contexts.top().birdPointer].cells[cellId].kind == CellKind::Tail))
    {
        throw InvalidCellKindException("DoGbzm " + CellIds[cellId]);
    }    

    // Copy the inputs for the "new interpreter instance" ! We assume the base 256
    std::vector<byte> bytes = NumberToByteStream(contexts.top().birds[contexts.top().birdPointer].cells[cellId].value);

    // Prepare a new context
    contexts.push(contexts.top()); // push inputs, outputs, instruction pointer, ...
    contexts.top().Clear();
    
    // Paste it ! 
    contexts.top().inputs.resize(bytes.size()); // Care ! https://stackoverflow.com/questions/29920394/vector-of-class-without-default-constructor
    //std::copy(bytes.begin(), bytes.end(), inputs.begin());
    for (int i = 0; i < bytes.size(); i++)
    {
        contexts.top().inputs[i] = BSII(Base::default_, bytes[i]);
    }

    InitP2(); // init inputs, outputs, instruction pointer, ...
    RunInterpreter(); // In RunInterpreter when current program is finished, check context stack content... Needed ?

    // Retrieve the ouputs for the "old interpreter instance" !
    std::vector<BSII> bigns = contexts.top().outputs;

    // Restore the old context
    contexts.pop(); // if not empty, pop inputs, outputs, instruction pointer, ...
    cellId = GetCellId(instructions[contexts.top().instructionPointer].operand2);
    contexts.top().birds[contexts.top().birdPointer].cells[cellId].value = ByteStreamToNumber(NumbersToByteStream(bigns, true));
    //std::cout << "DoGbzm ==> cellId = " << (int) cellId  << " & Return value = " << contexts.top().birds[contexts.top().birdPointer].cells[cellId].value << std::endl;
}

// We support the following bases (no need for octal yet) :
// 2 (#ZO), 4 (#BUGA still using GA BU ZO MEU obviously !-), 10 (#ZOZO), 16 (#BUGAGA), 64 (#BUGAGAGA) & 256 (#BUGAGAGAGA) 
void DoBase()
{
    std::string operand = instructions[contexts.top().instructionPointer].operand1;
    InfInt base;
    if (operand[0] == '#')
    {
        // Little dirty trick here... The BASE instruction awaits the new base number. Althouhg 256 doesn't fit into a byte !
        bool big_backup = big;
        big = true;
        base = LexerNibbleToNumber(operand.substr(1, operand.size() - 1));
        big = big_backup;
    }
    else
    {
        base = contexts.top().birds[contexts.top().birdPointer].cells[GetCellId(operand)].value;
    }
    //TODO: Use the Base enum ?
    if ((base != 2) and (base != 4) and (base != 10) and (base != 16) and (base != 64) and (base != 256))
    {
        throw BaseNotSupported(base.toString());
    }
    
    contexts.top().ioBase = (Base) base.toInt();
}

/////////
// Run // 
/////////

void RunInterpreter()
{
    contexts.top().gotoLabel = "";
    //std::cout << "contexts.top().instructionPointer = " << contexts.top().instructionPointer << " & instructions.size() = " << instructions.size() << std::endl;
    while ((contexts.top().instructionPointer < (int) instructions.size()) and ((limit == 0) or (totalInstructionCounter < limit)))
    {
        //TODO: Pff : Review this....
#ifdef _DEBUG
        std::cout << "TIC = " << totalInstructionCounter << ", BP = " << contexts.top().birdPointer
            << ", SL = " << contexts.size() << ", Opc = " << OpCodes[(int)instructions[contexts.top().instructionPointer].opCode]
            << ", inputs.size() = " << contexts.top().inputs.size()
            << ", outputs.size() = " << contexts.top().outputs.size()
            << ", op1 = '" << instructions[contexts.top().instructionPointer].operand1
            << "', op2 = '" << instructions[contexts.top().instructionPointer].operand2
            << "', GA = " << contexts.top().birds[contexts.top().birdPointer].cells[(int) CellId::ga].value
            << ", BU = " << contexts.top().birds[contexts.top().birdPointer].cells[(int) CellId::bu].value
            << ", ZO = " << contexts.top().birds[contexts.top().birdPointer].cells[(int) CellId::zo].value
            << "& MEU = " << contexts.top().birds[contexts.top().birdPointer].cells[(int) CellId::meu].value << std::endl;
#endif
        InterpretFilter(OpCodes[(int) instructions[contexts.top().instructionPointer].opCode]);

        switch (instructions[contexts.top().instructionPointer].opCode)
        {
        case OpCode::last: { DoLast(); break; }
        case OpCode::jump: { DoJump(); break; }
        case OpCode::pump: { DoPump(); break; }
        case OpCode::dump: { DoDump(); break; }
        case OpCode::free: { DoFree(); break; }
        case OpCode::bird: { DoBird(); break; }
        case OpCode::lift: { DoLift(); break; }
        case OpCode::calc: { DoCalc(); break; }
        case OpCode::head: { DoHead(); break; }
        case OpCode::tail: { DoTail(); break; }
        case OpCode::zero: { DoZero(); break; }
        case OpCode::else_: { DoElse(); break; }
        case OpCode::gbzm: { DoGbzm(); break; }
        case OpCode::base: { DoBase(); break; }

        default: { throw AlienException("Line = " + std::to_string(contexts.top().instructionPointer)); }
        } // switch

        if (contexts.top().gotoLabel != "")
        {
            //std::cout << "labels[gotoLabel] = " << labels[gotoLabel] << ", gotoLabel = " << gotoLabel << " & instructionPointer = " << instructionPointer << std::endl;
            if (labels.find(contexts.top().gotoLabel) == labels.end())
            {
                throw LabelNotFoundException(contexts.top().gotoLabel);
            }
            contexts.top().instructionPointer = labels[contexts.top().gotoLabel];
            contexts.top().gotoLabel = "";
        }
        else
        {
            contexts.top().instructionPointer++;
        }

        totalInstructionCounter++;
    } // while not end of program
}

void RunAnalyzers(const std::vector<std::string> &lines)
{
    // charset
    // Needed ?
    // bnf::Token value(1, 255);

    size_t pos;
    std::string line;
    for (const auto &it : lines)
    {
        if (it.size() == 0)
        {
            continue;
        }

        pos = it.find(";");
        if (pos == 0)
        {
            continue;
        } 
        else if (pos == std::string::npos)
        {
            line += StringUpper(it) + '\n';
        }
        else
        {
            line += StringUpper(it.substr(0, pos)) + '\n';
        }
    }

    const char justHelloWorld[] = "CALC GA, #BUZOZOGA DUMP GA CALC BU, #BUZOBUBU DUMP BU CALC ZO, #BUZOMEUGA DUMP ZO DUMP ZO CALC MEU, #BUZOMEUMEU DUMP MEU CALC GA, #ZOGAGA DUMP GA CALC BU, #BUMEUBUMEU DUMP BU DUMP MEU CALC BU, #BUMEUGAZO DUMP BU DUMP ZO CALC GA, #BUZOBUGA DUMP GA";
    //const char helloWorld[] = "CALC GA, #BUZOZOGA\nDUMP GA\nCALC BU, #BUZOBUBU\nDUMP BU\nCALC ZO, #BUZOMEUGA\nDUMP ZO\nDUMP ZO\nCALC MEU, #BUZOMEUMEU\nDUMP MEU\nCALC GA, #ZOGAGA\nDUMP GA\nCALC BU, #BUMEUBUMEU\nDUMP BU\nDUMP MEU\nCALC BU, #BUMEUGAZO\nDUMP BU\nDUMP ZO\nCALC GA, #BUZOBUGA\nDUMP GA";
    //const char helloWorld[] = "CALCGA,#BUZOZOGADUMPGACALCBU,#BUZOBUBUDUMPBUCALCZO,#BUZOMEUGADUMPZODUMPZOCALCMEU,#BUZOMEUMEUDUMPMEUCALCGA,#ZOGAGADUMPGACALCBU,#BUMEUBUMEUDUMPBUDUMPMEUCALCBU,#BUMEUGAZODUMPBUDUMPZOCALCGA,#BUZOBUGADUMPGA";
    const char justHelloZob[] = "CALC GA, #BUZOZOGA DUMP GA CALC BU, #BUZOBUBU DUMP BU CALC ZOB"; // KO so OK :)
    const char justCalc[] = "CALC GA, #BUZOZOGA"; // OK
    const char justMove[] = "LIFTGA"; // OK
    const char justJump[] = "JUMPTOTO"; // OK
    const char justDoubleJump[] = "JUMPTOTOJUMPTITI"; // OK
    const char justBirdWithSpace[] = "BIRD GA"; // OK
    const char justDoubleMove[] = "LIFTGALIFTBU"; // OK
    const char justBogusMove[] = "LIFTGO"; // KO so OK :)
    const char justDoubleMoveWithSpaces[] = "LIFT GA LIFT MEU"; // OK
    const char justNothing[] = ""; // OK
    const char justLabel[] = ":TOTO"; // OK
    const char justCell[] = "GA"; // KO so OK :)
    const char justGarbage[] = "TGA"; // KO so OK :)
    const char justSomeLabels[] = ":TOTO :TITI LIFT GA :AHU"; // OK
    const char justSameLabels[] = ":TOTO :TITI LIFT GA :TOTO"; // KO so OK :)
    const char justParenthesis[] = "CALC MEU, (#BUBU)"; // = 5 OK
    const char justAssignementThenCalc[] = "CALC MEU, #BUZO CALC ZO, (MEU * #BUZO) * #BU + #ZO"; // 36 OK
    const char justSimpleExpression[] = "CALC MEU, #BUBU * #BUZO / #BUMEU"; // AKA 5 * 6 / 7 = 4 (Nice we support left to right evaluation :)
    const char justSimpleExpressionLeft[] = "CALC MEU, (#BUBU * #BUZO) / #BUMEU"; // = 4 OK
    const char justSimpleExpressionRight[] = "CALC MEU, #BUBU * (#BUZO / #BUMEU)"; // = 0 OK
    const char justBiggerExpression[] = "CALC MEU, #MEU * #BU + #GA / #BU - (#ZO * #ZO)"; // AKA 3 * 1 + 0 / 1 - (2 * 2) = -1 OK

    // Valid but program passed as command line may have at most one comment. Here the line must be processed (uppercase + comment removal)
    // const char justInstructionCommentInstructionComment[] = "LAST BU;ahu ZERO GA, jmp ; So cool language";
 
    preludeExpression();
    int tst = bnf::Analyze(r_program, line.c_str(), &tailProg); // tailProg content is not meaningfull :(
    postludeExpression();

    if (tst > 0)
    {
        //std::cout << "Analyse OK" << std::endl;
    }
    else
    {
        //TODO: Replace the following by a nice error message...
        printf("Failed, stopped at=%.40s\n status = 0x%0X,  flg = %s%s%s%s%s%s%s%s\n",
            tailProg ? tailProg : "", tst,
            tst & bnf::eOk ? "eOk" : "Not",
            tst & bnf::eRest ? ", eRest" : "",
            tst & bnf::eOver ? ", eOver" : "",
            tst & bnf::eEof ? ", eEof" : "",
            tst & bnf::eBadRule ? ", eBadRule" : "",
            tst & bnf::eBadLexem ? ", eBadLexem" : "",
            tst & bnf::eSyntax ? ", eSyntax" : "",
            tst & bnf::eError ? ", eError" : "");

        exit(-8);
    }

    /*
    std::cout << "instructions.size() = " << instructions.size() << " & labels.size() = " << labels.size() << std::endl;
    for (const auto kvm : labels)
    {
        std::cout << "key = " << kvm.first << " & value = " << kvm.second << std::endl;
    }
    */
}

void ShowUsage()
{
    std::cout << "Usage :" << std::endl;
    std::cout << std::endl;
    std::cout << "gabuzomeu (--file=\"file_name.gbzm\" | --program=\"text\") [--source=\"file_name\" | --data=\"value\"]" << std::endl;
    std::cout << "          [--target = \"file_name\"] [--limit=\"number\"] [-i | --interpret] [-a | --analysis] [-b | --big]" << std::endl;
    std::cout << "          [-Q | --Quine] [-w | --write] [-v | --version] [-n | --noprompt]" << std::endl;
    std::cout << std::endl;
    std::cout << "Where the file or the program must be provided, the data can be empty" << std::endl;
    std::cout << "Strings can't contain double quote, screen data in input or output may hold " << std::endl; 
    std::cout << "#numeric_nibbles# in gabuzomeu base four in order to replace the non printable characters" << std::endl; // The nibble value for the # (35) is #ZOGAMEU and for " (34) it is #ZOGAZO
    std::cout << "On the other hand, file data are treated as stream of bytes" << std::endl;
    std::cout << "The optional target is to redirect the output toward a file instead of the screen as default" << std::endl;
    std::cout << "The two optional interpret and analysis flags are there for tracing purpose" << std::endl;
    std::cout << "Finally the optional big flag allows the support of number bigger than a byte" << std::endl;
    std::cout << "New flags ! Quine parameter allows nibble output without trailing #" << std::endl;
    std::cout << "write force the character printing on the console for the ASCII code below 32" << std::endl; // So the display will be OS dependant
    std::cout << "limit can be used in order to break infinite loop after a given number of instruction" << std::endl;
    std::cout << "noprompt remove the output formatting for the console and version just display information and stop" << std::endl;
    //TODO: separate the parameters like name=value and the toggles !
}

void Run(const std::vector<std::string> &lines)
{
    t_alpha.Add('_'); // Becoze we are kind :)
    InitP1();
    RunAnalyzers(lines); // Raise exception in debug mode... In string dtor
    //std::cout << "P1 done , about to start P2" << std::endl;

    if (instructions.size() == 0)
    {
        std::cout << "No instruction no babble..." << std::endl;
    }
    else
    {
        contexts.push(Context());
        contexts.top().inputs = inputs;
        InitP2();
        RunInterpreter(); // May call Analyze(...) again for CALC instructions
    }
}

//////////////////////////
// The main (of) course //
//////////////////////////

int main(int argc, char *argv[])
{
    ///////////////////////////////////
    // Grab the command line parameters
    ///////////////////////////////////

    // Code
    std::string file;
    std::string program;
    // Input
    std::string source;
    std::string data;
    // The default output is the screen
    std::string target;

    //std::cout << "Debug 0" << std::endl;

    try
    {
        cxxopts::Options options("gabuzomeu", "");
        //std::cout << "Debug 1" << std::endl;
        options.add_options()
            // Since file & source are mutually exclusive, we have to provide empty default values else the lib raise std::exception("no value") !
            ("f,file", "", cxxopts::value<std::string>()->default_value(""))
            ("p,program", "", cxxopts::value<std::string>()->default_value(""))
            ("s,source", "", cxxopts::value<std::string>()->default_value(""))
            ("d,data", "", cxxopts::value<std::string>()->default_value(""))
            ("t,target", "", cxxopts::value<std::string>()->default_value(""))
            ("l,limit", "", cxxopts::value<size_t>()->default_value("0"))
            ("a,analysis", "", cxxopts::value<bool>()->default_value("false"))
            ("i,interpret", "", cxxopts::value<bool>()->default_value("false"))
            ("b,big", "", cxxopts::value<bool>()->default_value("false"))
            ("w,write", "", cxxopts::value<bool>()->default_value("false"))
            //("")   Add support for the spaceship operators <-< & >-> ?
            ("Q,Quine", "", cxxopts::value<bool>()->default_value("false"))
            ("v,version", "", cxxopts::value<bool>()->default_value("false"))
            ("n,noprompt", "", cxxopts::value<bool>()->default_value("false")); // IE : for Quine

        //std::cout << "Debug 2" << std::endl;

        auto parameters = options.parse(argc, argv);
        file = parameters["file"].as<std::string>();
        program = parameters["program"].as<std::string>();
        source = parameters["source"].as<std::string>();
        data = parameters["data"].as<std::string>();
        target = parameters["target"].as<std::string>();
        analysis = parameters["analysis"].as<bool>();
        interpret = parameters["interpret"].as<bool>();
        big = parameters["big"].as<bool>();
        write = parameters["write"].as<bool>();
        Quine = parameters["Quine"].as<bool>();
        limit = parameters["limit"].as<size_t>();
        noprompt = parameters["noprompt"].as<bool>();
        
        if (parameters["version"].as<bool>())
        {
            std::cout << "This interpreter version is " << INTERPRETER_VERSION << " and the supported Gabuzomeu language version is " << LANGUAGE_VERSION << std::endl;
            return 0;
        }
        //TODO: Implement Omniscient mode ? Access other bird brains using [] accessor inside expr... Like CALC GA, BU[ZO] * MEU

        // Nice (shows the additional help strings), but incomplete syntax (ie for the "=")
        //std::cout << options.help() << std::endl;

        //throw std::exception("test command line");
    }
    catch (const std::exception& e)
    {
        //TODO: Remove this when done !
        std::cout << "Exception : " << e.what() << std::endl;
        std::cout << "file = '" << file << "', program = '" << program << "', source = '" << source << "', data = '" << data << "' & target = '" << target << "'" <<std::endl;
        std::cout << "limit = " << limit << ", big = " << big << ", write = " << write << " & Quine = " << Quine << std::endl;
        ShowUsage();
        return -1;
    }
    //std::cout << "Debug 4" << std::endl;

    // The lib kindly removes the double quote for key="value" arguments !
    //std::cout << "Debug : file = " << file << ", program = " << program << ", source = " << source << ", data = " << data << " & target = " << target << std::endl;

    // Source and data can be both empty... Usefull for Quine :)
    if (((file == "") and (program == "")) or ((file != "") and (program != "")) or ((source != "") and (data != "")))
    {
        ShowUsage();
        return -2;
    }

    //////////////
    // Validate it
    //////////////

    // Input...
    if (source == "")
    {
        try
        {
            inputs = CompositeStringToNumbers(data); // In return, base 0 means unknow yet base. Nibble ok
        }
        catch (const RuntimeException& re)
        {            
            std::cout << "Unable to parse the data string :(" << std::endl;
            return -3;
        }
    }
    else
    {
        try
        {
            if (!std::filesystem::exists(source))
            {
                throw FileNotFoundException(source);
            }
            std::vector<byte> bytes;
            bytes.resize(std::filesystem::file_size(source));
            std::ifstream ifs(source, std::ios::binary);
            ifs.read((char *) bytes.data(), std::filesystem::file_size(source));
            ifs.close();

            inputs.resize(std::filesystem::file_size(source));
            //std::copy(bytes.begin(), bytes.end(), inputs.begin());
            for (int i = 0; i < bytes.size(); i++)
            {
                inputs[i] = BSII(Base::default_, bytes[i]); //TODO: Nibble not supported yet from file !
            }
            //TODO: We need a kind of automate reader here : nibble mode in unkown base / byte mode...
        }
        catch (const FileNotFoundException &fnf)
        {
            std::cout << "Data file not found :(" << std::endl;
            return -4;
        }
        catch (const std::ifstream::failure &e)
        {
            std::cout << "Unable to read the data file :(" << std::endl;
            return -5;
        }
    }

    // Code...
    std::vector<std::string> lines;
    //if ((program.size() > 1) and (program.front() == '"') and (program.back() == '"'))
    if (file == "")
    {
        //lines.emplace_back(program.substr(1, program.size() - 2));
        lines.emplace_back(source);
    }
    else
    {
        try
        {
            if (!std::filesystem::exists(file))
            {
                throw FileNotFoundException(file);
            }

            std::ifstream ifs;
            // Don't put this... Raise exception
            // ifs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            ifs.open(file);
            while (ifs.good())
            {
                std::getline(ifs, source);
                lines.emplace_back(source);
            }
            ifs.close();
        }
        catch (const FileNotFoundException &fnf)
        {
            std::cout << "Program file not found :(" << std::endl;
            return -6;
        }
        catch (const std::ifstream::failure &e)
        {
            std::cout << "Unable to read the program file :(" << std::endl;
            return -7;
        }
    }

    ///////////////
    // Execute it !
    ///////////////

    try
    {        
        Run(lines);

        if (contexts.top().outputs.size() > 0)
        {
            if (!noprompt)
            { 
                std::cout << totalBirdCounder << " bird" << (totalBirdCounder > 1 ? "s babble" : " babbles") << " : "; // tweet is "reserved" !
            }
            if (target != "")
            {
                std::ofstream ofs(target, std::ios::out | std::ios::binary);
                std::vector<byte> bytes = NumbersToByteStream(contexts.top().outputs, false);
                ofs.write((const char*) bytes.data(), bytes.size());
                ofs.close();

                std::cout << target << " file created" << std::endl;
            }
            else
            {
                std::cout << NumbersToCompositeString(contexts.top().outputs);  // Nibble ok
                if (!noprompt)
                {
                    std::cout << std::endl;
                }
            }
        }
        else
        {
            std::cout << "No output no chocolate..." << std::endl;
        }
    }
    catch (const RuntimeException &re)
    {
        std::cout << "Ouch ! " << re.Message() << std::endl;
        //TODO: [future] Anything else ?
    }

    return 0;
}
