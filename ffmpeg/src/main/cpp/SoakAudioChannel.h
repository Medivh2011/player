#ifndef SOAKPLAYER_SOAKAUDIOCHANNEL_H
#define SOAKPLAYER_SOAKAUDIOCHANNEL_H

extern "C"
{
#include <libavutil/rational.h>
};

class SoakAudioChannel {
public:
    int channelId = -1;
    AVRational time_base;
    int fps;

public:
    SoakAudioChannel(int id, AVRational base);
    SoakAudioChannel(int id, AVRational base, int fps);
};


#endif
