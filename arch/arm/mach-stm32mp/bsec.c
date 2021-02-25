// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#define LOG_CATEGORY UCLASS_MISC

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <log.h>
#include <misc.h>
#include <asm/io.h>
#include <asm/arch/bsec.h>
#include <asm/arch/stm32mp1_smc.h>
#include <dm/device_compat.h>
#include <linux/arm-smccc.h>
#include <linux/iopoll.h>

#define BSEC_OTP_MAX_VALUE		95
#define BSEC_TIMEOUT_US			10000

/* BSEC REGISTER OFFSET (base relative) */
#define BSEC_OTP_CONF_OFF		0x000
#define BSEC_OTP_CTRL_OFF		0x004
#define BSEC_OTP_WRDATA_OFF		0x008
#define BSEC_OTP_STATUS_OFF		0x00C
#define BSEC_OTP_LOCK_OFF		0x010
#define BSEC_DENABLE_OFF		0x014
#define BSEC_DISTURBED_OFF		0x01C
#define BSEC_ERROR_OFF			0x034
#define BSEC_WRLOCK_OFF			0x04C /* OTP write permananet lock */
#define BSEC_SPLOCK_OFF			0x064 /* OTP write sticky lock */
#define BSEC_SWLOCK_OFF			0x07C /* shadow write sticky lock */
#define BSEC_SRLOCK_OFF			0x094 /* shadow read sticky lock */
#define BSEC_OTP_DATA_OFF		0x200

/* BSEC_CONFIGURATION Register MASK */
#define BSEC_CONF_POWER_UP		0x001

/* BSEC_CONTROL Register */
#define BSEC_READ			0x000
#define BSEC_WRITE			0x100

/* LOCK Register */
#define OTP_LOCK_MASK			0x1F
#define OTP_LOCK_BANK_SHIFT		0x05
#define OTP_LOCK_BIT_MASK		0x01

/* STATUS Register */
#define BSEC_MODE_BUSY_MASK		0x08
#define BSEC_MODE_PROGFAIL_MASK		0x10
#define BSEC_MODE_PWR_MASK		0x20

/* DENABLE Register */
#define BSEC_DENABLE_DBGSWENABLE	BIT(10)

/*
 * OTP Lock services definition
 * Value must corresponding to the bit number in the register
 */
#define BSEC_LOCK_PROGRAM		0x04

/**
 * bsec_lock() - manage lock for each type SR/SP/SW
 * @address: address of bsec IP register
 * @otp: otp number (0 - BSEC_OTP_MAX_VALUE)
 * Return: true if locked else false
 */
static bool bsec_read_lock(u32 address, u32 otp)
{
	u32 bit;
	u32 bank;

	bit = 1 << (otp & OTP_LOCK_MASK);
	bank = ((otp >> OTP_LOCK_BANK_SHIFT) & OTP_LOCK_MASK) * sizeof(u32);

	return !!(readl(address + bank) & bit);
}

/**
 * bsec_check_error() - Check status of one otp
 * @base: base address of bsec IP
 * @otp: otp number (0 - BSEC_OTP_MAX_VALUE)
 * Return: 0 if no error, -EAGAIN or -ENOTSUPP
 */
static u32 bsec_check_error(u32 base, u32 otp)
{
	u32 bit;
	u32 bank;

	bit = 1 << (otp & OTP_LOCK_MASK);
	bank = ((otp >> OTP_LOCK_BANK_SHIFT) & OTP_LOCK_MASK) * sizeof(u32);

	if (readl(base + BSEC_DISTURBED_OFF + bank) & bit)
		return -EAGAIN;
	else if (readl(base + BSEC_ERROR_OFF + bank) & bit)
		return -ENOTSUPP;

	return 0;
}

/**
 * bsec_read_SR_lock() - read SR lock (Shadowing)
 * @base: base address of bsec IP
 * @otp: otp number (0 - BSEC_OTP_MAX_VALUE)
 * Return: true if locked else false
 */
static bool bsec_read_SR_lock(u32 base, u32 otp)
{
	return bsec_read_lock(base + BSEC_SRLOCK_OFF, otp);
}

/**
 * bsec_read_SP_lock() - read SP lock (program Lock)
 * @base: base address of bsec IP
 * @otp: otp number (0 - BSEC_OTP_MAX_VALUE)
 * Return: true if locked else false
 */
static bool bsec_read_SP_lock(u32 base, u32 otp)
{
	return bsec_read_lock(base + BSEC_SPLOCK_OFF, otp);
}

