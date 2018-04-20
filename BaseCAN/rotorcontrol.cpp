#include "rotorcontrol.h"
#include "socketcan.h"

#include "unistd.h"

#include <iostream>

RotorControl::RotorControl()
{
}

RotorControl::~RotorControl(){

    m_sq_consumer_running = false;

    m_rq_consumer_running = false;
    m_rq_producer_runing = false;

    pthread_join(m_thread_rq_consumer, NULL);
    pthread_join(m_thread_sq_consumer, NULL);

    pthread_mutex_destroy(&m_sq_lock);
    pthread_mutex_destroy(&m_rq_lock);

    //pthread_cond_destroy(&m_signal_sq_producer);
    pthread_cond_destroy(&m_signal_sq_consumer);
    //pthread_cond_destroy(&m_signal_rq_producer);
    pthread_cond_destroy(&m_signal_rq_consumer);

    delete m_send_queue;
    delete m_recv_queue;
}

void RotorControl::Initialize(){

    // Read configuration to get can_type, interface_name and can_id_filter
    int can_type = 0;
    std::string interface_name = "can0";
    std::list<int> *can_id_filter = new std::list<int>();

    if(can_type == 0){

        m_can = new SocketCAN(8);
    }

    m_can->Open(interface_name, *can_id_filter);

    m_send_queue = new std::queue<CANFrame8>();
    m_recv_queue = new std::queue<CANFrame8>();

    // Initialize lock and signals of the 'send queue'
    ::pthread_mutex_init(&m_sq_lock, NULL);
    //::pthread_cond_init(&m_signal_sq_producer, NULL);
    ::pthread_cond_init(&m_signal_sq_consumer, NULL);

    // Initialize lock and signals of the 'recv queue'
    ::pthread_mutex_init(&m_rq_lock, NULL);
    //::pthread_cond_init(&m_signal_rq_producer, NULL);
    ::pthread_cond_init(&m_signal_rq_consumer, NULL);

    // Create the threads
    m_sq_consumer_running = true;
    if(::pthread_create(&m_thread_sq_consumer, NULL, &ThreadConsumeSendQueue, this) != 0){

        throw CANException("error while creating thread 'ThreadConsumeSendQueue");
    }

    m_rq_consumer_running = true;
    if(::pthread_create(&m_thread_rq_consumer, NULL, &ThreadConsumeRecvQueue, this) != 0){

        throw CANException("error while creating thread 'ThreadConsumeRecvQueue");
    }

    m_rq_producer_runing = true;
    if(::pthread_create(&m_thread_rq_producer, NULL, &ThreadProduceRecvQueue, this) != 0){

        throw CANException("error while creating thread 'ThreadProudceRecvQueue'");
    }
}

void RotorControl::SendFrame(CANFrame8 &frame){

    ProduceSendQueue(frame);
}

void RotorControl::ProduceSendQueue(CANFrame8& frame){

    pthread_mutex_lock(&m_rq_lock);

    m_send_queue->push(frame);

    pthread_cond_signal(&m_signal_sq_consumer);

    pthread_mutex_unlock(&m_rq_lock);
}

void RotorControl::ConsumeSendQueue(){

    while(m_sq_consumer_running){

        pthread_mutex_lock(&m_sq_lock);

        while(m_send_queue->empty()){

            pthread_cond_wait(&m_signal_sq_consumer, &m_sq_lock);
        }

        while(!m_send_queue->empty()){

            CANFrame8 my_frame = m_send_queue->front();

            m_send_queue->pop();

            m_can->WriteFrame(my_frame);

            ::usleep(1000);
        }

        //pthread_cond_signal(&m_signal_sq_producer);
        pthread_mutex_unlock(&m_sq_lock);
    }
}

void* RotorControl::ThreadConsumeSendQueue(void* this_pointer){

    static_cast<RotorControl*>(this_pointer)->ConsumeSendQueue();

    return NULL;
}

void RotorControl::ConsumeRecvQueue(){

    while(m_rq_consumer_running){

        pthread_mutex_lock(&m_rq_lock);

        while(m_recv_queue->empty()){

            pthread_cond_wait(&m_signal_rq_consumer, &m_rq_lock);
        }

        while(!m_recv_queue->empty()){

            CANFrame8 my_frame = m_recv_queue->front();
            m_recv_queue->pop();

            std::cout << my_frame.ToString() << std::endl;
        }

        //pthread_cond_signal(&m_signal_rq_producer);

        pthread_mutex_unlock(&m_rq_lock);
    }
}

void* RotorControl::ThreadConsumeRecvQueue(void *this_pointer){

    static_cast<RotorControl*>(this_pointer)->ConsumeRecvQueue();

    return NULL;
}

void RotorControl::ProduceRecvQueue(){

    while(m_rq_producer_runing){

        CANFrame8 *my_frame = new CANFrame8();

        m_can->ReadFrame(*my_frame);

        pthread_mutex_lock(&m_rq_lock);

        m_recv_queue->push(*my_frame);

        pthread_cond_signal(&m_signal_rq_consumer);

        uint8_t *my_data = new uint8_t[8];
        my_frame->GetData(my_data);
        if( my_data[0] == 0x80 ){

            //std::cout << std::hex << (int)my_data[0] << " Signal/Slot test" << std::endl;
            //emit CatchIt();
        }

        pthread_mutex_unlock(&m_rq_lock);
    }
}

void* RotorControl::ThreadProduceRecvQueue(void *this_pointer){

    static_cast<RotorControl*>(this_pointer)->ProduceRecvQueue();

    return NULL;
}
