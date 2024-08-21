#include <list>
#include <vector>
#include <string.h>
#include <pthread.h>
#include <thread>
#include <cstring>
#include <jni.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <dlfcn.h>
#include "Includes/Logger.h"
#include "Includes/obfuscate.h"
#include "Includes/Utils.h"
#include "KittyMemory/MemoryPatch.h"
#include "Menu/Setup.h"
#include "Localization/Localizations.h"

//Target lib here
#define targetLibName OBFUSCATE("libil2cpp.so")

#include "Includes/Macros.h"
#include "Includes/il2cpp.h"
#include "Includes/Source.h"

JavaVM *jvm;
std::string language = "enUS";
bool languageLoaded = false;
bool useDefaultLanguage = true;
bool timeScaleEnabled = false, timeScaleInGameOnlyEnabled = false, gameStarted = false, emoteSpamBlocker = false, disableThinkEmotes = false, copySelectedBattleTag = false, gameLoaded = false;
float originalTimeScale = 1, timeScale = 1, m_lastEnemyEmoteTime;
int emotesBeforeBlock = 0, m_lastPlayerId, m_chainedEnemyEmotes;
CardState golden, diamond, signature;
BnetPlayer_o *gameStateCurrentOpponent, *playerLeaderboardManagerCurrentOpponent;
uint gameStateCurrentOpponent_gchandle = -1, playerLeaderboardManagerCurrentOpponent_gchandle = -1;
DevicePreset devicePreset = DevicePreset::Default;
OSCategory _os;
ScreenCategory _screen;
System_String_o *_deviceName;

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

void ThinkEmoteManager_Update(ThinkEmoteManager_o *_this) {
    if (!disableThinkEmotes) {
        il2cpp::ThinkEmoteManager_Update(_this);
    }
}

void SocialToastMgr_AddToast(SocialToastMgr_o *_this, int blocker, System_String_o *textArg,
                             int toastType, float displayTime, bool playSound) {
    displayTime *= timeScale;
    il2cpp::SocialToastMgr_AddToast(_this, blocker, textArg, toastType, displayTime, playSound);
}

bool RemoteActionHandler_CanReceiveEnemyEmote(RemoteActionHandler_o *_this, int emoteType,
                                              int playerId) {
    auto result = il2cpp::RemoteActionHandler_CanReceiveEnemyEmote(_this, emoteType, playerId);
    if (result && emoteSpamBlocker && emotesBeforeBlock > 0) {
        float currentTime = il2cpp::UnityEngine_Time_get_time();
        if (m_lastPlayerId == playerId) {
            if (m_lastEnemyEmoteTime != 0.0f && currentTime - m_lastEnemyEmoteTime < 9.0f) {
                m_chainedEnemyEmotes++;
                if (m_chainedEnemyEmotes > emotesBeforeBlock) {
                    auto m_squelched = il2cpp::EnemyEmoteHandler_Get()->fields.m_squelched;
                    il2cpp::Blizzard_T5_Core_Map_int_bool_set_Item(m_squelched, playerId, true);
                }
            } else {
                m_chainedEnemyEmotes = 1;
            }
        } else {
            m_chainedEnemyEmotes = 1;
            m_lastPlayerId = playerId;
        }
        m_lastEnemyEmoteTime = currentTime;
    }
    return result;
}

