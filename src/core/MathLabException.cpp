#include "MathLabException.h"
#include "Logger.h"

MathLabException::MathLabException(const string &msg):fullMessage("[MathLab error] "+msg) {
    Logger::getInstance().log("EXCEPTION: " + fullMessage, "Exception");
}

DimensionMismatchException::DimensionMismatchException(const string &msg):MathLabException("Incompatible dimensions -> " + msg) {}

CalculationException::CalculationException(const string &msg,const unsigned int index):MathLabException("Calculation error -> " + msg + " (at index "+to_string(index)+")") {}

ParseException::ParseException(const string &msg):MathLabException("Parse error -> " + msg) {}

FileException::FileException(const string &msg):MathLabException("I/O error -> " + msg) {}

IndexException::IndexException(const string &msg):MathLabException("Index out of bounds -> " + msg) {}

IndexNotFoundException::IndexNotFoundException(const string &msg,const int index):MathLabException("ID " + to_string(index)+ " was not found: "+msg){}

InputException::InputException(const string &msg):MathLabException("Input error -> " + msg) {}

EmptySetException::EmptySetException(const string &msg):MathLabException("No data in the set -> "+msg) {}

const char* MathLabException::what() const noexcept {
    return fullMessage.c_str();
}

const string &MathLabException::getFullMessage() const {
    return fullMessage;
}
