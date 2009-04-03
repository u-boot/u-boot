/*
 * U-boot - main board file
 *
 * Copyright (c) 2008-2009 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <config.h>
#include <command.h>
#include <net.h>
#include <netdev.h>
#include <spi.h>
#include <asm/blackfin.h>
#include <asm/net.h>
#include <asm/mach-common/bits/otp.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	printf("Board: ADI BF518F EZ-Board board\n");
	printf("       Support: http://blackfin.uclinux.org/\n");
	return 0;
}

phys_size_t initdram(int board_type)
{
	gd->bd->bi_memstart = CONFIG_SYS_SDRAM_BASE;
	gd->bd->bi_memsize = CONFIG_SYS_MAX_RAM_SIZE;
	return gd->bd->bi_memsize;
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
	static bool switch_is_alive = false;
	int ret;

	if (!switch_is_alive) {
		struct spi_slave *slave = spi_setup_slave(0, 1, 5000000, SPI_MODE_3);
		if (slave) {
			if (!spi_claim_bus(slave)) {
				unsigned char dout[3] = { 2, 1, 1, };
				unsigned char din[3];
				ret = spi_xfer(slave, sizeof(dout) * 8, dout, din, SPI_XFER_BEGIN | SPI_XFER_END);
				if (!ret)
					switch_is_alive = true;
				spi_release_bus(slave);
			}
			spi_free_slave(slave);
		}
	}

	if (switch_is_alive)
		return bfin_EMAC_initialize(bis);
	else
		return -1;
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
