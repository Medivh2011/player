
#ifndef CUSTOM_VIDEO_H
#define CUSTOM_VIDEO_H


#include "SoakQueue.h"
#include "SoakCallJava.h"
#include "SoakAudio.h"

#define CODEC_YUV 0
#define CODEC_MEDIACODEC 1

extern "C"
{
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/time.h>
#include <libavcodec/avcodec.h>
};

class SoakVideo {

public:
    int streamIndex = -1;
    AVCodecContext *avCodecContext = nullptr;
    AVCodecParameters *codecPar = nullptr;
    SoakQueue *queue = nullptr;
    SoakPlaystatus *playStatus = nullptr;
    SoakCallJava *callJava = nullptr;
    AVRational time_base;
    pthread_t thread_play;
    SoakAudio *audio = nullptr;
    double clock = 0;
    double delayTime = 0;
    double defaultDelayTime = 0.04;
    pthread_mutex_t codecMutex;
    int codecType = CODEC_YUV;
    AVBSFContext *abs_ctx = nullptr;
public:
    SoakVideo(SoakPlaystatus *playStatus, SoakCallJava *callJava);
    ~SoakVideo();

    void play();

    void release();

    double getFrameDiffTime(AVFrame *avFrame, AVPacket *avPacket);

    double getDelayTime(double diff);

};


#endif
