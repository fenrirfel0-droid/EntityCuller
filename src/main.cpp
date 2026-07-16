#include <jni.h>
#include <android/log.h>
#include <stdint.h>
#include <math.h>
#include <string>
#include <dlfcn.h>
#include "dobby.h"

#define LOG_TAG "EntityCuller"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)

struct Vec3 {
    float x, y, z;
};

struct Actor {
    Vec3 pos; 
};

struct Player {
    virtual void displayChatMessage(const std::string& msg, const std::string& author) = 0;
    virtual void displayClientMessage(const std::string& msg) = 0;
};

typedef void (*EntityRender_t)(void* self, Actor* actor, void* region, const Vec3& cameraPos, float partialTicks);
EntityRender_t orig_EntityRender = nullptr;

void hook_EntityRender(void* self, Actor* actor, void* region, const Vec3& cameraPos, float partialTicks) {
    if (actor != nullptr) {
        float dx = actor->pos.x - cameraPos.x;
        float dy = actor->pos.y - cameraPos.y;
        float dz = actor->pos.z - cameraPos.z;
        float distanceSq = (dx * dx) + (dy * dy) + (dz * dz);

        // Render threshold check (40 block radius)
        if (distanceSq > 1600.0f) {
            return; 
        }
    }

    if (orig_EntityRender) {
        orig_EntityRender(self, actor, region, cameraPos, partialTicks);
    }
}

// Function to trigger a native Android Toast Notification
void showToast(JNIEnv* env, jobject context) {
    jclass toastClass = env->FindClass("android/widget/Toast");
    if (!toastClass) return;

    jmethodID makeTextMethod = env->GetStaticMethodID(toastClass, "makeText", 
        "(Landroid/content/Context;Ljava/lang/CharSequence;I)Landroid/widget/Toast;");
    if (!makeTextMethod) return;

    jstring text = env->NewStringUTF("Entity Culling: ON");
    jobject toastObj = env->CallStaticObjectMethod(toastClass, makeTextMethod, context, text, 0); // 0 = Short duration

    if (toastObj) {
        jmethodID showMethod = env->GetMethodID(toastClass, "show", "()V");
        if (showMethod) {
            env->CallVoidMethod(toastObj, showMethod);
        }
    }
}

// Automatically runs when LeviLauncher loads this library into the JVM
extern "C" jint JNI_OnLoad(JavaVM* vm, void* reserved) {
    JNIEnv* env;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) != JNI_OK) {
        return JNI_ERR;
    }

    // Try to retrieve the global context of Minecraft's active game activity
    jclass activityThread = env->FindClass("android/app/ActivityThread");
    if (activityThread) {
        jmethodID currentActivityThread = env->GetStaticMethodID(activityThread, "currentActivityThread", "()Landroid/app/ActivityThread;");
        jobject at = env->CallStaticObjectMethod(activityThread, currentActivityThread);
        if (at) {
            jmethodID getApplication = env->GetMethodID(activityThread, "getApplication", "()Landroid/app/Application;");
            jobject context = env->CallObjectMethod(at, getApplication);
            if (context) {
                showToast(env, context);
            }
        }
    }
    return JNI_VERSION_1_6;
}

__attribute__((constructor))
void initializeCuller() {
    LOGI("EntityCuller Preload Loaded!");

    void* handle = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (!handle) {
        LOGI("Failed to find libminecraftpe.so");
        return;
    }

    void* target_function = dlsym(handle, "_ZN13ActorRenderer6renderER5ActorRK4Vec3f"); 
    if (target_function) {
        DobbyHook(target_function, (dobby_dummy_func_t)hook_EntityRender, (dobby_dummy_func_t*)&orig_EntityRender);
        LOGI("Successfully Hooked ActorRenderer!");
    } else {
        LOGI("Could not locate target rendering function.");
    }
}
