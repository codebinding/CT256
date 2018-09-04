#include "xraymanager.h"
#include "log.h"

XrayManager::XrayManager(SocketCAN socketCAN, FPGABridge fpgaBridge)
{
    m_SocketCAN = socketCAN;
    m_FpgaBridge = fpgaBridge;
}

void XrayManager::ProcessRequest(RCBPacket request){

    switch (request.Command) {

    case HV_INIT:

        hvInit(request);
        break;

    case HV_START:

        hvStart(request);
        break;

    case HV_STOP:

        hvStop(request);
        break;

    case HV_PREPARE:

        hvPrepare(request);
        break;

    case HV_EXPOSE:

        hvExpose(request);
        break;

    }
}

void XrayManager::hvInit(RCBPacket packet){

    Log::Print("Init ...");

    try {

        std::time_t myNow = std::time(nullptr);
        std::tm *myLocalTime = std::localtime(&myNow);

        uint8_t myYear = static_cast<uint8_t>(myLocalTime->tm_year - 100);
        uint8_t myMonth = static_cast<uint8_t>(myLocalTime->tm_mon + 1);
        uint8_t myDay = static_cast<uint8_t>(myLocalTime->tm_mday);
        uint8_t myHour = static_cast<uint8_t>(myLocalTime->tm_hour);
        uint8_t myMinute = static_cast<uint8_t>(myLocalTime->tm_min);
        uint8_t mySecond = static_cast<uint8_t>(myLocalTime->tm_sec);

        m_FpgaBridge.AdbAuthenticateBegin(myYear, myMonth, myDay, myHour, myMinute, mySecond);

        m_FpgaBridge.AdbAuthenticateStatus();

        m_FpgaBridge.HvgWaitOnCommunication(20);

        m_FpgaBridge.HvgSetTime(myYear, myMonth, myDay, myHour, myMinute, mySecond);

        m_FpgaBridge.HvgInitialize();

        m_FpgaBridge.HvgWaitOnStandby(20);

        packet.Completed = true;

    } catch (...) {

        packet.Completed = false;
    }

    m_SocketCAN.PutResponse(packet);

    Log::Print("Init Done");
}

void XrayManager::hvStart(RCBPacket packet){

    Log::Print("Start ...");

    try {

        m_FpgaBridge.HvgDummyExposure();

        m_FpgaBridge.HvgWaitPreparedScan(2);

        packet.Completed = true;

    } catch (...) {

        packet.Completed = false;
    }

    m_SocketCAN.PutResponse(packet);

    Log::Print("Start Done");
}

void XrayManager::hvStop(RCBPacket packet){

    Log::Print("Stop ...");

    try {

        m_FpgaBridge.HvgEndScan();

        packet.Completed = true;

    } catch (...) {

        packet.Completed = false;
    }

    m_SocketCAN.PutResponse(packet);

    Log::Print("Stop Done");
}

void XrayManager::hvPrepare(RCBPacket packet){

    Log::Print("Prepare ...");

    try {

        if (packet.Parameter == 0){

            ScanParameter::TriggerMode = static_cast<uint8_t>(packet.Data8[7] & 0x07ul);

            ScanParameter::ZDitherEnabled = (packet.Data8[7] & 0x08) == 0x08;

            ScanParameter::XDitherEnabled = (packet.Data8[7] & 0x10) == 0x10;

            ScanParameter::ImaEnabled = (packet.Data8[7] & 0x20) == 0x20;

            ScanParameter::FocalSpotSize = static_cast<FSS>(packet.Data8[7] >> 6);

            ScanParameter::ExposureTimeInMSec = static_cast<uint32_t>(packet.Data64 >> 24 & 0xfffffful);

            ScanParameter::Ma = static_cast<uint16_t>(packet.Data64 >> 8 & 0xfffful);

            ScanParameter::Kv = packet.Data8[0];

            ScanParameter::PreviousPacket = packet.Parameter;
        }
        else if (packet.Parameter == 1){

            if (ScanParameter::PreviousPacket != packet.Parameter - 1){

                throw new Exception("prepare packet out of order");
            }

            ScanParameter::IntegrationLimit = static_cast<uint32_t>(packet.Data64 >> 32);

            ScanParameter::IntegrationTime = static_cast<uint32_t>(packet.Data64);

            ScanParameter::PreviousPacket = packet.Parameter;
        }
        else if (packet.Parameter == 2){

            if (ScanParameter::PreviousPacket != packet.Parameter - 1){

                throw new Exception("prepare packet out of order");
            }

            ScanParameter::TriggerPosition = static_cast<uint32_t>(packet.Data64);

            ScanParameter::PreviousPacket = ScanParameter::ImaEnabled? LAST_PACKET : packet.Parameter;
        }
        else{

            // dealing with imA table
        }

        if (ScanParameter::PreviousPacket == LAST_PACKET){

            m_FpgaBridge.Prepare();

            packet.Completed = true;

            m_SocketCAN.PutResponse(packet);
        }

    } catch (...) {

        packet.Completed = false;

        m_SocketCAN.PutResponse(packet);
    }

    Log::Print("Prepare Done");
}

void XrayManager::hvExpose(RCBPacket packet){

    Log::Print("Expose ...");

    try {

        m_FpgaBridge.HvgArm();

        hvNotifyXRay(true);

    } catch (...) {

    }
    Log::Print("Expose Done");
}

void XrayManager::hvNotifyXRay(bool isOn){

    RCBPacket myPacket;

    myPacket.Command = HV_XRAYONOFF;
    myPacket.Completed = true;
    myPacket.Parameter = 0;
    myPacket.Data64 = isOn? 0x01 : 0x00;

    m_SocketCAN.PutResponse(myPacket);
}









