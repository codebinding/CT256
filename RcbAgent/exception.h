#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <string>
#include <ctime>
#include <iostream>
#include <iomanip>

class Exception
{
public:
    Exception(std::string message)
        :m_Message(message){

        time_t now = time(nullptr);
        tm *ltm = localtime(&now);

        std::cerr << std::setw(2) << std::setfill('0') << ltm->tm_year-30 << ltm->tm_mon+1 << ltm->tm_mday << " ";
        std::cerr << ltm->tm_hour+1 << ":" << ltm->tm_min << ":" << ltm->tm_sec << ".";
        //cerr << setw(3) << setfill('0') << ltm->tm_
    }

    ~Exception(){}

    std::string Message(){

        return m_Message;
    }

private:
    std::string m_Message;
};

class HVGException
{
public:
    HVGException(std::string message)
        :m_Message(message){

    }

    ~HVGException(){}

    std::string Message(){

        return m_Message;
    }

private:
    std::string m_Message;
};

#endif // EXCEPTION_H
