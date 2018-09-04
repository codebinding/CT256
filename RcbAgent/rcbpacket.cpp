#include "rcbpacket.h"

RCBPacket::RCBPacket()
    : Command(0), Parameter(0), Completed(false), Data64(0) {

}

void RCBPacket::EncodeCANPacket(CANPacket& packet){

    uint32_t myId = static_cast<uint32_t>(Command)<<18 | (Completed? 0x00u : 0x01u)<<16 | Parameter;

    packet.SetFrame(myId, Data64);
}

void RCBPacket::DecodeCANPacket(CANPacket &packet){

    uint32_t myId = packet.GetId();
    Command = static_cast<uint16_t>(myId >> 18);
    Completed = static_cast<bool>(myId >> 16 & 0x01);
    Parameter = static_cast<uint16_t>(myId);

    packet.GetData(Data64);
}
