#ifndef __SBCOMMON_H__
#define __SBCOMMON_H__
/*
 *  Copyright (C) 2005 Sandburst Corporation
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <config.h>
#include <common.h>
#include <command.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <spd_sdram.h>
#include <i2c.h>

/*
 * GPIO Settings
 */
/* Chassis settings */
#define SBCOMMON_GPIO_PRI_N		0x00001000  /* 0 = Chassis Master, 1 = Slave */
#define SBCOMMON_GPIO_SEC_PRES		0x00000800  /* 1 = Other board present */

/* Debug LEDs */
#define SBCOMMON_GPIO_DBGLED_0		0x00000400
#define SBCOMMON_GPIO_DBGLED_1		0x00000200
#define SBCOMMON_GPIO_DBGLED_2		0x00100000
#define SBCOMMON_GPIO_DBGLED_3		0x00000100

#define SBCOMMON_GPIO_DBGLEDS		(SBCOMMON_GPIO_DBGLED_0 | \
					 SBCOMMON_GPIO_DBGLED_1 | \
					 SBCOMMON_GPIO_DBGLED_2 | \
					 SBCOMMON_GPIO_DBGLED_3)

#define SBCOMMON_GPIO_SYS_FAULT		0x00000080
#define SBCOMMON_GPIO_SYS_OTEMP		0x00000040
#define SBCOMMON_GPIO_SYS_STATUS	0x00000020

#define SBCOMMON_GPIO_SYS_LEDS		(SBCOMMON_GPIO_SYS_STATUS)

#define SBCOMMON_GPIO_LEDS		(SBCOMMON_GPIO_DBGLED_0 | \
					 SBCOMMON_GPIO_DBGLED_1 | \
					 SBCOMMON_GPIO_DBGLED_2 | \
					 SBCOMMON_GPIO_DBGLED_3 | \
					 SBCOMMON_GPIO_SYS_STATUS)

typedef struct ppc440_gpio_regs {
	volatile unsigned long out;
	volatile unsigned long tri_state;
	volatile unsigned long dummy[4];
	volatile unsigned long open_drain;
	volatile unsigned long in;
}  __attribute__((packed)) ppc440_gpio_regs_t;

int sbcommon_get_master(void);
int sbcommon_secondary_present(void);
unsigned short sbcommon_get_serial_number(void);
void sbcommon_fans(void);
void board_get_enetaddr(int macaddr_idx, uchar *enet);

#endif /* __SBCOMMON_H__ */
