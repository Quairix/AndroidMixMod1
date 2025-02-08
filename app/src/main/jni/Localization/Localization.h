#ifndef ANDROIDMIXMOD_LOCALIZATION_H
#define ANDROIDMIXMOD_LOCALIZATION_H

#define TIMESCALE_ENABLED 0
#define TIMESCALE_IN_GAME_ONLY 2
#define TIMESCALE 1
#define SAVE_SETTINGS 28
#define AUTO_SIZE 29
#define CLOSE_SETTINGS 30

#include <unordered_map>

typedef std::unordered_map<int, const char*> locale_map;
typedef std::unordered_map<std::string, locale_map> localization_map;
localization_map localization;

locale_map enUS = localization["enUS"] = std::unordered_map<int, const char*>{
	{ TIMESCALE_ENABLED, "0_Toggle_Timescale" },
	{ TIMESCALE_IN_GAME_ONLY, "25_Toggle_Timescale only in game" },
	{ TIMESCALE, "1_SeekBar_Timescale value_100_800" },
	{ SAVE_SETTINGS, "-1_Toggle_Save feature preferences"},
	{ AUTO_SIZE, "-3_Toggle_Auto size vertically"},
	{ CLOSE_SETTINGS, "-6_Button_<font color='red'>Close settings</font>"}
};

#endif //ANDROIDMIXMOD_LOCALIZATION_H
