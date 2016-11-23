/*
 * File:         drivers/i2c/pca9564.c
 * Based on:     drivers/i2c/s3c44b0_i2c.c
 * Author:
 *
 * Created:      2009-06-23
 * Description:  PCA9564 i2c bridge driver
 *
 * Modified:
 *               Copyright 2009 CJSC "NII STT", http://www.niistt.ru/
 *
 * Bugs:
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * NOTE: This driver should be converted to driver model before June 2017.
 * Please see doc/driver-model/i2c-howto.txt for instructions.
 */

#include <common.h>
#include <i2c.h>
#include <pca9564.h>
#include <asm/io.h>

#define PCA_STA			(CONFIG_PCA9564_BASE + 0)
#define PCA_TO			(CONFIG_PCA9564_BASE + 0)
#define PCA_DAT			(CONFIG_PCA9564_BASE + (1 << 2))
#define PCA_ADR			(CONFIG_PCA9564_BASE + (2 << 2))
#define PCA_CON			(CONFIG_PCA9564_BASE + (3 << 2))

static unsigned char pca_read_reg(unsigned int reg)
{
	return readb((void *)reg);
}

static void pca_write_reg(unsigned int reg, unsigned char value)
{
	writeb(value, (void *)reg);
}

static int pca_wait_busy(void)
{
	unsigned int timeout = 10000;

	while (!(pca_read_reg(PCA_CON) & PCA_CON_SI) && --timeout)
		udelay(1);

	if (timeout == 0)
		debug("I2C timeout!\n");

	debug("CON = 0x%02x, STA = 0x%02x\n", pca_read_reg(PCA_CON),
	       pca_read_reg(PCA_STA));

	return timeout ? 0 : 1;
}

/*=====================================================================*/
/*                         Public Functions                            */
/*=====================================================================*/

/*-----------------------------------------------------------------------
 * Initialization
 */
void i2c_init(int speed, int slaveaddr)
{
	pca_write_reg(PCA_CON, PCA_CON_ENSIO | speed);
}

/*
 * Probe the given I2C chip address.  Returns 0 if a chip responded,
 * not 0 on failure.
 */

int i2c_probe(uchar chip)
{
	unsigned char res;

	pca_write_reg(PCA_CON, PCA_CON_STA | PCA_CON_ENSIO);
	pca_wait_busy();

	pca_write_reg(PCA_CON, PCA_CON_STA | PCA_CON_ENSIO);

	pca_write_reg(PCA_DAT, (chip << 1) | 1);
	res = pca_wait_busy();

	if ((res == 0) && (pca_read_reg(PCA_STA) == 0x48))
		res = 1;

	pca_write_reg(PCA_CON, PCA_CON_STO | PCA_CON_ENSIO);

	return res;
}

/*
 * Read/Write interface:
 *   chip:    I2C chip address, range 0..127
 *   addr:    Memory (register) address within the chip
 *   alen:    Number of bytes to use for addr (typically 1, 2 for larger
 *              memories, 0 for register type devices with only one
 *              register)
 *   buffer:  Where to read/write the data
 *   len:     How many bytes to read/write
 *
 *   Returns: 0 on success, not 0 on failure
 */
int i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	int i;

	pca_write_reg(PCA_CON, PCA_CON_ENSIO | PCA_CON_STA);
	pca_wait_busy();

	pca_write_reg(PCA_CON, PCA_CON_ENSIO);

	pca_write_reg(PCA_DAT, (chip << 1));
	pca_wait_busy();
	pca_write_reg(PCA_CON, PCA_CON_ENSIO);

	if (alen > 0) {
		pca_write_reg(PCA_DAT, addr);
		pca_wait_busy();
		pca_write_reg(PCA_CON, PCA_CON_ENSIO);
	}

	pca_write_reg(PCA_CON, PCA_CON_ENSIO | PCA_CON_STO);

	udelay(500);

	pca_write_reg(PCA_CON, PCA_CON_ENSIO | PCA_CON_STA);
	pca_wait_busy();
	pca_write_reg(PCA_CON, PCA_CON_ENSIO);

	pca_write_reg(PCA_DAT, (chip << 1) | 1);
	pca_wait_busy();

	for (i = 0; i < len; ++i) {
		if (i == len - 1)
			pca_write_reg(PCA_CON, PCA_CON_ENSIO);
		else
			pca_write_reg(PCA_CON, PCA_CON_ENSIO | PCA_CON_AA);

		pca_wait_busy();
		buffer[i] = pca_read_reg(PCA_DAT);

	}

	pca_write_reg(PCA_CON, PCA_CON_ENSIO | PCA_CON_STO);

	return 0;
}

int i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len)
{
	int i;

	pca_write_reg(PCA_CON, PCA_CON_ENSIO | PCA_CON_STA);
	pca_wait_busy();
	pca_write_reg(PCA_CON, PCA_CON_ENSIO);

	pca_write_reg(PCA_DAT, chip << 1);
	pca_wait_busy();
	pca_write_reg(PCA_CON, PCA_CON_ENSIO);

	if (alen > 0) {
		pca_write_reg(PCA_DAT, addr);
		pca_wait_busy();
		pca_write_reg(PCA_CON, PCA_CON_ENSIO);
	}

	for (i = 0; i < len; ++i) {
		pca_write_reg(PCA_DAT, buffer[i]);
		pca_wait_busy();
		pca_write_reg(PCA_CON, PCA_CON_ENSIO);
	}

	pca_write_reg(PCA_CON, PCA_CON_STO | PCA_CON_ENSIO);

	return 0;
}
