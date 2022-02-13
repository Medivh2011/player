package com.jiayz.ffmpeg.soakplayer;


import android.graphics.Bitmap;
import android.media.MediaCodec;
import android.media.MediaFormat;
import android.text.TextUtils;
import android.util.Log;
import android.view.Surface;


import com.jiayz.ffmpeg.listener.SoakOnVideoSizeChangedListener;
import com.jiayz.ffmpeg.listener.SoakOnCompleteListener;
import com.jiayz.ffmpeg.listener.SoakOnCutVideoImgListener;
import com.jiayz.ffmpeg.listener.SoakOnErrorListener;
import com.jiayz.ffmpeg.listener.SoakOnGlSurfaceViewOnCreateListener;
import com.jiayz.ffmpeg.listener.SoakOnInfoListener;
import com.jiayz.ffmpeg.listener.SoakOnLoadListener;
import com.jiayz.ffmpeg.listener.SoakOnPreparedListener;
import com.jiayz.ffmpeg.listener.SoakOnStopListener;
import com.jiayz.ffmpeg.listener.SoakStatus;
import com.jiayz.ffmpeg.opengles.SoakGlSurfaceView;
import com.jiayz.ffmpeg.util.SoakLog;

import java.nio.ByteBuffer;



public class SoakPlayer {

    static {
        System.loadLibrary("soak_player");
    }

    /**
     * 播放文件路径
     */
    private String dataSource;
    /**
     * 硬解码mime
     */
    private MediaFormat mediaFormat;
    /**
     * 视频硬解码器
     */
    private MediaCodec mediaCodec;
    /**
     * 渲染surface
     */
    private Surface surface;
    /**
     * opengl surfaceview
     */
    private SoakGlSurfaceView glSurfaceView;
    /**
     * 视频解码器info
     */
    private MediaCodec.BufferInfo info = new MediaCodec.BufferInfo();

    /**
     * 准备好时的回调
     */
    private SoakOnPreparedListener soakOnPreparedListener;
    /**
     * 错误时的回调
     */
    private SoakOnErrorListener soakOnErrorListener;
    /**
     * 加载回调
     */
    private SoakOnLoadListener onLoadListener;
    /**
     * 更新时间回调
     */
    private SoakOnInfoListener onInfoListener;
    /**
     * 播放完成回调
     */
    private SoakOnCompleteListener onCompleteListener;
    /**
     * 视频截图回调
     */
    private SoakOnCutVideoImgListener onCutVideoImgListener;
    /**
     * 停止完成回调
     */
    private SoakOnStopListener onStopListener;


    private SoakOnVideoSizeChangedListener videoSizeChangedListener;
    /**
     * 是否已经准备好
     */
    private boolean parpared = false;
    /**
     * 时长实体类
     */
    private SoakTimeBean soakTimeBean;
    /**
     * 上一次播放时间
     */
    private int lastCurrTime = 0;

    /**
     * 是否只有音频（只播放音频流）
     */
    private boolean isOnlyMusic = false;

    private boolean isOnlySoft = false;

    public SoakPlayer()
    {
        soakTimeBean = new SoakTimeBean();
    }

    public void setDataSource(String dataSource) {
        this.dataSource = dataSource;
    }

    public void setOnlyMusic(boolean onlyMusic) {
        isOnlyMusic = onlyMusic;
    }

    private void setSurface(Surface surface) {
        this.surface = surface;
    }

    public void setGlSurfaceView(SoakGlSurfaceView soakGlSurfaceView) {
        this.glSurfaceView = soakGlSurfaceView;
        soakGlSurfaceView.setOnGlSurfaceViewOncreateListener(new SoakOnGlSurfaceViewOnCreateListener() {
            @Override
            public void onGlSurfaceViewOnCreate(Surface s) {
                if(surface == null)
                {
                    setSurface(s);
                }
                if(parpared && !TextUtils.isDigitsOnly(dataSource))
                {
                    nativePrepared(dataSource, isOnlyMusic);
                }
            }

            @Override
            public void onCutVideoImg(Bitmap bitmap) {
                if(onCutVideoImgListener != null)
                {
                    onCutVideoImgListener.onCutVideoImg(bitmap);
                }
            }
        });
    }


    /**
     * 准备
     * @param url
     */
    private native void nativePrepared(String url, boolean isOnlyMusic);

    /**
     * 开始
     */
    private native void nativeStart();

