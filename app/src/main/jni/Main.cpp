#include <list>
#include <string.h>
#include <pthread.h>
#include <thread>
#include <cstring>
#include <jni.h>
#include <unistd.h>
#include <fstream>
#include <dlfcn.h>
#include "Includes/Logger.h"
#include "Includes/obfuscate.h"
#include "Includes/Utils.h"
#include "KittyMemory/MemoryPatch.h"
#include "Menu/Setup.h"
#include "Localization/Localizations.h"
#include "Includes/NativeCrashHandler.h"

//Target lib here
#define targetLibName OBFUSCATE("libil2cpp.so")

#include "Includes/Macros.h"
#include "Includes/il2cpp.h"
#include "Includes/Source.h"

JavaVM *jvm;
std::string language = "ruRU";
bool timeScaleEnabled = true, timeScaleInGameOnlyEnabled = true, gameStarted = false, gameLoaded = false;
float originalTimeScale = 1, timeScale = 1;

void Time_set_timeScale(float value) {
    originalTimeScale = value;
    if (!timeScaleEnabled || (timeScaleInGameOnlyEnabled && !gameStarted)) {
        il2cpp::Time_set_timeScale(value);
        return;
    }
    if (timeScale > value) {
        il2cpp::Time_set_timeScale(timeScale * (value + 1.0f) * 0.5f);
        return;
    }
    il2cpp::Time_set_timeScale(value * (timeScale + 1.0f) * 0.5f);
}

void GameMgr_OnGameSetup(GameMgr_o *_this) {
    il2cpp::GameMgr_OnGameSetup(_this);
    gameStarted = true;
    if (timeScaleInGameOnlyEnabled) {
        Time_set_timeScale(originalTimeScale);
    }
}

void GameMgr_OnGameCanceled(GameMgr_o *_this) {
    il2cpp::GameMgr_OnGameCanceled(_this);
    gameStarted = false;
    if (timeScaleInGameOnlyEnabled) {
        Time_set_timeScale(originalTimeScale);
    }
}

void GameMgr_OnGameEnded(GameMgr_o *_this) {
    il2cpp::GameMgr_OnGameEnded(_this);
    gameStarted = false;
    if (timeScaleInGameOnlyEnabled) {
        Time_set_timeScale(originalTimeScale);
    }
}

void HearthstoneApplication_Awake(Hearthstone_HearthstoneApplication_o *_this) {
    il2cpp::HearthstoneApplication_Awake(_this);
    Time_set_timeScale(il2cpp::Time_get_timeScale());
    gameLoaded = true;
}

System_String_o *GetMD5(System_Byte_array *bytes) {
    auto md5 = il2cpp::System_Security_Cryptography_MD5_Create();
    auto hash = il2cpp::System_Security_Cryptography_HashAlgorithm_ComputeHash(
            (System_Security_Cryptography_HashAlgorithm_o *) md5, bytes);

    auto sb = (System_Text_StringBuilder_o *) il2cpp::il2cpp_object_new(
            *il2cpp::System_Text_StringBuilder_TypeInfo);
    il2cpp::System_Text_StringBuilder_ctor(sb);

    auto x2 = u"x2"_SS;
    for (int i = 0; i < hash->max_length; i++) {
        il2cpp::System_Text_StringBuilder_AppendString(sb, il2cpp::System_Byte_ToStringFormat(
                &hash->m_Items[i], x2));
    }
    return ((System_String_o *(*)(System_Text_StringBuilder_o *,
                                  const MethodInfo *)) sb->klass->vtable._3_ToString.methodPtr)(sb,
                                                                                                sb->klass->vtable._3_ToString.method);
}

