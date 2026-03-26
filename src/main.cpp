#include <cstdint>
#include <cstring>
#include <android/log.h>
#include <nise/stub.h>

extern "C" {
#include "mem.h"
#include "memscan.h"
}

#define TAG "EPPL"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TAG, __VA_ARGS__)

struct PatchInfo {
    uintptr_t addr;
    uint8_t original[8];
    size_t size;
    bool applied;
};

static PatchInfo pistonA{};
static PatchInfo pistonB{};

static uintptr_t FindPattern(const char* pattern) {
    sigscan_handle* handle = sigscan_setup(pattern, "libminecraftpe.so", 0);
    //if(!handle) return 0;
    void* addr = get_sigscan_result(handle);
    sigscan_cleanup(handle);
    return (uintptr_t)addr;
}

static bool ApplyPatch(PatchInfo& info, uintptr_t addr, const uint8_t* patch, size_t size) {
    if (!addr) return false;

    info.addr = addr;
    info.size = size;

    memcpy(info.original, (void*)addr, size);

    if (!write_mem((void*)addr, (void*)patch, size))
        return false;

    info.applied = true;
    return true;
}

static bool RestorePatch(PatchInfo& info) {
    if (!info.applied || !info.addr)
        return false;

    if (!write_mem((void*)info.addr, info.original, info.size))
        return false;

    info.applied = false;
    return true;
}

static bool PatchPistonLimit_A() {
#if !defined(__aarch64__)
    return false;
#endif

    uintptr_t addr = FindPattern("09 9B 1F 31 00 F1 E9 00 00 54");
    if (!addr) {
        LOGE("Signature(A) not found");
        return false;
    }

    LOGI("Piston(A): %p", (void*)addr);

    const uint8_t patch[] = { 0xFD, 0x3F, 0xB1 };
    return ApplyPatch(pistonA, addr + 3, patch, sizeof(patch));
}

static bool PatchPistonLimit_B() {
#if !defined(__aarch64__)
    return false;
#endif

    uintptr_t addr = FindPattern("09 9B 1F 35 00 F1 E0 27 9F 1A");
    if (!addr) {
        LOGE("Signature(B) not found");
        return false;
    }

    LOGI("Piston(B): %p", (void*)addr);

    const uint8_t patch[] = { 0xFD, 0x3F, 0xB1 };
    return ApplyPatch(pistonB, addr + 3, patch, sizeof(patch));
}

static bool PatchAll() {
    if (!PatchPistonLimit_A()) return false;
    if (!PatchPistonLimit_B()) return false;

    LOGI("All patches applied");
    return true;
}

static void UnpatchAll() {
    RestorePatch(pistonA);
    RestorePatch(pistonB);
    LOGI("All patches restored");
}

__attribute__((constructor))
void EPPL_Init() {
    PatchAll();
}

__attribute__((destructor))
void EPPL_Shutdown() {
    UnpatchAll();
}