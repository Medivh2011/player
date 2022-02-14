
#ifndef CUSTOM_PLAYSTATUS_H
#define CUSTOM_PLAYSTATUS_H


class SoakPlaystatus {

public:
    bool exit = false;
    bool load = true;
    bool seek = false;
    bool pause = false;

public:
    SoakPlaystatus();
    ~SoakPlaystatus();

};


#endif
