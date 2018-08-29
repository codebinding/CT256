#ifndef REGISTER_H
#define REGISTER_H

#include <cstdint>

struct reg_t {

    // 0x0000 Version
    uint64_t MinorVersion       : 16;
    uint64_t SubVersion         :  8;
    uint64_t MajorVersion       :  8;
    uint64_t VersionDay         :  8;
    uint64_t VersionMonth       :  8;
    uint64_t VersionYear        : 16;

    // 0x0008 SystemControl
    uint64_t ControlGeneral     : 32;
    uint64_t                    : 31;
    uint64_t SoftResetBit       :  1;

    // 0x0010 SystemStatus (Debug)
    uint64_t SystemStatus       : 64;

    // 0x0018 Position
    uint64_t TablePosition      : 32;
    uint64_t GantryPosition     : 32;

    // 0x0020 TriggerParameter
    uint64_t First2LastViews    : 32;
    uint64_t Start2FirstViews   : 16;
    uint64_t TriggerInterval    : 16;

    // 0x0028 TriggerControl
    uint64_t TriggerPosition    : 32;
    uint64_t IsRelative         :  1;
    uint64_t TriggerMode        :  2;
    uint64_t                    : 29;

    // 0x0030 ~ 0x00F8 Reserved
    uint64_t                    : 64;
    uint64_t                    : 64;
    uint64_t                    : 64;
    uint64_t                    : 64;
    uint64_t                    : 64;
    uint64_t                    : 64;
    uint64_t                    : 64;

    // 0x0100 ExposureTime
    uint64_t ExposureTime       : 32;
    uint64_t                    : 31;
    uint64_t                    :  1;

    // 0x0108 HvCanTxId
    uint64_t HvCanTxId          : 29;
    uint64_t                    : 35;

    // 0x0110 HvCanTxData
    uint64_t HvCanTxData        : 64;

    // 0x0118 HvCanRxId
    uint64_t HvCanRxId          : 29;
    uint64_t                    : 35;

    // 0x0120 HvCanRxData
    uint64_t HvCanRxData        : 64;

    // 0x0128 HvCanCtrl
    uint64_t HvCanTxStart       :  1;
    uint64_t                    : 60;
    uint64_t HvCanRxReady       :  1;
    uint64_t HvCanTxReady       :  1;
    uint64_t HvCanInitStatus    :  1;

    // 0x0130 HvDioCtrl
    uint64_t HvImaStart         :  1;
    uint64_t HvUseIma           :  1;
    uint64_t                    : 58;
    uint64_t HvDoseOk           :  1;
    uint64_t HvXrayEnable       :  1;
    uint64_t HvXrayRelease      :  1;
    uint64_t HvXrayOn           :  1;

    // 0x0138 HvImaTableInput
    uint64_t HvImaInterval      : 31;
    uint64_t HvImaMode          :  1;
    uint64_t HvImaCrc8          :  8;
    uint64_t HvImaMa            : 16;
    uint64_t HvImaCommand       :  8;

    // 0x0140 ~ 0x017F TcuRequest 8 Bytes/Registers
    uint64_t TcuRequest0        : 64;
    uint64_t TcuRequest1        : 64;
    uint64_t TcuRequest2        : 64;
    uint64_t TcuRequest3        : 64;
    uint64_t TcuRequest4        : 64;
    uint64_t TcuRequest5        : 64;
    uint64_t TcuRequest6        : 64;
    uint64_t TcuRequest7        : 64;

    // 0x0180 ~ 0x01BF TcuRequest 8 Bytes/Registers
    uint64_t TcuResponse0       : 64;
    uint64_t TcuResponse1       : 64;
    uint64_t TcuResponse2       : 64;
    uint64_t TcuResponse3       : 64;
    uint64_t TcuResponse4       : 64;
    uint64_t TcuResponse5       : 64;
    uint64_t TcuResponse6       : 64;
    uint64_t TcuResponse7       : 64;

    // 0x01C0 TcuControl
    uint64_t TcuTxStart         :  1;
    uint64_t TcuTxLen           :  6;
    uint64_t TcuRxLen           :  6;
    uint64_t TcuWatchDogStart   :  1;
    uint64_t TcuWatchDogFreq    : 16;
    uint64_t                    : 32;
    uint64_t TcuRxReady         :  1;
    uint64_t TcuTxReady         :  1;

    // 0x01C8 TcuDioCtrl
    uint64_t TcuFocusOn         :  1;
    uint64_t TcuFocusA          :  4;
    uint64_t TcuFocusB          :  4;
    uint64_t TcuDitherStart     :  1;
    uint64_t TcuReset           :  1;
    uint64_t                    : 52;
    uint64_t TcuErrorFlag       :  1;

    // 0x01D0 ~ 0x218 AdbRequest 10 Bytes/Registers
    uint64_t AdbRequest0        : 64;
    uint64_t AdbRequest1        : 64;
    uint64_t AdbRequest2        : 64;
    uint64_t AdbRequest3        : 64;
    uint64_t AdbRequest4        : 64;
    uint64_t AdbRequest5        : 64;
    uint64_t AdbRequest6        : 64;
    uint64_t AdbRequest7        : 64;
    uint64_t AdbRequest8        : 64;
    uint64_t AdbRequest9        : 64;

    // 0x0220 ~ 0x0268 AdbResponse 10 Bytes/Registers
    uint64_t AdbResponse0       : 64;
    uint64_t AdbResponse1       : 64;
    uint64_t AdbResponse2       : 64;
    uint64_t AdbResponse3       : 64;
    uint64_t AdbResponse4       : 64;
    uint64_t AdbResponse5       : 64;
    uint64_t AdbResponse6       : 64;
    uint64_t AdbResponse7       : 64;
    uint64_t AdbResponse8       : 64;
    uint64_t AdbResponse9       : 64;

    // 0x0270 AdbControl
    uint64_t AdbTxStart         :  1;
    uint64_t AdbTxLen           :  8;
    uint64_t AdbRxLen           :  8;
    uint64_t                    : 44;
    uint64_t AdbRxReady         :  1;
    uint64_t AdbTxReady         :  1;

    // 0x0278 ~ 0x02F8 Reserved
    uint64_t                    : 64;
    uint64_t                    : 64;
    uint64_t                    : 64;
    uint64_t                    : 64;
    uint64_t                    : 64;
    uint64_t                    : 64;
    uint64_t                    : 64;
    uint64_t                    : 64;
    uint64_t                    : 64;
    uint64_t                    : 64;
    uint64_t                    : 64;
    uint64_t                    : 64;
    uint64_t                    : 64;
    uint64_t                    : 64;
    uint64_t                    : 64;
    uint64_t                    : 64;
    uint64_t                    : 64;
    uint64_t                    : 64;

    // 0x0300 ~ 0x0318 Collimator Request 4 Bytes/Registers
    uint64_t ClmRequest0        : 64;
    uint64_t ClmRequest1        : 64;
    uint64_t ClmRequest2        : 64;
    uint64_t ClmRequest3        : 64;

    // 0x0320 Collimator Response FIFO
    uint64_t ClmResponse        : 64;

    // 0x0328 Collimator Control
    uint64_t ClmTxStart         :  1;
    uint64_t ClmTxLen           :  5;
    uint64_t                    : 50;
    uint64_t ClmRxLen           :  7;
    uint64_t ClmTxReady         :  1;
};

#endif // REGISTER_H
