/*
 * (C) Copyright 2008
 * Matthias Fuchs, esd gmbh, matthias.fuchs@esd-electronics.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#define SDR0_USB0		0x0320     /* USB Control Register */

#define CONFIG_SYS_GPIO0_EP_EEP	(0x80000000 >> 23)       /* GPIO0_23 */
#define CONFIG_SYS_GPIO1_DCF77		(0x80000000 >> (42-32))  /* GPIO1_42 */

#define CONFIG_SYS_GPIO1_IORSTN	(0x80000000 >> (55-32))  /* GPIO1_55 */
#define CONFIG_SYS_GPIO1_IORST2N	(0x80000000 >> (47-32))  /* GPIO1_47 */

#define CONFIG_SYS_GPIO1_HWVER_MASK	0x000000f0 /* GPIO1_56-59 */
#define CONFIG_SYS_GPIO1_HWVER_SHIFT	4
#define CONFIG_SYS_GPIO1_LEDUSR1	0x00000008 /* GPIO1_60 */
#define CONFIG_SYS_GPIO1_LEDUSR2	0x00000004 /* GPIO1_61 */
#define CONFIG_SYS_GPIO1_LEDPOST	0x00000002 /* GPIO1_62 */
#define CONFIG_SYS_GPIO1_LEDDU		0x00000001 /* GPIO1_63 */

#define CPLD_VERSION_MASK	0x0f
#define PWR_INT_FLAG		0x80
#define PWR_RDY			0x10

#define CPLD_IRQ		(32+30)
