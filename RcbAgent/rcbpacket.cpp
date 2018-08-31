#include "rcbpacket.h"

RCBPacket::RCBPacket()
{

}

void RCBPacket::EncodeCANPacket(CANPacket& packet){

    uint32_t myId = static_cast<uint32_t>(Command)<<18 | (Completed? 0x00u : 0x01u)<<16 | Parameter;

    packet.SetFrame(myId, Data);
}

void RCBPacket::DecodeCANPacket(CANPacket& packet){

    uint32_t myId = packet.GetId();
    Command = static_cast<uint16_t>(myId >> 18);
    Completed = static_cast<bool>(myId >> 16 & 0x01);
    Parameter = static_cast<uint16_t>(myId);

    uint8_t myData[8];
    packet.GetData(myData);

    for (unsigned i=0; i<8; i++){

        Data = Data<<8 | myData[i];
    }
}
