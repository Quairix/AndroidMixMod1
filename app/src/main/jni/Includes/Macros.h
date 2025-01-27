// thanks to shmoo and joeyjurjens for the usefull stuff under this comment.
#ifndef ANDROID_MOD_MENU_MACROS_H
#define ANDROID_MOD_MENU_MACROS_H

#include <And64InlineHook/And64InlineHook.hpp>

void hook(void *offset, void* ptr, void **orig)
{
    A64HookFunction(offset, ptr, orig);
}

#define HOOK(offset, ptr, orig) hook((void *)getAbsoluteAddress(targetLibName, string2Offset(AY_OBFUSCATE(offset))), (void *)ptr, (void **)&orig)
#define HOOK_LIB(lib, offset, ptr, orig) hook((void *)getAbsoluteAddress(AY_OBFUSCATE(lib), string2Offset(AY_OBFUSCATE(offset))), (void *)ptr, (void **)&orig)

#define HOOK_NO_ORIG(offset, ptr) hook((void *)getAbsoluteAddress(targetLibName, string2Offset(AY_OBFUSCATE(offset))), (void *)ptr, NULL)
#define HOOK_LIB_NO_ORIG(lib, offset, ptr) hook((void *)getAbsoluteAddress(AY_OBFUSCATE(lib), string2Offset(AY_OBFUSCATE(offset))), (void *)ptr, NULL)

#define HOOKSYM(sym, ptr, org) hook(dlsym(dlopen(targetLibName, 4), AY_OBFUSCATE(sym)), (void *)ptr, (void **)&org)
#define HOOKSYM_LIB(lib, sym, ptr, org) hook(dlsym(dlopen(AY_OBFUSCATE(lib), 4), AY_OBFUSCATE(sym)), (void *)ptr, (void **)&org)

#define HOOKSYM_NO_ORIG(sym, ptr)  hook(dlsym(dlopen(targetLibName, 4), AY_OBFUSCATE(sym)), (void *)ptr, NULL)
#define HOOKSYM_LIB_NO_ORIG(lib, sym, ptr) hook(dlsym(dlopen(AY_OBFUSCATE(lib), 4), AY_OBFUSCATE(sym)), (void *)ptr, NULL)

std::vector<MemoryPatch> memoryPatches;
std::vector<uint64_t> offsetVector;

// Patching a offset without switch.
void patchOffset(const char *fileName, uint64_t offset, std::string hexBytes, bool isOn) {

    MemoryPatch patch = MemoryPatch::createWithHex(fileName, offset, hexBytes);

    //Check if offset exists in the offsetVector
    if (std::find(offsetVector.begin(), offsetVector.end(), offset) != offsetVector.end()) {
        //LOGE(AY_OBFUSCATE("Already exists"));
        std::vector<uint64_t>::iterator itr = std::find(offsetVector.begin(), offsetVector.end(), offset);
        patch = memoryPatches[std::distance(offsetVector.begin(), itr)]; //Get index of memoryPatches vector
    } else {
        memoryPatches.push_back(patch);
        offsetVector.push_back(offset);
        //LOGI(AY_OBFUSCATE("Added"));
    }

    if (!patch.isValid()) {
        LOGE(AY_OBFUSCATE("Failing offset: 0x%llu, please re-check the hex"), offset);
        return;
    }
    if (isOn) {
        if (!patch.Modify()) {
            LOGE(AY_OBFUSCATE("Something went wrong while patching this offset: 0x%llu"), offset);
        }
    } else {
        if (!patch.Restore()) {
            LOGE(AY_OBFUSCATE("Something went wrong while restoring this offset: 0x%llu"), offset);
        }
    }
}

void patchOffsetSym(uintptr_t absolute_address, std::string hexBytes, bool isOn) {

    MemoryPatch patch = MemoryPatch::createWithHex(absolute_address, hexBytes);

    //Check if offset exists in the offsetVector
    if (std::find(offsetVector.begin(), offsetVector.end(), absolute_address) != offsetVector.end()) {
        //LOGE(AY_OBFUSCATE("Already exists"));
        std::vector<uint64_t>::iterator itr = std::find(offsetVector.begin(), offsetVector.end(), absolute_address);
        patch = memoryPatches[std::distance(offsetVector.begin(), itr)]; //Get index of memoryPatches vector
    } else {
        memoryPatches.push_back(patch);
        offsetVector.push_back(absolute_address);
        //LOGI(AY_OBFUSCATE("Added"));
    }

    if (!patch.isValid()) {
        LOGE(AY_OBFUSCATE("Failing offset: 0x%llu, please re-check the hex"), absolute_address);
        return;
    }
    if (isOn) {
        if (!patch.Modify()) {
            LOGE(AY_OBFUSCATE("Something went wrong while patching this offset: 0x%llu"), absolute_address);
        }
    } else {
        if (!patch.Restore()) {
            LOGE(AY_OBFUSCATE("Something went wrong while restoring this offset: 0x%llu"), absolute_address);
        }
    }
}

#define PATCH(offset, hex) patchOffset(targetLibName, string2Offset(AY_OBFUSCATE(offset)), AY_OBFUSCATE(hex), true)
#define PATCH_LIB(lib, offset, hex) patchOffset(AY_OBFUSCATE(lib), string2Offset(AY_OBFUSCATE(offset)), AY_OBFUSCATE(hex), true)

#define PATCH_SYM(sym, hex) patchOffset(dlsym(dlopen(targetLibName, 4), AY_OBFUSCATE(sym)), AY_OBFUSCATE(hex), true)
#define PATCH_LIB_SYM(lib, sym, hex) patchOffset(dlsym(dlopen(lib, 4), AY_OBFUSCATE(sym)), AY_OBFUSCATE(hex), true)

#define PATCH_SWITCH(offset, hex, boolean) patchOffset(targetLibName, string2Offset(AY_OBFUSCATE(offset)), AY_OBFUSCATE(hex), boolean)
#define PATCH_LIB_SWITCH(lib, offset, hex, boolean) patchOffset(AY_OBFUSCATE(lib), string2Offset(AY_OBFUSCATE(offset)), AY_OBFUSCATE(hex), boolean)

#define PATCH_SYM_SWITCH(sym, hex, boolean) patchOffsetSym((uintptr_t)dlsym(dlopen(targetLibName, 4), AY_OBFUSCATE(sym)), AY_OBFUSCATE(hex), boolean)
#define PATCH_LIB_SYM_SWITCH(lib, sym, hex, boolean) patchOffsetSym((uintptr_t)dlsym(dlopen(lib, 4), AY_OBFUSCATE(sym)), AY_OBFUSCATE(hex), boolean)

#define RESTORE(offset) patchOffset(targetLibName, string2Offset(AY_OBFUSCATE(offset)), "", false)
#define RESTORE_LIB(lib, offset) patchOffset(AY_OBFUSCATE(lib), string2Offset(AY_OBFUSCATE(offset)), "", false)

#define RESTORE_SYM(sym) patchOffsetSym((uintptr_t)dlsym(dlopen(targetLibName, 4), AY_OBFUSCATE(sym)), "", false)
#define RESTORE_LIB_SYM(lib, sym) patchOffsetSym((uintptr_t)dlsym(dlopen(lib, 4), AY_OBFUSCATE(sym)), "", false)

#endif //ANDROID_MOD_MENU_MACROS_H