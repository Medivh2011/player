package com.jiayz.ffmpeg.soakplayer;
public class SoakTimeBean {

    private int currentSeconds;
    private int totalSeconds;

    public void setCurrentSeconds(int currentSeconds) {
        this.currentSeconds = currentSeconds;
    }

    public void setTotalSeconds(int totalSeconds) {
        this.totalSeconds = totalSeconds;
    }

    public int getCurrentSeconds() {
        return currentSeconds;
    }

    public int getTotalSeconds() {
        return totalSeconds;
    }
}
