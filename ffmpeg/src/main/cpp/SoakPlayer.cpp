#include <jni.h>
#include <cstddef>
#include "AndroidLog.h"
#include "SoakJavaCall.h"
#include "SoakFFmpeg.h"

_JavaVM *javaVM = nullptr;
SoakJavaCall *pJavaCall = nullptr;
SoakFFmpeg *pFFmpeg = nullptr;

extern "C"
JNIEXPORT void JNICALL
Java_com_jiayz_ffmpeg_soakplayer_SoakPlayer_nativePrepared(JNIEnv *env, jobject instance, jstring url_, jboolean isOnlyMusic) {
    const char *url = env->GetStringUTFChars(url_, 0);
    // TODO
    if(pJavaCall == nullptr)
    {
        pJavaCall = new SoakJavaCall(javaVM, env, &instance);
    }
    if(pFFmpeg == nullptr)
    {
        pFFmpeg = new SoakFFmpeg(pJavaCall, url, isOnlyMusic);
        pJavaCall->onLoad(SOAK_THREAD_MAIN, true);
        pFFmpeg->preparedFFmpeg();
    }
}


extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
    jint result = -1;
    javaVM = vm;
    JNIEnv* env;

    if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK)
    {
        LOGE("GetEnv failed!")
        return result;
    }
    return JNI_VERSION_1_4;
}extern "C"
JNIEXPORT void JNICALL
Java_com_jiayz_ffmpeg_soakplayer_SoakPlayer_nativeStart(JNIEnv *env, jobject instance) {

    // TODO
    if(pFFmpeg != nullptr)
    {
        pFFmpeg->start();
    }

}

extern "C"
JNIEXPORT void JNICALL
Java_com_jiayz_ffmpeg_soakplayer_SoakPlayer_nativeStop(JNIEnv *env, jobject instance, jboolean exit) {
    // TODO
    if(pFFmpeg != nullptr)
    {

        pFFmpeg->exitByUser = true;
        pFFmpeg->release();
        delete(pFFmpeg);
        pFFmpeg = nullptr;
        if(pJavaCall != nullptr)
        {
            pJavaCall->release();
            pJavaCall = nullptr;
        }
        if(!exit)
        {
            jclass jlz = env->GetObjectClass(instance);
            jmethodID jmid_stop = env->GetMethodID(jlz, "onStopComplete", "()V");
            env->CallVoidMethod(instance, jmid_stop);
        }
    }

}extern "C"
JNIEXPORT void JNICALL
Java_com_jiayz_ffmpeg_soakplayer_SoakPlayer_nativePause(JNIEnv *env, jobject instance) {

    // TODO
    if(pFFmpeg != nullptr)
    {
        pFFmpeg->pause();
    }

}extern "C"
JNIEXPORT void JNICALL
Java_com_jiayz_ffmpeg_soakplayer_SoakPlayer_nativeResume(JNIEnv *env, jobject instance) {

    // TODO
    if(pFFmpeg != nullptr)
    {
        pFFmpeg->resume();
    }

}extern "C"
JNIEXPORT void JNICALL
Java_com_jiayz_ffmpeg_soakplayer_SoakPlayer_nativeSeek(JNIEnv *env, jobject instance, jint secds) {

    // TODO
    if(pFFmpeg != nullptr)
    {
        pFFmpeg->seek(secds);
    }

}extern "C"
JNIEXPORT jint JNICALL
Java_com_jiayz_ffmpeg_soakplayer_SoakPlayer_nativeGetDuration(JNIEnv *env, jobject instance) {

    // TODO
    if(pFFmpeg != nullptr)
    {
        return pFFmpeg->getDuration();
    }
    return 0;

}extern "C"
JNIEXPORT jint JNICALL
Java_com_jiayz_ffmpeg_soakplayer_SoakPlayer_nativeGetAudioChannels(JNIEnv *env, jobject instance) {

    if(pFFmpeg != nullptr)
    {
        return pFFmpeg->getAudioChannels();
    }
    return 0;
}extern "C"
JNIEXPORT jint JNICALL
Java_com_jiayz_ffmpeg_soakplayer_SoakPlayer_nativeGetVideoWidth(JNIEnv *env, jobject instance) {

    // TODO
    if(pFFmpeg != nullptr)
    {
        pFFmpeg->getVideoWidth();
    }

}extern "C"
JNIEXPORT jint JNICALL
Java_com_jiayz_ffmpeg_soakplayer_SoakPlayer_nativeGetVideoHeight(JNIEnv *env, jobject instance) {

    // TODO
    if(pFFmpeg != nullptr)
    {
        pFFmpeg->getVideoHeight();
    }

}extern "C"
JNIEXPORT void JNICALL
Java_com_jiayz_ffmpeg_soakplayer_SoakPlayer_nativeSetAudioChannels(JNIEnv *env, jobject instance, jint index) {

    // TODO
    if(pFFmpeg != nullptr)
    {
        pFFmpeg->setAudioChannel(index);
    }

}