TAG_PREMIUM Entity_GetPremiumType(Entity_o *_this) {
    auto gameMgr = il2cpp::GameMgr_Get();
    auto gameState = il2cpp::GameState_Get();
    if (gameMgr != NULL && !il2cpp::GameMgr_IsBattlegrounds(gameMgr) && gameState != NULL &&
        il2cpp::GameState_IsGameCreatedOrCreating(gameState)) {
        if (il2cpp::EntityBase_HasTag(reinterpret_cast<EntityBase_o *>(_this), 1932)) {
            if (diamond == CardState::All || diamond == CardState::OnlyMy &&
                                             il2cpp::Entity_GetController(_this) != NULL &&
                                             il2cpp::Entity_IsControlledByFriendlySidePlayer(_this)) {
                return TAG_PREMIUM::DIAMOND;
            }
            if (diamond == CardState::Disabled) {
                return TAG_PREMIUM::NORMAL;
            }
        }
        if (il2cpp::EntityBase_HasTag(reinterpret_cast<EntityBase_o *>(_this), 2589)) {
            if (signature == CardState::All || signature == CardState::OnlyMy &&
                                               il2cpp::Entity_GetController(_this) != NULL &&
                                               il2cpp::Entity_IsControlledByFriendlySidePlayer(_this)) {
                return TAG_PREMIUM::SIGNATURE;
            }
            if (signature == CardState::Disabled) {
                return TAG_PREMIUM::NORMAL;
            }
        }
        if (golden == CardState::All || golden == CardState::OnlyMy &&
                                        il2cpp::Entity_GetController(_this) != NULL &&
                                        il2cpp::Entity_IsControlledByFriendlySidePlayer(_this)) {
            return TAG_PREMIUM::GOLDEN;
        }
        if (golden == CardState::Disabled) {
            return TAG_PREMIUM::NORMAL;
        }
    }
    return il2cpp::Entity_GetPremiumType(_this);
}

void Entity_LoadCard(Entity_o *_this, System_String_o *cardId, Entity_LoadCardData_o *data) {
    _this->fields.m_realTimePremium = static_cast<int>(Entity_GetPremiumType(_this));
    il2cpp::Entity_LoadCard(_this, cardId, data);
}

void UpdateCurrentOpponent() {
    if (gameStateCurrentOpponent_gchandle != -1) {
        il2cpp::il2cpp_gchandle_free(gameStateCurrentOpponent_gchandle);
        gameStateCurrentOpponent_gchandle = -1;
    }
    auto gameState = il2cpp::GameState_Get();
    if (gameState == NULL) {
        gameStateCurrentOpponent = NULL;
        return;
    }
    auto opposingSidePlayer = il2cpp::GameState_GetOpposingSidePlayer(gameState);
    if (opposingSidePlayer == NULL) {
        gameStateCurrentOpponent = NULL;
        return;
    }
    gameStateCurrentOpponent = il2cpp::BnetPresenceMgr_GetPlayer(il2cpp::BnetPresenceMgr_Get(),
                                                                 opposingSidePlayer->fields.m_gameAccountId);
    if (gameStateCurrentOpponent) {
        gameStateCurrentOpponent_gchandle = il2cpp::il2cpp_gchandle_new(gameStateCurrentOpponent,
                                                                        false);
    }
}

void UpdateCurrentOpponent(int opponentPlayerId) {
    if (playerLeaderboardManagerCurrentOpponent_gchandle != -1) {
        il2cpp::il2cpp_gchandle_free(playerLeaderboardManagerCurrentOpponent_gchandle);
        playerLeaderboardManagerCurrentOpponent_gchandle = -1;
    }
    auto gameState = il2cpp::GameState_Get();
    auto playerInfoMap = reinterpret_cast<Blizzard_T5_Core_Map_TKey__TValue__o *>(il2cpp::GameState_GetPlayerInfoMap(gameState));
    if (gameState == NULL ||
        !il2cpp::Blizzard_T5_Core_Map_int_object_ContainsKey(playerInfoMap,
                                                                       opponentPlayerId,
                                                                       *il2cpp::Method_Blizzard_T5_Core_Map_int_SharedPlayerInfo_ContainsKey)) {
        playerLeaderboardManagerCurrentOpponent = NULL;
        return;
    }
    auto id = reinterpret_cast<SharedPlayerInfo_o *>(il2cpp::Blizzard_T5_Core_Map_int_object_get_Item(playerInfoMap,
                                                                         opponentPlayerId,
                                                                         *il2cpp::Method_Blizzard_T5_Core_Map_int_SharedPlayerInfo_get_Item))->fields.m_gameAccountId;
    if (id == NULL) {
        playerLeaderboardManagerCurrentOpponent = NULL;
        return;
    }
    playerLeaderboardManagerCurrentOpponent = il2cpp::BnetPresenceMgr_GetPlayer(
            il2cpp::BnetPresenceMgr_Get(), id);
    if (playerLeaderboardManagerCurrentOpponent) {
        playerLeaderboardManagerCurrentOpponent_gchandle = il2cpp::il2cpp_gchandle_new(
                playerLeaderboardManagerCurrentOpponent, false);
    }
}

