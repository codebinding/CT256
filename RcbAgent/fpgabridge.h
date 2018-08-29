#ifndef FPGAVAREX_H
#define FPGAVAREX_H

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <errno.h>
#include <string>
#include <vector>
#include <memory>

#include "scanparameter.h"
#include "register.h"
#include "exception.h"

#define PAGE_SIZE 4096
#define HPS2FPGA_BRIDGE_BASE 0xc0000000


class FPGABridge
{
public:
    FPGABridge();
    ~FPGABridge();

    void ReadRegister(uint16_t offset, uint64_t& value);
    void WriteRegister(uint16_t offset, uint64_t value);

    void Init();
    void Start();
    void Stop();
    void Prepare();
    void Expose();
    void Lock();
    void Unlock();
    void Reset();
    void Abort();
    void Warmup();
    void EndScan();

private:
    void* m_BridgeMap;
    int m_MemoryFile;

    struct reg_t *m_Register;
    uint8_t *m_BaseAddress;

    uint8_t *m_TcuRequestAddress, *m_TcuResponseAddress;
    uint8_t *m_AdbRequestAddress, *m_AdbResponseAddress;

    size_t m_AdbRetry;
    size_t m_TcuRetry;

    bool m_IsLocked;
    bool m_IsPrepared;

    void AdbAuthenticateBegin(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
    void AdbAuthenticateStatus();

    void AdbSendCommand(std::vector<uint8_t>& request, std::vector<uint8_t>& response);

    void TcuErrorRequest();
    void TcuStatusRequest(unsigned& status);
    void TcuSetKv();
    void TcuSetMa();
    void TcuSetFocalSpotSize();
    void TcuTurnFocalOn();
    void TcuTurnFocalOff();
    void TcuSetFocalPosition();
    void TcuTechniqueRequest(uint8_t& kv, uint16_t& ma, FSS& focalSpotSize);
    void TcuResetCriticalError();
    void TcuShutdown(bool isOn);

    void TcuSendCommand(std::vector<uint8_t>& request, std::vector<uint8_t>& response);

    void HvgWaitOnCommunication(unsigned sec);
    void HvgSetTime(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
    void HvgInitialize();
    void HvgWaitOnStandby(unsigned sec);
    void HvgModeData();
    void HvgDummyExposure();
    void HvgWaitPreparedScan(unsigned sec);
    void HvgEndScan();
    void HvgWaitScanData(unsigned sec);
    void HvgExpose();
    void HvgReset();
    void HvgReboot();
    void HvgRequestVersion();
    void HvgWaitVersion(unsigned sec);

    void HvgSend(const uint32_t& id, const uint64_t& request);
    void HvgSend(const uint32_t& id, const std::vector<uint8_t>& request);
    void HvgRead(uint32_t& id, uint64_t& response);
    void HvgRead(uint32_t& id, std::vector<uint8_t>& response);

    uint8_t CRC8(std::vector<uint8_t>& data, size_t length);
    uint16_t CRC16(std::vector<uint8_t>& data, size_t length);
};

#endif // FPGAVAREX_H
