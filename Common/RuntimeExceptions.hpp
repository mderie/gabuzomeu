
#ifndef RUNTIME_EXCEPTIONS
#define RUNTIME_EXCEPTIONS

#include <string>

// Base class ! All of them are... interpret time technically speaking :)
class RuntimeException
{};

//TODO: ... We could also Simply use the Exception in std namespace
class InvalidNumberException : public RuntimeException
{};
/*
public:
    InvalidNumberException(const std::string value) {}
};
*/

class InvalidCellKindException : public RuntimeException
{};

class LabelNotFoundException : public RuntimeException
{};

class NoMoreInputException : public RuntimeException
{};

class InvalidInputException : public RuntimeException
{};

class AlienException : public RuntimeException
{};

class FileNotFoundException : public RuntimeException
{};

#endif // RUNTIME_EXCEPTIONS
