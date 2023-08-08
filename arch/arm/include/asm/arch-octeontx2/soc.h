/* SPDX-License-Identifier:    GPL-2.0
 *
 * Copyright (C) 2019 Marvell International Ltd.
 *
 * https://spdx.org/licenses
 */

#ifndef __SOC_H__
#define __SOC_H__

/* Product PARTNUM */
#define CN98XX	0xB1
#define CN96XX	0xB2
#define CN95XX	0xB3
#define LOKI	0xB4
#define F95MM	0xB5

/* Register defines */

#define otx_is_soc(soc)	(read_partnum() == (soc))
#define otx_is_board(model) (!strcmp(read_board_name(), model))
#define otx_is_platform(platform) (read_platform() == (platform))

typedef enum {
	PLATFORM_HW = 0,
	PLATFORM_EMULATOR = 1,
	PLATFORM_ASIM = 3,
} platform_t;

platform_t read_platform(void);
u8 read_partnum(void);
u8 read_partvar(void);
const char *read_board_name(void);

#endif /* __SOC_H */
