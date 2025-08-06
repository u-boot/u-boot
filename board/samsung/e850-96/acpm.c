// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2025 Linaro Ltd.
 * Author: Sam Protsenko <semen.protsenko@linaro.org>
 *
 * ACPM (Active Clock and Power Management) is an IPC protocol for communicating
 * with APM (Active Power Management) core. The message exchange between AP
 * (Application Processor) and APM is happening by using shared memory in SRAM
 * (iRAM) and generating interrupts using Mailbox block. By using this IPC
 * interface it's possible to offload power management tasks to APM core, which
 * acts as a supervisor for CPU. One of the main tasks of APM is controlling
 * PMIC chip over I3C bus. So in order to access PMIC chip registers it's
 * recommended to do so by sending corresponding commands to APM via ACPM IPC
 * protocol. The IPC interaction sequence looks like this:
 *
 *     AP (CPU) <-> ACPM IPC (Mailbox + SRAM) <-> APM <-> I3C <-> PMIC
 *
 * This file contains functions for accessing I3C bus via APM block using
 * ACPM IPC.
 */

#include <linux/iopoll.h>
#include <linux/time.h>
#include <asm/io.h>
#include "acpm.h"

/* Mailbox registers */
#define MBOX_INTGR0			0x8	/* Interrupt Generation */
#define MBOX_INTCR1			0x20	/* Interrupt Clear */
#define MBOX_INTSR1			0x28	/* Interrupt Status */
#define MBOX_INTGR_OFFSET		16
#define MBOX_TIMEOUT			(1 * USEC_PER_SEC)

/* APM shared memory registers */
#define SHMEM_SR0			0x0
#define SHMEM_SR1			0x4
#define SHMEM_SR2			0x8
#define SHMEM_SR3			0xc

/* IPC functions */
#define IPC_FUNC_READ			0x0
#define IPC_FUNC_WRITE			0x1
/* Command 0 shifts and masks */
#define IPC_REG_SHIFT			0
#define IPC_REG_MASK			0xff
#define IPC_TYPE_SHIFT			8
#define IPC_TYPE_MASK			0xf
#define IPC_CHANNEL_SHIFT		12
#define IPC_CHANNEL_MASK		0xf
/* Command 1 shifts and masks */
#define IPC_FUNC_SHIFT			0
#define IPC_FUNC_MASK			0xff
#define IPC_WRITE_VAL_SHIFT		8
#define IPC_WRITE_VAL_MASK		0xff
/* Command 3 shifts and masks */
#define IPC_DEST_SHIFT			8
#define IPC_DEST_MASK			0xff
#define IPC_RETURN_SHIFT		24
#define IPC_RETURN_MASK			0xff

/**
 * acpm_ipc_send_data_async() - Send data to I3C block over ACPM IPC
 * @acpm: ACPM data
 * @cmd0: Command 0 value to send
 * @cmd1: Command 1 value to send
 */
static void acpm_ipc_send_data_async(struct acpm *acpm, u32 cmd0, u32 cmd1)
{
	u32 irq_bit = 1 << acpm->ipc_ch;
	u32 intgr = irq_bit << MBOX_INTGR_OFFSET;

	/* Write data to the shared memory */
	writel(cmd0, acpm->sram_base + SHMEM_SR0);
	writel(cmd1, acpm->sram_base + SHMEM_SR1);
	dsb();

	/* Generate interrupt for I3C block */
	writel(intgr, acpm->mbox_base + MBOX_INTGR0);
}

/**
 * acpm_ipc_wait_resp() - Read response data from I3C block over ACPM IPC
 * @acpm: ACPM data
 * @cmd2: Will contain read value for command 2
 * @cmd3: Will contain read value for command 3
 *
 * Return: 0 on success or negative value on error.
 */
static int acpm_ipc_wait_resp(struct acpm *acpm, u32 *cmd2, u32 *cmd3)
{
	u32 irq_bit = 1 << acpm->ipc_ch;
	u32 reg;
	int ret;

	/* Wait for the interrupt from I3C block */
	ret = readl_poll_timeout(acpm->mbox_base + MBOX_INTSR1, reg,
				 reg & irq_bit, MBOX_TIMEOUT);
	if (ret < 0)
		return ret;

	/* Clear the interrupt */
	writel(irq_bit, acpm->mbox_base + MBOX_INTCR1);

	/* Read data from the shared memory */
	*cmd2 = readl(acpm->sram_base + SHMEM_SR2);
	*cmd3 = readl(acpm->sram_base + SHMEM_SR3);

	return 0;
}

/**
 * acpm_i3c_read() - Read an I3C register of some I3C slave device
 * @acpm: ACPM data
 * @ch: I3C channel (bus) number (0-15)
 * @addr: I3C address of slave device (0-15)
 * @reg: Address of I3C register in the slave device to read from
 * @val: Will contain the read value
 *
 * Return: 0 on success or non-zero code on error (may be positive).
 */
int acpm_i3c_read(struct acpm *acpm, u8 ch, u8 addr, u8 reg, u8 *val)
{
	u32 cmd[4] = { 0 };
	u8 ret;

	cmd[0] = (ch & IPC_CHANNEL_MASK) << IPC_CHANNEL_SHIFT |
		 (addr & IPC_TYPE_MASK) << IPC_TYPE_SHIFT |
		 (reg & IPC_REG_MASK) << IPC_REG_SHIFT;
	cmd[1] = IPC_FUNC_READ << IPC_FUNC_SHIFT;

	acpm_ipc_send_data_async(acpm, cmd[0], cmd[1]);
	ret = acpm_ipc_wait_resp(acpm, &cmd[2], &cmd[3]);
	if (ret)
		return ret;

	*val = (cmd[3] >> IPC_DEST_SHIFT) & IPC_DEST_MASK;
	ret = (cmd[3] >> IPC_RETURN_SHIFT) & IPC_RETURN_MASK;
	return ret;
}

/**
 * acpm_i3c_write() - Write an I3C register of some I3C slave device
 * @acpm: ACPM data
 * @ch: I3C channel (bus) number (0-15)
 * @addr: I3C address of slave device (0-15)
 * @reg: Address of I3C register in the slave device to write into
 * @val: Value to write
 *
 * Return: 0 on success or non-zero code on error (may be positive).
 */
int acpm_i3c_write(struct acpm *acpm, u8 ch, u8 addr, u8 reg, u8 val)
{
	u32 cmd[4] = { 0 };
	u8 ret;

	cmd[0] = (ch & IPC_CHANNEL_MASK) << IPC_CHANNEL_SHIFT |
		 (addr & IPC_TYPE_MASK) << IPC_TYPE_SHIFT |
		 (reg & IPC_REG_MASK) << IPC_REG_SHIFT;
	cmd[1] = IPC_FUNC_WRITE << IPC_FUNC_SHIFT |
		 (val & IPC_WRITE_VAL_MASK) << IPC_WRITE_VAL_SHIFT;

	acpm_ipc_send_data_async(acpm, cmd[0], cmd[1]);
	ret = acpm_ipc_wait_resp(acpm, &cmd[2], &cmd[3]);
	if (ret)
		return ret;

	ret = (cmd[3] >> IPC_RETURN_SHIFT) & IPC_RETURN_MASK;
	return ret;
}
