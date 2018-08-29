#include "fpgabridge.h"
#include "log.h"


#include <ctime>
#include <chrono>

using namespace std::chrono;

FPGABridge::FPGABridge()
    : m_MemoryFile(-1),
      m_AdbRetry(3),
      m_TcuRetry(5),
      m_IsLocked(false)
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

    uint64_t* myRegister = static_cast<uint64_t *>(m_BridgeMap) + offset/8;

    value = *myRegister;
}

void FPGABridge::WriteRegister(uint16_t offset, uint64_t value){

    uint64_t* myRegister = static_cast<uint64_t *>(m_BridgeMap) + offset/8;

    *myRegister = value;
}

void FPGABridge::Init(){

    Log::Print("Init ...");

    std::time_t myNow = std::time(nullptr);
    std::tm *myLocalTime = std::localtime(&myNow);

    uint8_t myYear = static_cast<uint8_t>(myLocalTime->tm_year - 100);
    uint8_t myMonth = static_cast<uint8_t>(myLocalTime->tm_mon + 1);
    uint8_t myDay = static_cast<uint8_t>(myLocalTime->tm_mday);
    uint8_t myHour = static_cast<uint8_t>(myLocalTime->tm_hour);
    uint8_t myMinute = static_cast<uint8_t>(myLocalTime->tm_min);
    uint8_t mySecond = static_cast<uint8_t>(myLocalTime->tm_sec);

    AdbAuthenticateBegin(myYear, myMonth, myDay, myHour, myMinute, mySecond);

    AdbAuthenticateStatus();

    HvgWaitOnCommunication(20);

    HvgSetTime(myYear, myMonth, myDay, myHour, myMinute, mySecond);

    HvgInitialize();

    HvgWaitOnStandby(20);

    Log::Print("Init Done");
}

void FPGABridge::Start(){

    Log::Print("Start ...");

    HvgDummyExposure();

    HvgWaitPreparedScan(2);

    Log::Print("Start Done");
}

void FPGABridge::Stop(){

    Log::Print("Stop ...");

    HvgEndScan();

    Log::Print("Stop Done");
}

void FPGABridge::Prepare(){

    Log::Print("Prepare ...");

    m_Register->ExposureTime = ScanParameter::ExposureTimeInMSec;
    m_Register->TriggerMode = ScanParameter::TriggerMode;
    m_Register->HvUseIma = ScanParameter::ImaEnabled;
    m_Register->First2LastViews = ScanParameter::IntegrationLimit;
    m_Register->TriggerPosition = ScanParameter::TriggerPosition;

    TcuTurnFocalOff();
    TcuSetKv();
    TcuSetMa();
    TcuSetFocalSpotSize();
    TcuTurnFocalOn();

    unsigned myStatus;
    TcuStatusRequest(myStatus);

    m_Register->HvXrayRelease = 1;

    uint8_t myKv;
    uint16_t myMa;
    FSS myFss;
    TcuTechniqueRequest(myKv, myMa, myFss);
    if(myKv!=ScanParameter::Kv || myMa!=ScanParameter::Ma || myFss!=ScanParameter::FocalSpotSize){

        throw new Exception("Kv, Ma or FocalSpotSize feedback does not match requested values");
    }

    HvgModeData();
    HvgWaitPreparedScan(10);

    Log::Print("Prepare Done");
}

void FPGABridge::Expose(){

    Log::Print("Expose ...");

    try {

        HvgExpose();

        if (!m_IsLocked){

            EndScan();
        }

    } catch (HVGException e) {

        // Handle Generator Exception
    }

    Log::Print("Expose Done");
}

void FPGABridge::EndScan(){

    HvgEndScan();

    uint8_t myKv;
    uint16_t myMa;
    FSS myFss;

    unsigned myRetry = 3;
    while(myRetry--){

        TcuTechniqueRequest(myKv, myMa, myFss);

        if(myKv == 0 && myMa < 1){

            TcuTurnFocalOff();
            return;
        }

        // wait one more second to turn off focus
        usleep(1000000);
    }

    throw new Exception("Kv!=0 or Ma>1mA after turning off the generater");
}

void FPGABridge::Lock(){

    Log::Print("Lock ...");

    m_IsLocked = true;

    Log::Print("Lock Done");
}

