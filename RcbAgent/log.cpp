#include "log.h"

#include <iostream>
#include <iomanip>

using namespace std;

Log::Log()
{

}

void Log::Print(std::string message, const char *text){

    cout << message << text << endl;
}

void Log::Print(std::string message){

    cout << message << endl;
}

void Log::Print(std::vector<uint8_t>& data){

    cout << hex << setw(2) << setfill('0') << "len:" << data.size() << " ";

    for(auto it=data.cbegin(); it!=data.cend(); it++){

        cout << *it << " ";
    }

    cout << endl;
}

void Log::Print(std::string message, std::vector<uint8_t>& data){

    cout << message << " ";

    cout << hex << setw(2) << setfill('0') << "len:" << data.size() << " ";

    for(auto it=data.cbegin(); it!=data.cend(); it++){

        cout << *it << " ";
    }

    cout << endl;
}
