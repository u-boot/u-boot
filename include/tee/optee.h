/*
 * OP-TEE related definitions
 *
 * (C) Copyright 2016 Linaro Limited
 * Andrew F. Davis <andrew.davis@linaro.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#ifndef	_OPTEE_H
#define _OPTEE_H

#define OPTEE_MAGIC             0x4554504f
#define OPTEE_VERSION           1
#define OPTEE_ARCH_ARM32        0
#define OPTEE_ARCH_ARM64        1

struct optee_header {
	uint32_t magic;
	uint8_t version;
	uint8_t arch;
	uint16_t flags;
	uint32_t init_size;
	uint32_t init_load_addr_hi;
	uint32_t init_load_addr_lo;
	uint32_t init_mem_usage;
	uint32_t paged_size;
};

#endif /* _OPTEE_H */
