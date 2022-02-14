
#ifndef SOAK_AUDIO_H
#define SOAK_AUDIO_H

#include "SoakQueue.h"
#include "PlayStatus.h"
#include "SoakCallJava.h"

extern "C"
{
#include <libavutil/time.h>
#include "libavcodec/avcodec.h"
#include <libswresample/swresample.h>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
};

class SoakAudio {

public:
    int streamIndex = -1;
    AVCodecContext *avCodecContext = nullptr;
    AVCodecParameters *codecPar = nullptr;
    SoakQueue *queue = nullptr;
    PlayStatus *playStatus = nullptr;
    SoakCallJava *callJava = nullptr;

    pthread_t thread_play;
    AVPacket *avPacket = nullptr;
    AVFrame *avFrame = nullptr;
    int ret = 0;
    uint8_t *buffer = nullptr;
    int data_size = 0;
    int sample_rate = 0;

    int duration = 0;
    AVRational time_base;
    double clock;//总的播放时长
    double now_time;//当前frame时间
    double last_time; //上一次调用时间


    // 引擎接口
    SLObjectItf engineObject = nullptr;
    SLEngineItf engineEngine = nullptr;

    //混音器
    SLObjectItf outputMixObject = nullptr;
    SLEnvironmentalReverbItf outputMixEnvironmentalReverb = nullptr;
    SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;

    //pcm
    SLObjectItf pcmPlayerObject = nullptr;
    SLPlayItf pcmPlayerPlay = nullptr;

    //缓冲器队列接口
    SLAndroidSimpleBufferQueueItf pcmBufferQueue = nullptr;

    pthread_mutex_t codecMutex;

public:
    SoakAudio(PlayStatus *status, int sample_rate, SoakCallJava *callJava);
    ~SoakAudio();

    void play();
    int resampleAudio();

    void initOpenSLES();

    int getCurrentSampleRateForOpenSles(int sampleRate);

    void pause();

    void resume();

    void stop();

    void release();


};


#endif