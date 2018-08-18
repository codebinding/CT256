#include "socketcan.h"
#include "exception.h"

#include <iostream>
#include <ctime>
#include <chrono>

using namespace std;
using namespace std::chrono;

int main()
{
    SocketCAN mySocket;

    try {

        uint8_t *a = new uint8_t(10);

        cout << sizeof (a) << endl;
        //mySocket.Init();

        //time_t Now = system_clock::to_time_t(system_clock::now());
        //cout << ctime(&Now) << endl;
        system_clock::time_point start = system_clock::now();
        system_clock::time_point end = start + milliseconds(500);
        while(system_clock::now() < end);

        //Now = system_clock::to_time_t(system_clock::now());
        //cout << ctime(&Now) << endl;

        cout << duration_cast<microseconds>(system_clock::now() - start).count() << "ms" << endl;

    } catch (Exception e) {

        cout << e.Message() << endl;
    }

}
