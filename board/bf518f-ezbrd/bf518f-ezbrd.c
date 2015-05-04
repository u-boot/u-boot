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
#include <asm/portmux.h>
#include <asm/mach-common/bits/otp.h>
#include <asm/sdh.h>

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	printf("Board: ADI BF518F EZ-Board board\n");
	printf("       Support: http://blackfin.uclinux.org/\n");
	return 0;
}

#if defined(CONFIG_BFIN_MAC)
static void board_init_enetaddr(uchar *mac_addr)
{
#ifndef CONFIG_SYS_NO_FLASH
	/* we cram the MAC in the last flash sector */
	uchar *board_mac_addr = (uchar *)0x203F0096;
	if (is_valid_ethaddr(board_mac_addr)) {
		memcpy(mac_addr, board_mac_addr, 6);
		eth_setenv_enetaddr("ethaddr", mac_addr);
	}
#endif
}

/* Only the first run of boards had a KSZ switch */
#if defined(CONFIG_BFIN_SPI) && __SILICON_REVISION__ == 0
# define KSZ_POSSIBLE 1
#else
# define KSZ_POSSIBLE 0
#endif

#define KSZ_MAX_HZ    5000000

#define KSZ_WRITE     0x02
#define KSZ_READ      0x03

#define KSZ_REG_CHID  0x00	/* Register 0: Chip ID0 */
#define KSZ_REG_STPID 0x01	/* Register 1: Chip ID1 / Start Switch */
#define KSZ_REG_GC9   0x0b	/* Register 11: Global Control 9 */
#define KSZ_REG_P3C0  0x30	/* Register 48: Port 3 Control 0 */

static int ksz8893m_transfer(struct spi_slave *slave, uchar dir, uchar reg,
			     uchar data, uchar result[3])
{
	unsigned char dout[3] = { dir, reg, data, };
	return spi_xfer(slave, sizeof(dout) * 8, dout, result, SPI_XFER_BEGIN | SPI_XFER_END);
}

static int ksz8893m_reg_set(struct spi_slave *slave, uchar reg, uchar data)
{
	unsigned char din[3];
	return ksz8893m_transfer(slave, KSZ_WRITE, reg, data, din);
}

static int ksz8893m_reg_read(struct spi_slave *slave, uchar reg)
{
	int ret;
	unsigned char din[3];
	ret = ksz8893m_transfer(slave, KSZ_READ, reg, 0, din);
	return ret ? ret : din[2];
}

static int ksz8893m_reg_clear(struct spi_slave *slave, uchar reg, uchar mask)
{
	return ksz8893m_reg_set(slave, reg, ksz8893m_reg_read(slave, reg) & mask);
}

static int ksz8893m_reset(struct spi_slave *slave)
{
	int ret = 0;

	/* Disable STPID mode */
	ret |= ksz8893m_reg_clear(slave, KSZ_REG_GC9, 0x01);

	/* Disable VLAN tag insert on Port3 */
	ret |= ksz8893m_reg_clear(slave, KSZ_REG_P3C0, 0x04);

	/* Start switch */
	ret |= ksz8893m_reg_set(slave, KSZ_REG_STPID, 0x01);

	return ret;
}

static bool board_ksz_init(void)
{
	static bool switch_is_alive = false;

	if (!switch_is_alive) {
		struct spi_slave *slave = spi_setup_slave(0, 1, KSZ_MAX_HZ, SPI_MODE_3);
		if (slave) {
			if (!spi_claim_bus(slave)) {
				bool phy_is_ksz = (ksz8893m_reg_read(slave, KSZ_REG_CHID) == 0x88);
				int ret = phy_is_ksz ? ksz8893m_reset(slave) : 0;
				switch_is_alive = (ret == 0);
				spi_release_bus(slave);
			}
			spi_free_slave(slave);
		}
	}

	return switch_is_alive;
}

int board_eth_init(bd_t *bis)
{
	if (KSZ_POSSIBLE) {
		if (!board_ksz_init())
			return 0;
	}
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

#ifndef CONFIG_SYS_NO_FLASH
	/* we use the last sector for the MAC address / POST LDR */
	extern flash_info_t flash_info[];
	flash_protect(FLAG_PROTECT_SET, 0x203F0000, 0x203FFFFF, &flash_info[0]);
#endif

	return 0;
}

int board_early_init_f(void)
{
	/* connect async banks by default */
	const unsigned short pins[] = {
		P_AMS2, P_AMS3, 0,
	};
	return peripheral_request_list(pins, "async");
}

#ifdef CONFIG_BFIN_SDH
int board_mmc_init(bd_t *bis)
{
	return bfin_mmc_init(bis);
}
#endif
