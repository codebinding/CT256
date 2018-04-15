#ifndef CANFRAME8_H
#define CANFRAME8_H

#include "inttypes.h"
#include <string>

class CANFrame8
{
public:
    CANFrame8();
    CANFrame8(uint16_t id, uint64_t data);
    virtual ~CANFrame8();

    uint16_t GetId();
    void GetData(uint8_t data[8]);

    void SetId(uint16_t id);
    void SetData(uint8_t data[8]);

    void SetFrame(uint16_t id, uint8_t data[8]);
    void SetFrame(uint16_t id, uint64_t data);
    void SetFrame(uint16_t id, uint8_t start_bit, uint8_t length, uint64_t data);

    std::string ToString();

protected:
    uint16_t m_id;
    uint8_t m_data[8];
    // b63...b56, b55...b48, b47...b40, b39...b32, b31...b24, b23...b16, b15...b08, b07...b00
    //    [0]        [1]        [2]        [3]        [4]        [5]        [6]        [7]
};

#endif // CANFRAME8_H