// we will run our hacks in a new thread so our while loop doesn't block process main thread
void *hack_thread(void *) {
    LOGI(OBFUSCATE("pthread created"));

    //Check if target lib is loaded
    do {
        sleep(1);
    } while (!isLibraryLoaded(targetLibName));

    //Anti-lib rename
    /*
    do {
        sleep(1);
    } while (!isLibraryLoaded("libYOURNAME.so"));*/

    LOGI(OBFUSCATE("%s has been loaded"), (const char *) targetLibName);

    il2cpp::il2cpp_object_new = reinterpret_cast<Il2CppObject *(*)(
            Il2CppClass *klass)>(getSymAddress(targetLibName, OBFUSCATE("il2cpp_object_new")));
    il2cpp::il2cpp_domain_get = reinterpret_cast<void *(*)()>(getSymAddress(targetLibName,
                                                                            OBFUSCATE(
                                                                                    "il2cpp_domain_get")));
    il2cpp::il2cpp_thread_attach = reinterpret_cast<void *(*)(void *domain)>(getSymAddress(
            targetLibName, OBFUSCATE("il2cpp_thread_attach")));
    il2cpp::il2cpp_thread_detach = reinterpret_cast<void (*)(void *thread)>(getSymAddress(
            targetLibName, OBFUSCATE("il2cpp_thread_detach")));
    il2cpp::il2cpp_gchandle_new = reinterpret_cast<uint (*)(void *object, bool weak)>(getSymAddress(
            targetLibName, OBFUSCATE("il2cpp_gchandle_new")));
    il2cpp::il2cpp_gchandle_free = reinterpret_cast<void (*)(uint gchandle)>(getSymAddress(
            targetLibName, OBFUSCATE("il2cpp_gchandle_free")));

    il2cpp::il2cpp_string_new = reinterpret_cast<System_String_o *(*)(
            const char *text)>(getSymAddress(targetLibName, OBFUSCATE("il2cpp_string_new")));
    il2cpp::il2cpp_string_new_utf16 = reinterpret_cast<System_String_o *(*)(const Il2CppChar *text,
                                                                            int len)>(getSymAddress(
            targetLibName, OBFUSCATE("il2cpp_string_new_utf16")));

    il2cpp::il2cpp_array_new = reinterpret_cast<void *(*)(Il2CppClass *klass,
                                                          size_t length)>(getSymAddress(
            targetLibName, OBFUSCATE("il2cpp_array_new")));
    il2cpp::il2cpp_value_box = reinterpret_cast<Il2CppObject *(*)(Il2CppClass *klass,
                                                                  void *data)>(getSymAddress(
            targetLibName, OBFUSCATE("il2cpp_value_box")));

    il2cpp::System_String_Format = reinterpret_cast<System_String_o *(*)(System_String_o *format,
                                                                         System_Object_array *args)>(getAbsoluteAddressStr(
            targetLibName, System_String_Format_Offset));

    il2cpp::object_TypeInfo = reinterpret_cast<struct Il2CppClass **>(getAbsoluteAddressStr(
            targetLibName, object_TypeInfo_Offset));

    il2cpp::System_Text_StringBuilder_ctor = reinterpret_cast<void (*)(
            System_Text_StringBuilder_o *_this)>(getAbsoluteAddressStr(targetLibName,
                                                                       System_Text_StringBuilder_ctor_Offset));

    il2cpp::Time_get_timeScale = reinterpret_cast<float (*)()>(getAbsoluteAddressStr(targetLibName,
                                                                                     Time_get_timeScale_Offset));

    il2cpp::Crypto_SHA1_Calc = reinterpret_cast<System_String_o *(*)(
            System_String_o *message)>(getAbsoluteAddressStr(targetLibName,
                                                             Crypto_SHA1_Calc_Offset));

    il2cpp::System_Security_Cryptography_MD5_Create = reinterpret_cast<System_Security_Cryptography_MD5_o *(*)()>(getAbsoluteAddressStr(
            targetLibName, System_Security_Cryptography_MD5_Create_Offset));
    il2cpp::System_Security_Cryptography_HashAlgorithm_ComputeHash = reinterpret_cast<System_Byte_array *(*)(
            System_Security_Cryptography_HashAlgorithm_o *_this,
            System_Byte_array *buffer)>(getAbsoluteAddressStr(targetLibName,
                                                              System_Security_Cryptography_HashAlgorithm_ComputeHash_Offset));

    il2cpp::System_Text_StringBuilder_TypeInfo = reinterpret_cast<struct Il2CppClass **>(getAbsoluteAddressStr(
            targetLibName, System_Text_StringBuilder_TypeInfo_Offset));
    il2cpp::System_Text_StringBuilder_AppendString = reinterpret_cast<System_Text_StringBuilder_o *(*)(
            System_Text_StringBuilder_o *_this, System_String_o *value)>(getAbsoluteAddressStr(
            targetLibName, System_Text_StringBuilder_AppendString_Offset));

    il2cpp::System_Byte_ToStringFormat = reinterpret_cast<System_String_o *(*)(uint8_t *_this,
                                                                               System_String_o *format)>(getAbsoluteAddressStr(
            targetLibName, System_Byte_ToStringFormat_Offset));

    il2cpp::System_Guid_ctor = reinterpret_cast<void (*)(System_Guid_o _this,
                                                         System_String_o *g)>(getAbsoluteAddressStr(
            targetLibName, System_Guid_ctor_Offset));

    il2cpp::System_Text_Encoding_get_Default = reinterpret_cast<System_Text_Encoding_o *(*)()>(getAbsoluteAddressStr(
            targetLibName, System_Text_Encoding_get_Default_Offset));
    il2cpp::System_Text_Encoding_GetBytes = reinterpret_cast<System_Byte_array *(*)(
            System_Text_Encoding_o *_this, System_String_o *str)>(getAbsoluteAddressStr(
            targetLibName, System_Text_Encoding_GetBytes_Offset));

    il2cpp::System_String_ToUpper = reinterpret_cast<System_String_o *(*)(
            System_String_o *_this)>(getAbsoluteAddressStr(targetLibName,
                                                           System_String_ToUpper_Offset));

    il2cpp::UnityEngine_Time_get_time = reinterpret_cast<float (*)()>(getAbsoluteAddressStr(
            targetLibName, UnityEngine_Time_get_time_Offset));

    il2cpp::GameMgr_Get = reinterpret_cast<GameMgr_o *(*)()>(getAbsoluteAddressStr(targetLibName,
                                                                                   GameMgr_Get_Offset));
    il2cpp::GameMgr_IsBattlegrounds = reinterpret_cast<bool (*)(
            GameMgr_o *_this)>(getAbsoluteAddressStr(targetLibName,
                                                     GameMgr_IsBattlegrounds_Offset));

    il2cpp::GameState_Get = reinterpret_cast<GameState_o *(*)()>(getAbsoluteAddressStr(
            targetLibName, GameState_Get_Offset));
    il2cpp::GameState_IsGameCreatedOrCreating = reinterpret_cast<bool (*)(
            GameState_o *_this)>(getAbsoluteAddressStr(targetLibName,
                                                       GameState_IsGameCreatedOrCreating_Offset));

    il2cpp::System_String_IsNullOrEmpty = reinterpret_cast<bool (*)(
            System_String_o *value)>(getAbsoluteAddressStr(targetLibName,
                                                           System_String_IsNullOrEmpty_Offset));


    il2cpp::BnetPresenceMgr_Get = reinterpret_cast<BnetPresenceMgr_o *(*)()>(getAbsoluteAddressStr(
            targetLibName, BnetPresenceMgr_Get_Offset));

    il2cpp::ClipboardUtils_CopyToClipboard = reinterpret_cast<void (*)(
            System_String_o *copyText)>(getAbsoluteAddressStr(targetLibName,
                                                              ClipboardUtils_CopyToClipboard_Offset));

    il2cpp::UIStatus_Get = reinterpret_cast<UIStatus_o *(*)()>(getAbsoluteAddressStr(targetLibName,
                                                                                     UIStatus_Get_Offset));
    il2cpp::UIStatus_AddInfo = reinterpret_cast<void (*)(UIStatus_o *_this,
                                                         System_String_o *message)>(getAbsoluteAddressStr(
            targetLibName, UIStatus_AddInfo_Offset));

    il2cpp::EntityBase_GetTag = reinterpret_cast<int (*)(EntityBase_o *_this,
                                                         int tag)>(getAbsoluteAddressStr(
            targetLibName, EntityBase_GetTag_Offset));

    il2cpp::System_Xml_XmlConvert_ToGuid = reinterpret_cast<System_Guid_o (*)(
            System_String_o *s)>(getAbsoluteAddressStr(targetLibName,
                                                       System_Xml_XmlConvert_ToGuid_Offset));

    il2cpp::System_Guid_ToString = reinterpret_cast<System_String_o *(*)(
            System_Guid_o *_this)>(getAbsoluteAddressStr(targetLibName,
                                                         System_Guid_ToString_Offset));

    HOOK(HearthstoneApplication_Awake_Offset, HearthstoneApplication_Awake,
         il2cpp::HearthstoneApplication_Awake);

    HOOK(Time_set_timeScale_Offset, Time_set_timeScale, il2cpp::Time_set_timeScale);

    HOOK(GameMgr_OnGameSetup_Offset, GameMgr_OnGameSetup, il2cpp::GameMgr_OnGameSetup);
    HOOK(GameMgr_OnGameCanceled_Offset, GameMgr_OnGameCanceled, il2cpp::GameMgr_OnGameCanceled);
    HOOK(GameMgr_OnGameEnded_Offset, GameMgr_OnGameEnded, il2cpp::GameMgr_OnGameEnded);

    return NULL;
}

