#include "socketcan.h"
#include "exception.h"

#include <linux/can.h>
#include <linux/can/raw.h>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <iostream>

SocketCAN::SocketCAN()
    :m_sock(-1)
{

}

SocketCAN::~SocketCAN(){

    m_ReadRequestRunning = false;
    m_WriteResponseRunning = false;

    ::pthread_join(m_ThreadReadRequest, nullptr);
    ::pthread_join(m_ThreadWriteResponse, nullptr);

    ::pthread_mutex_destroy(&m_RequestMutex);
    ::pthread_mutex_destroy(&m_ResponseMutex);

    ::pthread_cond_destroy(&m_RequestReadySignal);
    ::pthread_cond_destroy(&m_ResponseReadySignal);

    if( isValid() ){

        ::close( m_sock );
    }
}

void SocketCAN::Init(){

    openCAN(const_cast<char *>("can0"));

    // Initialize mutex and signals of the 'request queue'
    ::pthread_mutex_init(&m_RequestMutex, nullptr);
    ::pthread_cond_init(&m_RequestReadySignal, nullptr);

    // Initialize lock and signals of the 'recv queue'
    ::pthread_mutex_init(&m_ResponseMutex, nullptr);
    ::pthread_cond_init(&m_ResponseReadySignal, nullptr);

    // Create the threads
    m_ReadRequestRunning = true;
    if(::pthread_create(&m_ThreadReadRequest, nullptr, &threadReadRequest, this) != 0){

        throw Exception("error creating threadReadRequest");
    }

    m_WriteResponseRunning = true;
    if(::pthread_create(&m_ThreadWriteResponse, nullptr, &threadWriteResponse, this) != 0){

        throw Exception("error creating threadWriteResponse");
    }
}

void SocketCAN::openCAN(char* interface){

    struct ifreq myIfr;
    struct sockaddr_can myAddr;

    m_sock = ::socket(PF_CAN, SOCK_RAW, CAN_RAW);

    if( !isValid() ){

        throw Exception("error opening socket");
    }

    struct can_filter myFilter;
    myFilter.can_id = CAN_EFF_FLAG;
    myFilter.can_mask = CAN_EFF_FLAG | CAN_RTR_FLAG;
    ::setsockopt(m_sock, SOL_CAN_RAW, CAN_RAW_FILTER, &myFilter, sizeof(struct can_filter));

    ::strcpy(myIfr.ifr_ifrn.ifrn_name, interface);
    ::ioctl(m_sock, SIOCGIFINDEX, &myIfr);

    myAddr.can_family = AF_CAN;
    myAddr.can_ifindex = myIfr.ifr_ifindex;

    if(::bind(m_sock, reinterpret_cast<struct sockaddr *>(&myAddr), sizeof(myAddr)) == -1) {

        throw Exception("error binding socket");
    }
}

void SocketCAN::WriteCAN(CANPacket& packet){

    if( !isValid() ){

        throw Exception("error writing CAN packet: socket not open");
    }

    struct can_frame myFrame;

    myFrame.can_id = packet.GetId();
    myFrame.can_dlc = packet.DataLength;

    packet.GetData(myFrame.data);

    if((::write(m_sock, &myFrame, sizeof(struct can_frame))) == -1){

        throw Exception("error writing CAN packet");
    }
}

void SocketCAN::ReadCAN(CANPacket& packet){

    if( !isValid() ){

        throw Exception("error reading CAN packet: socket not open");
    }

    struct can_frame myFrame;

    if(::read(m_sock, &myFrame, sizeof(struct can_frame)) == -1){

        throw Exception("error reading CAN packet");
    }

    packet.SetRawPacket(myFrame);
}

void SocketCAN::readRequest(){

    CANPacket myPacket;

    while (m_ReadRequestRunning){

        ReadCAN(myPacket);

        ::pthread_mutex_lock(&m_RequestMutex);

        m_RequestQueue.push(myPacket);

        std::cout << myPacket.ToString() << std::endl;

        ::pthread_cond_signal(&m_RequestReadySignal);
        ::pthread_mutex_unlock(&m_RequestMutex);
    }
}

void *SocketCAN::threadReadRequest(void *thisPointer){

    static_cast<SocketCAN*>(thisPointer)->readRequest();

    return nullptr;
}

void SocketCAN::PutResponse(RCBPacket &packet){

    ::pthread_mutex_lock(&m_ResponseMutex);

    CANPacket myPacket;
    packet.EncodeCANPacket(myPacket);

    m_ResponseQueue.push(myPacket);

    ::pthread_cond_signal(&m_ResponseReadySignal);

    ::pthread_mutex_unlock(&m_ResponseMutex);
}

void SocketCAN::PutResponse(std::vector<RCBPacket> &packets){

    ::pthread_mutex_lock(&m_ResponseMutex);

    CANPacket myPacket;
    for (std::vector<RCBPacket>::iterator it=packets.begin();it!=packets.end();it++){

        it->EncodeCANPacket(myPacket);
        m_ResponseQueue.push(myPacket);
    }

    ::pthread_cond_signal(&m_ResponseReadySignal);

    ::pthread_mutex_unlock(&m_ResponseMutex);
}

void SocketCAN::GetRequest(RCBPacket &packet){

    ::pthread_mutex_lock(&m_RequestMutex);

    CANPacket myCANPacket = m_RequestQueue.front();
    packet.DecodeCANPacket(myCANPacket);

    m_RequestQueue.pop();

    ::pthread_cond_signal(&m_RequestReadySignal);

    ::pthread_mutex_unlock(&m_RequestMutex);
}

void SocketCAN::writeResponse(){

    while (m_WriteResponseRunning) {

        ::pthread_mutex_lock(&m_ResponseMutex);

        while (m_ResponseQueue.empty()) {

            ::pthread_cond_wait(&m_ResponseReadySignal, &m_ResponseMutex);
        }

        while (!m_ResponseQueue.empty()) {

            CANPacket myPacket = m_ResponseQueue.front();

            WriteCAN(myPacket);

            m_ResponseQueue.pop();

            std::cout << myPacket.ToString() << std::endl;
        }

        ::pthread_mutex_unlock(&m_ResponseMutex);
    }
}

void *SocketCAN::threadWriteResponse(void *thisPointer){

    static_cast<SocketCAN*>(thisPointer)->writeResponse();

    return nullptr;
}
