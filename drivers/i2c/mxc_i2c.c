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
#include <asm/io.h>

#if defined(CONFIG_HARD_I2C)

#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>

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

#if defined(CONFIG_SYS_I2C_MX31_PORT1)
#define I2C_BASE	0x43f80000
#define I2C_CLK_OFFSET	26
#elif defined (CONFIG_SYS_I2C_MX31_PORT2)
#define I2C_BASE	0x43f98000
#define I2C_CLK_OFFSET	28
#elif defined (CONFIG_SYS_I2C_MX31_PORT3)
#define I2C_BASE	0x43f84000
#define I2C_CLK_OFFSET	30
#elif defined(CONFIG_SYS_I2C_MX53_PORT1)
#define I2C_BASE        I2C1_BASE_ADDR
#elif defined(CONFIG_SYS_I2C_MX53_PORT2)
#define I2C_BASE        I2C2_BASE_ADDR
#elif defined(CONFIG_SYS_I2C_MX35_PORT1)
#define I2C_BASE	I2C_BASE_ADDR
#else
#error "define CONFIG_SYS_I2C_MX<Processor>_PORTx to use the mx I2C driver"
#endif

#define I2C_MAX_TIMEOUT		10000
#define I2C_MAX_RETRIES		3

static u16 div[] = { 30, 32, 36, 42, 48, 52, 60, 72, 80, 88, 104, 128, 144,
	             160, 192, 240, 288, 320, 384, 480, 576, 640, 768, 960,
	             1152, 1280, 1536, 1920, 2304, 2560, 3072, 3840};

static inline void i2c_reset(void)
{
	writew(0, I2C_BASE + I2CR);	/* Reset module */
	writew(0, I2C_BASE + I2SR);
	writew(I2CR_IEN, I2C_BASE + I2CR);
}

void i2c_init(int speed, int unused)
{
	int freq;
	int i;

#if defined(CONFIG_MX31)
	struct clock_control_regs *sc_regs =
		(struct clock_control_regs *)CCM_BASE;
	/* start the required I2C clock */
	writel(readl(&sc_regs->cgr0) | (3 << I2C_CLK_OFFSET),
		&sc_regs->cgr0);
#endif
	freq = mxc_get_clock(MXC_IPG_PERCLK);

	for (i = 0; i < 0x1f; i++)
		if (freq / div[i] <= speed)
			break;

	debug("%s: speed: %d\n", __func__, speed);

	writew(i, I2C_BASE + IFDR);
	i2c_reset();
}

static int wait_idle(void)
{
	int timeout = I2C_MAX_TIMEOUT;

	while ((readw(I2C_BASE + I2SR) & I2SR_IBB) && --timeout) {
		writew(0, I2C_BASE + I2SR);
		udelay(1);
	}
	return timeout ? timeout : (!(readw(I2C_BASE + I2SR) & I2SR_IBB));
}

static int wait_busy(void)
{
	int timeout = I2C_MAX_TIMEOUT;

	while (!(readw(I2C_BASE + I2SR) & I2SR_IBB) && --timeout)
		udelay(1);
	writew(0, I2C_BASE + I2SR); /* clear interrupt */

	return timeout;
}

static int wait_complete(void)
{
	int timeout = I2C_MAX_TIMEOUT;

	while ((!(readw(I2C_BASE + I2SR) & I2SR_ICF)) && (--timeout)) {
		writew(0, I2C_BASE + I2SR);
		udelay(1);
	}
	udelay(200);

	writew(0, I2C_BASE + I2SR);	/* clear interrupt */

	return timeout;
}


static int tx_byte(u8 byte)
{
	writew(byte, I2C_BASE + I2DR);

	if (!wait_complete() || readw(I2C_BASE + I2SR) & I2SR_RX_NO_AK)
		return -1;
	return 0;
}

static int rx_byte(int last)
{
	if (!wait_complete())
		return -1;

	if (last)
		writew(I2CR_IEN, I2C_BASE + I2CR);

	return readw(I2C_BASE + I2DR);
}

