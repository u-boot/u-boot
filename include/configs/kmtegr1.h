/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2006 Freescale Semiconductor, Inc.
 *                    Dave Liu <daveliu@freescale.com>
 *
 * Copyright (C) 2007 Logic Product Development, Inc.
 *                    Peter Barada <peterb@logicpd.com>
 *
 * Copyright (C) 2007 MontaVista Software, Inc.
 *                    Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * (C) Copyright 2010
 * Heiko Schocher, DENX Software Engineering, hs@denx.de.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/*
 * High Level Configuration Options
 */

#define CONFIG_HOSTNAME   "kmtegr1"
#define CONFIG_KM_UBI_PARTITION_NAME_BOOT	"ubi0"
#define CONFIG_KM_UBI_PARTITION_NAME_APP	"ubi1"

#define CONFIG_NAND_ECC_BCH
#define CONFIG_NAND_KMETER1
#define CONFIG_SYS_MAX_NAND_DEVICE		1
#define NAND_MAX_CHIPS				1

#define CONFIG_SYS_SICRL (0     \
		| SICR_1_UART1_UART1RTS   \
		| SICR_1_I2C_CKSTOP   \
		| SICR_1_IRQ_A_IRQ    \
		| SICR_1_IRQ_B_IRQ    \
		| SICR_1_GPIO_A_GPIO    \
		| SICR_1_GPIO_B_GPIO    \
		| SICR_1_GPIO_C_GPIO    \
		| SICR_1_GPIO_D_GPIO    \
		| SICR_1_GPIO_E_LCS    \
		| SICR_1_GPIO_F_GPIO    \
		| SICR_1_USB_A_UART2S   \
		| SICR_1_USB_B_UART2RTS   \
		| SICR_1_FEC1_FEC1    \
		| SICR_1_FEC2_FEC2    \
	)

/* include common defines/options for all Keymile boards */
#include "km/keymile-common.h"
#include "km/km-powerpc.h"
#include "km/km-mpc83xx.h"
#include "km/km-mpc8309.h"

/* must be after the include because KMBEC_FPGA is otherwise undefined */
#define CONFIG_SYS_NAND_BASE CONFIG_SYS_KMBEC_FPGA_BASE /* PRIO_BASE_ADDRESS */

#endif /* __CONFIG_H */
