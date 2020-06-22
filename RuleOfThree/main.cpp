
#include <iostream>
#include <vector>
#include <stack>
#include <array>

// Third party
#include "..\Common\bnflite.hpp"
#include "..\Common\cxxopts.hpp"
#include "..\Common\infint.hpp"

// Home made
#include "..\Common\RuntimeExceptions.hpp"
#include "..\Common\ConverterTools.hpp"
#include "..\..\Common\StringTools.hpp"

enum class OpCode { last, jump, pump, dump, free, bird, lift, calc, head, tail, zero, else_, gbzm, base, last_item }; // Care of the else keyword !
const std::string OpCodes[(int) OpCode::last_item] = { "LAST", "JUMP", "PUMP", "DUMP", "FREE", "BIRD", "LIFT", "CALC", "HEAD", "TAIL", "ZERO", "ELSE", "GBZM", "BASE" };
struct Instruction
{
    OpCode opCode;
    std::string operand1;
    std::string operand2;
    Instruction(OpCode opc, const std::string& op1, const std::string& op2) : opCode(opc), operand1(op1), operand2(op2) {}
};
std::vector<Instruction> instructions; // Not in the context :)

enum class CellKind { Body, Head, Tail }; // the Body is optional :)
struct Cell
{
    CellKind kind;
    InfInt value; // Anyway holds either the index of anoter bird or just a number
};

struct Bird
{
    //Cell ga, bu, zo, meu;
    Cell cells[(int) CellId::last_item];
    // Far better !!!
    //std::array<Cell, (size_t) CellId::last_item> cells_; 

    Bird()
    {
        for (auto& it : cells)
        {
            it.kind = CellKind::Body;
            it.value = 0;
        }
        // But we still need to cast :(
        //cells_[(size_t) CellId::meu].kind = CellKind::Body; 
    }
};

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
        birds.clear();
        birds.emplace_back(Bird());
    }
};
std::stack<Context> contexts;

// Deep copy test, see https://en.cppreference.com/w/cpp/language/rule_of_three
int main(int argc, char* argv[])
{
    contexts.push(Context());
    contexts.top().birds[contexts.top().birdPointer].cells[(int) CellId::ga].value = 31; // Boring (int) cast for array access... Could template usage performs some magic ?
    contexts.push(Context());
    contexts.pop();
    std::cout << "...value = " << contexts.top().birds[contexts.top().birdPointer].cells[(int)CellId::ga].value << std::endl;

	return 0;
}
