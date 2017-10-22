/*
 * Copyright 2016 3ADEV <http://3adev.com>
 * Written-by: Albert ARIBAUD <albert.aribaud@3adev.fr>
 *
 * Configuration settings for the phytec PCM-052 SoM-based BK4R1.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/* Define the BK4r1-specific env commands */
#define PCM052_EXTRA_ENV_SETTINGS \
	"set_gpio103=mw 0x400ff0c4 0x0080; mw 0x4004819C 0x000011bf\0" \
	"set_gpio122=mw 0x400481e8 0x0282; mw 0x400ff0c4 0x04000000\0"

/* BK4r1 boot command sets GPIO103/PTC30 to force USB hub out of reset*/
#define PCM052_BOOTCOMMAND "run set_gpio103; sf probe; "

/* BK4r1 net init sets GPIO122/PTE17 to enable Ethernet */
#define PCM052_NET_INIT "run set_gpio122; "

/* add NOR to MTD env */

/* now include standard PCM052 config */

#include "configs/pcm052.h"
