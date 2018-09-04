#ifndef CANPACKET_H
#define CANPACKET_H

#include <linux/can.h>
#include <string>

class CANPacket
{
public:

    CANPacket();

    uint8_t DataLength = 8;

    uint32_t GetId();
    void GetData(uint8_t data[8]);
    void GetData(uint64_t& data);

    void SetId(const uint32_t id);
    void SetData(const uint8_t data[8]);
    void SetRawPacket(const can_frame& frame);

    void SetFrame(const uint32_t id, const uint8_t data[8]);
    void SetFrame(const uint32_t id, const uint64_t data);
    void SetFrame(const uint32_t id, const uint8_t start_bit, const uint8_t length, const uint64_t data);

    std::string ToString();

protected:
    uint32_t m_id;

    // b63...b56, b55...b48, b47...b40, b39...b32, b31...b24, b23...b16, b15...b08, b07...b00
    //    [7]        [6]        [5]        [4]        [3]        [2]        [1]        [0]
    union{

        uint8_t m_data8[8];
        uint64_t m_data64;
    };
};

#endif // CANPACKET_H
