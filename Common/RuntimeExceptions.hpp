
#ifndef RUNTIME_EXCEPTIONS
#define RUNTIME_EXCEPTIONS

#include <string>

// CARE : in VS 2019 Enable C++ must be set to No !!!

// Base class ! All of them are... interpret time technically speaking :)
//TODO: Some exceptions may happen before the Gabuzomeu program starts... Review exception naming !
class RuntimeException
{
protected:
    std::string m_className;
    std::string m_message;
    std::string m_location; //TODO: Finalize this :)
public:
    std::string Message() const { return m_className + (m_message != "" ? (" : " + m_message) : ""); }
};

//TODO: ... We could also simply use the Exception in std namespace
class InvalidNumberException : public RuntimeException
{
public:
    // Oddly enough this does not compile...
    // InvalidNumberException(const std::string &message) : RuntimeException::m_message(message) { ... }
    InvalidNumberException(const std::string& message)
    {
        m_message = message;
        m_className = "Invalid number";
    }    
};

class OverflowException : public RuntimeException
{
public:
    OverflowException(const std::string &message)
    {
        m_message = message;
        m_className = "Overflow";
    }
};

class InvalidCellKindException : public RuntimeException
{
public:
    InvalidCellKindException(const std::string &message)
    {
        m_message = message;
        m_className = "Invalid cell kind";
    }
};

class LabelNotFoundException : public RuntimeException
{
public:
    LabelNotFoundException(const std::string& message)
    {
        m_message = message;
        m_className = "Label not found";
    }
};

class NoMoreInputException : public RuntimeException
{
public:
    NoMoreInputException(const std::string &message)
    {
        m_message = message;
        m_className = "No more input";
    }
};

class InvalidInputException : public RuntimeException
{
public:
    InvalidInputException(const std::string &message)
    {
        m_message = message;
        m_className = "Invalid input";
    }
};

class AlienException : public RuntimeException
{
public:
    AlienException(const std::string &message)
    {
        m_message = message;
        m_className = "The truth is elsewhere...";
    }
};

class FileNotFoundException : public RuntimeException
{
public:
    FileNotFoundException(const std::string &message)
    {
        m_message = message;
        m_className = "File not found";
    }
};

class EmptyContainer : public RuntimeException
{
public:
    EmptyContainer(const std::string &message)
    {
        m_message = message;
        m_className = "Empty container";
    }
};

class BaseNotSupported : public RuntimeException
{
public:
    BaseNotSupported(const std::string &message)
    {
        m_message = message;
        m_className = "Base not supported";
    }
};

#endif // RUNTIME_EXCEPTIONS
