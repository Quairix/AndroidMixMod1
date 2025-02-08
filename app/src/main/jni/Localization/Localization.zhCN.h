#ifndef ANDROIDMIXMOD_LOCALIZATION_ZHCH_H
#define ANDROIDMIXMOD_LOCALIZATION_ZHCH_H

#include "Localization.h"

locale_map zhCN = localization["zhCN"] = std::unordered_map<int, const char*>{
	{ TIMESCALE_ENABLED, "0_Toggle_启用时间刻度" },
	{ TIMESCALE_IN_GAME_ONLY, "25_Toggle_时间刻度仅在游戏中" },
	{ TIMESCALE, "1_SeekBar_时间刻度值_100_800" },
	{ SAVE_SETTINGS, "-1_Toggle_保存功能首选项" },
	{ AUTO_SIZE, "-3_Toggle_自动调整垂直大小" },
	{ CLOSE_SETTINGS, "-6_Button_<font color='red'>关闭设置</font>" }
};

#endif //ANDROIDMIXMOD_LOCALIZATION_ZHCH_H