void FPGABridge::Unlock(){

    Log::Print("Unlock ...");

    EndScan();

    m_IsLocked = false;

    Log::Print("Unlock Done");
}
void FPGABridge::Reset(){

    Log::Print("Reset ...");

    HvgReset();
    TcuResetCriticalError();

    Log::Print("Reset Done");
}

void FPGABridge::Abort(){

    Log::Print("Expose ...");

    try {

        HvgExpose();

        HvgEndScan();

        uint8_t myKv;
        uint16_t myMa;
        FSS myFss;

        unsigned myRetry = 3;
        while(myRetry--){

            TcuTechniqueRequest(myKv, myMa, myFss);

            if(myKv == 0 && myMa < 1){

                TcuTurnFocalOff();
                return;
            }

            // wait one more second to turn off focus
            usleep(1000000);
        }

        throw new Exception("Kv!=0 or Ma>1mA after turning off the generater");

    } catch (HVGException e) {

        // Handle Generator Exception
    }

    Log::Print("Expose Done");
}

void FPGABridge::AdbAuthenticateBegin(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second){

    std::vector<uint8_t> myRequest = {0x2b, 0x0e, 0x01, 0xee, 0x00, 0x00, year, month, day, hour, minute, second};

    std::vector<uint8_t> myResponse;

    Log::Print( "Entering ", __func__ );

    AdbSendCommand(myRequest, myResponse);

    Log::Print( "Exiting ", __func__);
}

void FPGABridge::AdbAuthenticateStatus(){

    Log::Print("Entering ", __func__);

    std::vector<uint8_t> myRequest = {0x2b, 0x9, 0x02, 0xee, 0x00, 0x00, 0xee};

    std::vector<uint8_t> myResponse;

    size_t myRetry = m_AdbRetry;

    while (myRetry--){

        usleep(250000);

        AdbSendCommand(myRequest, myResponse);

        switch (myResponse[6]) {

        case 0x01:  // in progress

            Log::Print("[ADB] authentication in progress ...");

            continue;

        case 0x04:  // not started

            Log::Print("[ADB] authentication not started ...");

            continue;

        case 0x02:  // successful auth.

            Log::Print("[ADB] authentication was successful ...");

            Log::Print("Exiting ", __func__);
            return;

        case 0x03:  // failed auth.

            // myResponse[7];
            // myResponse[8];
            throw new Exception("[ADB] failed authentication");

        default:

            throw new Exception("[ADB] unknown Authentication Status");
        }
    }

    throw new Exception("[ADB] timeout authentication");
}

void FPGABridge::AdbSendCommand(std::vector<uint8_t>& request, std::vector<uint8_t>& response){

    uint16_t myCRC16 = CRC16(request, request.size());

    request.push_back(myCRC16>>8);
    request.push_back(myCRC16&0xff);

    Log::Print("-->> ", request);

    uint8_t *myPointer = m_AdbRequestAddress;

    m_Register->AdbTxLen = request.size();

    for(auto it=request.cbegin(); it!=request.cend(); it++){

        *myPointer++ = *it;
    }

    size_t myRetry = m_AdbRetry;
    while (myRetry--) {

        if (m_Register->AdbTxReady){

            m_Register->AdbTxStart = 1;
        }
        else{

            throw new Exception("[ADB] error sending request");
        }

        response.empty();

        system_clock::time_point myTimeout = system_clock::now() + milliseconds(250);
        while (system_clock::now() < myTimeout) {

            if (m_Register->AdbRxReady){

                uint8_t *myPointer = m_AdbResponseAddress;

                for (unsigned i=0; i<m_Register->AdbRxLen; i++){

                    response.push_back(*myPointer++);
                }

                Log::Print("<<-- ", response);

                break;
            }

            usleep(1000);
        }

        if (response.size()==0){

            if (myRetry>1){

                Log::Print("[ADB] timeout receiving response");
                continue;
            }

            throw new Exception("[ADB] timeout receiving response");
        }

        if (response[1]!=response.size()){

            if (myRetry>1){

                Log::Print("[ADB] incorrect response length");
                continue;
            }

            throw new Exception("[ADB] incorrect response length");
        }

        // check CRC
        std::vector<uint8_t>::const_iterator myIt = response.cend();
        myCRC16 = static_cast<uint16_t>(*(myIt-2)<<8) | static_cast<uint16_t>(*(myIt-1));

        if (myCRC16 != CRC16(response, response.size()-2)){

            if (myRetry>1){

                Log::Print("[ADB] CRC error in response");
                continue;
            }

            throw new Exception("[ADB] CRC error in response");
        }

        if (response[3] != 0xfd){

            if (myRetry>1){

                Log::Print("[ADB] NACK in response");
                continue;
            }

            throw new Exception("[ADB] NACK in response");
        }

        // we passed all the checks
        return;
    }
}

