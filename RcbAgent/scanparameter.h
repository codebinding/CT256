#ifndef SCANPARAMETER_H
#define SCANPARAMETER_H

#include <cstdint>

enum FSS {Small, Medium, Large};

class ScanParameter
{
public:

    ScanParameter();

    static uint8_t Kv;
    static uint16_t Ma;

    static FSS FocalSpotSize;   //
    static uint8_t GeneratorMode;   // 0x00=Normal, 0x69=PreAdjust, 0x52=FinalAdjust
    static uint8_t AnodeSpeed;      // 0 ~ 255Hz

    static uint32_t ExposureTimeInMSec;
    static uint32_t ScanTimeInMSec;

    static bool ImaEnabled;
    static uint16_t ImaStartingMa;
    static uint16_t ImaMaxMa;

    static bool XDitherEnabled;
    static bool ZDitherEnabled;

    static uint32_t IntegrationLimit;
    static unsigned IntegrationTime;

    static unsigned TriggerMode;
    static uint32_t TriggerPosition;
};
#endif // SCANPARAMETER_H
