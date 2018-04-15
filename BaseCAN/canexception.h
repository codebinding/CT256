#ifndef CANEXCEPTION_H
#define CANEXCEPTION_H

#include <string>

class CANException {

public:
    CANException(std::string message)
        : m_s(message) {}

    ~CANException(){}

    std::string Message(){

        return m_s;
    }

private:
    std::string m_s;
};
#endif // CANEXCEPTION_H
