#include "SoakQueue.h"
#include "AndroidLog.h"

SoakQueue::SoakQueue(SoakPlayStatus *status) {
    playStatus = status;
    pthread_mutex_init(&mutexPacket, nullptr);
    pthread_cond_init(&condPacket, nullptr);
    pthread_mutex_init(&mutexFrame, nullptr);
    pthread_cond_init(&condFrame, nullptr);
}

SoakQueue::~SoakQueue() {
    playStatus = nullptr;
    pthread_mutex_destroy(&mutexPacket);
    pthread_cond_destroy(&condPacket);
    pthread_mutex_destroy(&mutexFrame);
    pthread_cond_destroy(&condFrame);
    LOGE("SoakQueueue() 释放完了")
}

void SoakQueue::release() {
    LOGE("SoakQueue::release")
    noticeThread();
    clearAvpacket();
    clearAvFrame();
    LOGE("SoakQueue::release success")
}

int SoakQueue::putAvpacket(AVPacket *avPacket) {

    pthread_mutex_lock(&mutexPacket);
    queuePacket.push(avPacket);
    pthread_cond_signal(&condPacket);
    pthread_mutex_unlock(&mutexPacket);

    return 0;
}

int SoakQueue::getAvpacket(AVPacket *avPacket) {

    pthread_mutex_lock(&mutexPacket);

    while(playStatus != nullptr && !playStatus->exit)
    {
        if(queuePacket.size() > 0)
        {
            AVPacket *pkt = queuePacket.front();
            if(av_packet_ref(avPacket, pkt) == 0)
            {
                queuePacket.pop();
            }
            av_packet_free(&pkt);
            av_free(pkt);
            pkt = nullptr;
            break;
        } else{
            if(!playStatus->exit)
            {
                pthread_cond_wait(&condPacket, &mutexPacket);
            }
        }
    }
    pthread_mutex_unlock(&mutexPacket);
    return 0;
}

int SoakQueue::clearAvpacket() {

    pthread_cond_signal(&condPacket);
    pthread_mutex_lock(&mutexPacket);
    while (!queuePacket.empty())
    {
        AVPacket *pkt = queuePacket.front();
        queuePacket.pop();
        av_free(pkt->data);
        av_free(pkt->buf);
        av_free(pkt->side_data);
        pkt = nullptr;
    }
    pthread_mutex_unlock(&mutexPacket);
    return 0;
}

int SoakQueue::getAvPacketSize() {
    int size = 0;
    pthread_mutex_lock(&mutexPacket);
    size = queuePacket.size();
    pthread_mutex_unlock(&mutexPacket);
    return size;
}

int SoakQueue::putAvframe(AVFrame *avFrame) {
    pthread_mutex_lock(&mutexFrame);
    queueFrame.push(avFrame);
    pthread_cond_signal(&condFrame);
    pthread_mutex_unlock(&mutexFrame);
    return 0;
}

int SoakQueue::getAvframe(AVFrame *avFrame) {
    pthread_mutex_lock(&mutexFrame);

    while(playStatus != nullptr && !playStatus->exit)
    {
        if(queueFrame.size() > 0)
        {
            AVFrame *frame = queueFrame.front();
            if(av_frame_ref(avFrame, frame) == 0)
            {
                queueFrame.pop();
            }
            av_frame_free(&frame);
            av_free(frame);
            frame = nullptr;
            break;
        } else{
            if(!playStatus->exit)
            {
                pthread_cond_wait(&condFrame, &mutexFrame);
            }
        }
    }
    pthread_mutex_unlock(&mutexFrame);
    return 0;
}

int SoakQueue::clearAvFrame() {
    pthread_cond_signal(&condFrame);
    pthread_mutex_lock(&mutexFrame);
    while (!queueFrame.empty())
    {
        AVFrame *frame = queueFrame.front();
        queueFrame.pop();
        av_frame_free(&frame);
        av_free(frame);
        frame = nullptr;
    }
    pthread_mutex_unlock(&mutexFrame);
    return 0;
}

int SoakQueue::getAvFrameSize() {
    int size = 0;
    pthread_mutex_lock(&mutexFrame);
    size = queueFrame.size();
    pthread_mutex_unlock(&mutexFrame);
    return size;
}

int SoakQueue::noticeThread() {
    pthread_cond_signal(&condFrame);
    pthread_cond_signal(&condPacket);
    return 0;
}

int SoakQueue::clearToKeyFrame() {
    pthread_cond_signal(&condPacket);
    pthread_mutex_lock(&mutexPacket);
    while (!queuePacket.empty())
    {
        AVPacket *pkt = queuePacket.front();
        if(pkt->flags != AV_PKT_FLAG_KEY)
        {
            queuePacket.pop();
            av_free(pkt->data);
            av_free(pkt->buf);
            av_free(pkt->side_data);
            pkt = nullptr;
        } else{
            break;
        }
    }
    pthread_mutex_unlock(&mutexPacket);
    return 0;
}