/**
 * bsec_SW_lock() - manage SW lock (Write in Shadow)
 * @base: base address of bsec IP
 * @otp: otp number (0 - BSEC_OTP_MAX_VALUE)
 * Return: true if locked else false
 */
static bool bsec_read_SW_lock(u32 base, u32 otp)
{
	return bsec_read_lock(base + BSEC_SWLOCK_OFF, otp);
}

/**
 * bsec_power_safmem() - Activate or deactivate safmem power
 * @base: base address of bsec IP
 * @power: true to power up , false to power down
 * Return: 0 if succeed
 */
static int bsec_power_safmem(u32 base, bool power)
{
	u32 val;
	u32 mask;

	if (power) {
		setbits_le32(base + BSEC_OTP_CONF_OFF, BSEC_CONF_POWER_UP);
		mask = BSEC_MODE_PWR_MASK;
	} else {
		clrbits_le32(base + BSEC_OTP_CONF_OFF, BSEC_CONF_POWER_UP);
		mask = 0;
	}

	/* waiting loop */
	return readl_poll_timeout(base + BSEC_OTP_STATUS_OFF,
				  val, (val & BSEC_MODE_PWR_MASK) == mask,
				  BSEC_TIMEOUT_US);
}

/**
 * bsec_shadow_register() - copy safmen otp to bsec data
 * @base: base address of bsec IP
 * @otp: otp number (0 - BSEC_OTP_MAX_VALUE)
 * Return: 0 if no error
 */
static int bsec_shadow_register(struct udevice *dev, u32 base, u32 otp)
{
	u32 val;
	int ret;
	bool power_up = false;

	/* check if shadowing of otp is locked */
	if (bsec_read_SR_lock(base, otp))
		dev_dbg(dev, "OTP %d is locked and refreshed with 0\n",
			otp);

	/* check if safemem is power up */
	val = readl(base + BSEC_OTP_STATUS_OFF);
	if (!(val & BSEC_MODE_PWR_MASK)) {
		ret = bsec_power_safmem(base, true);
		if (ret)
			return ret;
		power_up = true;
	}
	/* set BSEC_OTP_CTRL_OFF with the otp value*/
	writel(otp | BSEC_READ, base + BSEC_OTP_CTRL_OFF);

	/* check otp status*/
	ret = readl_poll_timeout(base + BSEC_OTP_STATUS_OFF,
				 val, (val & BSEC_MODE_BUSY_MASK) == 0,
				 BSEC_TIMEOUT_US);
	if (ret)
		return ret;

	ret = bsec_check_error(base, otp);

	if (power_up)
		bsec_power_safmem(base, false);

	return ret;
}

/**
 * bsec_read_shadow() - read an otp data value from shadow
 * @base: base address of bsec IP
 * @val: read value
 * @otp: otp number (0 - BSEC_OTP_MAX_VALUE)
 * Return: 0 if no error
 */
static int bsec_read_shadow(struct udevice *dev, u32 base, u32 *val, u32 otp)
{
	*val = readl(base + BSEC_OTP_DATA_OFF + otp * sizeof(u32));

	return bsec_check_error(base, otp);
}

/**
 * bsec_write_shadow() - write value in BSEC data register in shadow
 * @base: base address of bsec IP
 * @val: value to write
 * @otp: otp number (0 - BSEC_OTP_MAX_VALUE)
 * Return: 0 if no error
 */
static int bsec_write_shadow(struct udevice *dev, u32 base, u32 val, u32 otp)
{
	/* check if programming of otp is locked */
	if (bsec_read_SW_lock(base, otp))
		dev_dbg(dev, "OTP %d is lock, write will be ignore\n", otp);

	writel(val, base + BSEC_OTP_DATA_OFF + otp * sizeof(u32));

	return bsec_check_error(base, otp);
}

/**
 * bsec_program_otp() - program a bit in SAFMEM
 * @base: base address of bsec IP
 * @val: value to program
 * @otp: otp number (0 - BSEC_OTP_MAX_VALUE)
 * after the function the otp data is not refreshed in shadow
 * Return: 0 if no error
 */
