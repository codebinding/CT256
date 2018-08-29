#include "socketcan.h"
#include "exception.h"
#include "fpgabridge.h"

#include <iostream>
#include <ctime>
#include <chrono>

#include "scanparameter.h"

using namespace std;
using namespace std::chrono;

int main()
{
    SocketCAN mySocket;

    try {

        //mySocket.Init();

        //time_t Now = system_clock::to_time_t(system_clock::now());
        //cout << ctime(&Now) << endl;
        system_clock::time_point start = system_clock::now();
        system_clock::time_point end = start + milliseconds(500);
        while(system_clock::now() < end);

        //Now = system_clock::to_time_t(system_clock::now());
        //cout << ctime(&Now) << endl;

        cout << duration_cast<microseconds>(system_clock::now() - start).count() << "ms" << endl;

        typedef duration<int, ratio<365*24*60*60>> year;
        system_clock::duration epoch = system_clock::now().time_since_epoch();

        cout << setw(2) << setfill('0') << duration_cast<year>(epoch).count() - 30;
        //cout << duration_cast<milliseconds>(t.time_since_epoch()).count() - duration_cast<seconds>(t.time_since_epoch()).count()*1000 << endl;

        FSS enumFss = FSS::Large;
        unsigned uintFss = 5;

        cout << "enum value: " << enumFss << " uint value: " << static_cast<unsigned>(enumFss) << endl;

        enumFss = static_cast<FSS>(uintFss);
        cout << "uint value: " << uintFss << " enum value: " << enumFss << endl;

    } catch (Exception e) {

        cout << e.Message() << endl;
    }

}