    /**
     * 停止并释放资源
     */
    private native void nativeStop(boolean exit);

    /**
     * 暂停
     */
    private native void nativePause();

    /**
     * 播放 对应暂停
     */
    private native void nativeResume();

    /**
     * seek
     * @param secds
     */
    private native void nativeSeek(int secds);

    /**
     * 设置音轨 根据获取的音轨数 排序
     * @param index
     */
    private native void nativeSetAudioChannels(int index);

    /**
     * 获取总时长
     * @return
     */
    private native int nativeGetDuration();

    /**
     * 获取音轨数
     * @return
     */
    private native int nativeGetAudioChannels();

    /**
     * 获取视频宽度
     * @return
     */
    private native int nativeGetVideoWidth();

    /**
     * 获取视频长度
     * @return
     */
    private native int nativeGetVideoHeight();

    public int getDuration()
    {
        return nativeGetDuration();
    }

    public int getAudioChannels()
    {
        return nativeGetAudioChannels();
    }

    public int getVideoWidth()
    {
        return nativeGetVideoWidth();
    }

    public int getVideoHeight()
    {
        return nativeGetVideoHeight();
    }

    public void setAudioChannels(int index)
    {
        nativeSetAudioChannels(index);
    }



    public void setOnPreparedListener(SoakOnPreparedListener soakOnPreparedListener) {
        this.soakOnPreparedListener = soakOnPreparedListener;
    }


    public void setVideoSizeChangedListener(SoakOnVideoSizeChangedListener videoSizeChangedListener) {
        this.videoSizeChangedListener = videoSizeChangedListener;
    }

    public void setOnErrorListener(SoakOnErrorListener soakOnErrorListener) {
        this.soakOnErrorListener = soakOnErrorListener;
    }

    public void prepared()
    {
        if(TextUtils.isEmpty(dataSource))
        {
            onError(SoakStatus.STATUS_DATASOURCE_NULL, "datasource is null");
            return;
        }
        parpared = true;
        if(isOnlyMusic)
        {
            nativePrepared(dataSource, isOnlyMusic);
        }
        else
        {
            if(surface != null)
            {
                nativePrepared(dataSource, isOnlyMusic);
            }
        }
    }

    public void start()
    {
        new Thread(() -> {
            if(TextUtils.isEmpty(dataSource))
            {
                onError(SoakStatus.STATUS_DATASOURCE_NULL, "datasource is null");
                return;
            }
            if(!isOnlyMusic)
            {
                if(surface == null)
                {
                    onError(SoakStatus.STATUS_SURFACE_NULL, "surface is null");
                    return;
                }
            }

            if(soakTimeBean == null)
            {
                soakTimeBean = new SoakTimeBean();
            }
            nativeStart();
        }).start();
    }

    public void stop(final boolean exit)
    {
       // new Thread(() -> {
            nativeStop(exit);
            if(mediaCodec != null)
            {
                try
                {
                    mediaCodec.flush();
                    mediaCodec.stop();
                    mediaCodec.release();
                }
                catch (Exception e)
                {
                    e.printStackTrace();
                }
                mediaCodec = null;
                mediaFormat = null;
            }
            if(glSurfaceView != null)
            {
                glSurfaceView.setCodecType(-1);
                glSurfaceView.requestRender();
            }

       // }).start();
    }

    public void pause()
    {
        nativePause();

    }

    public void resume()
    {
        nativeResume();
    }

    public void seek(final int secds)
    {
      //  new Thread(() -> {
            nativeSeek(secds);
            lastCurrTime = secds;
      //  }).start();
    }

    public void setOnlySoft(boolean soft)
    {
        this.isOnlySoft = soft;
    }

    public boolean isOnlySoft()
    {
        return isOnlySoft;
    }



    private void onLoad(boolean load)
    {
        if(onLoadListener != null)
        {
            onLoadListener.onLoad(load);
        }
    }

    private void onError(int code, String msg)
    {
        if(soakOnErrorListener != null)
        {
            soakOnErrorListener.onError(code, msg);
        }
        stop(true);
    }

    private void onParpared()
    {
        if(soakOnPreparedListener != null)
        {
            soakOnPreparedListener.onPrepared();
        }
    }

