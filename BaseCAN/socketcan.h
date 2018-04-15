#ifndef SOCKETCAN_H
#define SOCKETCAN_H

#include "basecan.h"

#include <linux/can.h>
#include <linux/can/raw.h>

class SocketCAN : public BaseCAN
{
public:
    SocketCAN(const int data_length);
    virtual ~SocketCAN();

    void Open(std::string interface_name, std::list<int>& can_id_filter);
    void WriteFrame(CANFrame8& frame);
    void ReadFrame(CANFrame8& frame);

private:
    int m_sock;
    struct can_filter *m_can_id_filter;
    int m_filter_count;

    bool isValid() const { return m_sock != -1; }
};

#endif // SOCKETCAN_H
