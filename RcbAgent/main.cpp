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
    RCBPacket myPacket;
    FPGABridge myFpga;

    try {

        mySocket.Init();

        while (true) {

            mySocket.GetRequest(myPacket);

            switch (myPacket.Command) {

            case HV_INIT:

                myFpga.Init();

                mySocket.PutResponse(myPacket);
                break;

            case HV_START:

                myFpga.Start();

                mySocket.PutResponse(myPacket);
                break;

            case HV_STOP:

                myFpga.Stop();

                mySocket.PutResponse(myPacket);
                break;

            case HV_PREPARE:

                if (myPacket.Parameter == 0){

                    ScanParameter::TriggerMode = static_cast<uint8_t>(myPacket.Data & 0x07ul);
                    myPacket.Data >>= 3;

                    ScanParameter::ZDitherEnabled = (myPacket.Data & 0x01ul) == 0x01ul;
                    myPacket.Data >>= 1;

                    ScanParameter::XDitherEnabled = (myPacket.Data & 0x01ul) == 0x01ul;
                    myPacket.Data >>= 1;

                    ScanParameter::ImaEnabled = (myPacket.Data & 0x01ul) == 0x01ul;
                    myPacket.Data >>= 1;

                    ScanParameter::FocalSpotSize = static_cast<FSS>(myPacket.Data & 0x03ul);
                    myPacket.Data >>= 10;

                    ScanParameter::ExposureTimeInMSec = static_cast<uint32_t>(myPacket.Data & 0xfffffful);
                    myPacket.Data >>= 24;

                    ScanParameter::Ma = static_cast<uint16_t>(myPacket.Data & 0xfffful);
                    myPacket.Data >>= 16;

                    ScanParameter::Kv = static_cast<uint8_t>(myPacket.Data & 0xfful);

                    ScanParameter::PreviousPacket = myPacket.Parameter;
                }
                else if (myPacket.Parameter == 1){

                    if (ScanParameter::PreviousPacket != myPacket.Data - 1){

                        throw new Exception("prepare packet out of order");
                    }

                    ScanParameter::IntegrationLimit = static_cast<uint32_t>(myPacket.Data);
                    myPacket.Data >>= 32;

                    ScanParameter::IntegrationTime = static_cast<uint32_t>(myPacket.Data);

                    ScanParameter::PreviousPacket = myPacket.Parameter;
                }
                else if (myPacket.Parameter == 2){

                    if (ScanParameter::PreviousPacket != myPacket.Data - 1){

                        throw new Exception("prepare packet out of order");
                    }

                    ScanParameter::TriggerPosition = static_cast<uint32_t>(myPacket.Data >> 32);

                    ScanParameter::PreviousPacket = ScanParameter::ImaEnabled? LAST_PACKET : myPacket.Parameter;
                }
                else{

                    // dealing with imA table
                }

                if (ScanParameter::PreviousPacket == LAST_PACKET){

                    myFpga.Prepare();

                    myPacket.Parameter = 0;
                    myPacket.Completed = true;

                    mySocket.PutResponse(myPacket);
                }

                break;

            case HV_EXPOSE:

                myFpga.Expose();

            }

        }


    } catch (Exception e) {

        myPacket.Completed = false;
        mySocket.PutResponse(myPacket);
        cout << e.Message() << endl;
    }

}


