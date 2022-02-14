package com.jiayz.ffmpeg.soakplayer;

import android.media.MediaCodec;
import android.media.MediaFormat;
import android.text.TextUtils;
import android.view.Surface;


import com.jiayz.ffmpeg.model.SoakTimeInfo;
import com.jiayz.ffmpeg.opengles.SoakGLSurfaceView;
import com.jiayz.ffmpeg.opengles.SoakRender;
import com.jiayz.ffmpeg.util.LogUtils;
import com.jiayz.ffmpeg.util.VideoSupportUtils;

import java.nio.ByteBuffer;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class SoakPlayer {

    static {
        System.loadLibrary("soak_player");
    }

    private native void _prepare(String url);
    private native void _start();
    private native void _pause();
    private native void _resume();
    private native void _stop();
    private native void _seek(int position);

    private String mUrl;
    private OnPreparedListener mOnPreparedListener;
    private OnVideoSizeChanged mOnVideoSizeChangedListener;
    private OnLoadListener mOnLoadListener;
    private OnPauseResumeListener mOnPauseResumeListener;
    private OnTimeUpdateListener mOnTimeUpdateListener;
    private OnErrorListener mOnErrorListener;
    private OnCompleteListener mOnCompleteListener;

    private SoakGLSurfaceView mSurfaceView;
    private Surface mSurface;
    private int mDuration = 0;
    private static boolean playNext = false;
    private SoakTimeInfo mTimeInfo;
    private MediaCodec mCodec;
    private MediaFormat mFormat;
    private MediaCodec.BufferInfo mBufferInfo;

    private ExecutorService service = Executors.newSingleThreadExecutor();

    public SoakPlayer() {}

    public void setDataSource(String url) {
        this.mUrl = url;
    }

    public void setGLSurfaceView(SoakGLSurfaceView surfaceView) {
        this.mSurfaceView = surfaceView;
        mSurfaceView.getRender().setOnSurfaceCreateListener(new SoakRender.OnSurfaceCreateListener() {
            @Override
            public void onSurfaceCreate(Surface s) {
                if(mSurface == null) {
                    mSurface = s;
                    LogUtils.d("onSurfaceCreate");
                }
            }
        });
    }

    public void setOnPreparedListener(OnPreparedListener listener) {
        this.mOnPreparedListener = listener;
    }

    public void setOnVideoSizeChangedListener(OnVideoSizeChanged listener) {
        this.mOnVideoSizeChangedListener = listener;
    }

    public void setOnLoadListener(OnLoadListener listener) {
        this.mOnLoadListener = listener;
    }

    public void setOnPauseResumeListener(OnPauseResumeListener listener) {
        this.mOnPauseResumeListener = listener;
    }

    public void setOnTimeUpdateListener(OnTimeUpdateListener listener) {
        this.mOnTimeUpdateListener = listener;
    }

    public void setOnErrorListener(OnErrorListener listener) {
        this.mOnErrorListener = listener;
    }

    public void setOnCompleteListener(OnCompleteListener listener) {
        this.mOnCompleteListener = listener;
    }


    public void prepare()
    {
        if(TextUtils.isEmpty(mUrl))
        {
            LogUtils.d("source not be empty");
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                _prepare(mUrl);
            }
        }).start();

    }

    public void start()
    {
        if(TextUtils.isEmpty(mUrl))
        {
            LogUtils.d("source is empty");
            return;
        }
        new Thread(new Runnable() {
            @Override
            public void run() {
                _start();
            }
        }).start();
    }

    public void pause()
    {
        _pause();
        if(mOnPauseResumeListener != null)
        {
            mOnPauseResumeListener.onPause(true);
        }
    }

    public void resume()
    {
        _resume();
        if(mOnPauseResumeListener != null)
        {
            mOnPauseResumeListener.onPause(false);
        }
    }

    public void stop()
    {
        mTimeInfo = null;
        mDuration = 0;
        new Thread(() -> {
            _stop();
            releaseMediaCodec();
        }).start();
    }

    public void seek(int seconds)
    {
        _seek(seconds);
    }

    public void playNext(String url)
    {
        mUrl = url;
        playNext = true;
        stop();
    }

    public int getDuration() {
        return mDuration;
    }

    private void releaseMediaCodec()
    {
        if(mCodec != null)
        {
            try
            {
                mCodec.flush();
                mCodec.stop();
                mCodec.release();
            }
            catch(Exception e)
            {
                //e.printStackTrace();
            }
            mCodec = null;
            mFormat = null;
            mBufferInfo = null;
        }

    }


    public interface OnCompleteListener {
        void onComplete();
    }

    public interface OnErrorListener {
        void onError(int err, String msg);
    }

    public interface OnLoadListener {
        void onLoad(boolean isLoad);
    }

    public interface OnPreparedListener {
        void onPrepared();
    }

    public interface OnPauseResumeListener {
        void onPause(boolean pause);
    }

    public interface OnTimeUpdateListener {
        void onTimeUpdate(SoakTimeInfo info);
    }

    public interface OnVideoSizeChanged {
        void onVideoSizeChanged(int width, int height, float dar);
    }

    //CalledByNative
    public void onCallPrepared() {
        if(mOnPreparedListener != null) {
            mOnPreparedListener.onPrepared();
        }
    }

    //CalledByNative
    public void onCallLoad(boolean load) {
        if(mOnLoadListener != null) {
            mOnLoadListener.onLoad(load);
        }
    }

    //CalledByNative
    public void onCallTimeInfo(int currentTime, int totalTime) {
        if(mOnTimeUpdateListener != null) {
            if(mTimeInfo == null) {
                mTimeInfo = new SoakTimeInfo();
            }
            mDuration = totalTime;
            mTimeInfo.setCurrentTime(currentTime);
            mTimeInfo.setTotalTime(totalTime);
            mOnTimeUpdateListener.onTimeUpdate(mTimeInfo);
        }
    }

    //CalledByNative
    public void onCallError(int code, String msg) {
        if(mOnErrorListener != null) {
            stop();
            mOnErrorListener.onError(code, msg);
        }
    }

    //CalledByNative
    public void onCallComplete() {
        if(mOnCompleteListener != null) {
            stop();
            mOnCompleteListener.onComplete();
        }
    }

    //CalledByNative
    public void onCallNext() {
        if(playNext) {
            playNext = false;
            prepare();
        }
    }

    //CalledByNative
    public void onCallRenderYUV(int width, int height, byte[] y, byte[] u, byte[] v) {
        LogUtils.d("获取到视频的yuv数据");
        if(mSurfaceView != null)
        {
            mSurfaceView.getRender().setRenderType(SoakRender.RENDER_YUV);
            mSurfaceView.setYUVData(width, height, y, u, v);
        }
    }

    //CalledByNative
    public boolean onCallIsSupportMediaCodec(String ffCodecName) {
        return VideoSupportUtils.isSupportCodec(ffCodecName);
    }

    //CalledByNative
    public void initMediaCodec(String codecName, int width, int height, byte[] csd_0, byte[] csd_1) {
        if(mSurface != null) {
            try {
                mSurfaceView.getRender().setRenderType(SoakRender.RENDER_MEDIACODEC);
                String mime = VideoSupportUtils.findVideoCodecName(codecName);
                mFormat = MediaFormat.createVideoFormat(mime, width, height);
                mFormat.setInteger(MediaFormat.KEY_MAX_INPUT_SIZE, width * height);
                mFormat.setByteBuffer("csd-0", ByteBuffer.wrap(csd_0));
                mFormat.setByteBuffer("csd-1", ByteBuffer.wrap(csd_1));
                LogUtils.d(mFormat.toString());
                mCodec = MediaCodec.createDecoderByType(mime);
                mBufferInfo = new MediaCodec.BufferInfo();
                mCodec.configure(mFormat, mSurface, null, 0);
                mCodec.start();

            }
            catch (Exception e) {
                e.printStackTrace();
            }
        }
        else {
            if(mOnErrorListener != null)
            {
                mOnErrorListener.onError(2001, "surface is null");
            }
        }
    }

    //CalledByNative
    public void decodeAVPacket(int datasize, byte[] data) {
        if(mSurface != null && datasize > 0 && data != null && mCodec != null) {
            try {
                int intputBufferIndex = mCodec.dequeueInputBuffer(10);
                if(intputBufferIndex >= 0) {
                    ByteBuffer byteBuffer = mCodec.getInputBuffers()[intputBufferIndex];
                    byteBuffer.clear();
                    byteBuffer.put(data);
                    mCodec.queueInputBuffer(intputBufferIndex, 0, datasize, 0, 0);
                }
                int outputBufferIndex = mCodec.dequeueOutputBuffer(mBufferInfo, 10);
                while(outputBufferIndex >= 0) {
                    mCodec.releaseOutputBuffer(outputBufferIndex, true);
                    outputBufferIndex = mCodec.dequeueOutputBuffer(mBufferInfo, 10);
                }
            }
            catch (Exception e) {
                e.printStackTrace();
            }

        }
    }

    //CalledByNative
    public void onCallVideoSizeChanged(int width, int height, float dar) {
        LogUtils.d("onCallVideoSizeChanged="+width+", height="+height + ", dar="+dar);
        if (mOnVideoSizeChangedListener != null ){
            if (Float.compare(dar,Float.NaN) == 0) dar = 1.7778f;
            mOnVideoSizeChangedListener.onVideoSizeChanged(width, height, dar);
        }

    }

}
