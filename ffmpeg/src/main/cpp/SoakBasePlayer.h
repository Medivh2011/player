#ifndef SOAKPLAYER_SOAKBASEPLAYER_H
#define SOAKPLAYER_SOAKBASEPLAYER_H

extern "C"
{
#include <libavcodec/avcodec.h>
};

class SoakBasePlayer {

public:
    int streamIndex;
    int duration;
    double clock = 0;
    double now_time = 0;
    AVCodecContext *avCodecContext = NULL;
    AVRational time_base;

public:
    SoakBasePlayer();
    ~SoakBasePlayer();
};


#endif //SOAKPLAYER_SOAKBASEPLAYER_H
