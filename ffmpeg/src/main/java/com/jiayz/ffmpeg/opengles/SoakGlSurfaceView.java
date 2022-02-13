package com.jiayz.ffmpeg.opengles;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;

import com.jiayz.ffmpeg.listener.SoakOnGlSurfaceViewOnCreateListener;




public class SoakGlSurfaceView extends GLSurfaceView{

    private SoakGlRender glRender;
    private SoakOnGlSurfaceViewOnCreateListener listener;

    public SoakGlSurfaceView(Context context) {
        this(context, null);
    }

    public SoakGlSurfaceView(Context context, AttributeSet attrs) {
        super(context, attrs);
        glRender = new SoakGlRender(context);
        //设置egl版本为3.0
        setEGLContextClientVersion(3);
        //设置render
        setRenderer(glRender);
        //设置为手动刷新模式
        setRenderMode(GLSurfaceView.RENDERMODE_WHEN_DIRTY);
        glRender.setRefreshListener(() -> requestRender());
    }

    public void setOnGlSurfaceViewOncreateListener(SoakOnGlSurfaceViewOnCreateListener onGlSurfaceViewOncreateListener) {
        if(glRender != null)
        {
            glRender.setOnGlSurfaceViewOncreateListener(onGlSurfaceViewOncreateListener);
        }
    }

    public void setCodecType(int type)
    {
        if(glRender != null)
        {
            glRender.setCodecType(type);
        }
    }


    public void setFrameData(int w, int h, byte[] y, byte[] u, byte[] v)
    {
        if(glRender != null)
        {
            glRender.setFrameData(w, h, y, u, v);
            requestRender();
        }
    }

    public void cutVideoImg()
    {
        if(glRender != null)
        {
            glRender.cutVideoImg();
            requestRender();
        }
    }
}
