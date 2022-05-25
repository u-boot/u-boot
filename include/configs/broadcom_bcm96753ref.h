/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2022 Philippe Reynes <philippe.reynes@softathome.com>
 */

#include <linux/sizes.h>

/*
 * common
 */

/* UART */
#define CONFIG_SYS_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200, \
					  230400, 500000, 1500000 }
/* Memory usage */

/*
 * 6853
 */

/* RAM */
#define CONFIG_SYS_SDRAM_BASE		0x00000000

/* U-Boot */

#ifdef CONFIG_MTD_RAW_NAND
#define CONFIG_SYS_MAX_NAND_DEVICE	1
#endif /* CONFIG_MTD_RAW_NAND */

/*
 * 96753ref
 */
