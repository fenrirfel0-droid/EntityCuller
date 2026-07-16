#include <jni.h>
#include <android/log.h>

#define LOG_TAG "EntityCuller"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    LOGI("EntityCuller: Native module loaded safely on Android 15.");
    return JNI_VERSION_1_6;
}
