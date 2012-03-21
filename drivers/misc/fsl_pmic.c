/*
 * (C) Copyright 2008-2009 Freescale Semiconductor, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <config.h>
#include <common.h>
#include <asm/errno.h>
#include <linux/types.h>
#include <fsl_pmic.h>

static int check_param(u32 reg, u32 write)
{
	if (reg > 63 || write > 1) {
		printf("<reg num> = %d is invalid. Should be less then 63\n",
			reg);
		return -1;
	}

	return 0;
}

#ifdef CONFIG_FSL_PMIC_I2C
#include <i2c.h>

u32 pmic_reg(u32 reg, u32 val, u32 write)
{
	unsigned char buf[4] = { 0 };
	u32 ret_val = 0;

	if (check_param(reg, write))
		return -1;

	if (write) {
		buf[0] = (val >> 16) & 0xff;
		buf[1] = (val >> 8) & 0xff;
		buf[2] = (val) & 0xff;
		if (i2c_write(CONFIG_SYS_FSL_PMIC_I2C_ADDR, reg, 1, buf, 3))
			return -1;
	} else {
		if (i2c_read(CONFIG_SYS_FSL_PMIC_I2C_ADDR, reg, 1, buf, 3))
			return -1;
		ret_val = buf[0] << 16 | buf[1] << 8 | buf[2];
	}

	return ret_val;
}
#else /* SPI interface */
#include <spi.h>
static struct spi_slave *slave;

struct spi_slave *pmic_spi_probe(void)
{
	return spi_setup_slave(CONFIG_FSL_PMIC_BUS,
		CONFIG_FSL_PMIC_CS,
		CONFIG_FSL_PMIC_CLK,
		CONFIG_FSL_PMIC_MODE);
}

void pmic_spi_free(struct spi_slave *slave)
{
	if (slave)
		spi_free_slave(slave);
}

u32 pmic_reg(u32 reg, u32 val, u32 write)
{
	u32 pmic_tx, pmic_rx;
	u32 tmp;

	if (!slave) {
		slave = pmic_spi_probe();

		if (!slave)
			return -1;
	}

	if (check_param(reg, write))
		return -1;

	if (spi_claim_bus(slave))
		return -1;

	pmic_tx = (write << 31) | (reg << 25) | (val & 0x00FFFFFF);

	tmp = cpu_to_be32(pmic_tx);

	if (spi_xfer(slave, 4 << 3, &tmp, &pmic_rx,
			SPI_XFER_BEGIN | SPI_XFER_END)) {
		spi_release_bus(slave);
		return -1;
	}

	if (write) {
		pmic_tx &= ~(1 << 31);
		tmp = cpu_to_be32(pmic_tx);
		if (spi_xfer(slave, 4 << 3, &tmp, &pmic_rx,
			SPI_XFER_BEGIN | SPI_XFER_END)) {
			spi_release_bus(slave);
			return -1;
		}
	}

	spi_release_bus(slave);
	return cpu_to_be32(pmic_rx);
}
#endif

void pmic_reg_write(u32 reg, u32 value)
{
	pmic_reg(reg, value, 1);
}

u32 pmic_reg_read(u32 reg)
{
	return pmic_reg(reg, 0, 0);
}

void pmic_show_pmic_info(void)
{
	u32 rev_id;

	rev_id = pmic_reg_read(REG_IDENTIFICATION);
	printf("PMIC ID: 0x%08x [Rev: ", rev_id);
	switch (rev_id & 0x1F) {
	case 0x1:
		puts("1.0");
		break;
	case 0x9:
		puts("1.1");
		break;
	case 0xA:
		puts("1.2");
		break;
	case 0x10:
		puts("2.0");
		break;
	case 0x11:
		puts("2.1");
		break;
	case 0x18:
		puts("3.0");
		break;
	case 0x19:
		puts("3.1");
		break;
	case 0x1A:
		puts("3.2");
		break;
	case 0x2:
		puts("3.2A");
		break;
	case 0x1B:
		puts("3.3");
		break;
	case 0x1D:
		puts("3.5");
		break;
	default:
		puts("unknown");
		break;
	}
	puts("]\n");
}

static void pmic_dump(int numregs)
{
	u32 val;
	int i;

	pmic_show_pmic_info();
	for (i = 0; i < numregs; i++) {
		val = pmic_reg_read(i);
		if (!(i % 8))
			printf ("\n0x%02x: ", i);
		printf("%08x ", val);
	}
	puts("\n");
}

int do_pmic(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	char *cmd;
	int nregs;
	u32 val;

	/* at least two arguments please */
	if (argc < 2)
		return cmd_usage(cmdtp);

	cmd = argv[1];
	if (strcmp(cmd, "dump") == 0) {
		if (argc < 3)
			return cmd_usage(cmdtp);

		nregs = simple_strtoul(argv[2], NULL, 16);
		pmic_dump(nregs);
		return 0;
	}
	if (strcmp(cmd, "write") == 0) {
		if (argc < 4)
			return cmd_usage(cmdtp);

		nregs = simple_strtoul(argv[2], NULL, 16);
		val = simple_strtoul(argv[3], NULL, 16);
		pmic_reg_write(nregs, val);
		return 0;
	}
	/* No subcommand found */
	return 1;
}

U_BOOT_CMD(
	pmic,	CONFIG_SYS_MAXARGS, 1, do_pmic,
	"Freescale PMIC (Atlas)",
	"dump [numregs] dump registers\n"
	"pmic write <reg> <value> - write register"
);
