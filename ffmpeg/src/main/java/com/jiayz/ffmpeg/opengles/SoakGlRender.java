//package com.jiayz.ffmpeg.opengles;
//
//import android.content.Context;
//import android.graphics.Bitmap;
//import android.graphics.SurfaceTexture;
//import android.opengl.GLES11Ext;
//import android.opengl.GLES30;
//import android.opengl.GLException;
//import android.opengl.GLSurfaceView;
//import android.view.Surface;
//
//
//import com.jiayz.ffmpeg.R;
//import com.jiayz.ffmpeg.listener.SoakOnGlSurfaceViewOnCreateListener;
//import com.jiayz.ffmpeg.listener.SoakOnRenderRefreshListener;
//import com.jiayz.ffmpeg.util.SoakLog;
//
//import java.nio.Buffer;
//import java.nio.ByteBuffer;
//import java.nio.ByteOrder;
//import java.nio.FloatBuffer;
//import java.nio.IntBuffer;
//
//import javax.microedition.khronos.egl.EGLConfig;
//import javax.microedition.khronos.opengles.GL10;
//
//
//
//public class SoakGlRender implements GLSurfaceView.Renderer, SurfaceTexture.OnFrameAvailableListener{
//
//    private Context context;
//    private FloatBuffer vertexBuffer;
//    private final float[] vertexData = {
//            1f,1f,0f,
//            -1f,1f,0f,
//            1f,-1f,0f,
//            -1f,-1f,0f
//    };
//
//    private FloatBuffer textureBuffer;
//    private final float[] textureVertexData = {
//            1f,0f,
//            0f,0f,
//            1f,1f,
//            0f,1f
//    };
//
//    /**
//     * mediacodec
//     */
//
//    private int programId_mediacodec;
//    private int aPositionHandle_mediacodec;
//    private int textureid_mediacodec;
//    private int uTextureSamplerHandle_mediacodec;
//    private int aTextureCoordHandle_mediacodec;
//
//    private SurfaceTexture surfaceTexture;
//    private Surface surface;
//
//    /**
//     * yuv
//     */
//    private int programId_yuv;
//    private int aPositionHandle_yuv;
//    private int aTextureCoordHandle_yuv;
//    private int sampler_y;
//    private int sampler_u;
//    private int sampler_v;
//    private int [] textureid_yuv;
//
//    int w;
//    int h;
//
//    Buffer y;
//    Buffer u;
//    Buffer v;
//
//    /**
//     * stop
//     */
//    private int programId_stop;
//    private int aPositionHandle_stop;
//    private int aTextureCoordHandle_stop;
//
//
//    int codecType = -1;
//    private boolean cutImg = false;
//    private int sWidth = 0;
//    private int sHeight = 0;
//
//
//
//    private SoakOnGlSurfaceViewOnCreateListener onCreateListener;
//    private SoakOnRenderRefreshListener refreshListener;
//
//    public SoakGlRender(Context context) {
//        this.context = context;
//
//        vertexBuffer = ByteBuffer.allocateDirect(vertexData.length * 4)
//                .order(ByteOrder.nativeOrder())
//                .asFloatBuffer()
//                .put(vertexData);
//        vertexBuffer.position(0);
//
//        textureBuffer = ByteBuffer.allocateDirect(textureVertexData.length * 4)
//                .order(ByteOrder.nativeOrder())
//                .asFloatBuffer()
//                .put(textureVertexData);
//        textureBuffer.position(0);
//
//    }
//
//    public void setFrameData(int w, int h, byte[] by, byte[] bu, byte[] bv)
//    {
//        this.w = w;
//        this.h = h;
//        this.y = ByteBuffer.wrap(by);
//        this.u = ByteBuffer.wrap(bu);
//        this.v = ByteBuffer.wrap(bv);
//
//    }
//
//    public void setCodecType(int codecType) {
//        this.codecType = codecType;
//    }
//
//    @Override
//    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
//        SoakLog.d("onSurfaceCreated");
//        initMediacodecShader();
//        initYuvShader();
//        initStop();
//    }
//
//    @Override
//    public void onSurfaceChanged(GL10 gl, int width, int height) {
//        SoakLog.d("onSurfaceChanged, width:" + width + ",height :" + height);
//        sWidth = width;
//        sHeight = height;
//        GLES30.glViewport(0,0,width, height);
//    }
//
//
//    @Override
//    public void onDrawFrame(GL10 gl) {
//        GLES30.glClear(GLES30.GL_COLOR_BUFFER_BIT | GLES30.GL_DEPTH_BUFFER_BIT);
//        GLES30.glClearColor(0f, 0f, 0f, 1f);
//        if(codecType == 1)
//        {
//            renderMediacodec();
//            SoakLog.d("mediaocdec.......");
//        }
//        else if(codecType == 0)
//        {
//            renderYuv();
//            SoakLog.d("yuv.......");
//        }
//        else
//        {
//            renderStop();
//        }
//        GLES30.glDrawArrays(GLES30.GL_TRIANGLE_STRIP, 0, 4);
//        if(cutImg)
//        {
//            cutImg = false;
//            Bitmap bitmap = cutBitmap(0, 0, sWidth, sHeight);
//            if(onCreateListener != null)
//            {
//                onCreateListener.onCutVideoImg(bitmap);
//            }
//        }
//
//    }
//
//    @Override
//    public void onFrameAvailable(SurfaceTexture surfaceTexture) {
//        SoakLog.d("updateSurface");
//        if(refreshListener != null)
//        {
//            refreshListener.onRefresh();
//        }
//    }
//
//    public void setOnGlSurfaceViewOncreateListener(SoakOnGlSurfaceViewOnCreateListener listener) {
//        this.onCreateListener = listener;
//    }
//
//    public void setRefreshListener(SoakOnRenderRefreshListener refreshListener) {
//        this.refreshListener = refreshListener;
//    }
//
//    /**
//     * ?????????????????????shader
//     */
//    private void initMediacodecShader()
//    {
//        String vertexShader = SoakShaderUtils.readRawTextFile(context, R.raw.vertex_shader_play);
//        String fragmentShader = SoakShaderUtils.readRawTextFile(context, R.raw.fragment_mediacodec_play);
//        programId_mediacodec = SoakShaderUtils.createProgram(vertexShader, fragmentShader);
//        aPositionHandle_mediacodec= GLES30.glGetAttribLocation(programId_mediacodec,"av_Position");
//        aTextureCoordHandle_mediacodec =GLES30.glGetAttribLocation(programId_mediacodec,"af_Position");
//        uTextureSamplerHandle_mediacodec =GLES30.glGetUniformLocation(programId_mediacodec,"sTexture");
//
//        int[] textures = new int[1];
//        GLES30.glGenTextures(1, textures, 0);
//
//        textureid_mediacodec = textures[0];
//        GLES30.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, textureid_mediacodec);
//        SoakShaderUtils.checkGlError("glBindTexture mTextureID");
//
//        GLES30.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES30.GL_TEXTURE_MIN_FILTER,
//                GLES30.GL_NEAREST);
//        GLES30.glTexParameterf(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, GLES30.GL_TEXTURE_MAG_FILTER,
//                GLES30.GL_LINEAR);
//        surfaceTexture = new SurfaceTexture(textureid_mediacodec);
//        surfaceTexture.setOnFrameAvailableListener(this);
//        surface = new Surface(surfaceTexture);
//        if(onCreateListener != null)
//        {
//            onCreateListener.onGlSurfaceViewOnCreate(surface);
//        }
//    }
//
//    /**
//     * ??????????????????shader
//     */
//    private void renderMediacodec()
//    {
//        GLES30.glUseProgram(programId_mediacodec);
//        surfaceTexture.updateTexImage();
//        vertexBuffer.position(0);
//        GLES30.glEnableVertexAttribArray(aPositionHandle_mediacodec);
//        GLES30.glVertexAttribPointer(aPositionHandle_mediacodec, 3, GLES30.GL_FLOAT, false,
//                12, vertexBuffer);
//        textureBuffer.position(0);
//        GLES30.glEnableVertexAttribArray(aTextureCoordHandle_mediacodec);
//        GLES30.glVertexAttribPointer(aTextureCoordHandle_mediacodec,2,GLES30.GL_FLOAT,false,8, textureBuffer);
//
//        GLES30.glActiveTexture(GLES30.GL_TEXTURE0);
//        GLES30.glBindTexture(GLES11Ext.GL_TEXTURE_EXTERNAL_OES, textureid_mediacodec);
//        GLES30.glUniform1i(uTextureSamplerHandle_mediacodec,0);
//    }
//
//    private void initYuvShader()
//    {
//        String vertexShader = SoakShaderUtils.readRawTextFile(context, R.raw.vertex_shader_play);
//        String fragmentShader = SoakShaderUtils.readRawTextFile(context, R.raw.fragment_yuv_play);
//        programId_yuv = SoakShaderUtils.createProgram(vertexShader, fragmentShader);
//        aPositionHandle_yuv= GLES30.glGetAttribLocation(programId_yuv,"av_Position");
//        aTextureCoordHandle_yuv =GLES30.glGetAttribLocation(programId_yuv,"af_Position");
//
//        sampler_y = GLES30.glGetUniformLocation(programId_yuv, "sampler_y");
//        sampler_u = GLES30.glGetUniformLocation(programId_yuv, "sampler_u");
//        sampler_v = GLES30.glGetUniformLocation(programId_yuv, "sampler_v");
//
//        textureid_yuv = new int[3];
//        GLES30.glGenTextures(3, textureid_yuv, 0);
//        for (int i = 0; i < 3; i++) {
//            // ??????????????????
//            GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, textureid_yuv[i]);
//            //???????????? ?????????????????????????????????????????? ???????????????????????????????????????????????? ????????????????????????????????????
//            GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MAG_FILTER, GLES30.GL_LINEAR);
//            // ???????????????
//            GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_MIN_FILTER, GLES30.GL_LINEAR);
//            // ?????????????????????????????? 0,0-1,1 ??????????????????????????????
//            GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_WRAP_S, GLES30.GL_CLAMP_TO_EDGE);
//            GLES30.glTexParameteri(GLES30.GL_TEXTURE_2D, GLES30.GL_TEXTURE_WRAP_T, GLES30.GL_CLAMP_TO_EDGE);
//        }
//    }
//
//    private void renderYuv()
//    {
//        if(w > 0 && h > 0 && y != null && u != null && v != null)
//        {
//            GLES30.glUseProgram(programId_yuv);
//            GLES30.glEnableVertexAttribArray(aPositionHandle_yuv);
//            GLES30.glVertexAttribPointer(aPositionHandle_yuv, 3, GLES30.GL_FLOAT, false,
//                    12, vertexBuffer);
//            textureBuffer.position(0);
//            GLES30.glEnableVertexAttribArray(aTextureCoordHandle_yuv);
//            GLES30.glVertexAttribPointer(aTextureCoordHandle_yuv,2,GLES30.GL_FLOAT,false,8, textureBuffer);
//
//            SoakLog.d("renderFFmcodec");
//            //??? GL_TEXTURE0 ?????? ?????? opengl????????????16?????????
//            //?????????????????????????????????????????????shader?????????????????????????????? ????????????????????????????????????16???
//            GLES30.glActiveTexture(GLES30.GL_TEXTURE0);
//            //?????????????????? ?????????????????????????????????????????????
//            GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, textureid_yuv[0]);
//            //????????????2d?????? ??????????????????????????????????????????????????????????????????
//            GLES30.glTexImage2D(GLES30.GL_TEXTURE_2D, 0, GLES30.GL_LUMINANCE, w, h, 0, GLES30.GL_LUMINANCE, GLES30.GL_UNSIGNED_BYTE, y);
//            //??????????????????????????????
//            GLES30.glUniform1i(sampler_y, 0);
//
//
//            GLES30.glActiveTexture(GLES30.GL_TEXTURE1);
//            GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, textureid_yuv[1]);
//            GLES30.glTexImage2D(GLES30.GL_TEXTURE_2D, 0, GLES30.GL_LUMINANCE, w / 2, h / 2, 0, GLES30.GL_LUMINANCE, GLES30.GL_UNSIGNED_BYTE,
//                    u);
//            GLES30.glUniform1i(sampler_u, 1);
//
//            GLES30.glActiveTexture(GLES30.GL_TEXTURE2);
//            GLES30.glBindTexture(GLES30.GL_TEXTURE_2D, textureid_yuv[2]);
//            GLES30.glTexImage2D(GLES30.GL_TEXTURE_2D, 0, GLES30.GL_LUMINANCE, w / 2, h / 2, 0, GLES30.GL_LUMINANCE, GLES30.GL_UNSIGNED_BYTE,
//                    v);
//            GLES30.glUniform1i(sampler_v, 2);
//            y.clear();
//            u.clear();
//            v.clear();
//            y = null;
//            u = null;
//            v = null;
//        }
//    }
//
//    private void initStop()
//    {
//        String vertexShader = SoakShaderUtils.readRawTextFile(context, R.raw.vertex_shader_play);
//        String fragmentShader = SoakShaderUtils.readRawTextFile(context, R.raw.fragment_no);
//        programId_stop = SoakShaderUtils.createProgram(vertexShader, fragmentShader);
//        aPositionHandle_stop= GLES30.glGetAttribLocation(programId_stop,"av_Position");
//        aTextureCoordHandle_stop =GLES30.glGetAttribLocation(programId_stop,"af_Position");
//    }
//
//    private void renderStop()
//    {
//        GLES30.glUseProgram(programId_stop);
//        vertexBuffer.position(0);
//        GLES30.glEnableVertexAttribArray(aPositionHandle_stop);
//        GLES30.glVertexAttribPointer(aPositionHandle_stop, 3, GLES30.GL_FLOAT, false,
//                12, vertexBuffer);
//        textureBuffer.position(0);
//        GLES30.glEnableVertexAttribArray(aTextureCoordHandle_stop);
//        GLES30.glVertexAttribPointer(aTextureCoordHandle_stop,2,GLES30.GL_FLOAT,false,8, textureBuffer);
//    }
//
//    private Bitmap cutBitmap(int x, int y, int w, int h) {
//        int bitmapBuffer[] = new int[w * h];
//        int bitmapSource[] = new int[w * h];
//        IntBuffer intBuffer = IntBuffer.wrap(bitmapBuffer);
//        intBuffer.position(0);
//        try {
//            GLES30.glReadPixels(x, y, w, h, GL10.GL_RGBA, GL10.GL_UNSIGNED_BYTE,
//                    intBuffer);
//            int offset1, offset2;
//            for (int i = 0; i < h; i++) {
//                offset1 = i * w;
//                offset2 = (h - i - 1) * w;
//                for (int j = 0; j < w; j++) {
//                    int texturePixel = bitmapBuffer[offset1 + j];
//                    int blue = (texturePixel >> 16) & 0xff;
//                    int red = (texturePixel << 16) & 0x00ff0000;
//                    int pixel = (texturePixel & 0xff00ff00) | red | blue;
//                    bitmapSource[offset2 + j] = pixel;
//                }
//            }
//        } catch (GLException e) {
//            return null;
//        }
//        Bitmap bitmap = Bitmap.createBitmap(bitmapSource, w, h, Bitmap.Config.ARGB_8888);
//        intBuffer.clear();
//        return bitmap;
//    }
//
//    public void cutVideoImg()
//    {
//        cutImg = true;
//    }
//
//}