jobjectArray SettingsList(JNIEnv *env, jobject activityObject) {
    jobjectArray ret;

    const char *features[] = {
            "Category_Settings",
            localization[language][SAVE_SETTINGS], //-1 is checked on Preferences.java
            localization[language][AUTO_SIZE],
            "Category_Menu",
            localization[language][CLOSE_SETTINGS],
    };

    int Total_Feature = (sizeof features /
                         sizeof features[0]); //Now you dont have to manually update the number everytime;
    ret = (jobjectArray)
            env->NewObjectArray(Total_Feature, env->FindClass(OBFUSCATE("java/lang/String")),
                                env->NewStringUTF(""));
    int i;
    for (i = 0; i < Total_Feature; i++)
        env->SetObjectArrayElement(ret, i, env->NewStringUTF(features[i]));

    settingsValid = true;

    return (ret);
}

// Do not change or translate the first text unless you know what you are doing
// Assigning feature numbers is optional. Without it, it will automatically count for you, starting from 0
// Assigned feature numbers can be like any numbers 1,3,200,10... instead in order 0,1,2,3,4,5...
// ButtonLink, Category, RichTextView and RichWebView is not counted. They can't have feature number assigned
// Toggle, ButtonOnOff and Checkbox can be switched on by default, if you add True_. Example: CheckBox_True_The Check Box
// To learn HTML, go to this page: https://www.w3schools.com/

