/*
 * blackfin-config-post.h - setup common defines for Blackfin boards based on config.h
 *
 * Copyright (c) 2007 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#ifndef __ASM_BLACKFIN_CONFIG_POST_H__
#define __ASM_BLACKFIN_CONFIG_POST_H__

/* Check to make sure everything fits in external RAM */
#if ((CFG_MONITOR_BASE + CFG_MONITOR_LEN) > CFG_MAX_RAM_SIZE)
# error Memory Map does not fit into configuration
#endif

/* Sanity check BFIN_CPU */
#ifndef BFIN_CPU
# error BFIN_CPU: your board config needs to define this
#endif

/* Make sure the structure is properly aligned */
#if ((CFG_GBL_DATA_ADDR & -4) != CFG_GBL_DATA_ADDR)
# error CFG_GBL_DATA_ADDR: must be 4 byte aligned
#endif

/* Set default CONFIG_VCO_HZ if need be */
#if !defined(CONFIG_VCO_HZ)
# if (CONFIG_CLKIN_HALF == 0)
#  define CONFIG_VCO_HZ (CONFIG_CLKIN_HZ * CONFIG_VCO_MULT)
# else
#  define CONFIG_VCO_HZ ((CONFIG_CLKIN_HZ * CONFIG_VCO_MULT) / 2)
# endif
#endif

/* Set default CONFIG_CCLK_HZ if need be */
#if !defined(CONFIG_CCLK_HZ)
# if (CONFIG_PLL_BYPASS == 0)
#  define CONFIG_CCLK_HZ (CONFIG_VCO_HZ / CONFIG_CCLK_DIV)
# else
#  define CONFIG_CCLK_HZ CONFIG_CLKIN_HZ
# endif
#endif

/* Set default CONFIG_SCLK_HZ if need be */
#if !defined(CONFIG_SCLK_HZ)
# if (CONFIG_PLL_BYPASS == 0)
#  define CONFIG_SCLK_HZ (CONFIG_VCO_HZ / CONFIG_SCLK_DIV)
# else
#  define CONFIG_SCLK_HZ CONFIG_CLKIN_HZ
# endif
#endif

/* Since we use these to program PLL registers directly,
 * make sure the values are sane and won't screw us up.
 */
#if (CONFIG_VCO_MULT & 0x3F) != CONFIG_VCO_MULT
# error CONFIG_VCO_MULT: Invalid value: must fit in 6 bits (0 - 63)
#endif
#if (CONFIG_CLKIN_HALF & 0x1) != CONFIG_CLKIN_HALF
# error CONFIG_CLKIN_HALF: Invalid value: must be 0 or 1
#endif
#if (CONFIG_PLL_BYPASS & 0x1) != CONFIG_PLL_BYPASS
# error CONFIG_PLL_BYPASS: Invalid value: must be 0 or 1
#endif

/* Using L1 scratch pad makes sense for everyone by default. */
#ifndef CMD_LINE_ADDR
# define CMD_LINE_ADDR L1_SRAM_SCRATCH
#endif

#endif
