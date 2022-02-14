
#ifndef PLAY_STATUS_H
#define PLAY_STATUS_H
class PlayStatus {

public:
    bool exit = false;
    bool load = true;
    bool seek = false;
    bool pause = false;

public:
    PlayStatus();
    ~PlayStatus();
};

#endif

