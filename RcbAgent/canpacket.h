#ifndef CANPACKET_H
#define CANPACKET_H

#include "inttypes.h"

#include <linux/can.h>
#include <string>

class CANPacket
{
public:

    const unsigned char DataLength = 8;

    uint32_t GetId();
    void GetData(uint8_t data[8]);

    void SetId(uint32_t id);
    void SetData(uint8_t data[8]);
    void SetRawPacket(can_frame frame);

    void SetFrame(uint32_t id, uint8_t data[8]);
    void SetFrame(uint32_t id, uint32_t data[8]);
    void SetFrame(uint32_t id, uint64_t data);
    void SetFrame(uint32_t id, uint8_t start_bit, uint8_t length, uint64_t data);

    std::string ToString();

protected:
    uint32_t m_id;
    uint8_t m_data[8];
    // b63...b56, b55...b48, b47...b40, b39...b32, b31...b24, b23...b16, b15...b08, b07...b00
    //    [0]        [1]        [2]        [3]        [4]        [5]        [6]        [7]
};

#endif // CANPACKET_H
