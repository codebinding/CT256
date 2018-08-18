#include "fpgabridge.h"

#include <ctime>
#include <chrono>

using namespace chrono;

FPGABridge::FPGABridge()
    : m_MemoryFile(-1),
      m_AdbRetry(3)
{
    off_t bridge_base = HPS2FPGA_BRIDGE_BASE;

    /* open the memory device file */
    m_MemoryFile = ::open("/dev/mem", O_RDWR|O_SYNC);
    if (m_MemoryFile < 0) {

        throw new Exception(strerror(errno));
    }

    /* map the LWHPS2FPGA bridge into process memory */
    m_BridgeMap = mmap(nullptr, PAGE_SIZE, PROT_WRITE, MAP_SHARED, m_MemoryFile, bridge_base);
    if (m_BridgeMap == reinterpret_cast<void *>(-1)) {

        ::close(m_MemoryFile);
        throw new Exception("error calling mmap");
    }

    /* get the registers base address */
    m_Register = static_cast<reg_t *>(m_BridgeMap);
    m_BaseAddress = static_cast<uint8_t *>(m_BridgeMap);

    m_TcuRequestAddress = static_cast<uint8_t *>(m_BridgeMap) + 0x0140;
    m_TcuResponseAddress = static_cast<uint8_t *>(m_BridgeMap) + 0x0180;

    m_AdbRequestAddress = static_cast<uint8_t *>(m_BridgeMap) + 0x01d0;
    m_AdbResponseAddress = static_cast<uint8_t *>(m_BridgeMap) + 0x0220;
}

FPGABridge::~FPGABridge(){

    ::munmap(m_BridgeMap, PAGE_SIZE);
    ::close(m_MemoryFile);
}

void FPGABridge::ReadRegister(uint16_t offset, uint64_t& value){

    uint64_t* myRegister = static_cast<uint64_t *>(m_BridgeMap) + offset;

    value = *myRegister;
}

void FPGABridge::WriteRegister(uint16_t offset, uint64_t value){

    uint64_t* myRegister = static_cast<uint64_t *>(m_BridgeMap) + offset;

    *myRegister = value;
}

void FPGABridge::Init(){


}

void FPGABridge::AdbAuthenticate(){

    uint8_t myRequest[12] = {0x2b, 0x0e, 0x01, 0xee, 0x00, 0x00, 0x18, 0x12, 0x31, 0x23, 0x59, 0x59};

    std::time_t myNow = std::time(nullptr);
    std::tm *myLocalTime = std::localtime(&myNow);

    myRequest[6] = static_cast<uint8_t>(myLocalTime->tm_year - 100);
    myRequest[7] = static_cast<uint8_t>(myLocalTime->tm_mon + 1);
    myRequest[8] = static_cast<uint8_t>(myLocalTime->tm_mday);
    myRequest[9] = static_cast<uint8_t>(myLocalTime->tm_hour);
    myRequest[10] = static_cast<uint8_t>(myLocalTime->tm_min);
    myRequest[11] = static_cast<uint8_t>(myLocalTime->tm_sec);

    uint8_t *myResponse = nullptr;

    uint16_t myLen;

    int myRetry = m_AdbRetry;

    while (myRetry--) {

        AdbSendRequest(myRequest, sizeof (myRequest));

        AdbReceiveResponse(myResponse, &myLen);

        if (myResponse[3] == 0xfd){     // ACK

            return;
        }

        delete [] myResponse;
    }

    throw new Exception("NACK response: Authenticate Begin");
}

void FPGABridge::AdbSendRequest(uint8_t *request, uint16_t len){

    uint16_t myCRC16 = crc16( request, len);

    uint8_t *myPointer = m_AdbRequestAddress;

    m_Register->AdbTxLen = len+2;

    while (len--) {

        *myPointer++ = *request++;
    }

    *myPointer++ = myCRC16 >>8;
    *myPointer = myCRC16 & 0xff;

    if (m_Register->AdbTxReady){

        m_Register->AdbTxStart = 1;
    }
    else{

        throw new Exception("error writing Authenticate Begin request");
    }
}

void FPGABridge::AdbReceiveResponse(uint8_t *response, uint16_t *len){

    system_clock::time_point start = system_clock::now();

    uint8_t *myResponse = new uint8_t[2];

    int myIndex = 0;
    *len = 0;

    while (true){

        if (m_Register->AdbRxReady){

            uint8_t *myPointer = m_AdbResponseAddress;

            if (myIndex == 0){

                start = system_clock::now();
            }

            int myRxLen = m_Register->AdbRxLen;
            while (myRxLen-- > 0){

                myResponse[myIndex] = *myPointer++;

                if (myIndex == 1){  // message length byte

                    *len = myResponse[myIndex];
                    response = new uint8_t[*len];
                    response[0] = myResponse[0];
                    response[1] = myResponse[1];
                    delete [] myResponse;
                    myResponse = response;
                }

                myIndex++;
            }

            if (myIndex == *len) break;     // end of message
        }
        else{

            if (myIndex == 0){

                if(duration_cast<milliseconds>(system_clock::now() - start).count() > 250){

                    throw new Exception("timeout receving response");
                }
            }
            else{

                if (duration_cast<milliseconds>(system_clock::now() - start).count() > 100){

                    throw new Exception("timeout message completion");
                }
            }

        }
    }

    // check crc ...
}

uint8_t FPGABridge::crc8(uint8_t *data, uint16_t len){

    uint8_t crc = 0;

    while(len--){

        crc ^= *data++;

        crc = crc & 0x80 ? static_cast<uint8_t>(crc << 1) ^ 0x31 : static_cast<uint8_t>(crc << 1);
        crc = crc & 0x80 ? static_cast<uint8_t>(crc << 1) ^ 0x31 : static_cast<uint8_t>(crc << 1);
        crc = crc & 0x80 ? static_cast<uint8_t>(crc << 1) ^ 0x31 : static_cast<uint8_t>(crc << 1);
        crc = crc & 0x80 ? static_cast<uint8_t>(crc << 1) ^ 0x31 : static_cast<uint8_t>(crc << 1);
        crc = crc & 0x80 ? static_cast<uint8_t>(crc << 1) ^ 0x31 : static_cast<uint8_t>(crc << 1);
        crc = crc & 0x80 ? static_cast<uint8_t>(crc << 1) ^ 0x31 : static_cast<uint8_t>(crc << 1);
        crc = crc & 0x80 ? static_cast<uint8_t>(crc << 1) ^ 0x31 : static_cast<uint8_t>(crc << 1);
        crc = crc & 0x80 ? static_cast<uint8_t>(crc << 1) ^ 0x31 : static_cast<uint8_t>(crc << 1);
    }

    return crc;
}

uint16_t FPGABridge::crc16(uint8_t *data, uint16_t len){

    uint16_t crc=0;

    while(len--){

        crc ^= *data++;

        crc = crc & 1 ? (crc >> 1) ^ 0xa001 : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ 0xa001 : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ 0xa001 : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ 0xa001 : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ 0xa001 : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ 0xa001 : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ 0xa001 : crc >> 1;
        crc = crc & 1 ? (crc >> 1) ^ 0xa001 : crc >> 1;
    }

    return crc;
}
