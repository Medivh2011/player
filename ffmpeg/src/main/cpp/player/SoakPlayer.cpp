#include <jni.h>
#include <string>
#include "SoakFFmpeg.h"
#include "PlayStatus.h"

extern "C"
{
#include <libavformat/avformat.h>
}

_JavaVM *javaVM = nullptr;
SoakCallJava *callJava = nullptr;
SoakFFmpeg *fFmpeg = nullptr;
PlayStatus *playstatus = nullptr;

bool nexit = true;
pthread_t thread_start;

extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
    jint result = -1;
    javaVM = vm;
    JNIEnv *env;
    if(vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK)
    {

        return result;
    }
    return JNI_VERSION_1_4;

}

extern "C" JNIEXPORT void JNICALL
Java_com_jiayz_ffmpeg_soakplayer_SoakPlayer__1prepare(
        JNIEnv *env,
        jobject /* this */obj,
        jstring url) {
    const char *m_url = env->GetStringUTFChars(url, 0);

    if(fFmpeg == nullptr)
    {
        if(callJava == nullptr)
        {
            callJava = new SoakCallJava(javaVM, env, &obj);
        }
        callJava->onCallLoad(MAIN_THREAD, true);
        playstatus = new PlayStatus();
        fFmpeg = new SoakFFmpeg(playstatus, callJava, m_url);
        fFmpeg->prepare();
    }
}

void *startCallBack(void *data)
{
    auto *fFmpeg = (SoakFFmpeg *) data;
    fFmpeg->start();
    return 0;
}

extern "C" JNIEXPORT void JNICALL
Java_com_jiayz_ffmpeg_soakplayer_SoakPlayer__1start(
        JNIEnv *env,
        jobject /* this */) {
    if (fFmpeg != nullptr) {
        pthread_create(&thread_start, nullptr, startCallBack, fFmpeg);
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_jiayz_ffmpeg_soakplayer_SoakPlayer__1pause(
        JNIEnv *env,
        jobject /* this */) {
    if(fFmpeg != nullptr)
    {
        fFmpeg->pause();
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_jiayz_ffmpeg_soakplayer_SoakPlayer__1resume(
        JNIEnv *env,
        jobject /* this */) {
    if(fFmpeg != nullptr)
    {
        fFmpeg->resume();
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_jiayz_ffmpeg_soakplayer_SoakPlayer__1seek(
        JNIEnv *env,
        jobject /* this */,
        jint secds) {
    if(fFmpeg != nullptr)
    {
        fFmpeg->seek(secds);
    }

}

extern "C" JNIEXPORT void JNICALL
Java_com_jiayz_ffmpeg_soakplayer_SoakPlayer__1stop(
        JNIEnv *env,
        jobject /* this */obj) {

    if(!nexit)
    {
        return;
    }

    jclass clz = env->GetObjectClass(obj);
    jmethodID jmid_next = env->GetMethodID(clz, "onCallNext", "()V");

    nexit = false;
    if(fFmpeg != nullptr)
    {
        fFmpeg->release();
        pthread_join(thread_start, nullptr);
        delete(fFmpeg);
        fFmpeg = nullptr;
        if(callJava != nullptr)
        {
            delete(callJava);
            callJava = nullptr;
        }
        if(playstatus != nullptr)
        {
            delete(playstatus);
            playstatus = nullptr;
        }
    }
    nexit = true;
    env->CallVoidMethod(obj, jmid_next);

}
