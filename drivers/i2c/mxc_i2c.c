/*
 * i2c driver for Freescale mx31
 *
 * (c) 2007 Pengutronix, Sascha Hauer <s.hauer@pengutronix.de>
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>

#if defined(CONFIG_HARD_I2C) && defined (CONFIG_I2C_MXC)

#include <asm/arch/mx31.h>
#include <asm/arch/mx31-regs.h>

#define IADR	0x00
#define IFDR	0x04
#define I2CR	0x08
#define I2SR	0x0c
#define I2DR	0x10

#define I2CR_IEN	(1 << 7)
#define I2CR_IIEN	(1 << 6)
#define I2CR_MSTA	(1 << 5)
#define I2CR_MTX	(1 << 4)
#define I2CR_TX_NO_AK	(1 << 3)
#define I2CR_RSTA	(1 << 2)

#define I2SR_ICF	(1 << 7)
#define I2SR_IBB	(1 << 5)
#define I2SR_IIF	(1 << 1)
#define I2SR_RX_NO_AK	(1 << 0)

#ifdef CFG_I2C_MX31_PORT1
#define I2C_BASE	0x43f80000
#elif defined (CFG_I2C_MX31_PORT2)
#define I2C_BASE	0x43f98000
#elif defined (CFG_I2C_MX31_PORT3)
#define I2C_BASE	0x43f84000
#else
#error "define CFG_I2C_MX31_PORTx to use the mx31 I2C driver"
#endif

#ifdef DEBUG
#define DPRINTF(args...)  printf(args)
#else
#define DPRINTF(args...)
#endif

static u16 div[] = { 30, 32, 36, 42, 48, 52, 60, 72, 80, 88, 104, 128, 144,
	             160, 192, 240, 288, 320, 384, 480, 576, 640, 768, 960,
	             1152, 1280, 1536, 1920, 2304, 2560, 3072, 3840};

void i2c_init(int speed, int unused)
{
	int freq = mx31_get_ipg_clk();
	int i;

	for (i = 0; i < 0x1f; i++)
		if (freq / div[i] <= speed)
			break;

	DPRINTF("%s: speed: %d\n",__FUNCTION__, speed);

	__REG16(I2C_BASE + I2CR) = 0; /* Reset module */
	__REG16(I2C_BASE + IFDR) = i;
	__REG16(I2C_BASE + I2CR) = I2CR_IEN;
	__REG16(I2C_BASE + I2SR) = 0;
}

static int wait_busy(void)
{
	int timeout = 10000;

	while (!(__REG16(I2C_BASE + I2SR) & I2SR_IIF) && --timeout)
		udelay(1);
	__REG16(I2C_BASE + I2SR) = 0; /* clear interrupt */

	return timeout;
}

static int tx_byte(u8 byte)
{
	__REG16(I2C_BASE + I2DR) = byte;

	if (!wait_busy() || __REG16(I2C_BASE + I2SR) & I2SR_RX_NO_AK)
		return -1;
	return 0;
}

static int rx_byte(void)
{
	if (!wait_busy())
		return -1;

	return __REG16(I2C_BASE + I2DR);
}

int i2c_probe(uchar chip)
{
	int ret;

	__REG16(I2C_BASE + I2CR) = 0; /* Reset module */
	__REG16(I2C_BASE + I2CR) = I2CR_IEN;

	__REG16(I2C_BASE + I2CR) = I2CR_IEN |  I2CR_MSTA | I2CR_MTX;
	ret = tx_byte(chip << 1);
	__REG16(I2C_BASE + I2CR) = I2CR_IEN | I2CR_MTX;

	return ret;
}

static int i2c_addr(uchar chip, uint addr, int alen)
{
	__REG16(I2C_BASE + I2SR) = 0; /* clear interrupt */
	__REG16(I2C_BASE + I2CR) = I2CR_IEN |  I2CR_MSTA | I2CR_MTX;

	if (tx_byte(chip << 1))
		return -1;

	while (alen--)
		if (tx_byte((addr >> (alen * 8)) & 0xff))
			return -1;
	return 0;
}

int i2c_read(uchar chip, uint addr, int alen, uchar *buf, int len)
{
	int timeout = 10000;
	int ret;

	DPRINTF("%s chip: 0x%02x addr: 0x%04x alen: %d len: %d\n",__FUNCTION__, chip, addr, alen, len);

	if (i2c_addr(chip, addr, alen)) {
		printf("i2c_addr failed\n");
		return -1;
	}

	__REG16(I2C_BASE + I2CR) = I2CR_IEN |  I2CR_MSTA | I2CR_MTX | I2CR_RSTA;

	if (tx_byte(chip << 1 | 1))
		return -1;

	__REG16(I2C_BASE + I2CR) = I2CR_IEN |  I2CR_MSTA | ((len == 1) ? I2CR_TX_NO_AK : 0);

	ret = __REG16(I2C_BASE + I2DR);

	while (len--) {
		if ((ret = rx_byte()) < 0)
			return -1;
		*buf++ = ret;
		if (len <= 1)
			__REG16(I2C_BASE + I2CR) = I2CR_IEN |  I2CR_MSTA | I2CR_TX_NO_AK;
	}

	wait_busy();

	__REG16(I2C_BASE + I2CR) = I2CR_IEN;

	while (__REG16(I2C_BASE + I2SR) & I2SR_IBB && --timeout)
		udelay(1);

	return 0;
}

int i2c_write(uchar chip, uint addr, int alen, uchar *buf, int len)
{
	int timeout = 10000;
	DPRINTF("%s chip: 0x%02x addr: 0x%04x alen: %d len: %d\n",__FUNCTION__, chip, addr, alen, len);

	if (i2c_addr(chip, addr, alen))
		return -1;

	while (len--)
		if (tx_byte(*buf++))
			return -1;

	__REG16(I2C_BASE + I2CR) = I2CR_IEN;

	while (__REG16(I2C_BASE + I2SR) & I2SR_IBB && --timeout)
		udelay(1);

	return 0;
}

#endif /* CONFIG_HARD_I2C */
