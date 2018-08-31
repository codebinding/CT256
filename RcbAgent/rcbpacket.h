#ifndef RCBPACKET_H
#define RCBPACKET_H

#include <cstdint>

#include "canpacket.h"

// Module Definition
const uint8_t MODULE_GENERAL        = 0x00;
const uint8_t MODULE_XRAY           = 0x01;
const uint8_t MODULE_COLLIMATOR     = 0x02;
const uint8_t MODULE_DETECTOR       = 0x03;
const uint8_t MODULE_DEBUG          = 0x04;

// General Commands
const uint16_t GN_DOOR      = MODULE_GENERAL << 6 | 0x3f;

// X-Ray Commands
const uint16_t HV_VERSION   = MODULE_XRAY << 6 | 0x00;
const uint16_t HV_INIT      = MODULE_XRAY << 6 | 0x01;
const uint16_t HV_START     = MODULE_XRAY << 6 | 0x02;
const uint16_t HV_STOP      = MODULE_XRAY << 6 | 0x03;
const uint16_t HV_PREPARE   = MODULE_XRAY << 6 | 0x04;
const uint16_t HV_EXPOSE    = MODULE_XRAY << 6 | 0x05;
const uint16_t HV_LOCK      = MODULE_XRAY << 6 | 0x06;
const uint16_t HV_RESET     = MODULE_XRAY << 6 | 0x07;
const uint16_t HV_ABORT     = MODULE_XRAY << 6 | 0x08;
const uint16_t HV_WARMUP    = MODULE_XRAY << 6 | 0x10;
const uint16_t HV_SEASONING = MODULE_XRAY << 6 | 0x11;
const uint16_t HV_FILCAL    = MODULE_XRAY << 6 | 0x12;

const uint16_t HV_TUBEHEAT  = MODULE_XRAY << 6 | 0x3d;
const uint16_t HV_STATE     = MODULE_XRAY << 6 | 0x3e;
const uint16_t HV_XRAYONOFF = MODULE_XRAY << 6 | 0x3f;

// Collimator Commands
const uint16_t CM_VERSION   = MODULE_COLLIMATOR << 6 | 0x00;
const uint16_t CM_HOME      = MODULE_COLLIMATOR << 6 | 0x01;
const uint16_t CM_SELECT    = MODULE_COLLIMATOR << 6 | 0x02;
const uint16_t CM_MOVETO    = MODULE_COLLIMATOR << 6 | 0x03;
const uint16_t CM_STOP      = MODULE_COLLIMATOR << 6 | 0x04;

const uint16_t CM_STATE     = MODULE_COLLIMATOR << 6 | 0x3e;

// Debug Commands
const uint16_t DB_READREG   = MODULE_DEBUG << 6 | 0x01;
const uint16_t DB_WRITEREG  = MODULE_DEBUG << 6 | 0x02;

// Other Constants
const uint16_t LAST_PACKET  = 0xffffu;

class RCBPacket
{
public:
    RCBPacket();

    void EncodeCANPacket(CANPacket& packet);
    void DecodeCANPacket(CANPacket& packet);

    uint16_t Command;
    uint16_t Parameter;
    bool Completed;
    uint64_t Data;
};

#endif // RCBPACKET_H
