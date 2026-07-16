#include <jni.h>
#include <android/log.h>
#include <stdint.h>
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

typedef void (*EntityRender_t)(void* self, Actor* actor, void* region, const Vec3& cameraPos, float partialTicks);
EntityRender_t orig_EntityRender = nullptr;

void hook_EntityRender(void* self, Actor* actor, void* region, const Vec3& cameraPos, float partialTicks) {
    if (actor != nullptr) {
        float dx = actor->pos.x - cameraPos.x;
        float dy = actor->pos.y - cameraPos.y;
        float dz = actor->pos.z - cameraPos.z;
        float distanceSq = (dx * dx) + (dy * dy) + (dz * dz);

        // 40 Blocks Culling Radius (40 * 40 = 1600)
        // Stops rendering entities past this boundary
        if (distanceSq > 1600.0f) {
            return; 
        }
    }

    if (orig_EntityRender) {
        orig_EntityRender(self, actor, region, cameraPos, partialTicks);
    }
}

__attribute__((constructor))
void initializeCuller() {
    LOGI("EntityCuller Engine Initialized!");

    void* handle = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (!handle) {
        LOGI("Failed to link libminecraftpe.so");
        return;
    }

    // Dynamic rendering function hook target symbol
    void* target_function = dlsym(handle, "_ZN13ActorRenderer6renderER5ActorRK4Vec3f"); 
    if (target_function) {
        DobbyHook(target_function, (dobby_dummy_func_t)hook_EntityRender, (dobby_dummy_func_t*)&orig_EntityRender);
        LOGI("Successfully Hooked ActorRenderer!");
    } else {
        LOGI("Could not locate target rendering function symbol.");
    }
}
