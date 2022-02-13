
#ifndef SOAKPLAYER_QUEUE_H
#define SOAKPLAYER_QUEUE_H

#include "queue"
#include "SoakPlayStatus.h"

extern "C"
{
#include <libavcodec/avcodec.h>
#include "pthread.h"
};

class SoakQueue {

public:
    std::queue<AVPacket*> queuePacket;
    std::queue<AVFrame*> queueFrame;
    pthread_mutex_t mutexFrame;
    pthread_cond_t condFrame;
    pthread_mutex_t mutexPacket;
    pthread_cond_t condPacket;
    SoakPlayStatus *playStatus = NULL;

public:
    SoakQueue(SoakPlayStatus *status);
    ~SoakQueue();
    int putAvpacket(AVPacket *avPacket);
    int getAvpacket(AVPacket *avPacket);
    int clearAvpacket();
    int clearToKeyFrame();

    int putAvframe(AVFrame *avFrame);
    int getAvframe(AVFrame *avFrame);
    int clearAvFrame();

    void release();
    int getAvPacketSize();
    int getAvFrameSize();

    int noticeThread();
};


#endif //SOAKPLAYER_QUEUE_H
