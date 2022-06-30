/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright (C) 2015-2017, STMicroelectronics - All Rights Reserved
 */

/* ID = Device Version (bit31:16) + Device Part Number (RPN) (bit15:0) */
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

#define CPU_STM32MP135Cxx	0x05010000
#define CPU_STM32MP135Axx	0x05010001
#define CPU_STM32MP133Cxx	0x050100C0
#define CPU_STM32MP133Axx	0x050100C1
#define CPU_STM32MP131Cxx	0x050106C8
#define CPU_STM32MP131Axx	0x050106C9
#define CPU_STM32MP135Fxx	0x05010800
#define CPU_STM32MP135Dxx	0x05010801
#define CPU_STM32MP133Fxx	0x050108C0
#define CPU_STM32MP133Dxx	0x050108C1
#define CPU_STM32MP131Fxx	0x05010EC8
#define CPU_STM32MP131Dxx	0x05010EC9

/* return CPU_STMP32MP...Xxx constants */
u32 get_cpu_type(void);

#define CPU_DEV_STM32MP15	0x500
#define CPU_DEV_STM32MP13	0x501

/* return CPU_DEV constants */
u32 get_cpu_dev(void);

#define CPU_REV1	0x1000
#define CPU_REV1_1	0x1001
#define CPU_REV1_2	0x1003
#define CPU_REV2	0x2000
#define CPU_REV2_1	0x2001

/* return Silicon revision = REV_ID[15:0] of Device Version */
u32 get_cpu_rev(void);

/* Get Package options from OTP */
u32 get_cpu_package(void);

/* package used for STM32MP15x */
#define STM32MP15_PKG_AA_LBGA448	4
#define STM32MP15_PKG_AB_LBGA354	3
#define STM32MP15_PKG_AC_TFBGA361	2
#define STM32MP15_PKG_AD_TFBGA257	1
#define STM32MP15_PKG_UNKNOWN		0

/* Get SOC name */
#define SOC_NAME_SIZE 20
void get_soc_name(char name[SOC_NAME_SIZE]);

/* return boot mode */
u32 get_bootmode(void);

int get_eth_nb(void);
int setup_mac_address(void);

/* board power management : configure vddcore according OPP */
void board_vddcore_init(u32 voltage_mv);

/* weak function */
void stm32mp_cpu_init(void);
void stm32mp_misc_init(void);

/* helper function: read data from OTP */
u32 get_otp(int index, int shift, int mask);