void FPGABridge::TcuErrorRequest(){

    std::vector<uint8_t> myRequest = {0x01};

    std::vector<uint8_t> myResponse;

    Log::Print( "Entering ", __func__ );

    TcuSendCommand(myRequest, myResponse);

    std::cout << std::hex << std::setw(2) << std::setfill('0');
    std::cout << myResponse[3] << "-" << myResponse[4] << "-" << myResponse[5] << "-" << myResponse[6] << std::endl;

    Log::Print( "Exiting ", __func__);
}

void FPGABridge::TcuStatusRequest(unsigned &status){

    std::vector<uint8_t> myRequest = {0x02};

    std::vector<uint8_t> myResponse;

    Log::Print( "Entering ", __func__ );

    TcuSendCommand(myRequest, myResponse);

    status = myResponse[3]; // status

    Log::Print( "Exiting ", __func__);
}

void FPGABridge::TcuSetKv(){

    Log::Print( "Entering ", __func__ );

    std::vector<uint8_t> myRequest = {0x03, 0x00, 0x00};

    myRequest[1] = 0x00;
    myRequest[2] = ScanParameter::Kv;

    std::vector<uint8_t> myResponse;

    TcuSendCommand(myRequest, myResponse);

    if (myResponse[0] == 0xee){

        throw new Exception("[TCU] Kv value is out of range");
    }

    if (myResponse[0] == 0xa0){

        throw new Exception("[TCU] Focus is on");
    }

    Log::Print( "Exiting ", __func__);
}

void FPGABridge::TcuSetMa(){

    Log::Print( "Entering ", __func__ );

    std::vector<uint8_t> myRequest = {0x04, 0x00, 0x00};

    myRequest[1] = static_cast<uint8_t>(ScanParameter::Ma >> 8);
    myRequest[2] = static_cast<uint8_t>(ScanParameter::Ma);

    std::vector<uint8_t> myResponse;

    TcuSendCommand(myRequest, myResponse);

    if (myResponse[0] == 0xee){

        throw new Exception("[TCU] Ma value is out of range");
    }

    if (myResponse[0] == 0xa0){

        throw new Exception("[TCU] Focus is on");
    }

    Log::Print( "Exiting ", __func__);
}

void FPGABridge::TcuSetFocalSpotSize(){

    Log::Print( "Entering ", __func__ );

    std::vector<uint8_t> myRequest = {0x09, 0x00, 0x00};

    myRequest[1] = 0x00;
    myRequest[2] = static_cast<uint8_t>(ScanParameter::FocalSpotSize);

    std::vector<uint8_t> myResponse;

    TcuSendCommand(myRequest, myResponse);

    if (myResponse[0] == 0xa0){

        throw new Exception("[TCU] Focus is on");
    }

    Log::Print( "Exiting ", __func__);
}

void FPGABridge::TcuTurnFocalOn(){

    Log::Print( "Entering ", __func__ );

    m_Register->TcuFocusOn = 1;

    Log::Print( "Exiting ", __func__);
}

void FPGABridge::TcuTurnFocalOff(){

    Log::Print( "Entering ", __func__ );

    m_Register->TcuFocusOn = 0;

    Log::Print( "Exiting ", __func__);
}

void FPGABridge::TcuSetFocalPosition(){

    Log::Print( "Entering ", __func__ );

    // !!! need to be reviewed here !!!

    m_Register->TcuFocusA = 0;
    m_Register->TcuFocusB = 0;

    Log::Print( "Exiting ", __func__);
}