int i2c_probe(uchar chip)
{
	int ret;

	writew(0, I2C_BASE + I2CR); /* Reset module */
	writew(I2CR_IEN, I2C_BASE + I2CR);

	writew(I2CR_IEN |  I2CR_MSTA | I2CR_MTX, I2C_BASE + I2CR);
	ret = tx_byte(chip << 1);
	writew(I2CR_IEN | I2CR_MTX, I2C_BASE + I2CR);

	return ret;
}

static int i2c_addr(uchar chip, uint addr, int alen)
{
	int i, retry = 0;
	for (retry = 0; retry < 3; retry++) {
		if (wait_idle())
			break;
		i2c_reset();
		for (i = 0; i < I2C_MAX_TIMEOUT; i++)
			udelay(1);
	}
	if (retry >= I2C_MAX_RETRIES) {
		debug("%s:bus is busy(%x)\n",
		       __func__, readw(I2C_BASE + I2SR));
		return -1;
	}
	writew(I2CR_IEN | I2CR_MSTA | I2CR_MTX, I2C_BASE + I2CR);

	if (!wait_busy()) {
		debug("%s:trigger start fail(%x)\n",
		       __func__, readw(I2C_BASE + I2SR));
		return -1;
	}

	if (tx_byte(chip << 1) || (readw(I2C_BASE + I2SR) & I2SR_RX_NO_AK)) {
		debug("%s:chip address cycle fail(%x)\n",
		       __func__, readw(I2C_BASE + I2SR));
		return -1;
	}
	while (alen--)
		if (tx_byte((addr >> (alen * 8)) & 0xff) ||
		    (readw(I2C_BASE + I2SR) & I2SR_RX_NO_AK)) {
			debug("%s:device address cycle fail(%x)\n",
			       __func__, readw(I2C_BASE + I2SR));
			return -1;
		}
	return 0;
}

int i2c_read(uchar chip, uint addr, int alen, uchar *buf, int len)
{
	int timeout = I2C_MAX_TIMEOUT;
	int ret;

	debug("%s chip: 0x%02x addr: 0x%04x alen: %d len: %d\n",
		__func__, chip, addr, alen, len);

	if (i2c_addr(chip, addr, alen)) {
		printf("i2c_addr failed\n");
		return -1;
	}

	writew(I2CR_IEN | I2CR_MSTA | I2CR_MTX | I2CR_RSTA, I2C_BASE + I2CR);

	if (tx_byte(chip << 1 | 1))
		return -1;

	writew(I2CR_IEN | I2CR_MSTA |
		((len == 1) ? I2CR_TX_NO_AK : 0),
		I2C_BASE + I2CR);

	ret = readw(I2C_BASE + I2DR);

	while (len--) {
		ret = rx_byte(len == 0);
		if (ret  < 0)
			return -1;
		*buf++ = ret;
		if (len <= 1)
			writew(I2CR_IEN | I2CR_MSTA |
				I2CR_TX_NO_AK,
				I2C_BASE + I2CR);
	}

	writew(I2CR_IEN, I2C_BASE + I2CR);

	while (readw(I2C_BASE + I2SR) & I2SR_IBB && --timeout)
		udelay(1);

	return 0;
}

int i2c_write(uchar chip, uint addr, int alen, uchar *buf, int len)
{
	int timeout = I2C_MAX_TIMEOUT;
	debug("%s chip: 0x%02x addr: 0x%04x alen: %d len: %d\n",
		__func__, chip, addr, alen, len);

	if (i2c_addr(chip, addr, alen))
		return -1;

	while (len--)
		if (tx_byte(*buf++))
			return -1;

	writew(I2CR_IEN, I2C_BASE + I2CR);

	while (readw(I2C_BASE + I2SR) & I2SR_IBB && --timeout)
		udelay(1);

	return 0;
}

#endif /* CONFIG_HARD_I2C */