BnetPlayer_o *GetSelectedOpponent(PlayerLeaderboardCard_o *_this) {
    auto opponentPlayerId = il2cpp::EntityBase_GetTag(
            reinterpret_cast<EntityBase_o *>(_this->fields.m_playerHeroEntity), 30);
    auto gameState = il2cpp::GameState_Get();
    auto playerInfoMap = reinterpret_cast<Blizzard_T5_Core_Map_TKey__TValue__o *>(il2cpp::GameState_GetPlayerInfoMap(gameState));
    if (gameState == NULL ||
        !il2cpp::Blizzard_T5_Core_Map_int_object_ContainsKey(playerInfoMap,
                                                                       opponentPlayerId,
                                                                       *il2cpp::Method_Blizzard_T5_Core_Map_int_SharedPlayerInfo_ContainsKey)) {
        return NULL;
    }
    auto id = reinterpret_cast<SharedPlayerInfo_o *>(il2cpp::Blizzard_T5_Core_Map_int_object_get_Item(playerInfoMap,
                                                                         opponentPlayerId,
                                                                         *il2cpp::Method_Blizzard_T5_Core_Map_int_SharedPlayerInfo_get_Item))->fields.m_gameAccountId;
    if (id == NULL) {
        return NULL;
    }
    return il2cpp::BnetPresenceMgr_GetPlayer(il2cpp::BnetPresenceMgr_Get(), id);
}

void Gameplay_OnCreateGame(Gameplay_o *_this, int phase, Il2CppObject *userData) {
    il2cpp::Gameplay_OnCreateGame(_this, phase, userData);
    UpdateCurrentOpponent();
}

void
PlayerLeaderboardManager_SetNextOpponent(PlayerLeaderboardManager_o *_this, int opponentPlayerId) {
    if (opponentPlayerId != 0) {
        UpdateCurrentOpponent(opponentPlayerId);
        il2cpp::PlayerLeaderboardManager_SetNextOpponent(_this, opponentPlayerId);
    }
}

void PlayerLeaderboardManager_SetCurrentOpponent(PlayerLeaderboardManager_o *_this,
                                                 int opponentPlayerId) {
    if (opponentPlayerId != -1) {
        UpdateCurrentOpponent(opponentPlayerId);
    }
    il2cpp::PlayerLeaderboardManager_SetCurrentOpponent(_this, opponentPlayerId);
}

