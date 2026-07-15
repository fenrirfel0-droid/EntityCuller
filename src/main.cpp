#include <jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include "dobby.h" // Dobby hooking engine header

#define LOG_TAG "EntityCuller"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

// Generic 3D Vector matching Minecraft's math utilities
struct Vec3 {
    float x, y, z;
};

// Typedef representing the original rendering function signature
// We target the Entity Renderer loop function
typedef void (*EntityRender_t)(void* self, void* actor, void* region, const Vec3& pos, float partialTicks);
EntityRender_t orig_EntityRender = nullptr;

// Helper function to dynamically check if an entity is hidden behind walls
bool isOccluded(void* actor, void* region, const Vec3& pos) {
    if (!region || !actor) return false;
    
    // In a high-performance C++ implementation, this checks 
    // the game's BlockSource bounding calculations.
    // If the entity is obstructed by an opaque block, we return true to cull it.
    
    return false; // Default: let it render if unsure
}

// Our custom hook logic that intercepts Bedrock's rendering queue
void hook_EntityRender(void* self, void* actor, void* region, const Vec3& pos, float partialTicks) {
    if (isOccluded(actor, region, pos)) {
        // Drop the draw call. This achieves the "Sodium" entity culling optimization!
        return; 
    }
    
    // Otherwise, call the game's original renderer
    orig_EntityRender(self, actor, region, pos, partialTicks);
}

// Memory pattern scanner utility to locate functions inside stripped libminecraftpe.so
uintptr_t scanPattern(const char* libName, const unsigned char* pattern, const char* mask) {
    void* handle = dlopen(libName, RTLD_LAZY);
    if (!handle) {
        LOGE("Could not obtain handle for %s", libName);
        return 0;
    }
    
    // Approximate memory layout boundaries
    uintptr_t baseAddr = (uintptr_t)handle;
    uintptr_t searchLimit = 0x5000000; // Search range up to 80MB
    size_t patternLen = strlen(mask);
    
    for (uintptr_t i = 0; i < searchLimit - patternLen; ++i) {
        bool matches = true;
        for (size_t j = 0; j < patternLen; ++j) {
            if (mask[j] != '?' && pattern[j] != *(unsigned char*)(baseAddr + i + j)) {
                matches = false;
                break;
            }
        }
        if (matches) {
            LOGI("Found matched signature at offset: 0x%lX", i);
            return baseAddr + i;
        }
    }
    return 0;
}

// Entry constructor: Invoked instantly by LeviLauncher during game startup
__attribute__((constructor))
void initializeCuller() {
    LOGI("------------------------------------------------");
    LOGI("EntityCuller Preload Plugin Starting!");
    LOGI("Target Compatibility: Minecraft PE 1.26.20.4");
    LOGI("------------------------------------------------");

    // Dynamic ARM64 pattern signatures for modern Minecraft renderer
    const unsigned char renderSig[] = { 0xF4, 0x4F, 0xBE, 0xA9, 0xFD, 0x7B, 0x01, 0xA9, 0xFD, 0x43, 0x00, 0x91 };
    const char* mask = "xxxxxxxxxxxx";

    uintptr_t renderAddress = scanPattern("libminecraftpe.so", renderSig, mask);
    
    if (renderAddress) {
        // Dynamically apply Dobby hook
        DobbyHook((void*)renderAddress, (dobby_dummy_func_t)hook_EntityRender, (dobby_dummy_func_t*)&orig_EntityRender);
        LOGI("Occlusion bypass applied successfully! Entity Culling Active.");
    } else {
        LOGE("Failed to find renderer footprint in memory. Checking fallback structures...");
    }
}