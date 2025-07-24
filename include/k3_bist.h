/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Texas Instruments' BIST (Built-In Self-Test) driver
 *
 * Copyright (C) 2025 Texas Instruments Incorporated - https://www.ti.com/
 *      Neha Malcom Francis <n-francis@ti.com>
 *
 */

#ifndef _INCLUDE_BIST_H_
#define _INCLUDE_BIST_H_

#define PROC_BOOT_CTRL_FLAG_R5_CORE_HALT	0x00000001
#define PROC_ID_MCU_R5FSS2_CORE0		0x0A
#define PROC_ID_MCU_R5FSS2_CORE1		0x0B
#define PROC_BOOT_CTRL_FLAG_R5_LPSC		0x00000002

#define TISCI_DEV_PBIST14 237
#define TISCI_DEV_R5FSS2_CORE0 343
#define TISCI_DEV_R5FSS2_CORE1 344

#define TISCI_MSG_VALUE_DEVICE_SW_STATE_AUTO_OFF 0
#define TISCI_MSG_VALUE_DEVICE_SW_STATE_RETENTION 1
#define TISCI_MSG_VALUE_DEVICE_SW_STATE_ON 2

#define TISCI_BIT(n)  ((1) << (n))

struct bist_ops {
	int (*run_lbist)(void);
	int (*run_lbist_post)(void);
	int (*run_pbist_post)(void);
	int (*run_pbist_neg)(void);
	int (*run_pbist_rom)(void);
	int (*run_pbist)(void);
};

void lbist_enable_isolation(void);
void lbist_disable_isolation(void);
int prepare_pbist(struct ti_sci_handle *handle);
int deprepare_pbist(struct ti_sci_handle *handle);
int prepare_lbist(struct ti_sci_handle *handle);
int deprepare_lbist(struct ti_sci_handle *handle);

#endif /* _INCLUDE_BIST_H_ */
