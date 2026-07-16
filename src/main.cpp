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

// Abstracted player interface to call Minecraft's text message function
struct Player {
    virtual void displayChatMessage(const std::string& msg, const std::string& author) = 0;
    virtual void displayClientMessage(const std::string& msg) = 0; // Sends action bar/system alerts
};

typedef void (*EntityRender_t)(void* self, Actor* actor, void* region, const Vec3& cameraPos, float partialTicks);
EntityRender_t orig_EntityRender = nullptr;

// Active culling logic with screen feedback
void hook_EntityRender(void* self, Actor* actor, void* region, const Vec3& cameraPos, float partialTicks) {
    Player* localPlayer = reinterpret_cast<Player*>(self);
    
    // §a turns the text green, §l makes it bold in Minecraft Bedrock
    if (localPlayer != nullptr) {
        localPlayer->displayClientMessage("§a§l[EntityCuller: Working]");
    }

    if (actor != nullptr) {
        float dx = actor->pos.x - cameraPos.x;
        float dy = actor->pos.y - cameraPos.y;
        float dz = actor->pos.z - cameraPos.z;
        float distanceSq = (dx * dx) + (dy * dy) + (dz * dz);

        // If further than 40 blocks, drop the render load entirely
        if (distanceSq > 1600.0f) {
            return; 
        }
    }

    if (orig_EntityRender) {
        orig_EntityRender(self, actor, region, cameraPos, partialTicks);
    }
}

// Entry constructor: Invoked instantly by LeviLauncher during game startup
__attribute__((constructor))
void initializeCuller() {
    LOGI("EntityCuller Preload Loaded!");

    // 1. Locate the Minecraft game library in memory
    void* handle = dlopen("libminecraftpe.so", RTLD_LAZY);
    if (!handle) {
        LOGI("Failed to find libminecraftpe.so!");
        return;
    }

    // 2. Find the internal Entity Rendering function
    // (Note: This address/symbol changes depending on your exact version. This is a common template lookup symbol)
    void* target_function = dlsym(handle, "_ZN13ActorRenderer6renderER5ActorRK4Vec3f"); 

    if (target_function) {
        // 3. Inject our hook using Dobby!
        DobbyHook(target_function, (dobby_dummy_func_t)hook_EntityRender, (dobby_dummy_func_t*)&orig_EntityRender);
        LOGI("Successfully Hooked ActorRenderer!");
    } else {
        LOGI("Could not locate the rendering target symbol.");
    }
}
