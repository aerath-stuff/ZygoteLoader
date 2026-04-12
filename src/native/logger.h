#ifndef LOGGER_H
#define LOGGER_H

#include <stdlib.h>

#include <android/log.h>

#define TAG "ZygoteLoader"

#ifdef DEBUG
  #define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
  #define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)
#else
  #define LOGD(...)
  #define LOGE(...)
#endif

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGF(...) do { __android_log_print(ANDROID_LOG_FATAL, TAG, __VA_ARGS__); abort(); } while (0)

#endif /* LOGGER_H */
