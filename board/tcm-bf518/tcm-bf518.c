/*
 * U-boot - main board file
 *
 * Copyright (c) 2008-2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <config.h>
#include <net.h>
#include <netdev.h>
#include <asm/blackfin.h>
#include <asm/net.h>
#include <asm/mach-common/bits/otp.h>
#include <asm/sdh.h>

int checkboard(void)
{
	printf("Board: Bluetechnix TCM-BF518 board\n");
	printf("       Support: http://www.bluetechnix.com/\n");
	printf("                http://blackfin.uclinux.org/\n");
	return 0;
}

#if defined(CONFIG_BFIN_MAC)
static void board_init_enetaddr(uchar *mac_addr)
{
	bool valid_mac = false;

#if 0
	/* the MAC is stored in OTP memory page 0xDF */
	uint32_t ret;
	uint64_t otp_mac;

	ret = bfrom_OtpRead(0xDF, OTP_LOWER_HALF, &otp_mac);
	if (!(ret & OTP_MASTER_ERROR)) {
		uchar *otp_mac_p = (uchar *)&otp_mac;

		for (ret = 0; ret < 6; ++ret)
			mac_addr[ret] = otp_mac_p[5 - ret];

		if (is_valid_ether_addr(mac_addr))
			valid_mac = true;
	}
#endif

	if (!valid_mac) {
		puts("Warning: Generating 'random' MAC address\n");
		bfin_gen_rand_mac(mac_addr);
	}

	eth_setenv_enetaddr("ethaddr", mac_addr);
}

int board_eth_init(bd_t *bis)
{
	return bfin_EMAC_initialize(bis);
}
#endif

int misc_init_r(void)
{
#ifdef CONFIG_BFIN_MAC
	uchar enetaddr[6];
	if (!eth_getenv_enetaddr("ethaddr", enetaddr))
		board_init_enetaddr(enetaddr);
#endif

	return 0;
}

#ifdef CONFIG_BFIN_SDH
int board_mmc_init(bd_t *bis)
{
	return bfin_mmc_init(bis);
}
#endif
