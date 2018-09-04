#ifndef XRAYMANAGER_H
#define XRAYMANAGER_H

#include "socketcan.h"
#include "fpgabridge.h"

class XrayManager
{
public:
    XrayManager(SocketCAN socketCAN, FPGABridge fpgaBridge);

    void ProcessRequest(RCBPacket request);

private:

    SocketCAN m_SocketCAN;
    FPGABridge m_FpgaBridge;

    void hvInit(RCBPacket packet);
    void hvStart(RCBPacket packet);
    void hvStop(RCBPacket packet);
    void hvPrepare(RCBPacket packet);
    void hvExpose(RCBPacket packet);
    void hvLock();
    void hvUnlock();
    void hvReset();
    void hvAbort();
    void hvWarmup();
    void hvEndScan();

    void hvNotifyXRay(bool isOn);
};

#endif // XRAYMANAGER_H
