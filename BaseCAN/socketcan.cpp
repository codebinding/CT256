#include "basecan.h"
#include "socketcan.h"

#include <linux/can.h>
#include <linux/can/raw.h>

#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

SocketCAN::SocketCAN(const int data_length)
    : BaseCAN(data_length),
      m_sock(-1),
      m_filter_count(0)
{
}

SocketCAN::~SocketCAN(){

    delete m_can_id_filter;

    if( isValid() ){

        ::close( m_sock );
    }
}

void SocketCAN::Open(std::string interface_name, std::list<int>& can_id_filter){

    struct ifreq my_ifr;
    struct sockaddr_can my_addr;

    m_sock = ::socket(PF_CAN, SOCK_RAW, CAN_RAW);

    if( !isValid() ){

        throw CANException("error while opening socket");
    }

    m_filter_count = can_id_filter.size();

    if(m_filter_count > 0){

        m_can_id_filter = new struct can_filter[m_filter_count];
        for(int i = 0; i < m_filter_count; i++){

            m_can_id_filter[i].can_id = can_id_filter.front();
            m_can_id_filter[i].can_mask = CAN_SFF_ID_BITS;

            can_id_filter.pop_front();
        }
        ::setsockopt(m_sock, SOL_CAN_RAW, CAN_RAW_FILTER, m_can_id_filter, sizeof(struct can_filter)*m_filter_count);
    }

    ::strcpy(my_ifr.ifr_ifrn.ifrn_name, interface_name.c_str());
    ::ioctl(m_sock, SIOCGIFINDEX, &my_ifr);

    my_addr.can_family = AF_CAN;
    my_addr.can_ifindex = my_ifr.ifr_ifindex;

    if(::bind(m_sock, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1) {

        throw CANException("error while socket binding");
    }
}

void SocketCAN::WriteFrame(CANFrame8& frame){

    if( !isValid() ){

        throw CANException("open socket before calling WriteFrame");
    }

    struct can_frame my_frame;

    my_frame.can_id = frame.GetId();
    my_frame.can_dlc = k_data_length;

    frame.GetData(my_frame.data);

    if((::write(m_sock, &my_frame, sizeof(struct can_frame))) == -1){

        throw CANException("error while writing CAN frame");
    }
}

void SocketCAN::ReadFrame(CANFrame8& frame){

    if( !isValid() ){

        throw CANException("open socket before calling ReadFrame");
    }

    struct can_frame my_frame;

    if(::read(m_sock, &my_frame, sizeof(struct can_frame)) == -1){

        throw CANException("error while reading CAN frame");
    }

    frame.SetId(my_frame.can_id);
    frame.SetData(my_frame.data);
}
