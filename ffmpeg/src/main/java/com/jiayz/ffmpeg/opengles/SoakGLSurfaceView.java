package com.jiayz.ffmpeg.opengles;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

public class SoakGLSurfaceView extends GLSurfaceView {

    private SoakRender mRender;

    public SoakGLSurfaceView(Context context) {
        this(context, null);
    }

    public SoakGLSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        setEGLContextClientVersion(3);
        mRender = new SoakRender(context);
        setRenderer(mRender);
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        mRender.setOnRenderListener(() -> requestRender());
    }

    public void setYUVData(int width, int height, byte[] y, byte[] u, byte[] v)
    {
        if(mRender != null)
        {
            mRender.setYUVRenderData(width, height, y, u, v);
            requestRender();
        }
    }

    public SoakRender getRender() {
        return mRender;
    }
}
