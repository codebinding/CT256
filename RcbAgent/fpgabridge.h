#ifndef FPGAVAREX_H
#define FPGAVAREX_H

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <cstring>
#include <errno.h>
#include <string>

#include "register.h"
#include "exception.h"

using namespace std;

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
    void Reset();
    void Abort();
    void Warmup();

private:
    void* m_BridgeMap;
    int m_MemoryFile;

    struct reg_t *m_Register;
    uint8_t *m_BaseAddress;

    uint8_t *m_TcuRequestAddress, *m_TcuResponseAddress;
    uint8_t *m_AdbRequestAddress, *m_AdbResponseAddress;

    int m_AdbRetry;
    void AdbAuthenticate();

    void AdbSendRequest(uint8_t *request, uint16_t len);
    void AdbReceiveResponse(uint8_t *response, uint16_t *len);

    uint8_t crc8(uint8_t *data, uint16_t len);
    uint16_t crc16(uint8_t *data, uint16_t len);
};

#endif // FPGAVAREX_H
