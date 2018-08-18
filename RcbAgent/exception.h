#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <string>

class Exception
{
public:
    Exception(std::string message)
        :m_Message(message){

    }

    ~Exception(){}

    std::string Message(){

        return m_Message;
    }

private:
    std::string m_Message;
};

#endif // EXCEPTION_H
