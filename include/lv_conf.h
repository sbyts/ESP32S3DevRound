#ifndef LV_CONF_H
#define LV_CONF_H

// СУВОРИЙ ЗАХИСТ: C-код бачать ТІЛЬКИ C/C++ компілятори, а асемблер його ігнорує
#ifndef __ASSEMBLY__
#include <stdint.h>

#if 1 /* Активація конфігурації (Змінено з 0 на 1) */

/* ==========================================================
   ГЛОБАЛЬНЕ ВИМКНЕННЯ ЗГЛАДЖУВАННЯ (АНТІАЛІАСИНГУ)
   ========================================================== */
#define LV_USE_ANTIALIASING 0

/* Налаштування графічного ядра */
#define LV_COLOR_DEPTH 16
#define LV_DISPLAY_DEF_REFRESH_RATE 30

/* Виділення пам'яті всередині SRAM чипа */
#define LV_MEM_SIZE (48U * 1024U)
#define LV_MEM_ADR 0

/* АКТИВАЦІЯ СИСТЕМНИХ ШРИФТІВ */
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_22 1
#define LV_FONT_MONTSERRAT_32 1

/* Налаштування віджетів */
#define LV_USE_ARC 1
#define LV_USE_BAR 1
#define LV_USE_LINE 1
#define LV_USE_LABEL 1

#endif /* Активація */
#endif /* Кінець перевірки __ASSEMBLY__ */

#endif /* LV_CONF_H */