void PlayerLeaderboardCard_NotifyMousedOver(PlayerLeaderboardCard_o *_this) {
    if (_this->fields.m_mousedOver ||
        _this == il2cpp::HistoryManager_GetCurrentBigCard(il2cpp::HistoryManager_Get())) {
        return;
    }
    il2cpp::PlayerLeaderboardCard_NotifyMousedOver(_this);
    if (copySelectedBattleTag) {
        auto currentOpponent = GetSelectedOpponent(_this);
        if (currentOpponent != NULL) {
            BnetBattleTag_o *battleTag = il2cpp::BnetPlayer_GetBattleTag(currentOpponent);
            if (battleTag != NULL) {
                System_String_o *str = il2cpp::BnetBattleTag_GetString(battleTag);
                il2cpp::ClipboardUtils_CopyToClipboard(str);
                il2cpp::UIStatus_AddInfo(il2cpp::UIStatus_Get(), str);
            }
        }
    }
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

System_String_o *GetUniqueDeviceID(OSCategory os, ScreenCategory screen,
                                   System_String_o *deviceName/*, string operatingSystem*/) {
    /*
     * public class XmlConvert
     * {
     *     //
     *     public static Guid ToGuid(string s)
     *	   {
     *         return new Guid(s);
     *	   }
     *	   //
     * }
     *
     * Can also be:
     * System_Guid_o guid;
     * guid.fields._d = str; ?
     * il2cpp::System_Guid_ctor(guid, NULL);
    */
    auto deviceId = il2cpp::UnityEngine_SystemInfo_get_deviceUniqueIdentifier();
    auto arr = (System_Object_array *) il2cpp::il2cpp_array_new(*il2cpp::object_TypeInfo, 4);
    arr->m_Items[0] = (Il2CppObject *) deviceId;
    arr->m_Items[1] = (Il2CppObject *) il2cpp::il2cpp_value_box(*il2cpp::OSCategory_TypeInfo, &os);
    arr->m_Items[2] = (Il2CppObject *) il2cpp::il2cpp_value_box(*il2cpp::ScreenCategory_TypeInfo,
                                                                &screen);
    arr->m_Items[3] = (Il2CppObject *) deviceName;
    auto str = il2cpp::System_String_Format(u"MixModeD{0}{1}{2}{3}"_SS, arr);
    switch (os) {
        case OSCategory::PC:
            return il2cpp::Crypto_SHA1_Calc(str);
        case OSCategory::Mac:
        case OSCategory::iOS: {
            auto guid = il2cpp::System_Xml_XmlConvert_ToGuid(
                    GetMD5(il2cpp::System_Text_Encoding_GetBytes(
                            il2cpp::System_Text_Encoding_get_Default(), str)));
            return il2cpp::System_String_ToUpper(il2cpp::System_Guid_ToString(&guid));
        }
        case OSCategory::Android:
            return GetMD5(il2cpp::System_Text_Encoding_GetBytes(
                    il2cpp::System_Text_Encoding_get_Default(), str));
        default:
            return u"n/a"_SS;
    }
}

PegasusShared_Platform_o *Network_GetPlatformBuilder(Network_o *_this) {
    auto result = il2cpp::Network_GetPlatformBuilder(_this);
    OSCategory os;
    ScreenCategory screen;
    System_String_o *deviceName;
    switch (devicePreset) {
        case DevicePreset::iPad:
            os = OSCategory::iOS;
            screen = ScreenCategory::Tablet;
            deviceName = u"iPad13,11"_SS;
            break;
        case DevicePreset::iPhone:
            os = OSCategory::iOS;
            screen = ScreenCategory::Phone;
            deviceName = u"iPhone13,4"_SS;
            break;
        case DevicePreset::Phone:
            os = OSCategory::Android;
            screen = ScreenCategory::Phone;
            deviceName = u"SAMSUNG-SM-G930FD"_SS;
            break;
        case DevicePreset::Tablet:
            os = OSCategory::Android;
            screen = ScreenCategory::Tablet;
            deviceName = u"SAMSUNG-SM-G920F"_SS;
            break;
        case DevicePreset::HuaweiPhone:
            os = OSCategory::Android;
            screen = ScreenCategory::Phone;
            deviceName = u"Huawei Nova 8"_SS;
            break;
        case DevicePreset::PC:
            os = OSCategory::PC;
            screen = ScreenCategory::PC;
            deviceName = u"System Product Name (System manufacturer)"_SS;
            break;
        case DevicePreset::Mac:
            os = OSCategory::Mac;
            screen = ScreenCategory::PC;
            deviceName = u"MacBookPro11,3"_SS;
            break;
        case DevicePreset::Custom:
            os = _os;
            screen = _screen;
            deviceName = _deviceName;
            break;
        case DevicePreset::Default:
        default:
            return result;
    }

    result->fields._Os_k__BackingField = static_cast<int>(os);
    result->fields._Screen_k__BackingField = static_cast<int>(screen);
    result->fields._Name_k__BackingField = deviceName;
    result->fields._UniqueDeviceIdentifier = GetUniqueDeviceID(os, screen, deviceName);

    return result;
}

void Localization_SetPegLocaleName(Localization_o* _this, System_String_o* localeName) {
    il2cpp::Localization_SetPegLocaleName(_this, localeName);
    if (useDefaultLanguage) {
        auto newLanguage = SS_to_str(localeName);
        if (localization.find(newLanguage) == localization.end()) {
            language = "enUS";
        } else {
            language = newLanguage;
        }

        if (jvm == NULL)
        {
            return;
        }
        JNIEnv *env;
        jvm->GetEnv((void **) &env, JNI_VERSION_1_6);
        jclass clazz = env->FindClass("com/android/support/Main");
        jmethodID reloadFeaturesMethod = env->GetStaticMethodID(clazz, "ReloadFeatures", "()V") ;
        env->CallStaticVoidMethod(clazz, reloadFeaturesMethod);
    }
    languageLoaded = true;
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
    il2cpp::OSCategory_TypeInfo = reinterpret_cast<struct Il2CppClass **>(getAbsoluteAddressStr(
            targetLibName, OSCategory_TypeInfo_Offset));
    il2cpp::ScreenCategory_TypeInfo = reinterpret_cast<struct Il2CppClass **>(getAbsoluteAddressStr(
            targetLibName, ScreenCategory_TypeInfo_Offset));

    il2cpp::UnityEngine_SystemInfo_get_deviceUniqueIdentifier = reinterpret_cast<System_String_o *(*)()>(getAbsoluteAddressStr(
            targetLibName, UnityEngine_SystemInfo_get_deviceUniqueIdentifier_Offset));

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

    il2cpp::Method_Blizzard_T5_Core_Map_int_SharedPlayerInfo_ContainsKey = reinterpret_cast<struct MethodInfo **>(getAbsoluteAddressStr(
            targetLibName, Method_Blizzard_T5_Core_Map_int_SharedPlayerInfo_ContainsKey_Offset));
    il2cpp::Method_Blizzard_T5_Core_Map_int_SharedPlayerInfo_get_Item = reinterpret_cast<struct MethodInfo **>(getAbsoluteAddressStr(
            targetLibName, Method_Blizzard_T5_Core_Map_int_SharedPlayerInfo_get_Item_Offset));

    il2cpp::RemoteActionHandler_CanReceiveEnemyEmote = reinterpret_cast<bool (*)(
            RemoteActionHandler_o *_this, int emoteType, int playerId)>(getAbsoluteAddressStr(
            targetLibName, RemoteActionHandler_CanReceiveEnemyEmote_Offset));

    il2cpp::EnemyEmoteHandler_Get = reinterpret_cast<EnemyEmoteHandler_o *(*)()>(getAbsoluteAddressStr(
            targetLibName, EnemyEmoteHandler_Get_Offset));

    il2cpp::Blizzard_T5_Core_Map_int_bool_set_Item = reinterpret_cast<void (*)(
            Blizzard_T5_Core_Map_int__bool__o *__this, int key, bool value)>(getAbsoluteAddressStr(
            targetLibName, Blizzard_T5_Core_Map_int_bool_set_Item_Offset));

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
    il2cpp::GameState_GetOpposingSidePlayer = reinterpret_cast<Player_o *(*)(
            GameState_o *_this)>(getAbsoluteAddressStr(targetLibName,
                                                       GameState_GetOpposingSidePlayer_Offset));
    il2cpp::GameState_GetPlayerInfoMap = reinterpret_cast<Blizzard_T5_Core_Map_int__SharedPlayerInfo__o *(*)(
            GameState_o *_this)>(getAbsoluteAddressStr(targetLibName,
                                                       GameState_GetPlayerInfoMap_Offset));

    il2cpp::Blizzard_T5_Core_Map_int_object_ContainsKey = reinterpret_cast<bool (*)(
            Blizzard_T5_Core_Map_TKey__TValue__o *_this, int key,
            const MethodInfo *method)>(getAbsoluteAddressStr(targetLibName,
                                                             Blizzard_T5_Core_Map_int_object_ContainsKey_Offset));
    il2cpp::Blizzard_T5_Core_Map_int_object_get_Item = reinterpret_cast<Il2CppObject *(*)(
            Blizzard_T5_Core_Map_TKey__TValue__o *_this, int key,
            const MethodInfo *method)>(getAbsoluteAddressStr(targetLibName,
                                                             Blizzard_T5_Core_Map_int_object_get_Item_Offset));

    il2cpp::EntityBase_HasTag = reinterpret_cast<bool (*)(EntityBase_o *_this,
                                                          int tag)>(getAbsoluteAddressStr(
            targetLibName, EntityBase_HasTag_Offset));
    il2cpp::Entity_IsControlledByFriendlySidePlayer = reinterpret_cast<bool (*)(
            Entity_o *_this)>(getAbsoluteAddressStr(targetLibName,
                                                    Entity_IsControlledByFriendlySidePlayer_Offset));
    il2cpp::Entity_GetController = reinterpret_cast<Player_o *(*)(Entity_o *_this)>(
                                                    getAbsoluteAddressStr(targetLibName,
                                                    Entity_GetController_Offset));

    il2cpp::System_String_IsNullOrEmpty = reinterpret_cast<bool (*)(
            System_String_o *value)>(getAbsoluteAddressStr(targetLibName,
                                                           System_String_IsNullOrEmpty_Offset));

    il2cpp::Network_Get = reinterpret_cast<Network_o *(*)()>(getAbsoluteAddressStr(targetLibName,
                                                                                   Network_Get_Offset));
    il2cpp::Network_SimulateUncleanDisconnectFromGameServer = reinterpret_cast<void (*)(
            Network_o *_this)>(getAbsoluteAddressStr(targetLibName,
                                                     Network_SimulateUncleanDisconnectFromGameServer_Offset));

    il2cpp::BnetPresenceMgr_Get = reinterpret_cast<BnetPresenceMgr_o *(*)()>(getAbsoluteAddressStr(
            targetLibName, BnetPresenceMgr_Get_Offset));
    il2cpp::BnetPresenceMgr_GetPlayer = reinterpret_cast<BnetPlayer_o *(*)(BnetPresenceMgr_o *_this,
                                                                           Blizzard_GameService_SDK_Client_Integration_BnetGameAccountId_o *id)>(getAbsoluteAddressStr(
            targetLibName, BnetPresenceMgr_GetPlayer_Offset));
    il2cpp::BnetPlayer_GetBattleTag = reinterpret_cast<BnetBattleTag_o *(*)(
            BnetPlayer_o *_this)>(getAbsoluteAddressStr(targetLibName,
                                                        BnetPlayer_GetBattleTag_Offset));
    il2cpp::BnetBattleTag_GetString = reinterpret_cast<System_String_o *(*)(
            BnetBattleTag_o *_this)>(getAbsoluteAddressStr(targetLibName,
                                                           BnetBattleTag_GetString_Offset));

    il2cpp::ClipboardUtils_CopyToClipboard = reinterpret_cast<void (*)(
            System_String_o *copyText)>(getAbsoluteAddressStr(targetLibName,
                                                              ClipboardUtils_CopyToClipboard_Offset));

    il2cpp::UIStatus_Get = reinterpret_cast<UIStatus_o *(*)()>(getAbsoluteAddressStr(targetLibName,
                                                                                     UIStatus_Get_Offset));
    il2cpp::UIStatus_AddInfo = reinterpret_cast<void (*)(UIStatus_o *_this,
                                                         System_String_o *message)>(getAbsoluteAddressStr(
            targetLibName, UIStatus_AddInfo_Offset));

    il2cpp::HistoryManager_Get = reinterpret_cast<HistoryManager_o *(*)()>(getAbsoluteAddressStr(
            targetLibName, HistoryManager_Get_Offset));
    il2cpp::HistoryManager_GetCurrentBigCard = reinterpret_cast<PlayerLeaderboardCard_o *(*)(
            HistoryManager_o *_this)>(getAbsoluteAddressStr(targetLibName,
                                                            HistoryManager_GetCurrentBigCard_Offset));

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

    HOOK(SocialToastMgr_AddToast_Offset, SocialToastMgr_AddToast, il2cpp::SocialToastMgr_AddToast);

    HOOK(RemoteActionHandler_CanReceiveEnemyEmote_Offset, RemoteActionHandler_CanReceiveEnemyEmote,
         il2cpp::RemoteActionHandler_CanReceiveEnemyEmote);

    HOOK(Entity_LoadCard_Offset, Entity_LoadCard, il2cpp::Entity_LoadCard);
    HOOK(Entity_GetPremiumType_Offset, Entity_GetPremiumType, il2cpp::Entity_GetPremiumType);

    HOOK(Gameplay_OnCreateGame_Offset, Gameplay_OnCreateGame, il2cpp::Gameplay_OnCreateGame);

    HOOK(PlayerLeaderboardManager_SetNextOpponent_Offset, PlayerLeaderboardManager_SetNextOpponent,
         il2cpp::PlayerLeaderboardManager_SetNextOpponent);
    HOOK(PlayerLeaderboardManager_SetCurrentOpponent_Offset,
         PlayerLeaderboardManager_SetCurrentOpponent,
         il2cpp::PlayerLeaderboardManager_SetCurrentOpponent);

    HOOK(PlayerLeaderboardCard_NotifyMousedOver_Offset, PlayerLeaderboardCard_NotifyMousedOver,
         il2cpp::PlayerLeaderboardCard_NotifyMousedOver);

    HOOK(Network_GetPlatformBuilder_Offset, Network_GetPlatformBuilder,
         il2cpp::Network_GetPlatformBuilder);

    il2cpp::Localization_GetLocaleName = reinterpret_cast<System_String_o *(*)()>(
        getAbsoluteAddressStr(targetLibName, Localization_GetLocaleName_Offset));

    HOOK(Localization_SetPegLocaleName_Offset, Localization_SetPegLocaleName,
        il2cpp::Localization_SetPegLocaleName);

//#if MatchingQueueTab_Update_Patch_Offset = "0"
    PATCH(MatchingQueueTab_Update_Patch_Offset, MatchingQueueTab_Update_Patch_Data);
//#endif

//#if ThinkEmoteManager_Update_Patch_Offset != "0"
    PATCH(ThinkEmoteManager_Update_Patch_Offset, ThinkEmoteManager_Update_Patch_Data);
//#endif

//#if PlatformSettings_EmulateMobileDevice_Patch_Offset != "0"
    PATCH(PlatformSettings_EmulateMobileDevice_Patch_Offset,
          PlatformSettings_EmulateMobileDevice_Patch_Data);
//#endif

    //Anti-leech
    /*if (!iconValid || !initValid || !settingsValid) {
        //Bad function to make it crash
        sleep(5);
        int *p = 0;
        *p = 0;
    }*/

    return NULL;
}

jobjectArray SettingsList(JNIEnv *env, jobject activityObject) {
    jobjectArray ret;

    const char *features[] = {
            "Category_Settings",
            localization[language][SAVE_SETTINGS], //-1 is checked on Preferences.java
            localization[language][AUTO_SIZE],
            localization[language][IGNORE_SCREEN_CUTOUTS],
            "Category_Menu",
            localization[language][LANGUAGE],
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
            localization[language][TIMESCALE],
            "Category_Gameplay",
            localization[language][SKIP_HERO_INTRO],
            localization[language][SHUTUP_BOB],
            localization[language][EXTENDED_BM],
            localization[language][DISABLE_RANDOM_FOR_EMOTES],
            localization[language][EMOTE_SPAM_BLOCKER],
            localization[language][EMOTES_BEFORE_BLOCK],
            localization[language][DISABLE_THINK_EMOTES],
            localization[language][GOLDEN_TEXT],
            localization[language][DIAMOND_TEXT],
            localization[language][SIGNATURE_TEXT],
            localization[language][SHOW_OPPONENT_RANK_IN_GAME],
            localization[language][DEVICE_PRESET],
            localization[language][OPERATING_SYSTEM],
            localization[language][SCREEN_TEXT],
            localization[language][DEVICE_NAME],
            /*"Category_Others",
            localization[language][MOVE_ENEMY_CARDS],*/
            "Category_Shortcuts",
            localization[language][COPY_BATTLETAG],
            localization[language][COPY_BATTLETAG_ON_BATTLEGROUNDS],
            localization[language][SIMULATE_DISCONNECT],
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
        case 0:
            timeScaleEnabled = boolean;
            if (gameLoaded) {
                Time_set_timeScale(originalTimeScale);
            }
            break;
        case 1:
            timeScale = value / 100.0f;
            if (gameLoaded) {
                Time_set_timeScale(originalTimeScale);
            }
            break;
        case 2:
            // тут был MulliganManager
            break;
        case 3:
            PATCH_SWITCH(TB_BaconShop_HandleGameOverWithTiming_Patch_Offset,
                         TB_BaconShop_HandleGameOverWithTiming_Patch_Data, boolean);
            PATCH_SWITCH(TB_BaconShop_PlayBobLineWithoutText_Patch_Offset,
                         TB_BaconShop_PlayBobLineWithoutText_Patch_Data, boolean);
            break;
        case 4:
            PATCH_SWITCH(EmoteHandler_EmoteSpamBlocked_Patch_Offset,
                         EmoteHandler_EmoteSpamBlocked_Patch_Data, boolean);
            break;
        case 5:
            PATCH_SWITCH(EmoteHandler_HandleInput_Patch_Offset, EmoteHandler_HandleInput_Patch_Data,
                         boolean);
            break;
        case 6:
            emoteSpamBlocker = boolean;
            PATCH_SWITCH(EnemyEmoteHandler_Awake_Patch_Offset, EnemyEmoteHandler_Awake_Patch_Data,
                         emoteSpamBlocker && emotesBeforeBlock == 0);
            break;
        case 7:
            emotesBeforeBlock = value;
            PATCH_SWITCH(EnemyEmoteHandler_Awake_Patch_Offset, EnemyEmoteHandler_Awake_Patch_Data,
                         emoteSpamBlocker && emotesBeforeBlock == 0);
            break;
        case 8:
            disableThinkEmotes = boolean;
            break;
        case 9:
            golden = static_cast<CardState>(value);
            break;
        case 10:
            diamond = static_cast<CardState>(value);
            break;
        case 21:
            signature = static_cast<CardState>(value);
            break;
        case 11:
            PATCH_SWITCH(NameBanner_Initialize_Patch_Offset, NameBanner_Initialize_Patch_Data,
                         boolean);
            PATCH_SWITCH(NameBanner_UpdateMedalWhenReady_Patch_Offset,
                         NameBanner_UpdateMedalWhenReady_Patch_Data, boolean);
            break;
        case 16:
            devicePreset = static_cast<DevicePreset>(value);
            break;
        case 17:
            _os = static_cast<OSCategory>(value);
            break;
        case 18:
            _screen = static_cast<ScreenCategory>(value);
            break;
        case 19:
            _deviceName = il2cpp::il2cpp_string_new(cstr);
            break;
        case 22:
            if (gameLoaded) {
                void *thread = il2cpp::il2cpp_thread_attach(il2cpp::il2cpp_domain_get());
                BnetPlayer_o *currentOpponent;
                if (il2cpp::GameMgr_IsBattlegrounds(il2cpp::GameMgr_Get())) {
                    currentOpponent = playerLeaderboardManagerCurrentOpponent;
                } else {
                    currentOpponent = gameStateCurrentOpponent;
                }
                if (currentOpponent != NULL) {
                    BnetBattleTag_o *battleTag = il2cpp::BnetPlayer_GetBattleTag(currentOpponent);
                    if (battleTag != NULL) {
                        System_String_o *str = il2cpp::BnetBattleTag_GetString(battleTag);
                        il2cpp::ClipboardUtils_CopyToClipboard(str);
                        il2cpp::UIStatus_AddInfo(il2cpp::UIStatus_Get(), str);
                    }
                }
                il2cpp::il2cpp_thread_detach(thread);
            }
            break;
        case 23:
            copySelectedBattleTag = boolean;
            break;
        case 24:
            if (gameLoaded) {
                void *thread = il2cpp::il2cpp_thread_attach(il2cpp::il2cpp_domain_get());
                il2cpp::Network_SimulateUncleanDisconnectFromGameServer(il2cpp::Network_Get());
                il2cpp::il2cpp_thread_detach(thread);
            }
            break;
        case 25:
            timeScaleInGameOnlyEnabled = boolean;
            if (gameLoaded) {
                Time_set_timeScale(originalTimeScale);
            }
            break;
        case -11:
            PATCH_LIB_SWITCH("libunity.so", Unity_AndroidRenderOutsideSafeArea_Offset, "31",
                             boolean);
            break;
        case -10:
            switch (value) {
                case 0: {
                    useDefaultLanguage = true;
                    if (!languageLoaded) {
                        break;
                    }
                    void *thread = il2cpp::il2cpp_thread_attach(il2cpp::il2cpp_domain_get());
                    auto locale = il2cpp::Localization_GetLocaleName();
                    if (locale == NULL) {
                        language = "enUS";
                        break;
                    }
                    auto newLanguage = SS_to_str(locale);
                    if (localization.find(newLanguage) == localization.end()) {
                        language = "enUS";
                    } else {
                        language = newLanguage;
                    }
                    il2cpp::il2cpp_thread_detach(thread);
                    break;
                }
                case 1:
                    useDefaultLanguage = false;
                    language = "enUS";
                    break;
                case 2:
                    useDefaultLanguage = false;
                    language = "ruRU";
                    break;
                case 3:
                    useDefaultLanguage = false;
                    language = "znCN";
                    break;
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