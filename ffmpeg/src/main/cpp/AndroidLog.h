#ifndef _ANDROIDLOG_H
#define _ANDROIDLOG_H

#include <android/log.h>

#define LOG_DEBUG 0
#define LOG_ERROR 1
#define LOG_ERROR_P 0
#define LOG_FFMPEG_TOOLS 0

#if LOG_DEBUG
#define LOGD(FORMAT,...) __android_log_print(ANDROID_LOG_DEBUG, "JiayzJNI", FORMAT,##__VA_ARGS__);
#else
#define LOGD(FORMAT,...)
#endif

#if LOG_ERROR
#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR, "JiayzJNI", FORMAT,##__VA_ARGS__);
#else
#define LOGE(FORMAT,...)
#endif

#if LOG_ERROR_P
#define LOGP(FORMAT,...) __android_log_print(ANDROID_LOG_WARN, "JiayzJNI", FORMAT,##__VA_ARGS__);
#else
#define LOGP(FORMAT,...)
#endif

#endif