/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2003
 * Steven Scholz, imc Measurement & Control, steven.scholz@imc-berlin.de
 *
 * (C) Copyright 2002
 * Rich Ireland, Enterasys Networks, rireland@enterasys.com.
 */

#ifndef _ACEX1K_H_
#define _ACEX1K_H_

#include <altera.h>

extern int CYC2_load(Altera_desc *desc, const void *image, size_t size);
extern int CYC2_dump(Altera_desc *desc, const void *buf, size_t bsize);
extern int CYC2_info(Altera_desc *desc);

/* Slave Serial Implementation function table */
typedef struct {
	Altera_pre_fn		pre;
	Altera_config_fn	config;
	Altera_status_fn	status;
	Altera_done_fn		done;
	Altera_write_fn		write;
	Altera_abort_fn		abort;
	Altera_post_fn		post;
} Altera_CYC2_Passive_Serial_fns;

/* Device Image Sizes
 *********************************************************************/
#define Altera_EP2C8_SIZE	247942
#define Altera_EP2C20_SIZE	586562
#define Altera_EP2C35_SIZE	883905
#define Altera_EP3C5_SIZE	368011		/* .rbf size in bytes */

#define ALTERA_EP4CE6_SIZE	368011		/* 2944088 Bits */
#define ALTERA_EP4CE10_SIZE	368011		/* 2944088 Bits */
#define ALTERA_EP4CE15_SIZE	510856		/* 4086848 Bits */
#define ALTERA_EP4CE22_SIZE	718569		/* 5748552 Bits */
#define ALTERA_EP4CE30_SIZE	1191788		/* 9534304 Bits */
#define ALTERA_EP4CE40_SIZE	1191788		/* 9534304 Bits */
#define ALTERA_EP4CE55_SIZE	1861195		/* 14889560 Bits */
#define ALTERA_EP4CE75_SIZE	2495719		/* 19965752 Bits */
#define ALTERA_EP4CE115_SIZE	3571462		/* 28571696 Bits */

#endif /* _ACEX1K_H_ */
