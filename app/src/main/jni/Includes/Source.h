#pragma once

#include <codecvt>
#include "Offsets.h"

#if _MSC_VER
typedef wchar_t Il2CppChar;
#elif __has_feature(cxx_unicode_literals)
typedef char16_t Il2CppChar;
#else
typedef uint16_t Il2CppChar;
#endif

enum class OSCategory {
    PC = 1,
    Mac,
    iOS,
    Android
};

enum class ScreenCategory {
    Phone = 1,
    MiniTablet,
    Tablet,
    PC
};

enum class CardState {
    Default = 0,
    OnlyMy,
    All,
    Disabled
};

enum class TAG_PREMIUM {
    NORMAL = 0,
    GOLDEN,
    DIAMOND,
    SIGNATURE,
    MAX = 3
};

enum class DevicePreset {
    Default,
    iPad,
    iPhone,
    Phone,
    Tablet,
    HuaweiPhone,
    PC,
    Mac,
    Custom
};

namespace il2cpp {
    Il2CppObject *(*il2cpp_object_new)(Il2CppClass *klass);

    void *(*il2cpp_domain_get)();

    void *(*il2cpp_thread_attach)(void *domain);

    void (*il2cpp_thread_detach)(void *thread);

    uint (*il2cpp_gchandle_new)(void *object, bool weak);

    void (*il2cpp_gchandle_free)(uint gchandle);

    void *(*il2cpp_array_new)(Il2CppClass *klass, size_t length);

    Il2CppObject *(*il2cpp_value_box)(Il2CppClass *klass, void *data);

    struct Il2CppClass **object_TypeInfo;
    struct Il2CppClass **OSCategory_TypeInfo;
    struct Il2CppClass **ScreenCategory_TypeInfo;

    System_String_o *(*UnityEngine_SystemInfo_get_deviceUniqueIdentifier)();

    System_String_o *(*System_String_Format)(System_String_o *format, System_Object_array *args);

    struct Il2CppClass **System_Text_StringBuilder_TypeInfo;

    void (*System_Text_StringBuilder_ctor)(System_Text_StringBuilder_o *_this);

    System_Text_StringBuilder_o *
    (*System_Text_StringBuilder_AppendString)(System_Text_StringBuilder_o *_this,
                                              System_String_o *value);

    System_String_o *(*Crypto_SHA1_Calc)(System_String_o *message);

    System_Security_Cryptography_MD5_o *(*System_Security_Cryptography_MD5_Create)();

    System_Byte_array *(*System_Security_Cryptography_HashAlgorithm_ComputeHash)(
            System_Security_Cryptography_HashAlgorithm_o *_this, System_Byte_array *buffer);

    System_Text_Encoding_o *(*System_Text_Encoding_get_Default)();

    System_Byte_array *
    (*System_Text_Encoding_GetBytes)(System_Text_Encoding_o *_this, System_String_o *str);

    System_String_o *(*System_Byte_ToStringFormat)(uint8_t *_this, System_String_o *format);

    void (*System_Guid_ctor)(System_Guid_o _this, System_String_o *g);

    System_String_o *(*System_String_ToUpper)(System_String_o *_this);

    struct MethodInfo **Method_Blizzard_T5_Core_Map_int_SharedPlayerInfo_ContainsKey;
    struct MethodInfo **Method_Blizzard_T5_Core_Map_int_SharedPlayerInfo_get_Item;

    float (*Time_get_timeScale)();

    void (*Time_set_timeScale)(float value);

    void (*GameMgr_OnGameSetup)(GameMgr_o *_this);

    void (*GameMgr_OnGameCanceled)(GameMgr_o *_this);

    void (*GameMgr_OnGameEnded)(GameMgr_o *_this);

    void (*HearthstoneApplication_Awake)(Hearthstone_HearthstoneApplication_o *_this);

    bool (*RemoteActionHandler_CanReceiveEnemyEmote)(RemoteActionHandler_o *_this, int emoteType,
                                                     int playerId);

    EnemyEmoteHandler_o *(*EnemyEmoteHandler_Get)();

    void
    (*Blizzard_T5_Core_Map_int_bool_set_Item)(Blizzard_T5_Core_Map_int__bool__o *__this, int key,
                                              bool value);

    float (*UnityEngine_Time_get_time)();

    void (*Entity_LoadCard)(Entity_o *_this, System_String_o *cardId, Entity_LoadCardData_o *data);

    TAG_PREMIUM (*Entity_GetPremiumType)(Entity_o *_this);