jobjectArray GetFeatureList(JNIEnv *env, jobject context) {
    jobjectArray ret;

    const char *features[] = {
            "Category_Global",
            localization[language][TIMESCALE_ENABLED],
            localization[language][TIMESCALE_IN_GAME_ONLY],
            localization[language][TIMESCALE]
    };

    //Now you dont have to manually update the number everytime;
    int Total_Feature = (sizeof features / sizeof features[0]);
    ret = (jobjectArray)
            env->NewObjectArray(Total_Feature, env->FindClass(OBFUSCATE("java/lang/String")),
                                env->NewStringUTF(""));

    for (int i = 0; i < Total_Feature; i++)
        env->SetObjectArrayElement(ret, i, env->NewStringUTF(features[i]));

    return (ret);
}

void Changes(JNIEnv *env, jclass clazz, jobject obj,
             jint featNum, jstring featName, jint value,
             jboolean boolean, jstring str) {

    auto cstr = str != NULL ? env->GetStringUTFChars(str, 0) : "";

    LOGD(OBFUSCATE("Feature name: %d - %s | Value: = %d | Bool: = %d | Text: = %s"), featNum,
         env->GetStringUTFChars(featName, 0), value,
         boolean, cstr);

    //BE CAREFUL NOT TO ACCIDENTLY REMOVE break;

    switch (featNum) {
        case TIMESCALE_ENABLED:
            timeScaleEnabled = boolean;
            if (gameLoaded) {
                Time_set_timeScale(originalTimeScale);
            }
            break;
        case TIMESCALE:
            timeScale = value / 100.0f;
            if (gameLoaded) {
                Time_set_timeScale(originalTimeScale);
            }
            break;
        case TIMESCALE_IN_GAME_ONLY:
            timeScaleInGameOnlyEnabled = boolean;
            if (gameLoaded) {
                Time_set_timeScale(originalTimeScale);
            }
            break;
    }
}

