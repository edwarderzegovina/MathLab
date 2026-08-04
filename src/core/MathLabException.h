#ifndef PROIECT02_MATHLABEXCEPTION_H
#define PROIECT02_MATHLABEXCEPTION_H

#include <exception>
#include <string>
using namespace std;

class MathLabException:public exception {
    string fullMessage;
public:
    explicit MathLabException(const string& msg);
    const char* what() const noexcept override;
    const string& getFullMessage() const;
};

class DimensionMismatchException: public MathLabException {
public:
    explicit DimensionMismatchException(const string& msg);
};

class CalculationException: public MathLabException {
public:
    explicit CalculationException(const string& msg,unsigned int index);
};

class ParseException: public MathLabException {
public:
    explicit ParseException(const string& msg);
};

class FileException: public MathLabException {
public:
    explicit FileException(const string& msg);
};

class IndexException:public MathLabException {
public:
    explicit IndexException(const string& msg);
};

class IndexNotFoundException:public MathLabException {
public:
    explicit IndexNotFoundException(const string& msg,int index);
};

// Recovering the input stream (clear()/ignore()) after this is thrown is
// ConsoleApp's job, done in its catch handlers -- a GUI has no terminal input
// to recover, and core must not name one, so what() itself stays inert.
class InputException:public MathLabException {
public:
    explicit InputException(const string& msg);
};

class EmptySetException: public MathLabException {
public:
    explicit EmptySetException(const string& msg);
};

#endif //PROIECT02_MATHLABEXCEPTION_H