package com.jiayz.ffmpeg.model;

public class SoakVideoSizeInfo {

    private final int mWidth;
    private final int mHeight;
    private final float mDar;

    public SoakVideoSizeInfo(int width, int height, float dar) {
        this.mWidth = width;
        this.mHeight = height;
        this.mDar = dar;
    }

    public int getWidth() {
        return mWidth;
    }

    public int getHeight() {
        return mHeight;
    }

    public float getDar() {
        return mDar;
    }
}
