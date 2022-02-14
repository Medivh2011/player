package com.jiayz.ffmpeg.opengles;

import android.content.Context;
import android.graphics.SurfaceTexture;
import android.opengl.GLES11Ext;
import android.opengl.GLES30;
import android.opengl.GLSurfaceView;
import android.view.Surface;


import com.jiayz.ffmpeg.R;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.FloatBuffer;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

public class SoakRender implements GLSurfaceView.Renderer, SurfaceTexture.OnFrameAvailableListener {
    public static final int RENDER_YUV = 1;
    public static final int RENDER_MEDIACODEC = 2;

    private Context context;
    private final float[] vertexData ={
            -1f, -1f,
            1f, -1f,
            -1f, 1f,
            1f, 1f

    };
    private final float[] textureData ={
            0f,1f,
            1f, 1f,
            0f, 0f,
            1f, 0f
    };
    private FloatBuffer vertexBuffer;
    private FloatBuffer textureBuffer;
    private int renderType = RENDER_YUV;
    //yuv
    private int program_yuv;
    private int avPosition_yuv;
    private int afPosition_yuv;

    private int sampler_y;
    private int sampler_u;
    private int sampler_v;
    private int[] textureId_yuv;

    private int width_yuv;
    private int height_yuv;
    private ByteBuffer y;
    private ByteBuffer u;
    private ByteBuffer v;

    //mediacodec
    private int program_mediacodec;
    private int avPosition_mediacodec;
    private int afPosition_mediacodec;
    private int samplerOES_mediacodec;
    private int textureId_mediacodec;
    private SurfaceTexture surfaceTexture;
    private Surface surface;

    private OnSurfaceCreateListener onSurfaceCreateListener;
    private OnRenderListener onRenderListener;

    public SoakRender(Context context)
    {
        this.context = context;
        vertexBuffer = ByteBuffer.allocateDirect(vertexData.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer()
                .put(vertexData);
        vertexBuffer.position(0);

        textureBuffer = ByteBuffer.allocateDirect(textureData.length * 4)
                .order(ByteOrder.nativeOrder())
                .asFloatBuffer()
                .put(textureData);
        textureBuffer.position(0);
    }

    public void setRenderType(int renderType) {
        this.renderType = renderType;
    }

    public void setOnSurfaceCreateListener(OnSurfaceCreateListener onSurfaceCreateListener) {
        this.onSurfaceCreateListener = onSurfaceCreateListener;
    }

    public void setOnRenderListener(OnRenderListener onRenderListener) {
        this.onRenderListener = onRenderListener;
    }

    @Override
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        initRenderYUV();
        initRenderMediacodec();
    }

    @Override
    public void onSurfaceChanged(GL10 gl, int width, int height) {
        GLES30.glViewport(0, 0, width, height);
    }

    @Override
    public void onDrawFrame(GL10 gl) {
        GLES30.glClear(GLES30.GL_COLOR_BUFFER_BIT);
        GLES30.glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        if(renderType == RENDER_YUV)
        {
            renderYUV();
        }
        else if(renderType == RENDER_MEDIACODEC)
        {
            renderMediacodec();
        }
        GLES30.glDrawArrays(GLES30.GL_TRIANGLE_STRIP, 0, 4);
    }

    @Override
    public void onFrameAvailable(SurfaceTexture surfaceTexture) {
        if(onRenderListener != null)
        {
            onRenderListener.onRender();
        }
    }

