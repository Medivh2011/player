#ifndef SOAKPLAYER_SOAKFFMPEG_H
#define SOAKPLAYER_SOAKFFMPEG_H

#include "AndroidLog.h"

#include "SoakBasePlayer.h"
#include "SoakJavaCall.h"
#include "SoakAudio.h"
#include "SoakVideo.h"
#include "SoakPlayStatus.h"
#include "SoakAudioChannel.h"

extern "C"
{
#include <libavformat/avformat.h>
#include "pthread.h"
}


class SoakFFmpeg {

public:
    const char *urlpath = NULL;
    SoakJavaCall *soakJavaCall = NULL;
    pthread_t decodThread;
    AVFormatContext *pFormatCtx = NULL;//封装格式上下文
    int duration = 0;
    SoakAudio *soakAudio = NULL;
    SoakVideo *soakVideo = NULL;
    SoakPlayStatus *soakPlayStatus = NULL;
    bool exit = false;
    bool exitByUser = false;
    int mimeType = 1;
    bool isavi = false;
    bool isOnlyMusic = false;

    std::deque<SoakAudioChannel*> audiochannels;
    std::deque<SoakAudioChannel*> videochannels;

    pthread_mutex_t init_mutex;
    pthread_mutex_t seek_mutex;

public:
    SoakFFmpeg(SoakJavaCall *javaCall, const char *urlpath, bool onlymusic);
    ~SoakFFmpeg();
    int preparedFFmpeg();
    int decodeFFmpeg();
    int start();
    int seek(int64_t sec);
    int getDuration();
    int getAvCodecContext(AVCodecParameters * parameters, SoakBasePlayer *basePlayer);
    void release();
    void pause();
    void resume();
    int getMimeType(const char* codecName);
    void setAudioChannel(int id);
    void setVideoChannel(int id);
    int getAudioChannels();
    int getVideoWidth();
    int getVideoHeight();
};


#endif
