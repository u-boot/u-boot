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

#define KSZ_MAX_HZ    5000000

#define KSZ_WRITE     0x02
#define KSZ_READ      0x03

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

static int ksz8893m_reg_clear(struct spi_slave *slave, uchar reg, uchar mask)
{
	int ret = 0;
	unsigned char din[3];

	ret |= ksz8893m_transfer(slave, KSZ_READ, reg, 0, din);
	ret |= ksz8893m_reg_set(slave, reg, din[2] & mask);

	return ret;
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

int board_eth_init(bd_t *bis)
{
	static bool switch_is_alive = false;
	int ret;

	if (!switch_is_alive) {
		struct spi_slave *slave = spi_setup_slave(0, 1, KSZ_MAX_HZ, SPI_MODE_3);
		if (slave) {
			if (!spi_claim_bus(slave)) {
				ret = ksz8893m_reset(slave);
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

int board_early_init_f(void)
{
#if !defined(CONFIG_SYS_NO_FLASH)
	/* setup BF518-EZBRD GPIO pin PG11 to AMS2. */
	bfin_write_PORTG_MUX((bfin_read_PORTG_MUX() & ~PORT_x_MUX_6_MASK) | PORT_x_MUX_6_FUNC_2);
	bfin_write_PORTG_FER(bfin_read_PORTG_FER() | PG11);

# if !defined(CONFIG_BFIN_SPI)
	/* setup BF518-EZBRD GPIO pin PG15 to AMS3. */
	bfin_write_PORTG_MUX((bfin_read_PORTG_MUX() & ~PORT_x_MUX_7_MASK) | PORT_x_MUX_7_FUNC_3);
	bfin_write_PORTG_FER(bfin_read_PORTG_FER() | PG15);
# endif
#endif
	return 0;
}