void FPGABridge::TcuTechniqueRequest(uint8_t& kv, uint16_t& ma, FSS& focalSpotSize){

    Log::Print( "Entering ", __func__ );

    std::vector<uint8_t> myRequest = {0x11};

    std::vector<uint8_t> myResponse;

    TcuSendCommand(myRequest, myResponse);

    kv = myResponse[3];
    ma = static_cast<uint16_t>(myResponse[4]<<8) | myResponse[5];
    focalSpotSize = static_cast<FSS>(myRequest[7]);

    std::cout << "Kv = " << kv << ", Ma = " << ma << ", FSS = " << focalSpotSize << std::endl;

    Log::Print( "Exiting ", __func__);
}

void FPGABridge::TcuResetCriticalError(){

    Log::Print( "Entering ", __func__ );

    std::vector<uint8_t> myRequest = {0x12};

    std::vector<uint8_t> myResponse;

    TcuSendCommand(myRequest, myResponse);

    Log::Print( "Exiting ", __func__);
}

void FPGABridge::TcuShutdown(bool isOn){

    Log::Print( "Entering ", __func__ );

    std::vector<uint8_t> myRequest = {0x13, 0x00, 0x00};

    myRequest[2] = isOn? 0x01 : 0x00;

    std::vector<uint8_t> myResponse;

    TcuSendCommand(myRequest, myResponse);

    Log::Print( "Exiting ", __func__);
}

void FPGABridge::TcuSendCommand(std::vector<uint8_t>& request, std::vector<uint8_t>& response){

    uint16_t myCRC16 = CRC16(request, request.size());

    request.push_back(myCRC16>>8);
    request.push_back(myCRC16&0xff);

    Log::Print("-->> ", request);

    uint8_t *myPointer = m_TcuResponseAddress;

    m_Register->TcuTxLen = request.size();

    for(auto it=request.cbegin(); it!=request.cend(); it++){

        *myPointer++ = *it;
    }

    size_t myRetry = m_TcuRetry;
    while (myRetry--) {

        if (m_Register->TcuTxReady){

            m_Register->TcuTxStart = 1;
        }
        else{

            throw new Exception("[TCU] error sending request");
        }

        response.empty();

        system_clock::time_point myTimeout = system_clock::now() + milliseconds(150);
        while (system_clock::now() < myTimeout) {

            if (m_Register->TcuRxReady){

                uint8_t *myPointer = m_TcuResponseAddress;

                for (unsigned i=0; i<m_Register->TcuRxLen; i++){

                    response.push_back(*myPointer++);
                }

                Log::Print("<<-- ", response);

                break;
            }

            usleep(1000);
        }

        if (response.size()==0){

            if (myRetry>1){

                Log::Print("[TCU] timeout receiving response");
                continue;
            }

            throw new Exception("[TCU] timeout receiving response");
        }

        // check CRC
        std::vector<uint8_t>::const_iterator myIt = response.cend();
        myCRC16 = static_cast<uint16_t>(*(myIt-2)<<8) | static_cast<uint16_t>(*(myIt-1));

        if (myCRC16 != CRC16(response, response.size()-2)){

            if (myRetry>1){

                Log::Print("[TCU] CRC error in response");
                continue;
            }

            throw new Exception("[TCU] CRC error in response");
        }

        if (response[0] == 0xdd){   // NACK received

            if (myRetry>1){

                Log::Print("[TCU] NACK in response, try re-sending...");
                continue;
            }

            throw new Exception("[TCU] NACK in response");
        }

        if (response.size() > 3 && request[0] != response[1]){

            throw new Exception("[TCU] response does not match request");
        }
        // we passed all the checks
        return;
    }
}

void FPGABridge::HvgWaitOnCommunication(unsigned sec){

    Log::Print( "Entering ", __func__ );

    uint32_t myId;
    std::vector<uint8_t> myResponse;

    system_clock::time_point timeout = system_clock::now() + seconds(sec);
    while (system_clock::now() < timeout) {

        if (m_Register->HvCanRxReady){

            HvgRead(myId, myResponse);
            if (myId==0x04200000u && myResponse[0]=='W' && myResponse[1]=='C'){ // WC

                Log::Print( "Exiting ", __func__);
                return;
            }

            if (myId==0x04240000 && myResponse[0]=='E' && myResponse[1]=='R'){  // ER

                // ... print error message here

                throw new HVGException("[HVG] received ER message");
            }
        }

        myResponse.empty();
        usleep(1000);
    }

    throw new Exception("[HVG] timeout receiving 'Waiting on Communication'");
}

