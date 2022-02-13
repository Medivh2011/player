#include "SoakAudioChannel.h"

SoakAudioChannel::SoakAudioChannel(int id, AVRational base) {
    channelId = id;
    time_base = base;
}

SoakAudioChannel::SoakAudioChannel(int id, AVRational base, int f) {
    channelId = id;
    time_base = base;
    fps = f;
}
