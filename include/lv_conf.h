/**
 * @file lv_conf.h
 * Configuration file for v9.2.2 / v9.6.0
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#define LV_USE_STDLIB_MALLOC LV_STDLIB_BUILTIN
#define LV_MEM_SIZE 49152  // Ставимо 48 KB (це захистить від Memory Overflow у внутрішній SRAM)


/* --- КРИТИЧНИЙ ФІКС ДЛЯ АСЕМБЛЕРА (ESP32-S3 / XTENSA) --- */
/* Якщо файл читає асемблер (наприклад, для рендерерів), ми приховуємо весь C-код */
#ifndef __ASSEMBLY__

#include <stdint.h>

/*============================================================================
 * MEMORY AND STANDARD LIBRARY
 *============================================================================*/

#ifndef LV_USE_STDLIB_MALLOC
#define LV_USE_STDLIB_MALLOC LV_STDLIB_CLIB
#endif

#ifndef LV_USE_STDLIB_STRING
#define LV_USE_STDLIB_STRING LV_STDLIB_CLIB
#endif

#ifndef LV_USE_STDLIB_SPRINTF
#define LV_USE_STDLIB_SPRINTF LV_STDLIB_CLIB
#endif

#define LV_STDINT_INCLUDE "stdint.h"
#define LV_STDDEF_INCLUDE "stddef.h"
#define LV_STDBOOL_INCLUDE "stdbool.h"
#define LV_INTTYPES_INCLUDE "inttypes.h"
#define LV_LIMITS_INCLUDE "limits.h"
#define LV_STDARG_INCLUDE "stdarg.h"

/*============================================================================
 * OPERATING SYSTEM (OS)
 *============================================================================*/

#ifndef LV_USE_OS
#define LV_USE_OS LV_OS_NONE
#endif

/*============================================================================
 * RENDERING CONFIGURATION
 *============================================================================*/

#ifndef LV_COLOR_FORMAT_DEFAULT
#define LV_COLOR_FORMAT_DEFAULT LV_COLOR_FORMAT_RGB565
#endif

#define LV_COLOR_MIX_ROUND_OFS 128

#ifndef LV_DEF_REFR_PERIOD
#define LV_DEF_REFR_PERIOD 16
#endif

#ifndef LV_DPI_DEF
#define LV_DPI_DEF 130
#endif

#define LV_DRAW_BUF_STRIDE_ALIGN 1
#define LV_DRAW_BUF_ALIGN 4
#define LV_USE_MATRIX 0

#define LV_DRAW_LAYER_SIMPLE_BUF_SIZE 24576
#define LV_DRAW_LAYER_MAX_MEMORY 0

#define LV_USE_VECTOR_GRAPHIC 0
#define LV_USE_SNAPSHOT 0

#define LV_USE_DRAW_SW 1

#if LV_USE_DRAW_SW
    #define LV_DRAW_SW_SUPPORT_RGB565 1
    #define LV_DRAW_SW_SUPPORT_RGB565_SWAPPED 1
    #define LV_DRAW_SW_SUPPORT_RGB565A8 1
    #define LV_DRAW_SW_SUPPORT_RGB888 1
    #define LV_DRAW_SW_SUPPORT_XRGB8888 1
    #define LV_DRAW_SW_SUPPORT_ARGB8888 1
    #define LV_DRAW_SW_SUPPORT_ARGB8888_PREMULTIPLIED 1
    #define LV_DRAW_SW_SUPPORT_L8 1
    #define LV_DRAW_SW_SUPPORT_AL88 1
    #define LV_DRAW_SW_SUPPORT_A8 1
    #define LV_DRAW_SW_SUPPORT_I1 0
#endif

#define LV_USE_DRAW_ARM2D_SYNC 0

/*============================================================================
 * FEATURE CONFIGURATION
 *============================================================================*/

#define LV_USE_LOG 0
#define LV_USE_ASSERT_NULL 1
#define LV_USE_ASSERT_MALLOC 1
#define LV_USE_ASSERT_STYLE 0
#define LV_USE_ASSERT_REFR 0
#define LV_USE_ASSERT_OBJ 1

#define LV_LOG_PRINTF 0

/* МОНІТОР FPS ТА ПРОЦЕСОРА */
#ifndef LV_USE_PERF_MONITOR
#define LV_USE_PERF_MONITOR 1
#endif

#if LV_USE_PERF_MONITOR
    #define LV_USE_PERF_MONITOR_POS LV_ALIGN_BOTTOM_RIGHT
#endif

#define LV_USE_MEM_MONITOR 0
#define LV_USE_ANIMATION 1

/*============================================================================
 * WIDGETS CONFIGURATION
 *============================================================================*/

#define LV_USE_ARC        1
#define LV_USE_BAR        1
#define LV_USE_CHART      1
#define LV_USE_LABEL      1
#define LV_USE_SCALE      1  /* Потрібно для оновленого Екрана 4 */
#define LV_USE_LINE       1  /* Потрібно для стрілки Екрана 4 */
#define LV_USE_SLIDER     1
#define LV_USE_SWITCH     1
#define LV_USE_GRID       1

/*============================================================================
 * FONT CONFIGURATION
 *============================================================================*/

#ifndef LV_FONT_MONTSERRAT_14
#define LV_FONT_MONTSERRAT_14 1
#endif

#ifndef LV_FONT_MONTSERRAT_18
#define LV_FONT_MONTSERRAT_18 1
#endif

#ifndef LV_FONT_MONTSERRAT_22
#define LV_FONT_MONTSERRAT_22 1
#endif

#ifndef LV_FONT_MONTSERRAT_32
#define LV_FONT_MONTSERRAT_32 1
#endif

#define LV_FONT_DEFAULT &lv_font_montserrat_14
#define LV_USE_FONT_PLACEHOLDER 1

/*============================================================================
 * THEMES CONFIGURATION
 *============================================================================*/

#define LV_USE_THEME_DEFAULT 1
#if LV_USE_THEME_DEFAULT
    #define LV_THEME_DEFAULT_DARK 1
    #define LV_THEME_DEFAULT_GROW 1
    #define LV_THEME_DEFAULT_TRANSITION_TIME 80
#endif

#endif /* __ASSEMBLY__ */
#endif /* LV_CONF_H */