void FPGABridge::HvgSetTime(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second){

    Log::Print( "Entering ", __func__ );

    std::vector<uint8_t> myRequest = {'S', 'T', year, month, day, hour, minute, second};

    if (m_Register->HvCanTxReady){

        HvgSend(0x08100000u, myRequest);    // ST Set Time
    }
    else{

        throw new Exception("[HVG] error sending 'Set Time' request");
    }

    usleep(1000);

    Log::Print( "Exiting ", __func__);
}

void FPGABridge::HvgInitialize(){

    Log::Print( "Entering ", __func__ );

    std::vector<uint8_t> myRequest = {'I', 'N', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    if (m_Register->HvCanTxReady){

        HvgSend(0x08100000u, myRequest);    // IN Initialize
    }
    else{

        throw new Exception("[HVG] error sending Initialize request");
    }

    Log::Print( "Exiting ", __func__);
}

void FPGABridge::HvgWaitOnStandby(unsigned sec){

    Log::Print( "Entering ", __func__ );

    uint32_t myId;
    std::vector<uint8_t> myResponse;

    system_clock::time_point timeout = system_clock::now() + seconds(sec);
    while (system_clock::now() < timeout) {

        if (m_Register->HvCanRxReady){

            HvgRead(myId, myResponse);
            if (myId==0x04200000u && myResponse[0]=='O' && myResponse[1]=='S'){ // OS

                Log::Print( "Exiting ", __func__);
                return;
            }

            if (myId==0x04240000 && myResponse[0]=='E' && myResponse[1]=='R'){  // ER

                // ... print error message here

                throw new HVGException("[HVG] received ER message");
            }
        }

        myResponse.empty();
        usleep(1000);
    }

    throw new Exception("[HVG] timeout receiving 'On Standby'");
}

void FPGABridge::HvgModeData(){

    Log::Print( "Entering ", __func__ );

    std::vector<uint8_t> myRequest = {'M', '1', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    myRequest[2] = ScanParameter::ImaEnabled? 0x01 : 0x00;  // Scan Type
    myRequest[3] = ScanParameter::GeneratorMode;

    throw new Exception("Pay attention to the value of FocalSpotSize!!!");

    myRequest[4] = ScanParameter::FocalSpotSize;
    myRequest[5] = ScanParameter::AnodeSpeed;
    myRequest[6] = ScanParameter::ImaStartingMa >> 8;
    myRequest[7] = static_cast<uint8_t>(ScanParameter::ImaStartingMa);

    if (m_Register->HvCanTxReady){

        HvgSend(0x08100000u, myRequest);    // M1 Mode Data 1
    }
    else{

        throw new Exception("[HVG] error sending 'Mode Data 1' request");
    }

    usleep(1000);

    myRequest = {'M', 'E', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    myRequest[2] = ScanParameter::Kv;
    myRequest[3] = ScanParameter::ImaMaxMa >> 8;
    myRequest[4] = static_cast<uint8_t>(ScanParameter::ImaMaxMa);
    myRequest[5] = static_cast<uint8_t>(ScanParameter::ScanTimeInMSec >> 16);
    myRequest[6] = static_cast<uint8_t>(ScanParameter::ScanTimeInMSec >> 8);
    myRequest[7] = static_cast<uint8_t>(ScanParameter::ScanTimeInMSec);

    if (m_Register->HvCanTxReady){

        HvgSend(0x08100000u, myRequest);    // ME Mode Data End
    }
    else{

        throw new Exception("[HVG] error sending 'Mode Data End' request");
    }

    Log::Print( "Exiting ", __func__);
}

void FPGABridge::HvgDummyExposure(){

    Log::Print( "Entering ", __func__ );

    // FSS=Large, Anode Speed=60Hz
    std::vector<uint8_t> myRequest = {'M', '1', 0x00, 0x00, 0x02, 0x3c, 0x00, 0x00};

    if (m_Register->HvCanTxReady){

        HvgSend(0x08100000u, myRequest);    // M1 Mode Data 1
    }
    else{

        throw new Exception("[HVG] error sending 'Mode Data 1' request");
    }

    usleep(1000);

    myRequest = {'M', 'E', 0x46, 0x00, 0x00, 0x00, 0x00, 0x00}; // 70kV, 0mA, 0mSec

    if (m_Register->HvCanTxReady){

        HvgSend(0x08100000u, myRequest);    // ME Mode Data End
    }
    else{

        throw new Exception("[HVG] error sending 'Mode Data End' request");
    }

    Log::Print( "Exiting ", __func__);
}

void FPGABridge::HvgWaitPreparedScan(unsigned sec){

    Log::Print( "Entering ", __func__ );

    uint32_t myId;
    std::vector<uint8_t> myResponse;

    system_clock::time_point timeout = system_clock::now() + seconds(sec);
    while (system_clock::now() < timeout) {

        if (m_Register->HvCanRxReady){

            HvgRead(myId, myResponse);
            if (myId==0x04200000u && myResponse[0]=='P' && myResponse[1]=='S'){ // PS

                Log::Print( "Exiting ", __func__);
                return;
            }

            if (myId==0x04240000 && myResponse[0]=='E' && myResponse[1]=='R'){  // ER

                // ... print error message here

                throw new HVGException("[HVG] received ER message");
            }
        }

        myResponse.empty();
        usleep(1000);
    }

    throw new Exception("[HVG] timeout receiving 'Prepared Scan'");
}

void FPGABridge::HvgEndScan(){

    Log::Print( "Entering ", __func__ );

    m_Register->HvXrayEnable = 0;

    std::vector<uint8_t> myRequest = {'E', 'S', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    if (m_Register->HvCanTxReady){

        HvgSend(0x08100000u, myRequest);    // End Scan
    }
    else{

        throw new Exception("[HVG] error sending 'End Scan' request");
    }

    Log::Print( "Exiting ", __func__);
}

void FPGABridge::HvgWaitScanData(unsigned sec){

    Log::Print( "Entering ", __func__ );

    uint32_t myId;
    std::vector<uint8_t> myResponse;

    system_clock::time_point timeout = system_clock::now() + seconds(sec);
    while (system_clock::now() < timeout) {

        if (m_Register->HvCanRxReady){

            HvgRead(myId, myResponse);
            if (myId==0x04240000u && myResponse[0]=='S'){ // S0, S1, S2, SE

                switch (myResponse[1]) {

                case '0':
                case '1':
                case '2':

                    Log::Print("Scan Data: ", myResponse);
                    continue;

                case 'E':

                    Log::Print("Scan Data: ", myResponse);
                    Log::Print( "Exiting ", __func__);
                    return;
                }
            }

            if (myId==0x04240000 && myResponse[0]=='E' && myResponse[1]=='R'){  // ER

                // ... print error message here

                throw new HVGException("[HVG] received ER message");
            }
        }

        myResponse.empty();
        usleep(1000);
    }

    throw new Exception("[HVG] timeout receiving 'Scan Data'");
}

void FPGABridge::HvgExpose(){

    Log::Print( "Entering ", __func__ );

    uint32_t myId;
    std::vector<uint8_t> myResponse;

    system_clock::time_point timeout = system_clock::now() + milliseconds(1000);

    m_Register->HvXrayEnable = 1;

    while (!m_Register->HvXrayOn) {

        if (system_clock::now() > timeout){

            if (m_Register->HvCanRxReady){

                HvgRead(myId, myResponse);
                if (myId==0x04240000 && myResponse[0]=='E' && myResponse[1]=='R'){  // ER

                    // ... print error code here

                    throw new HVGException("[HVG] received ER message");
                }
            }

            throw new Exception("X-Ray was not turned on within 1000mSec.");
        }

        usleep(1000);
    }

    // X-Ray is On
    std::cout << "\033[33m" << "== X-Ray is On ==" << "\033[0m" << std::endl;

    timeout = system_clock::now() + milliseconds(static_cast<long>(ScanParameter::ExposureTimeInMSec*1.2));

    while (system_clock::now() < timeout) {

        if (!m_Register->HvXrayOn){

            std::cout << std::endl << "\033[32m" << "== X-Ray is Off ==" << "\033[0m" << std::endl;

            break;
        }

        // do we need to check the generator ER message during exposure? or only after the X-Ray is off?

        std::cout << ".";
        usleep(100000); // 100ms
    }

    if (m_Register->HvXrayOn){

        m_Register->HvXrayEnable = 0;
        m_Register->HvXrayRelease = 0;

        throw new Exception("[HVG] max scan timeout, turning off the X-Ray");
    }

    if (m_Register->HvCanRxReady){

        HvgRead(myId, myResponse);
        if (myId==0x04240000 && myResponse[0]=='E' && myResponse[1]=='R'){  // ER

            // ... print error message here

            throw new HVGException("[HVG] received ER message");
        }
    }

    Log::Print( "Entering ", __func__ );
}

void FPGABridge::HvgReset(){

    Log::Print( "Entering ", __func__ );

    std::vector<uint8_t> myRequest = {'R', 'S', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    if (m_Register->HvCanTxReady){

        HvgSend(0x08100000u, myRequest);    // Reset
    }
    else{

        throw new Exception("[HVG] error sending 'Reset' request");
    }

    Log::Print( "Exiting ", __func__);
}

void FPGABridge::HvgReboot(){

    Log::Print( "Entering ", __func__ );

    std::vector<uint8_t> myRequest = {'R', 'B', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    if (m_Register->HvCanTxReady){

        HvgSend(0x08100000u, myRequest);    // Reboot
    }
    else{

        throw new Exception("[HVG] error sending 'Reboot' request");
    }

    Log::Print( "Exiting ", __func__);
}

void FPGABridge::HvgRequestVersion(){

    Log::Print( "Entering ", __func__ );

    std::vector<uint8_t> myRequest = {'R', 'V', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    if (m_Register->HvCanTxReady){

        HvgSend(0x08100000u, myRequest);    // Request Version
    }
    else{

        throw new Exception("[HVG] error sending 'Request Version' request");
    }

    Log::Print( "Exiting ", __func__);
}

void FPGABridge::HvgWaitVersion(unsigned sec){

    Log::Print( "Entering ", __func__ );

    uint32_t myId;
    std::vector<uint8_t> myResponse;

    system_clock::time_point timeout = system_clock::now() + seconds(sec);
    while (system_clock::now() < timeout) {

        if (m_Register->HvCanRxReady){

            HvgRead(myId, myResponse);
            if (myId==0x04200000u && myResponse[0]=='S' && myResponse[1]=='V'){ // Send Version

                Log::Print( "Exiting ", __func__);
                return;
            }

            if (myId==0x04240000 && myResponse[0]=='E' && myResponse[1]=='R'){  // ER

                // ... print error message here

                throw new HVGException("[HVG] received ER message");
            }
        }

        myResponse.empty();
        usleep(1000);
    }

    throw new Exception("[HVG] timeout receiving 'Send Version'");
}

void FPGABridge::HvgRead(uint32_t& id, uint64_t& response){

    id = m_Register->HvCanRxId;
    response = m_Register->HvCanRxData;
}

void FPGABridge::HvgRead(uint32_t& id, std::vector<uint8_t>& response){

    response.empty();

    id = m_Register->HvCanRxId;

    uint64_t myData = m_Register->HvCanRxData;
    for(unsigned i=0; i<8; i++){

        response.insert(response.begin(), static_cast<uint8_t>(myData));
        myData = myData >> 8;
    }
}

void FPGABridge::HvgSend(const uint32_t &id, const uint64_t &request){

    m_Register->HvCanTxId = id;
    m_Register->HvCanTxData = request;
    m_Register->HvCanTxStart = 1;
}

void FPGABridge::HvgSend(const uint32_t &id, const std::vector<uint8_t>& request){

    uint64_t myRequest = 0;
    for(auto it=request.cbegin(); it!=request.cend(); it++){

        myRequest = myRequest<<8 | *it;
    }

    m_Register->HvCanTxId = id;
    m_Register->HvCanTxData = myRequest;
    m_Register->HvCanTxStart = 1;
}

uint8_t FPGABridge::CRC8(std::vector<uint8_t>& data, size_t length){

    uint8_t crc = 0;

    std::vector<uint8_t>::const_iterator it = data.cbegin();
    while (length--){

        crc ^= *it++;

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

uint16_t FPGABridge::CRC16(std::vector<uint8_t>& data, size_t length){

    uint16_t crc = 0;

    std::vector<uint8_t>::const_iterator it = data.cbegin();
    while (length--){

        crc ^= *it++;

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
