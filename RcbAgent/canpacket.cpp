#include "canpacket.h"
#include "exception.h"

#include <sstream>
#include <iomanip>

uint32_t CANPacket::GetId(){

    return m_id;
}

void CANPacket::GetData(uint8_t data[8]){

    for(int i = 0; i < 8; i++){

        data[i] = m_data[i];
    }
}

void CANPacket::SetId(uint32_t id){

    m_id = id;
}

void CANPacket::SetData(uint8_t data[8]){

    for(int i = 0; i < 8; i++){

        m_data[i] = data[i];
    }
}

void CANPacket::SetRawPacket(can_frame frame){

    m_id = frame.can_id & CAN_EFF_MASK;
    for(int i = 0; i < 8; i++){

        m_data[i] = frame.data[i];
    }
}

std::string CANPacket::ToString(){

    std::ostringstream oss;

    oss << "id: " << std::hex << std::setw(8) << std::setfill('0') << m_id << " data: ";

    for(int i = 0; i < 8; i++){

        oss << std::setw(2) << std::setfill('0') << static_cast<int>(m_data[i]) << ((i!=7) ? "-" : "");
    }

    return oss.str();
}

void CANPacket::SetFrame(uint32_t id, uint8_t data[]){

    m_id = id;

    for(int i = 0; i < 8; i++){

        m_data[i] = data[i];
    }
}

void CANPacket::SetFrame(uint32_t id, uint32_t data[]){

    m_id = id;

    for(int i = 0; i < 8; i++){

        m_data[i] = static_cast<uint8_t>(data[i]);
    }
}

void CANPacket::SetFrame(uint32_t id, uint64_t data){

    m_id = id;

    for(int i = 0; i < 8; i++){

        m_data[i] = static_cast<uint8_t>(data >> ((7 - i) * 8));
    }
}

void CANPacket::SetFrame(uint32_t id, uint8_t start_bit, uint8_t bit_length, uint64_t data){

    if( start_bit > 63 ){

        throw Exception("start_bit should be in the range of 0 .. 63");
    }

    if( bit_length > 64 ){

        throw Exception("bit_length should be in the range of 0 .. 63");
    }

    if( start_bit < bit_length - 1 ){

        throw Exception("start_bit should be less than bit_length - 1");
    }

    m_id = id;

    uint64_t my_mask = (0x01ul << bit_length) - 1;

    data = (data & my_mask) << (start_bit - bit_length + 1);

    for(int i = 0; i < 8; i++){

        m_data[i] = m_data[i] | static_cast<uint8_t>(data >> ((7 - i) * 8));
    }
}
