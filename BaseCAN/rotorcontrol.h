#ifndef ROTORCONTROL_H
#define ROTORCONTROL_H

#include "basecan.h"

#include <string>
#include <list>
#include <queue>
#include <pthread.h>

class RotorControl
{

public:
    RotorControl();
    virtual ~RotorControl();

    void Initialize();
    void SendFrame(CANFrame8 &frame);

private:
    BaseCAN *m_can;

    pthread_mutex_t m_sq_lock;  // send queue lock
    pthread_cond_t /*m_signal_sq_producer,*/ m_signal_sq_consumer;

    pthread_mutex_t m_rq_lock;
    pthread_cond_t /*m_signal_rq_producer,*/ m_signal_rq_consumer;

    std::queue<CANFrame8> *m_send_queue;
    std::queue<CANFrame8> *m_recv_queue;

    void ConsumeSendQueue();
    static void* ThreadConsumeSendQueue(void* this_pointer);
    pthread_t m_thread_sq_consumer;
    bool m_sq_consumer_running;

    void ProduceSendQueue(CANFrame8 &frame);

    void  ConsumeRecvQueue();
    static void* ThreadConsumeRecvQueue(void* this_pointer);
    pthread_t m_thread_rq_consumer;
    bool m_rq_consumer_running;

    void ProduceRecvQueue();
    static void* ThreadProduceRecvQueue(void* this_pointer);
    pthread_t m_thread_rq_producer;
    bool m_rq_producer_runing;
};

#endif // ROTORCONTROL_H
