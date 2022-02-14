
#ifndef SOAK_QUEUE_H
#define SOAK_QUEUE_H

#include "queue"
#include "pthread.h"
#include "../AndroidLog.h"
#include "PlayStatus.h"

extern "C"
{
#include "libavcodec/avcodec.h"
};


class SoakQueue {

public:
    std::queue<AVPacket *> queuePacket;
    pthread_mutex_t mutexPacket;
    pthread_cond_t condPacket;
    PlayStatus *playStatus = nullptr;

public:

    SoakQueue(PlayStatus *status);
    ~SoakQueue();

    int putAvPacket(AVPacket *packet);

    int getAvPacket(AVPacket *packet);

    int getQueueSize();

    void clearAvPacket();

    void noticeQueue();

};
#endif