    private void initRenderYUV()
    {
        String vertexSource = SoakShaderUtils.readRawTxt(context, R.raw.vertex_shader);
        String fragmentSource = SoakShaderUtils.readRawTxt(context, R.raw.fragment_yuv);
        program_yuv = SoakShaderUtils.createProgram(vertexSource, fragmentSource);

        avPosition_yuv = GLES30.glGetAttribLocation(program_yuv, "av_Position");
        afPosition_yuv = GLES30.glGetAttribLocation(program_yuv, "af_Position");

        sampler_y = GLES30.glGetUniformLocation(program_yuv, "sampler_y");
        sampler_u = GLES30.glGetUniformLocation(program_yuv, "sampler_u");
        sampler_v = GLES30.glGetUniformLocation(program_yuv, "sampler_v");

        textureId_yuv = new int[3];
        GLES30.glGenTextures(3, textureId_yuv, 0);

        for(int i = 0; i < 3; i++)
        {
            GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, textureId_yuv[i]);
            GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_WRAP_S, GLES30.GL_REPEAT);
            GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_WRAP_T, GLES30.GL_REPEAT);
            GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MIN_FILTER, GLES30.GL_LINEAR);
            GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MAG_FILTER, GLES30.GL_LINEAR);
        }
    }

    public void setYUVRenderData(int width, int height, byte[] y, byte[] u, byte[] v)
    {
        this.width_yuv = width;
        this.height_yuv = height;
        this.y = ByteBuffer.wrap(y);
        this.u = ByteBuffer.wrap(u);
        this.v = ByteBuffer.wrap(v);
    }

    private void renderYUV()
    {
        if(width_yuv > 0 && height_yuv > 0 && y != null && u != null && v != null)
        {
            GLES30.glUseProgram(program_yuv);

            GLES30.glEnableVertexAttribArray(avPosition_yuv);
            GLES30.glVertexAttribPointer(avPosition_yuv, 2, GLES30.GL_FLOAT, false, 8, vertexBuffer);

            GLES30.glEnableVertexAttribArray(afPosition_yuv);
            GLES30.glVertexAttribPointer(afPosition_yuv, 2, GLES30.GL_FLOAT, false, 8, textureBuffer);

            GLES30.glActiveTexture(GLES30.GL_TEXTURE0);
            GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, textureId_yuv[0]);
            GLES30.glTexImage2D(GLES30.GL_TEXTURE_2D, 0, GLES30.GL_LUMINANCE, width_yuv, height_yuv, 0, GLES30.GL_LUMINANCE, GLES30.GL_UNSIGNED_BYTE, y);

            GLES30.glActiveTexture(GLES30.GL_TEXTURE1);
            GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, textureId_yuv[1]);
            GLES30.glTexImage2D(GLES30.GL_TEXTURE_2D, 0, GLES30.GL_LUMINANCE, width_yuv / 2, height_yuv / 2, 0, GLES30.GL_LUMINANCE, GLES30.GL_UNSIGNED_BYTE, u);

            GLES30.glActiveTexture(GLES30.GL_TEXTURE2);
            GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, textureId_yuv[2]);
            GLES30.glTexImage2D(GLES30.GL_TEXTURE_2D, 0, GLES30.GL_LUMINANCE, width_yuv / 2, height_yuv / 2, 0, GLES30.GL_LUMINANCE, GLES30.GL_UNSIGNED_BYTE, v);

            GLES30.glUniform1i(sampler_y, 0);
            GLES30.glUniform1i(sampler_u, 1);
            GLES30.glUniform1i(sampler_v, 2);

            y.clear();
            u.clear();
            v.clear();
            y = null;
            u = null;
            v = null;
        }
    }

    private void initRenderMediacodec()
    {
        String vertexSource = SoakShaderUtils.readRawTxt(context, R.raw.vertex_shader);
        String fragmentSource = SoakShaderUtils.readRawTxt(context, R.raw.fragment_mediacodec);
        program_mediacodec = SoakShaderUtils.createProgram(vertexSource, fragmentSource);

        avPosition_mediacodec = GLES30.glGetAttribLocation(program_mediacodec, "av_Position");
        afPosition_mediacodec = GLES30.glGetAttribLocation(program_mediacodec, "af_Position");
        samplerOES_mediacodec = GLES30.glGetUniformLocation(program_mediacodec, "sTexture");

        int[] textureids = new int[1];
        GLES30.glGenTextures(1, textureids, 0);
        textureId_mediacodec = textureids[0];

        GLES30.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES30.GL_TEXTURE_WRAP_S, GLES30.GL_REPEAT);
        GLES30.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES30.GL_TEXTURE_WRAP_T, GLES30.GL_REPEAT);
        GLES30.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES30.GL_TEXTURE_MIN_FILTER, GLES30.GL_LINEAR);
        GLES30.glTexParameteri(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES30.GL_TEXTURE_MAG_FILTER, GLES30.GL_LINEAR);

        surfaceTexture = new SurfaceTexture(textureId_mediacodec);
        surface = new Surface(surfaceTexture);
        surfaceTexture.setOnFrameAvailableListener(this);

        if(onSurfaceCreateListener != null)
        {
            onSurfaceCreateListener.onSurfaceCreate(surface);
        }
    }

    private void renderMediacodec()
    {
        surfaceTexture.updateTexImage();
        GLES30.glUseProgram(program_mediacodec);

        GLES30.glEnableVertexAttribArray(avPosition_mediacodec);
        GLES30.glVertexAttribPointer(avPosition_mediacodec, 2, GLES30.GL_FLOAT, false, 8, vertexBuffer);

        GLES30.glEnableVertexAttribArray(afPosition_mediacodec);
        GLES30.glVertexAttribPointer(afPosition_mediacodec, 2, GLES30.GL_FLOAT, false, 8, textureBuffer);

        GLES30.glActiveTexture(GLES30.GL_TEXTURE0);
        GLES30.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, textureId_mediacodec);
        GLES30.glUniform1i(samplerOES_mediacodec, 0);
    }


    public interface OnSurfaceCreateListener
    {
        void onSurfaceCreate(Surface surface);
    }

    public interface OnRenderListener{
        void onRender();
    }
}
