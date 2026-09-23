// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2026 Free Mobile - Vincent Jardin
 *
 * Nodebox v3 - DPLL identification probe (both DPLLs).
 *
 * It reads a small set of read-only descriptor registers from each
 * Microsemi/Microchip DPLL on the board and prints the result on the
 * console at EVT_LAST_STAGE_INIT time, just before the prompt.
 * It is purely a basic power on test diagnostic: no configuration
 * register is written, the DPLL register profile loaded at power-on
 * is left untouched.
 */

#include <command.h>
#include <dm.h>
#include <event.h>
#include <log.h>
#include <spi.h>
#include <vsprintf.h>
#include <linux/delay.h>

#define ZL30733_SPI_HZ		12500000
#define ZL30733_SPI_MODE	SPI_MODE_0

/* The two DPLLs sharing the ZL3073x register protocol on this board */
static const struct nbxv3_dpll {
	const char *label;	/* expected part at this location */
	const char *devname;	/* DM device name for the ad-hoc slave */
	int bus;		/* U-Boot SPI bus number */
	int cs;			/* chip select on that bus */
	u32 hz;			/* SCK ceiling */
	const char *hint;	/* what to check when reads are implausible */
} nbxv3_dplls[] = {
	{
		.label = "ZL30733", .devname = "zl30733_dpll",
		.bus = 0, .cs = 0, .hz = ZL30733_SPI_HZ,
		.hint = "check PROG_DPLL_EN mux",
	},
	{
		.label = "ZL30643", .devname = "zl30643_dpll",
		.bus = 2, .cs = 1, .hz = 10000000,
		.hint = "check CS1 rework / RST_B",
	},
};

#define ZL30733_REG_INFO	0x00
#define ZL30733_REG_ID_HI	0x01
#define ZL30733_REG_ID_LO	0x02
#define ZL30733_REG_REVISION	0x03
#define ZL30733_REG_FW_VER	0x05	/* 6 bytes 0x05..0x0A   */
#define ZL30733_REG_FW_VER_LEN	6
#define ZL30733_REG_RST_STATUS	0x18
#define ZL30733_REG_PAGE_SEL	0x7F

#define ZL30733_CMD_READ	0x80	/* OR'd into the 7-bit address */

#define ZL30733_INFO_DEFAULT		0x21
#define ZL30733_INFO_ID_MASK		0x7f
#define ZL30733_INFO_EEPROM_BOOTED	0x80
#define ZL30733_REV_DEFAULT		0x03

static int zl30733_read_block(struct spi_slave *slave, u8 addr, u8 *buf,
			      size_t len)
{
	u8 cmd = ZL30733_CMD_READ | (addr & 0x7F);
	int ret;

	ret = spi_xfer(slave, 8, &cmd, NULL, SPI_XFER_BEGIN);
	if (ret)
		return ret;

	return spi_xfer(slave, len * 8, NULL, buf, SPI_XFER_END);
}

static int zl30733_select_page0(struct spi_slave *slave)
{
	u8 cmd[2] = { ZL30733_REG_PAGE_SEL & 0x7F, 0x00 };

	return spi_xfer(slave, sizeof(cmd) * 8, cmd, NULL,
			SPI_XFER_BEGIN | SPI_XFER_END);
}

static int zl30733_dump(struct spi_slave *slave, const struct nbxv3_dpll *d)
{
	u8 info, rev, rst;
	u8 fw[ZL30733_REG_FW_VER_LEN];
	u8 idbuf[2];
	u16 id;
	int ret;
	bool plausible;

	ret = zl30733_select_page0(slave);
	if (ret) {
		printf("DPLL: %s page select failed (%d)\n", d->label, ret);
		return ret;
	}

	ret = zl30733_read_block(slave, ZL30733_REG_INFO, &info, 1);
	if (ret)
		return ret;
	ret = zl30733_read_block(slave, ZL30733_REG_ID_HI, idbuf, sizeof(idbuf));
	if (ret)
		return ret;
	id = ((u16)idbuf[0] << 8) | idbuf[1];
	ret = zl30733_read_block(slave, ZL30733_REG_REVISION, &rev, 1);
	if (ret)
		return ret;
	ret = zl30733_read_block(slave, ZL30733_REG_FW_VER, fw, sizeof(fw));
	if (ret)
		return ret;
	ret = zl30733_read_block(slave, ZL30733_REG_RST_STATUS, &rst, 1);
	if (ret)
		return ret;

	plausible = (info & ZL30733_INFO_ID_MASK) == ZL30733_INFO_DEFAULT &&
		    id != 0x0000 && id != 0xFFFF;

	printf("DPLL: %s @ spi%d:%d  info=0x%02x (%s) id=0x%04x rev=0x%02x\n",
	       d->label, d->bus, d->cs, info,
	       (info & ZL30733_INFO_EEPROM_BOOTED) ? "EEPROM boot" : "MCU default boot",
	       id, rev);
	printf("      fw_ver=%02x:%02x:%02x:%02x:%02x:%02x  reset_status=0x%02x",
	       fw[0], fw[1], fw[2], fw[3], fw[4], fw[5], rst);
	if (!plausible)
		printf("  [implausible; %s]", d->hint);
	printf("\n");

	return plausible ? 0 : 1;
}

static int zl30733_probe_print(void)
{
	struct udevice *bus;
	struct spi_slave *slave;
	int i, ret;

	for (i = 0; i < ARRAY_SIZE(nbxv3_dplls); i++) {
		const struct nbxv3_dpll *d = &nbxv3_dplls[i];

		ret = _spi_get_bus_and_cs(d->bus, d->cs, d->hz,
					  ZL30733_SPI_MODE,
					  "spi_generic_drv", d->devname,
					  &bus, &slave);
		if (ret) {
			printf("DPLL: %s spi%d:%d not reachable (%d)\n",
			       d->label, d->bus, d->cs, ret);
			continue;
		}

		ret = spi_claim_bus(slave);
		if (ret) {
			printf("DPLL: %s spi_claim_bus failed (%d)\n",
			       d->label, ret);
			continue;
		}

		zl30733_dump(slave, d);

		spi_release_bus(slave);
	}

	return 0;
}

static int nbxv3_dpll_last_stage_init(void)
{
	zl30733_probe_print();
	return 0;
}

EVENT_SPY_SIMPLE(EVT_LAST_STAGE_INIT, nbxv3_dpll_last_stage_init);

static int do_dpll_info(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	return zl30733_probe_print() < 0 ? CMD_RET_FAILURE : CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	dpll_info, 1, 0, do_dpll_info,
	"display ZL30733 + ZL30643 DPLL identification + reset status",
	""
);
