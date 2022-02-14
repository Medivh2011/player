
#ifndef SOAK_FFMPEG_H
#define SOAK_FFMPEG_H

#include "SoakCallJava.h"
#include "pthread.h"
#include "SoakAudio.h"
#include "PlayStatus.h"
#include "SoakVideo.h"

extern "C"
{
#include "libavformat/avformat.h"
#include "libavutil/rational.h"
#include <libavutil/time.h>
};


class SoakFFmpeg {

public:
    SoakCallJava *callJava = NULL;
    const char* url = NULL;
    pthread_t decodeThread;
    AVFormatContext *pFormatCtx = NULL;
    SoakAudio *audio = NULL;
    SoakVideo *video = NULL;
    PlayStatus *playStatus = NULL;
    pthread_mutex_t init_mutex;
    bool exit = false;
    int duration = 0;
    pthread_mutex_t seek_mutex;
    bool supportMediacodec = false;

    const AVBitStreamFilter *bsFilter = NULL;

public:
    SoakFFmpeg(PlayStatus *status, SoakCallJava *callJava, const char *url);
    ~SoakFFmpeg();

    void prepare();
    void decodeFFmpegThread();
    void start();

    void pause();

    void resume();

    void release();

    void seek(int64_t seconds);

    int getCodecContext(AVCodecParameters *avCodecParameters, AVCodecContext **avCodecContext);

};


#endif //CUSTOM_FFMPEG_H
