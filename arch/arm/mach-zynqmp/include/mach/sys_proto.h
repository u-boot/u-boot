/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2014 - 2015 Xilinx, Inc.
 * Michal Simek <michal.simek@xilinx.com>
 */

#ifndef _ASM_ARCH_SYS_PROTO_H
#define _ASM_ARCH_SYS_PROTO_H

#define PAYLOAD_ARG_CNT		5

#define ZYNQMP_CSU_SILICON_VER_MASK	0xF
#define KEY_PTR_LEN	32

#define ZYNQMP_FPGA_BIT_AUTH_DDR	1
#define ZYNQMP_FPGA_BIT_AUTH_OCM	2
#define ZYNQMP_FPGA_BIT_ENC_USR_KEY	3
#define ZYNQMP_FPGA_BIT_ENC_DEV_KEY	4
#define ZYNQMP_FPGA_BIT_NS		5

#define ZYNQMP_FPGA_AUTH_DDR	1

enum {
	IDCODE,
	VERSION,
	IDCODE2,
};

enum {
	ZYNQMP_SILICON_V1,
	ZYNQMP_SILICON_V2,
	ZYNQMP_SILICON_V3,
	ZYNQMP_SILICON_V4,
};

enum {
	TCM_LOCK,
	TCM_SPLIT,
};

struct zynqmp_ipi_msg {
	size_t len;
	u32 *buf;
};

int zynq_board_read_rom_ethaddr(unsigned char *ethaddr);
unsigned int zynqmp_get_silicon_version(void);

void handoff_setup(void);

int zynqmp_mmio_write(const u32 address, const u32 mask, const u32 value);
int zynqmp_mmio_read(const u32 address, u32 *value);
int invoke_smc(u32 pm_api_id, u32 arg0, u32 arg1, u32 arg2, u32 arg3,
	       u32 *ret_payload);

void initialize_tcm(bool mode);
void mem_map_fill(void);
int chip_id(unsigned char id);
#if defined(CONFIG_SYS_MEM_RSVD_FOR_MMU) || defined(CONFIG_DEFINE_TCM_OCM_MMAP)
void tcm_init(u8 mode);
#endif

#endif /* _ASM_ARCH_SYS_PROTO_H */
