cmake_minimum_required(VERSION 3.30)
project(AndroidMixMod1)

set(CMAKE_CXX_STANDARD 17)

include_directories(app/src/main/jni/And64InlineHook)
include_directories(app/src/main/jni/Includes)
include_directories(app/src/main/jni/KittyMemory)
include_directories(app/src/main/jni/Localization)
include_directories(app/src/main/jni/Menu)
include_directories(app/src/main/jni/Substrate)

add_executable(AndroidMixMod1
    app/src/main/jni/And64InlineHook/And64InlineHook.cpp
    app/src/main/jni/And64InlineHook/And64InlineHook.hpp
    app/src/main/jni/Includes/il2cpp.h
    app/src/main/jni/Includes/Logger.h
    app/src/main/jni/Includes/Macros.h
    app/src/main/jni/Includes/obfuscate.h
    app/src/main/jni/Includes/Offsets.h
    app/src/main/jni/Includes/Source.h
    app/src/main/jni/Includes/Utils.h
    app/src/main/jni/KittyMemory/KittyMemory.cpp
    app/src/main/jni/KittyMemory/KittyMemory.h
    app/src/main/jni/KittyMemory/KittyUtils.cpp
    app/src/main/jni/KittyMemory/KittyUtils.h
    app/src/main/jni/KittyMemory/MemoryBackup.cpp
    app/src/main/jni/KittyMemory/MemoryBackup.h
    app/src/main/jni/KittyMemory/MemoryPatch.cpp
    app/src/main/jni/KittyMemory/MemoryPatch.h
    app/src/main/jni/Localization/Localization.h
    app/src/main/jni/Localization/Localization.ruRU.h
    app/src/main/jni/Localization/Localization.zhCN.h
    app/src/main/jni/Localization/Localizations.h
    app/src/main/jni/Menu/get_device_api_level_inlines.h
    app/src/main/jni/Menu/Menu.h
    app/src/main/jni/Menu/Setup.h
    app/src/main/jni/Substrate/Buffer.hpp
    app/src/main/jni/Substrate/CydiaSubstrate.h
    app/src/main/jni/Substrate/hde64.c
    app/src/main/jni/Substrate/hde64.h
    app/src/main/jni/Substrate/SubstrateARM.hpp
    app/src/main/jni/Substrate/SubstrateDebug.cpp
    app/src/main/jni/Substrate/SubstrateDebug.hpp
    app/src/main/jni/Substrate/SubstrateHook.cpp
    app/src/main/jni/Substrate/SubstrateHook.h
    app/src/main/jni/Substrate/SubstrateLog.hpp
    app/src/main/jni/Substrate/SubstratePosixMemory.cpp
    app/src/main/jni/Substrate/SubstrateX86.hpp
    app/src/main/jni/Substrate/SymbolFinder.cpp
    app/src/main/jni/Substrate/SymbolFinder.h
    app/src/main/jni/Substrate/table64.h
    app/src/main/jni/Main.cpp)

find_package(JNI REQUIRED)
if(JNI_FOUND)
    message(STATUS "JNI_INCLUDE_DIRS = ${JNI_INCLUDE_DIRS}")
    include_directories(${JNI_INCLUDE_DIRS})
endif()
include_directories($ENV{ANDROID_HOME}/ndk/27.0.12077973/toolchains/llvm/prebuilt/windows-x86_64/sysroot/usr/include)