    GameMgr_o *(*GameMgr_Get)();

    bool (*GameMgr_IsBattlegrounds)(GameMgr_o *_this);

    GameState_o *(*GameState_Get)();

    bool (*GameState_IsGameCreatedOrCreating)(GameState_o *_this);

    Player_o *(*GameState_GetOpposingSidePlayer)(GameState_o *_this);

    Blizzard_T5_Core_Map_int__SharedPlayerInfo__o *
    (*GameState_GetPlayerInfoMap)(GameState_o *_this);

    bool (*Blizzard_T5_Core_Map_int_object_ContainsKey)(
            Blizzard_T5_Core_Map_TKey__TValue__o *_this, int key,
            const MethodInfo *method);

    Il2CppObject *(*Blizzard_T5_Core_Map_int_object_get_Item)(
            Blizzard_T5_Core_Map_TKey__TValue__o *_this, int key,
            const MethodInfo *method);

    bool (*EntityBase_HasTag)(EntityBase_o *_this, int tag);

    bool (*Entity_IsControlledByFriendlySidePlayer)(Entity_o *_this);

    Player_o *(*Entity_GetController)(Entity_o *_this);

    bool (*System_String_IsNullOrEmpty)(System_String_o *value);

    Network_o *(*Network_Get)();

    void (*Network_SimulateUncleanDisconnectFromGameServer)(Network_o *_this);

    void (*Gameplay_OnCreateGame)(Gameplay_o *_this, int phase, Il2CppObject *userData);

    void (*PlayerLeaderboardManager_SetNextOpponent)(PlayerLeaderboardManager_o *_this,
                                                     int opponentPlayerId);

    void (*PlayerLeaderboardManager_SetCurrentOpponent)(PlayerLeaderboardManager_o *_this,
                                                        int opponentPlayerId);

    void (*PlayerLeaderboardCard_NotifyMousedOver)(PlayerLeaderboardCard_o *_this);

    BnetPresenceMgr_o *(*BnetPresenceMgr_Get)();

    BnetPlayer_o *(*BnetPresenceMgr_GetPlayer)(BnetPresenceMgr_o *_this,
                                               Blizzard_GameService_SDK_Client_Integration_BnetGameAccountId_o *id);

    BnetBattleTag_o *(*BnetPlayer_GetBattleTag)(BnetPlayer_o *_this);

    System_String_o *(*BnetBattleTag_GetString)(BnetBattleTag_o *_this);

    void (*ClipboardUtils_CopyToClipboard)(System_String_o *copyText);

    UIStatus_o *(*UIStatus_Get)();

    void (*UIStatus_AddInfo)(UIStatus_o *_this, System_String_o *message);

    HistoryManager_o *(*HistoryManager_Get)();

    PlayerLeaderboardCard_o *(*HistoryManager_GetCurrentBigCard)(HistoryManager_o *_this);

    int (*EntityBase_GetTag)(EntityBase_o *_this, int tag);

    System_String_o *(*il2cpp_string_new)(const char *text);

    System_String_o *(*il2cpp_string_new_utf16)(const Il2CppChar *text, int len);

    void (*ThinkEmoteManager_Update)(ThinkEmoteManager_o *_this);

    void (*SocialToastMgr_AddToast)(SocialToastMgr_o *_this, int blocker, System_String_o *textArg,
                                    int toastType, float displayTime, bool playSound);

    PegasusShared_Platform_o *(*Network_GetPlatformBuilder)(Network_o *_this);

    System_Guid_o (*System_Xml_XmlConvert_ToGuid)(System_String_o *s);

    System_String_o *(*System_Guid_ToString)(System_Guid_o *_this);

    System_String_o *(*Localization_GetLocaleName)();

    void (*Localization_SetPegLocaleName)(Localization_o *_this, System_String_o *localeName);
};

std::wstring_convert<std::codecvt_utf8_utf16<Il2CppChar>, Il2CppChar> utf16conv;

std::string SS_to_str(System_String_o *SS) {
    return utf16conv.to_bytes(reinterpret_cast<Il2CppChar*>(&SS->fields._firstChar));
}

const char *SS_to_cstr(System_String_o *SS) {
    return SS_to_str(SS).c_str();
}

System_String_o *operator "" _SS(const char *c_str) {
    return il2cpp::il2cpp_string_new(c_str);
}

System_String_o *operator "" _SS(const Il2CppChar *c_str, size_t len) {
    return il2cpp::il2cpp_string_new_utf16(c_str, len);
}