#ifndef SOAKPLAYER_SOAKPLAYSTATUS_H
#define SOAKPLAYER_SOAKPLAYSTATUS_H


class SoakPlayStatus {

public:
    bool exit;
    bool pause;
    bool load;
    bool seek;

public:
    SoakPlayStatus();
    ~SoakPlayStatus();

};


#endif
