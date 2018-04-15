#ifndef BASECAN_H
#define BASECAN_H

#include "canframe8.h"
#include "canexception.h"

#include <list>

class BaseCAN
{

public:
    BaseCAN(const int data_length);
    virtual ~BaseCAN();

    virtual void Open(std::string interface_name, std::list<int>& can_id_filter) = 0;
    virtual void WriteFrame(CANFrame8& frame) = 0;
    virtual void ReadFrame(CANFrame8& frame) = 0;

private:
    virtual bool isValid() const = 0;

protected:
        const int k_data_length;
};

#endif // BASECAN_H
