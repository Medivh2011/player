package com.jiayz.ffmpeg.listener;

import android.graphics.Bitmap;
import android.view.Surface;

public interface SoakOnGlSurfaceViewOnCreateListener {

    void onGlSurfaceViewOnCreate(Surface surface);

    void onCutVideoImg(Bitmap bitmap);

}