static int bsec_program_otp(struct udevice *dev, long base, u32 val, u32 otp)
{
	u32 ret;
	bool power_up = false;

	if (bsec_read_SP_lock(base, otp))
		dev_dbg(dev, "OTP %d locked, prog will be ignore\n", otp);

	if (readl(base + BSEC_OTP_LOCK_OFF) & (1 << BSEC_LOCK_PROGRAM))
		dev_dbg(dev, "Global lock, prog will be ignore\n");

	/* check if safemem is power up */
	if (!(readl(base + BSEC_OTP_STATUS_OFF) & BSEC_MODE_PWR_MASK)) {
		ret = bsec_power_safmem(base, true);
		if (ret)
			return ret;

		power_up = true;
	}
	/* set value in write register*/
	writel(val, base + BSEC_OTP_WRDATA_OFF);

	/* set BSEC_OTP_CTRL_OFF with the otp value */
	writel(otp | BSEC_WRITE, base + BSEC_OTP_CTRL_OFF);

	/* check otp status*/
	ret = readl_poll_timeout(base + BSEC_OTP_STATUS_OFF,
				 val, (val & BSEC_MODE_BUSY_MASK) == 0,
				 BSEC_TIMEOUT_US);
	if (ret)
		return ret;

	if (val & BSEC_MODE_PROGFAIL_MASK)
		ret = -EACCES;
	else
		ret = bsec_check_error(base, otp);

	if (power_up)
		bsec_power_safmem(base, false);

	return ret;
}

/* BSEC MISC driver *******************************************************/
struct stm32mp_bsec_plat {
	u32 base;
};

static int stm32mp_bsec_read_otp(struct udevice *dev, u32 *val, u32 otp)
{
	struct stm32mp_bsec_plat *plat;
	u32 tmp_data = 0;
	int ret;

	if (IS_ENABLED(CONFIG_TFABOOT))
		return stm32_smc(STM32_SMC_BSEC,
				 STM32_SMC_READ_OTP,
				 otp, 0, val);

	plat = dev_get_plat(dev);

	/* read current shadow value */
	ret = bsec_read_shadow(dev, plat->base, &tmp_data, otp);
	if (ret)
		return ret;

	/* copy otp in shadow */
	ret = bsec_shadow_register(dev, plat->base, otp);
	if (ret)
		return ret;

	ret = bsec_read_shadow(dev, plat->base, val, otp);
	if (ret)
		return ret;

	/* restore shadow value */
	ret = bsec_write_shadow(dev, plat->base, tmp_data, otp);

	return ret;
}

static int stm32mp_bsec_read_shadow(struct udevice *dev, u32 *val, u32 otp)
{
	struct stm32mp_bsec_plat *plat;

	if (IS_ENABLED(CONFIG_TFABOOT))
		return stm32_smc(STM32_SMC_BSEC,
				 STM32_SMC_READ_SHADOW,
				 otp, 0, val);

	plat = dev_get_plat(dev);

	return bsec_read_shadow(dev, plat->base, val, otp);
}

static int stm32mp_bsec_read_lock(struct udevice *dev, u32 *val, u32 otp)
{
	struct stm32mp_bsec_plat *plat = dev_get_plat(dev);

	/* return OTP permanent write lock status */
	*val = bsec_read_lock(plat->base + BSEC_WRLOCK_OFF, otp);

	return 0;
}

static int stm32mp_bsec_write_otp(struct udevice *dev, u32 val, u32 otp)
{
	struct stm32mp_bsec_plat *plat;

	if (IS_ENABLED(CONFIG_TFABOOT))
		return stm32_smc_exec(STM32_SMC_BSEC,
				      STM32_SMC_PROG_OTP,
				      otp, val);

	plat = dev_get_plat(dev);

	return bsec_program_otp(dev, plat->base, val, otp);

}

static int stm32mp_bsec_write_shadow(struct udevice *dev, u32 val, u32 otp)
{
	struct stm32mp_bsec_plat *plat;

	if (IS_ENABLED(CONFIG_TFABOOT))
		return stm32_smc_exec(STM32_SMC_BSEC,
				      STM32_SMC_WRITE_SHADOW,
				      otp, val);

	plat = dev_get_plat(dev);

	return bsec_write_shadow(dev, plat->base, val, otp);
}

static int stm32mp_bsec_write_lock(struct udevice *dev, u32 val, u32 otp)
{
	if (!IS_ENABLED(CONFIG_TFABOOT))
		return -ENOTSUPP;

	if (val == 1)
		return stm32_smc_exec(STM32_SMC_BSEC,
				      STM32_SMC_WRLOCK_OTP,
				      otp, 0);
	if (val == 0)
		return 0; /* nothing to do */

	return -EINVAL;
}

