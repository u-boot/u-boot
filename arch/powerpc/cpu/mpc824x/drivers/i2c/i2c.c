/*
 * (C) Copyright 2003
 * Gleb Natapov <gnatapov@mrv.com>
 * Some bits are taken from linux driver writen by adrian@humboldt.co.uk
 *
 * Hardware I2C driver for MPC107 PCI bridge.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#undef I2CDBG

#ifdef CONFIG_HARD_I2C
#include <i2c.h>

#define TIMEOUT (CONFIG_SYS_HZ/4)

#define I2C_Addr ((unsigned *)(CONFIG_SYS_EUMB_ADDR + 0x3000))

#define I2CADR &I2C_Addr[0]
#define I2CFDR  &I2C_Addr[1]
#define I2CCCR  &I2C_Addr[2]
#define I2CCSR  &I2C_Addr[3]
#define I2CCDR  &I2C_Addr[4]

#define MPC107_CCR_MEN  0x80
#define MPC107_CCR_MIEN 0x40
#define MPC107_CCR_MSTA 0x20
#define MPC107_CCR_MTX  0x10
#define MPC107_CCR_TXAK 0x08
#define MPC107_CCR_RSTA 0x04

#define MPC107_CSR_MCF  0x80
#define MPC107_CSR_MAAS 0x40
#define MPC107_CSR_MBB  0x20
#define MPC107_CSR_MAL  0x10
#define MPC107_CSR_SRW  0x04
#define MPC107_CSR_MIF  0x02
#define MPC107_CSR_RXAK 0x01

#define I2C_READ  1
#define I2C_WRITE 0

/* taken from linux include/asm-ppc/io.h */
inline unsigned in_le32 (volatile unsigned *addr)
{
	unsigned ret;

	__asm__ __volatile__ ("lwbrx %0,0,%1;\n"
			      "twi 0,%0,0;\n"
			      "isync":"=r" (ret): "r" (addr), "m" (*addr));
	return ret;
}

inline void out_le32 (volatile unsigned *addr, int val)
{
	__asm__ __volatile__ ("stwbrx %1,0,%2; eieio":"=m" (*addr):"r" (val),
			      "r" (addr));
}

#define writel(val, addr) out_le32(addr, val)
#define readl(addr) in_le32(addr)

void i2c_init (int speed, int slaveadd)
{
	/* stop I2C controller */
	writel (0x0, I2CCCR);
	/* set clock */
	writel (0x1020, I2CFDR);
	/* write slave address */
	writel (slaveadd, I2CADR);
	/* clear status register */
	writel (0x0, I2CCSR);
	/* start I2C controller */
	writel (MPC107_CCR_MEN, I2CCCR);

	return;
}

static __inline__ int i2c_wait4bus (void)
{
	ulong timeval = get_timer (0);

	while (readl (I2CCSR) & MPC107_CSR_MBB)
		if (get_timer (timeval) > TIMEOUT)
			return -1;

	return 0;
}

static __inline__ int i2c_wait (int write)
{
	u32 csr;
	ulong timeval = get_timer (0);

	do {
		csr = readl (I2CCSR);

		if (!(csr & MPC107_CSR_MIF))
			continue;

		writel (0x0, I2CCSR);

		if (csr & MPC107_CSR_MAL) {
#ifdef I2CDBG
			printf ("i2c_wait: MAL\n");
#endif
			return -1;
		}

		if (!(csr & MPC107_CSR_MCF)) {
#ifdef I2CDBG
			printf ("i2c_wait: unfinished\n");
#endif
			return -1;
		}

		if (write == I2C_WRITE && (csr & MPC107_CSR_RXAK)) {
#ifdef I2CDBG
			printf ("i2c_wait: No RXACK\n");
#endif
			return -1;
		}

		return 0;
	} while (get_timer (timeval) < TIMEOUT);

#ifdef I2CDBG
	printf ("i2c_wait: timed out\n");
#endif
	return -1;
}

static __inline__ int i2c_write_addr (u8 dev, u8 dir, int rsta)
{
	writel (MPC107_CCR_MEN | MPC107_CCR_MSTA | MPC107_CCR_MTX |
		(rsta ? MPC107_CCR_RSTA : 0), I2CCCR);

	writel ((dev << 1) | dir, I2CCDR);

	if (i2c_wait (I2C_WRITE) < 0)
		return 0;

	return 1;
}

static __inline__ int __i2c_write (u8 * data, int length)
{
	int i;

	writel (MPC107_CCR_MEN | MPC107_CCR_MSTA | MPC107_CCR_MTX, I2CCCR);

	for (i = 0; i < length; i++) {
		writel (data[i], I2CCDR);

		if (i2c_wait (I2C_WRITE) < 0)
			break;
	}

	return i;
}

static __inline__ int __i2c_read (u8 * data, int length)
{
	int i;

	writel (MPC107_CCR_MEN | MPC107_CCR_MSTA |
		((length == 1) ? MPC107_CCR_TXAK : 0), I2CCCR);

	/* dummy read */
	readl (I2CCDR);

	for (i = 0; i < length; i++) {
		if (i2c_wait (I2C_READ) < 0)
			break;

		/* Generate ack on last next to last byte */
		if (i == length - 2)
			writel (MPC107_CCR_MEN | MPC107_CCR_MSTA |
				MPC107_CCR_TXAK, I2CCCR);

		/* Generate stop on last byte */
		if (i == length - 1)
			writel (MPC107_CCR_MEN | MPC107_CCR_TXAK, I2CCCR);

		data[i] = readl (I2CCDR);
	}

	return i;
}

int i2c_read (u8 dev, uint addr, int alen, u8 * data, int length)
{
	int i = 0;
	u8 *a = (u8 *) & addr;

	if (i2c_wait4bus () < 0)
		goto exit;

	if (i2c_write_addr (dev, I2C_WRITE, 0) == 0)
		goto exit;

	if (__i2c_write (&a[4 - alen], alen) != alen)
		goto exit;

	if (i2c_write_addr (dev, I2C_READ, 1) == 0)
		goto exit;

	i = __i2c_read (data, length);

exit:
	writel (MPC107_CCR_MEN, I2CCCR);

	return !(i == length);
}

int i2c_write (u8 dev, uint addr, int alen, u8 * data, int length)
{
	int i = 0;
	u8 *a = (u8 *) & addr;

	if (i2c_wait4bus () < 0)
		goto exit;

	if (i2c_write_addr (dev, I2C_WRITE, 0) == 0)
		goto exit;

	if (__i2c_write (&a[4 - alen], alen) != alen)
		goto exit;

	i = __i2c_write (data, length);

exit:
	writel (MPC107_CCR_MEN, I2CCCR);

	return !(i == length);
}

int i2c_probe (uchar chip)
{
	int tmp;

	/*
	 * Try to read the first location of the chip.  The underlying
	 * driver doesn't appear to support sending just the chip address
	 * and looking for an <ACK> back.
	 */
	udelay (10000);
	return i2c_read (chip, 0, 1, (uchar *) &tmp, 1);
}

#endif /* CONFIG_HARD_I2C */
