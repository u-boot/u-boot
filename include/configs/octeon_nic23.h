/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019-2020
 * Marvell <www.marvell.com>
 */

#ifndef __CONFIG_H__
#define __CONFIG_H__

/*
 * SATA/SCSI/AHCI configuration
 */
/* AHCI support Definitions */
/** Enable 48-bit SATA addressing */
#define CONFIG_LBA48
/** Enable 64-bit addressing */
#define CONFIG_SYS_64BIT_LBA

#include "octeon_common.h"

#endif /* __CONFIG_H__ */
