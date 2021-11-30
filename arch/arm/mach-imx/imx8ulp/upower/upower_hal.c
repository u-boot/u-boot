// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2021 NXP
 */

#include <log.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <linux/delay.h>

#include "upower_api.h"

#define UPOWER_AP_MU1_ADDR	0x29280000
static struct mu_type *muptr = (struct mu_type *)UPOWER_AP_MU1_ADDR;

void upower_wait_resp(void)
{
	while (!(readl(&muptr->rsr) & BIT(0))) {
		debug("%s: poll the mu:%x\n", __func__, readl(&muptr->rsr));
		udelay(100);
	}

	upwr_txrx_isr();
}

u32 upower_status(int status)
{
	u32 ret = -1;

	switch (status) {
	case 0:
		debug("%s: finished successfully!\n", __func__);
		ret = 0;
		break;
	case -1:
		printf("%s: memory allocation or resource failed!\n", __func__);
		break;
	case -2:
		printf("%s: invalid argument!\n", __func__);
		break;
	case -3:
		printf("%s: called in an invalid API state!\n", __func__);
		break;
	default:
		printf("%s: invalid return status\n", __func__);
		break;
	}
	return ret;
}

void user_upwr_rdy_callb(u32 soc, u32 vmajor, u32 vminor)
{
	printf("%s: soc=%x\n", __func__, soc);
	printf("%s: RAM version:%d.%d\n", __func__, vmajor, vminor);
}

int upower_pmic_i2c_write(u32 reg_addr, u32 reg_val)
{
	int ret, ret_val;
	enum upwr_resp err_code;

	ret = upwr_xcp_i2c_access(0x32, 1, 1, reg_addr, reg_val, NULL);
	if (ret) {
		printf("pmic i2c write failed ret %d\n", ret);
		return ret;
	}

	upower_wait_resp();
	ret = upwr_poll_req_status(UPWR_SG_EXCEPT, NULL, &err_code, &ret_val, 1000);
	if (ret != UPWR_REQ_OK) {
		printf("i2c poll Failure %d, err_code %d, ret_val 0x%x\n", ret, err_code, ret_val);
		return ret;
	}

	debug("PMIC write reg[0x%x], val[0x%x]\n", reg_addr, reg_val);

	return 0;
}

int upower_pmic_i2c_read(u32 reg_addr, u32 *reg_val)
{
	int ret, ret_val;
	enum upwr_resp err_code;

	if (!reg_val)
		return -1;

	ret = upwr_xcp_i2c_access(0x32, -1, 1, reg_addr, 0, NULL);
	if (ret) {
		printf("pmic i2c read failed ret %d\n", ret);
		return ret;
	}

	upower_wait_resp();
	ret = upwr_poll_req_status(UPWR_SG_EXCEPT, NULL, &err_code, &ret_val, 1000);
	if (ret != UPWR_REQ_OK) {
		printf("i2c poll Failure %d, err_code %d, ret_val 0x%x\n", ret, err_code, ret_val);
		return ret;
	}

	*reg_val = ret_val;

	debug("PMIC read reg[0x%x], val[0x%x]\n", reg_addr, *reg_val);

	return 0;
}

int upower_init(void)
{
	u32 fw_major, fw_minor, fw_vfixes;
	u32 soc_id;
	int status;

	u32 swton;
	u64 memon;
	int ret, ret_val;

	do {
		status = upwr_init(1, muptr);
		if (upower_status(status)) {
			printf("%s: upower init failure\n", __func__);
			break;
		}

		soc_id = upwr_rom_version(&fw_major, &fw_minor, &fw_vfixes);
		if (!soc_id) {
			printf("%s:, soc_id not initialized\n", __func__);
			break;
		}

		printf("%s: soc_id=%d\n", __func__, soc_id);
		printf("%s: version:%d.%d.%d\n", __func__, fw_major, fw_minor, fw_vfixes);

		printf("%s: start uPower RAM service\n", __func__);
		status = upwr_start(1, user_upwr_rdy_callb);
		upower_wait_resp();
		if (upower_status(status)) {
			printf("%s: upower init failure\n", __func__);
			break;
		}
	} while (0);

	swton = 0xfff80;
	ret = upwr_pwm_power_on(&swton, NULL, NULL);
	if (ret)
		printf("Turn on switches fail %d\n", ret);
	else
		printf("Turn on switches ok\n");
	upower_wait_resp();
	ret = upwr_poll_req_status(UPWR_SG_PWRMGMT, NULL, NULL, &ret_val, 1000);
	if (ret != UPWR_REQ_OK)
		printf("Failure %d\n", ret);

	memon = 0x3FFFFFFFFFFFFCUL;
	ret = upwr_pwm_power_on(NULL, (const u32 *)&memon, NULL);
	if (ret)
		printf("Turn on memories fail %d\n", ret);
	else
		printf("Turn on memories ok\n");
	upower_wait_resp();
	ret = upwr_poll_req_status(UPWR_SG_PWRMGMT, NULL, NULL, &ret_val, 1000);
	if (ret != UPWR_REQ_OK)
		printf("Failure %d\n", ret);

	mdelay(1);

	ret = upwr_xcp_set_ddr_retention(APD_DOMAIN, 0, NULL);
	if (ret)
		printf("Clear DDR retention fail %d\n", ret);
	else
		printf("Clear DDR retention ok\n");

	upower_wait_resp();

	ret = upwr_poll_req_status(UPWR_SG_EXCEPT, NULL, NULL, &ret_val, 1000);
	if (ret != UPWR_REQ_OK)
		printf("Failure %d\n", ret);

	return 0;
}
