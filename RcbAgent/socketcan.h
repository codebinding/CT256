#ifndef SOCKETCAN_H
#define SOCKETCAN_H

#include "canpacket.h"
#include "rcbpacket.h"

#include <pthread.h>
#include <queue>
#include <vector>

class SocketCAN
{
public:
    SocketCAN();

    ~SocketCAN();

    void Init();

    void WriteCAN(CANPacket& packet);
    void ReadCAN(CANPacket& packet);

    void PutResponse(RCBPacket &packet);
    void PutResponse(std::vector<RCBPacket> &packets);
    void GetRequest(RCBPacket& packet);

private:
    int m_sock;

    void openCAN(char* interface);
    bool isValid() const { return m_sock != -1; }

    std::queue<CANPacket> m_RequestQueue;
    std::queue<CANPacket> m_ResponseQueue;

    pthread_mutex_t m_RequestMutex;
    pthread_mutex_t m_ResponseMutex;

    pthread_cond_t m_RequestReadySignal;
    pthread_cond_t m_ResponseReadySignal;

    static void *threadReadRequest(void *thisPointer);
    void readRequest();
    pthread_t m_ThreadReadRequest;
    bool m_ReadRequestRunning;

    static void *threadWriteResponse(void *thisPointer);
    void writeResponse();
    pthread_t m_ThreadWriteResponse;
    bool m_WriteResponseRunning;
};

#endif // SOCKETCAN_H
