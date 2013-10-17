/*
 * Copyright (C) 2009
 * Guennadi Liakhovetski, DENX Software Engineering, <lg@denx.de>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef _S6E63D6_H_
#define _S6E63D6_H_

struct s6e63d6 {
	unsigned int bus;
	unsigned int cs;
	unsigned int id;
	struct spi_slave *slave;
};

extern int s6e63d6_init(struct s6e63d6 *data);
extern int s6e63d6_index(struct s6e63d6 *data, u8 idx);
extern int s6e63d6_param(struct s6e63d6 *data, u16 param);

#endif
