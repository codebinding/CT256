#include "socketcan.h"
#include "exception.h"
#include "fpgabridge.h"
#include "xraymanager.h"

#include <iostream>
#include <ctime>
#include <chrono>

#include "scanparameter.h"

using namespace std;
using namespace std::chrono;

int main()
{
    SocketCAN mySocket;
    RCBPacket myRCBPacket;
    FPGABridge myFpga;

    try {

        mySocket.Init();

        XrayManager myXrayManager(mySocket, myFpga);

        while (true) {

            mySocket.GetRequest(myRCBPacket);

            uint16_t myModule = myRCBPacket.Command >> 6;

            switch (myModule) {

            case MODULE_GENERAL:
                break;

            case MODULE_XRAY:

                myXrayManager.ProcessRequest(myRCBPacket);
                break;

            case MODULE_COLLIMATOR:
                break;

            case MODULE_DETECTOR:
                break;

            case MODULE_DEBUG:
                break;
            }
        }


    } catch (Exception e) {

        myRCBPacket.Completed = false;
        mySocket.PutResponse(myRCBPacket);
        cout << e.Message() << endl;
    }

}


