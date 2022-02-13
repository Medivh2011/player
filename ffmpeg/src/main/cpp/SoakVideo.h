
#ifndef SOAKPLAYER_SOAKVIDEO_H
#define SOAKPLAYER_SOAKVIDEO_H

#include "SoakBasePlayer.h"
#include "SoakQueue.h"
#include "SoakJavaCall.h"
#include "AndroidLog.h"
#include "SoakAudio.h"

extern "C"
{
    #include <libavutil/time.h>
};

class SoakVideo : public SoakBasePlayer{

public:
    SoakQueue *queue = NULL;
    SoakAudio *soakAudio = NULL;
    SoakPlayStatus *soakPlayStatus = NULL;
    pthread_t videoThread;
    pthread_t decFrame;
    SoakJavaCall *soakJavaCall = NULL;

    double delayTime = 0;
    double defaultDelayTime = 0.04;
    int rate = 0;
    bool isExit = true;
    bool isExit2 = true;
    int codecType = -1;
    double video_clock = 0;
    double framePts = 0;
    bool frameratebig = false;
    int playcount = -1;

public:
    SoakVideo(SoakJavaCall *javaCall, SoakAudio *audio, SoakPlayStatus *playStatus);
    ~SoakVideo();

    void playVideo(int codecType);
    void decodeVideo();
    void release();
    double synchronize(AVFrame *srcFrame, double pts);

    double getDelayTime(double diff);

    void setClock(int secds);

};


#endif //SOAKPLAYER_SOAKVIDEO_H
