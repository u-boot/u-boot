/*
 * (C) Copyright 2005
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __P3P440_H__
#define __P3P440_H__

#define CONFIG_SYS_GPIO_RDY	(0x80000000 >> 11)
#define CONFIG_SYS_MONARCH_IO	(0x80000000 >> 18)
#define CONFIG_SYS_EREADY_IO	(0x80000000 >> 20)
#define CONFIG_SYS_LED_GREEN	(0x80000000 >> 21)
#define CONFIG_SYS_LED_RED	(0x80000000 >> 22)

#define LED_OFF		1
#define LED_GREEN	2
#define LED_RED		3
#define LED_ORANGE	4

long int fixed_sdram(void);

#endif /* __P3P440_H__ */