static int stm32mp_bsec_read(struct udevice *dev, int offset,
			     void *buf, int size)
{
	int ret;
	int i;
	bool shadow = true, lock = false;
	int nb_otp = size / sizeof(u32);
	int otp;
	unsigned int offs = offset;

	if (offs >= STM32_BSEC_LOCK_OFFSET) {
		offs -= STM32_BSEC_LOCK_OFFSET;
		lock = true;
	} else if (offs >= STM32_BSEC_OTP_OFFSET) {
		offs -= STM32_BSEC_OTP_OFFSET;
		shadow = false;
	}

	if ((offs % 4) || (size % 4))
		return -EINVAL;

	otp = offs / sizeof(u32);

	for (i = otp; i < (otp + nb_otp) && i <= BSEC_OTP_MAX_VALUE; i++) {
		u32 *addr = &((u32 *)buf)[i - otp];

		if (lock)
			ret = stm32mp_bsec_read_lock(dev, addr, i);
		else if (shadow)
			ret = stm32mp_bsec_read_shadow(dev, addr, i);
		else
			ret = stm32mp_bsec_read_otp(dev, addr, i);

		if (ret)
			break;
	}
	if (ret)
		return ret;
	else
		return (i - otp) * 4;
}

static int stm32mp_bsec_write(struct udevice *dev, int offset,
			      const void *buf, int size)
{
	int ret = 0;
	int i;
	bool shadow = true, lock = false;
	int nb_otp = size / sizeof(u32);
	int otp;
	unsigned int offs = offset;

	if (offs >= STM32_BSEC_LOCK_OFFSET) {
		offs -= STM32_BSEC_LOCK_OFFSET;
		lock = true;
	} else if (offs >= STM32_BSEC_OTP_OFFSET) {
		offs -= STM32_BSEC_OTP_OFFSET;
		shadow = false;
	}

	if ((offs % 4) || (size % 4))
		return -EINVAL;

	otp = offs / sizeof(u32);

	for (i = otp; i < otp + nb_otp && i <= BSEC_OTP_MAX_VALUE; i++) {
		u32 *val = &((u32 *)buf)[i - otp];

		if (lock)
			ret = stm32mp_bsec_write_lock(dev, *val, i);
		else if (shadow)
			ret = stm32mp_bsec_write_shadow(dev, *val, i);
		else
			ret = stm32mp_bsec_write_otp(dev, *val, i);
		if (ret)
			break;
	}
	if (ret)
		return ret;
	else
		return (i - otp) * 4;
}

static const struct misc_ops stm32mp_bsec_ops = {
	.read = stm32mp_bsec_read,
	.write = stm32mp_bsec_write,
};

static int stm32mp_bsec_of_to_plat(struct udevice *dev)
{
	struct stm32mp_bsec_plat *plat = dev_get_plat(dev);

	plat->base = (u32)dev_read_addr_ptr(dev);

	return 0;
}

static int stm32mp_bsec_probe(struct udevice *dev)
{
	int otp;
	struct stm32mp_bsec_plat *plat;
	struct clk_bulk clk_bulk;
	int ret;

	ret = clk_get_bulk(dev, &clk_bulk);
	if (!ret) {
		ret = clk_enable_bulk(&clk_bulk);
		if (ret)
			return ret;
	}

	/*
	 * update unlocked shadow for OTP cleared by the rom code
	 * only executed in U-Boot proper when TF-A is not used
	 */

	if (!IS_ENABLED(CONFIG_TFABOOT) && !IS_ENABLED(CONFIG_SPL_BUILD)) {
		plat = dev_get_plat(dev);

		for (otp = 57; otp <= BSEC_OTP_MAX_VALUE; otp++)
			if (!bsec_read_SR_lock(plat->base, otp))
				bsec_shadow_register(dev, plat->base, otp);
	}

	return 0;
}

static const struct udevice_id stm32mp_bsec_ids[] = {
	{ .compatible = "st,stm32mp15-bsec" },
	{}
};

U_BOOT_DRIVER(stm32mp_bsec) = {
	.name = "stm32mp_bsec",
	.id = UCLASS_MISC,
	.of_match = stm32mp_bsec_ids,
	.of_to_plat = stm32mp_bsec_of_to_plat,
	.plat_auto	= sizeof(struct stm32mp_bsec_plat),
	.ops = &stm32mp_bsec_ops,
	.probe = stm32mp_bsec_probe,
};

bool bsec_dbgswenable(void)
{
	struct udevice *dev;
	struct stm32mp_bsec_plat *plat;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(stm32mp_bsec), &dev);
	if (ret || !dev) {
		log_debug("bsec driver not available\n");
		return false;
	}

	plat = dev_get_plat(dev);
	if (readl(plat->base + BSEC_DENABLE_OFF) & BSEC_DENABLE_DBGSWENABLE)
		return true;

	return false;
}