    public void mediacodecInit(int mimetype, int width, int height, byte[] csd0, byte[] csd1)
    {
        if(surface != null)
        {
            try {
                glSurfaceView.setCodecType(1);
                String mtype = getMimeType(mimetype);
                mediaFormat = MediaFormat.createVideoFormat(mtype, width, height);
                mediaFormat.setInteger(MediaFormat.KEY_WIDTH, width);
                mediaFormat.setInteger(MediaFormat.KEY_HEIGHT, height);
                mediaFormat.setLong(MediaFormat.KEY_MAX_INPUT_SIZE, width * height);
                mediaFormat.setByteBuffer("csd-0", ByteBuffer.wrap(csd0));
                mediaFormat.setByteBuffer("csd-1", ByteBuffer.wrap(csd1));
                mediaCodec = MediaCodec.createDecoderByType(mtype);
                if(surface != null)
                {
                    mediaCodec.configure(mediaFormat, surface, null, 0);
                    mediaCodec.start();
                }
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        else
        {
            if(soakOnErrorListener != null)
            {
                soakOnErrorListener.onError(SoakStatus.STATUS_SURFACE_NULL, "surface is null");
            }
        }
    }

    public void mediacodecDecode(byte[] bytes, int size, int pts)
    {
        if(bytes != null && mediaCodec != null && info != null)
        {
            try
            {
                int inputBufferIndex = mediaCodec.dequeueInputBuffer(10);
                if(inputBufferIndex >= 0)
                {
                    ByteBuffer byteBuffer = mediaCodec.getInputBuffers()[inputBufferIndex];
                    byteBuffer.clear();
                    byteBuffer.put(bytes);
                    mediaCodec.queueInputBuffer(inputBufferIndex, 0, size, pts, 0);
                }
                int index = mediaCodec.dequeueOutputBuffer(info, 10);
                while (index >= 0) {
                    mediaCodec.releaseOutputBuffer(index, true);
                    index = mediaCodec.dequeueOutputBuffer(info, 10);
                }
            }catch (Exception e)
            {
                e.printStackTrace();
            }
        }
    }

    public void setOnLoadListener(SoakOnLoadListener onLoadListener) {
        this.onLoadListener = onLoadListener;
    }

    private String getMimeType(int type)
    {
        if(type == 1)
        {
            return "video/avc";
        }
        else if(type == 2)
        {
            return "video/hevc";
        }
        else if(type == 3)
        {
            return "video/mp4v-es";
        }
        else if(type == 4)
        {
            return "video/x-ms-wmv";
        }
        return "";
    }

    public void setFrameData(int w, int h, byte[] y, byte[] u, byte[] v)
    {
        if(glSurfaceView != null)
        {
            SoakLog.d("setFrameData");
            glSurfaceView.setCodecType(0);
            glSurfaceView.setFrameData(w, h, y, u, v);
        }
    }

    public void setOnInfoListener(SoakOnInfoListener onInfoListener) {
        this.onInfoListener = onInfoListener;
    }

    public void setVideoInfo(int currentSeconds, int totalSeconds)
    {
        if(onInfoListener != null && soakTimeBean != null)
        {
            if(currentSeconds < lastCurrTime)
            {
                currentSeconds = lastCurrTime;
            }
            soakTimeBean.setCurrentSeconds(currentSeconds);
            soakTimeBean.setTotalSeconds(totalSeconds);
            onInfoListener.onInfo(soakTimeBean);
            lastCurrTime = currentSeconds;
        }
    }

    public void setOnCompleteListener(SoakOnCompleteListener onCompleteListener) {
        this.onCompleteListener = onCompleteListener;
    }

    public void videoComplete()
    {
        if(onCompleteListener != null)
        {
            setVideoInfo(nativeGetDuration(), nativeGetDuration());
            soakTimeBean = null;
            onCompleteListener.onComplete();
        }
    }

    public void setOnCutVideoImgListener(SoakOnCutVideoImgListener onCutVideoImgListener) {
        this.onCutVideoImgListener = onCutVideoImgListener;
    }

    public void cutVideoImg()
    {
        if(glSurfaceView != null)
        {
            glSurfaceView.cutVideoImg();
        }
    }

    public void setOnStopListener(SoakOnStopListener onStopListener) {
        this.onStopListener = onStopListener;
    }

    public void onStopComplete()
    {
        if(onStopListener != null)
        {
            onStopListener.onStop();
        }
    }

    public void onVideoSizeChanged(int width,int height,float dar){
        if (videoSizeChangedListener != null ){
            videoSizeChangedListener.onVideoSizeChanged(width, height, dar);
        }
    }
}
