#include "canframe8.h"
#include "canexception.h"

#include <sstream>
#include <iomanip>

CANFrame8::CANFrame8(){

}

CANFrame8::CANFrame8(uint16_t id, uint64_t data){

    SetFrame(id, data);
}

CANFrame8::~CANFrame8(){

}

uint16_t CANFrame8::GetId(){

    return m_id;
}

void CANFrame8::GetData(uint8_t data[]){

    for(int i = 0; i < 8; i++){

        data[i] = m_data[i];
    }
}

void CANFrame8::SetId(uint16_t id){

    m_id = id;
}

void CANFrame8::SetData(uint8_t data[]){

    for(int i = 0; i < 8; i++){

        m_data[i] = data[i];
    }
}

std::string CANFrame8::ToString(){

    std::ostringstream oss;

    oss << "id: " << std::hex << std::setw(3) << std::setfill('0') << m_id << " data: ";

    for(int i = 0; i < 8; i++){

        oss << std::setw(2) << std::setfill('0') << (unsigned short)m_data[i] << ((i!=7) ? "-" : "");
    }

    return oss.str();
}

void CANFrame8::SetFrame(uint16_t id, uint8_t data[8]){

    m_id = id;

    for(int i = 0; i < 8; i++){

        m_data[i] = data[i];
    }
}

void CANFrame8::SetFrame(uint16_t id, uint64_t data){

    m_id = id;

    for(int i = 0; i < 8; i++){

        m_data[i] = data >> ((7 - i) * 8);
    }
}

void CANFrame8::SetFrame(uint16_t id, uint8_t start_bit, uint8_t bit_length, uint64_t data){

    if( start_bit > 63 ){

        throw CANException("start_bit should be in the range of 0 .. 63");
    }

    if( bit_length > 64 ){

        throw CANException("bit_length should be in the range of 0 .. 63");
    }

    if( start_bit < bit_length - 1 ){

        throw CANException("start_bit should be less than bit_length - 1");
    }

    m_id = id;

    uint64_t my_mask = ((uint64_t)1 << bit_length) - 1;

    data = (data & my_mask) << (start_bit - bit_length + 1);

    for(int i = 0; i < 8; i++){

        m_data[i] = m_data[i] | (data >> ((7 - i) * 8));
    }
}