__attribute__((constructor))
void lib_main() {
    // Create a new thread so it does not block the main thread, means the game would not freeze
    pthread_t ptid;
    pthread_create(&ptid, NULL, hack_thread, NULL);
}

int RegisterMenu(JNIEnv *env) {
    JNINativeMethod methods[] = {
            {OBFUSCATE("Icon"),            OBFUSCATE(
                                                   "()Ljava/lang/String;"),                                                           reinterpret_cast<void *>(Icon)},
            {OBFUSCATE("IconWebViewData"), OBFUSCATE(
                                                   "()Ljava/lang/String;"),                                                           reinterpret_cast<void *>(IconWebViewData)},
            {OBFUSCATE("IsGameLibLoaded"), OBFUSCATE(
                                                   "()Z"),                                                                            reinterpret_cast<void *>(isGameLibLoaded)},
            {OBFUSCATE("Init"),            OBFUSCATE(
                                                   "(Landroid/content/Context;Landroid/widget/TextView;Landroid/widget/TextView;)V"), reinterpret_cast<void *>(Init)},
            {OBFUSCATE("SettingsList"),    OBFUSCATE(
                                                   "()[Ljava/lang/String;"),                                                          reinterpret_cast<void *>(SettingsList)},
            {OBFUSCATE("GetFeatureList"),  OBFUSCATE(
                                                   "()[Ljava/lang/String;"),                                                          reinterpret_cast<void *>(GetFeatureList)},
    };

    jclass clazz = env->FindClass(OBFUSCATE("com/android/support/Menu"));
    if (!clazz)
        return JNI_ERR;
    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) != 0)
        return JNI_ERR;
    return JNI_OK;
}

int RegisterPreferences(JNIEnv *env) {
    JNINativeMethod methods[] = {
            {OBFUSCATE("Changes"),
             OBFUSCATE("(Landroid/content/Context;ILjava/lang/String;IZLjava/lang/String;)V"),
             reinterpret_cast<void *>(Changes)},
    };
    jclass clazz = env->FindClass(OBFUSCATE("com/android/support/Preferences"));
    if (!clazz)
        return JNI_ERR;
    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) != 0)
        return JNI_ERR;
    return JNI_OK;
}

int RegisterMain(JNIEnv *env) {
    JNINativeMethod methods[] = {
            {OBFUSCATE("CheckOverlayPermission"), OBFUSCATE("(Landroid/content/Context;)V"),
             reinterpret_cast<void *>(CheckOverlayPermission)},
    };
    jclass clazz = env->FindClass(OBFUSCATE("com/android/support/Main"));
    if (!clazz)
        return JNI_ERR;
    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) != 0)
        return JNI_ERR;

    return JNI_OK;
}

extern "C"
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved) {
    jvm = vm;
    JNIEnv *env;
    vm->GetEnv((void **) &env, JNI_VERSION_1_6);
    if (RegisterMenu(env) != 0)
        return JNI_ERR;
    if (RegisterPreferences(env) != 0)
        return JNI_ERR;
    if (RegisterMain(env) != 0)
        return JNI_ERR;
    return JNI_VERSION_1_6;
}
