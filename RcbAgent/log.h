#ifndef LOG_H
#define LOG_H

#include <vector>
#include <cstdint>
#include <string>

class Log
{
public:
    Log();

    static void Print(std::string message, const char*);
    static void Print(std::string message);
    static void Print(std::vector<uint8_t>& data);
    static void Print(std::string message, std::vector<uint8_t>& data);
};

#endif // LOG_H
