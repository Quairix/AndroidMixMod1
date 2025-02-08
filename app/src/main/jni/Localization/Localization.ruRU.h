#ifndef ANDROIDMIXMOD_LOCALIZATION_RURU_H
#define ANDROIDMIXMOD_LOCALIZATION_RURU_H

#include "Localization.h"

locale_map ruRU = localization["ruRU"] = std::unordered_map<int, const char*>{
	{ TIMESCALE_ENABLED, "0_Toggle_Ускорение анимации" },
	{ TIMESCALE_IN_GAME_ONLY, "25_Toggle_Ускорение только в игре" },
	{ TIMESCALE, "1_SeekBar_Cкорость_100_800" },
	{ SAVE_SETTINGS, "-1_Toggle_Сохранять настройки" },
	{ AUTO_SIZE, "-3_Toggle_Авторазмер по вертикали" },
	{ CLOSE_SETTINGS, "-6_Button_<font color='red'>Закрыть настройки</font>" }
};

#endif //ANDROIDMIXMOD_LOCALIZATION_RURU_H