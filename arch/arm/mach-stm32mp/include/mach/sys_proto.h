/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright (C) 2015-2017, STMicroelectronics - All Rights Reserved
 */

/* ID = Device Version (bit31:16) + Device Part Number (RPN) (bit7:0) */
#define CPU_STM32MP157Cxx	0x05000000
#define CPU_STM32MP157Axx	0x05000001
#define CPU_STM32MP153Cxx	0x05000024
#define CPU_STM32MP153Axx	0x05000025
#define CPU_STM32MP151Cxx	0x0500002E
#define CPU_STM32MP151Axx	0x0500002F
#define CPU_STM32MP157Fxx	0x05000080
#define CPU_STM32MP157Dxx	0x05000081
#define CPU_STM32MP153Fxx	0x050000A4
#define CPU_STM32MP153Dxx	0x050000A5
#define CPU_STM32MP151Fxx	0x050000AE
#define CPU_STM32MP151Dxx	0x050000AF

/* return CPU_STMP32MP...Xxx constants */
u32 get_cpu_type(void);

#define CPU_DEV_STM32MP15	0x500

/* return CPU_DEV constants */
u32 get_cpu_dev(void);

#define CPU_REVA	0x1000
#define CPU_REVB	0x2000
#define CPU_REVZ	0x2001

/* return CPU_REV constants */
u32 get_cpu_rev(void);

/* Get Package options from OTP */
u32 get_cpu_package(void);

#define PKG_AA_LBGA448	4
#define PKG_AB_LBGA354	3
#define PKG_AC_TFBGA361	2
#define PKG_AD_TFBGA257	1

/* Get SOC name */
#define SOC_NAME_SIZE 20
void get_soc_name(char name[SOC_NAME_SIZE]);

/* return boot mode */
u32 get_bootmode(void);

int setup_mac_address(void);

/* board power management : configure vddcore according OPP */
void board_vddcore_init(u32 voltage_mv);
