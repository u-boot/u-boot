/*
 * Driver for Blackfin on-chip ATAPI controller.
 *
 * Enter bugs at http://blackfin.uclinux.org/
 *
 * Copyright (c) 2008 Analog Devices Inc.
 *
 * Licensed under the GPL-2 or later.
 */

#include <common.h>
#include <command.h>
#include <config.h>
#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/errno.h>
#include <asm/portmux.h>
#include <asm/mach-common/bits/pata.h>
#include <ata.h>
#include <sata.h>
#include <libata.h>
#include "pata_bfin.h"

static struct ata_port port[CONFIG_SYS_SATA_MAX_DEVICE];

/**
 * PIO Mode - Frequency compatibility
 */
/* mode: 0         1         2         3         4 */
static const u32 pio_fsclk[] =
{ 33333333, 33333333, 33333333, 33333333, 33333333 };

/**
 * MDMA Mode - Frequency compatibility
 */
/*               mode:      0         1         2        */
static const u32 mdma_fsclk[] = { 33333333, 33333333, 33333333 };

/**
 * UDMA Mode - Frequency compatibility
 *
 * UDMA5 - 100 MB/s   - SCLK  = 133 MHz
 * UDMA4 - 66 MB/s    - SCLK >=  80 MHz
 * UDMA3 - 44.4 MB/s  - SCLK >=  50 MHz
 * UDMA2 - 33 MB/s    - SCLK >=  40 MHz
 */
/* mode: 0         1         2         3         4          5 */
static const u32 udma_fsclk[] =
{ 33333333, 33333333, 40000000, 50000000, 80000000, 133333333 };

/**
 * Register transfer timing table
 */
/*               mode:       0    1    2    3    4    */
/* Cycle Time                     */
static const u32 reg_t0min[]   = { 600, 383, 330, 180, 120 };
/* DIOR/DIOW to end cycle         */
static const u32 reg_t2min[]   = { 290, 290, 290, 70,  25  };
/* DIOR/DIOW asserted pulse width */
static const u32 reg_teocmin[] = { 290, 290, 290, 80,  70  };

/**
 * PIO timing table
 */
/*               mode:       0    1    2    3    4    */
/* Cycle Time                     */
static const u32 pio_t0min[]   = { 600, 383, 240, 180, 120 };
/* Address valid to DIOR/DIORW    */
static const u32 pio_t1min[]   = { 70,  50,  30,  30,  25  };
/* DIOR/DIOW to end cycle         */
static const u32 pio_t2min[]   = { 165, 125, 100, 80,  70  };
/* DIOR/DIOW asserted pulse width */
static const u32 pio_teocmin[] = { 165, 125, 100, 70,  25  };
/* DIOW data hold                 */
static const u32 pio_t4min[]   = { 30,  20,  15,  10,  10  };

/* ******************************************************************
 * Multiword DMA timing table
 * ******************************************************************
 */
/*               mode:       0   1    2        */
/* Cycle Time                     */
static const u32 mdma_t0min[]  = { 480, 150, 120 };
/* DIOR/DIOW asserted pulse width */
static const u32 mdma_tdmin[]  = { 215, 80,  70  };
/* DMACK to read data released    */
static const u32 mdma_thmin[]  = { 20,  15,  10  };
/* DIOR/DIOW to DMACK hold        */
static const u32 mdma_tjmin[]  = { 20,  5,   5   };
/* DIOR negated pulse width       */
static const u32 mdma_tkrmin[] = { 50,  50,  25  };
/* DIOR negated pulse width       */
static const u32 mdma_tkwmin[] = { 215, 50,  25  };
/* CS[1:0] valid to DIOR/DIOW     */
static const u32 mdma_tmmin[]  = { 50,  30,  25  };
/* DMACK to read data released    */
static const u32 mdma_tzmax[]  = { 20,  25,  25  };

/**
 * Ultra DMA timing table
 */
/*               mode:         0    1    2    3    4    5       */
static const u32 udma_tcycmin[]  = { 112, 73,  54,  39,  25,  17 };
static const u32 udma_tdvsmin[]  = { 70,  48,  31,  20,  7,   5  };
static const u32 udma_tenvmax[]  = { 70,  70,  70,  55,  55,  50 };
static const u32 udma_trpmin[]   = { 160, 125, 100, 100, 100, 85 };
static const u32 udma_tmin[]     = { 5,   5,   5,   5,   3,   3  };


static const u32 udma_tmlimin = 20;
static const u32 udma_tzahmin = 20;
static const u32 udma_tenvmin = 20;
static const u32 udma_tackmin = 20;
static const u32 udma_tssmin = 50;

static void msleep(int count)
{
	int i;

	for (i = 0; i < count; i++)
		udelay(1000);
}

/**
 *
 *	Function:       num_clocks_min
 *
 *	Description:
 *	calculate number of SCLK cycles to meet minimum timing
 */
static unsigned short num_clocks_min(unsigned long tmin,
				unsigned long fsclk)
{
	unsigned long tmp ;
	unsigned short result;

	tmp = tmin * (fsclk/1000/1000) / 1000;
	result = (unsigned short)tmp;
	if ((tmp*1000*1000) < (tmin*(fsclk/1000)))
		result++;

	return result;
}

/**
 *	bfin_set_piomode - Initialize host controller PATA PIO timings
 *	@ap: Port whose timings we are configuring
 *	@pio_mode: mode
 *
 *	Set PIO mode for device.
 *
 *	LOCKING:
 *	None (inherited from caller).
 */

static void bfin_set_piomode(struct ata_port *ap, int pio_mode)
{
	int mode = pio_mode - XFER_PIO_0;
	void __iomem *base = (void __iomem *)ap->ioaddr.ctl_addr;
	unsigned int fsclk = get_sclk();
	unsigned short teoc_reg, t2_reg, teoc_pio;
	unsigned short t4_reg, t2_pio, t1_reg;
	unsigned short n0, n6, t6min = 5;

	/* the most restrictive timing value is t6 and tc, the DIOW - data hold
	* If one SCLK pulse is longer than this minimum value then register
	* transfers cannot be supported at this frequency.
	*/
	n6 = num_clocks_min(t6min, fsclk);
	if (mode >= 0 && mode <= 4 && n6 >= 1) {
		debug("set piomode: mode=%d, fsclk=%ud\n", mode, fsclk);
		/* calculate the timing values for register transfers. */
		while (mode > 0 && pio_fsclk[mode] > fsclk)
			mode--;

		/* DIOR/DIOW to end cycle time */
		t2_reg = num_clocks_min(reg_t2min[mode], fsclk);
		/* DIOR/DIOW asserted pulse width */
		teoc_reg = num_clocks_min(reg_teocmin[mode], fsclk);
		/* Cycle Time */
		n0  = num_clocks_min(reg_t0min[mode], fsclk);

		/* increase t2 until we meed the minimum cycle length */
		if (t2_reg + teoc_reg < n0)
			t2_reg = n0 - teoc_reg;

		/* calculate the timing values for pio transfers. */

		/* DIOR/DIOW to end cycle time */
		t2_pio = num_clocks_min(pio_t2min[mode], fsclk);
		/* DIOR/DIOW asserted pulse width */
		teoc_pio = num_clocks_min(pio_teocmin[mode], fsclk);
		/* Cycle Time */
		n0  = num_clocks_min(pio_t0min[mode], fsclk);

		/* increase t2 until we meed the minimum cycle length */
		if (t2_pio + teoc_pio < n0)
			t2_pio = n0 - teoc_pio;

		/* Address valid to DIOR/DIORW */
		t1_reg = num_clocks_min(pio_t1min[mode], fsclk);

		/* DIOW data hold */
		t4_reg = num_clocks_min(pio_t4min[mode], fsclk);

		ATAPI_SET_REG_TIM_0(base, (teoc_reg<<8 | t2_reg));
		ATAPI_SET_PIO_TIM_0(base, (t4_reg<<12 | t2_pio<<4 | t1_reg));
		ATAPI_SET_PIO_TIM_1(base, teoc_pio);
		if (mode > 2) {
			ATAPI_SET_CONTROL(base,
				ATAPI_GET_CONTROL(base) | IORDY_EN);
		} else {
			ATAPI_SET_CONTROL(base,
				ATAPI_GET_CONTROL(base) & ~IORDY_EN);
		}

		/* Disable host ATAPI PIO interrupts */
		ATAPI_SET_INT_MASK(base, ATAPI_GET_INT_MASK(base)
			& ~(PIO_DONE_MASK | HOST_TERM_XFER_MASK));
		SSYNC();
	}
}

/**
 *
 *    Function:       wait_complete
 *
 *    Description:    Waits the interrupt from device
 *
 */
static inline void wait_complete(void __iomem *base, unsigned short mask)
{
	unsigned short status;
	unsigned int i = 0;

	for (i = 0; i < PATA_BFIN_WAIT_TIMEOUT; i++) {
		status = ATAPI_GET_INT_STATUS(base) & mask;
		if (status)
			break;
	}

	ATAPI_SET_INT_STATUS(base, mask);
}

/**
 *
 *    Function:       write_atapi_register
 *
 *    Description:    Writes to ATA Device Resgister
 *
 */

static void write_atapi_register(void __iomem *base,
		unsigned long ata_reg, unsigned short value)
{
	/* Program the ATA_DEV_TXBUF register with write data (to be
	 * written into the device).
	 */
	ATAPI_SET_DEV_TXBUF(base, value);

	/* Program the ATA_DEV_ADDR register with address of the
	 * device register (0x01 to 0x0F).
	 */
	ATAPI_SET_DEV_ADDR(base, ata_reg);

	/* Program the ATA_CTRL register with dir set to write (1)
	 */
	ATAPI_SET_CONTROL(base, (ATAPI_GET_CONTROL(base) | XFER_DIR));

	/* ensure PIO DMA is not set */
	ATAPI_SET_CONTROL(base, (ATAPI_GET_CONTROL(base) & ~PIO_USE_DMA));

	/* and start the transfer */
	ATAPI_SET_CONTROL(base, (ATAPI_GET_CONTROL(base) | PIO_START));

	/* Wait for the interrupt to indicate the end of the transfer.
	 * (We need to wait on and clear rhe ATA_DEV_INT interrupt status)
	 */
	wait_complete(base, PIO_DONE_INT);
}

/**
 *
 *	Function:       read_atapi_register
 *
 *Description:    Reads from ATA Device Resgister
 *
 */

static unsigned short read_atapi_register(void __iomem *base,
		unsigned long ata_reg)
{
	/* Program the ATA_DEV_ADDR register with address of the
	 * device register (0x01 to 0x0F).
	 */
	ATAPI_SET_DEV_ADDR(base, ata_reg);

	/* Program the ATA_CTRL register with dir set to read (0) and
	 */
	ATAPI_SET_CONTROL(base, (ATAPI_GET_CONTROL(base) & ~XFER_DIR));

	/* ensure PIO DMA is not set */
	ATAPI_SET_CONTROL(base, (ATAPI_GET_CONTROL(base) & ~PIO_USE_DMA));

	/* and start the transfer */
	ATAPI_SET_CONTROL(base, (ATAPI_GET_CONTROL(base) | PIO_START));

	/* Wait for the interrupt to indicate the end of the transfer.
	 * (PIO_DONE interrupt is set and it doesn't seem to matter
	 * that we don't clear it)
	 */
	wait_complete(base, PIO_DONE_INT);

	/* Read the ATA_DEV_RXBUF register with write data (to be
	 * written into the device).
	 */
	return ATAPI_GET_DEV_RXBUF(base);
}

/**
 *
 *    Function:       write_atapi_register_data
 *
 *    Description:    Writes to ATA Device Resgister
 *
 */

static void write_atapi_data(void __iomem *base,
		int len, unsigned short *buf)
{
	int i;

	/* Set transfer length to 1 */
	ATAPI_SET_XFER_LEN(base, 1);

	/* Program the ATA_DEV_ADDR register with address of the
	 * ATA_REG_DATA
	 */
	ATAPI_SET_DEV_ADDR(base, ATA_REG_DATA);

	/* Program the ATA_CTRL register with dir set to write (1)
	 */
	ATAPI_SET_CONTROL(base, (ATAPI_GET_CONTROL(base) | XFER_DIR));

	/* ensure PIO DMA is not set */
	ATAPI_SET_CONTROL(base, (ATAPI_GET_CONTROL(base) & ~PIO_USE_DMA));

	for (i = 0; i < len; i++) {
		/* Program the ATA_DEV_TXBUF register with write data (to be
		 * written into the device).
		 */
		ATAPI_SET_DEV_TXBUF(base, buf[i]);

		/* and start the transfer */
		ATAPI_SET_CONTROL(base, (ATAPI_GET_CONTROL(base) | PIO_START));

		/* Wait for the interrupt to indicate the end of the transfer.
		 * (We need to wait on and clear rhe ATA_DEV_INT
		 * interrupt status)
		 */
		wait_complete(base, PIO_DONE_INT);
	}
}

/**
 *
 *	Function:       read_atapi_register_data
 *
 *	Description:    Reads from ATA Device Resgister
 *
 */

static void read_atapi_data(void __iomem *base,
		int len, unsigned short *buf)
{
	int i;

	/* Set transfer length to 1 */
	ATAPI_SET_XFER_LEN(base, 1);

	/* Program the ATA_DEV_ADDR register with address of the
	 * ATA_REG_DATA
	 */
	ATAPI_SET_DEV_ADDR(base, ATA_REG_DATA);

	/* Program the ATA_CTRL register with dir set to read (0) and
	 */
	ATAPI_SET_CONTROL(base, (ATAPI_GET_CONTROL(base) & ~XFER_DIR));

	/* ensure PIO DMA is not set */
	ATAPI_SET_CONTROL(base, (ATAPI_GET_CONTROL(base) & ~PIO_USE_DMA));

	for (i = 0; i < len; i++) {
		/* and start the transfer */
		ATAPI_SET_CONTROL(base, (ATAPI_GET_CONTROL(base) | PIO_START));

		/* Wait for the interrupt to indicate the end of the transfer.
		 * (PIO_DONE interrupt is set and it doesn't seem to matter
		 * that we don't clear it)
		 */
		wait_complete(base, PIO_DONE_INT);

		/* Read the ATA_DEV_RXBUF register with write data (to be
		 * written into the device).
		 */
		buf[i] = ATAPI_GET_DEV_RXBUF(base);
	}
}

/**
 *	bfin_check_status - Read device status reg & clear interrupt
 *	@ap: port where the device is
 *
 *	Note: Original code is ata_check_status().
 */

static u8 bfin_check_status(struct ata_port *ap)
{
	void __iomem *base = (void __iomem *)ap->ioaddr.ctl_addr;
	return read_atapi_register(base, ATA_REG_STATUS);
}

/**
 *	bfin_check_altstatus - Read device alternate status reg
 *	@ap: port where the device is
 */

static u8 bfin_check_altstatus(struct ata_port *ap)
{
	void __iomem *base = (void __iomem *)ap->ioaddr.ctl_addr;
	return read_atapi_register(base, ATA_REG_ALTSTATUS);
}

/**
 *      bfin_ata_busy_wait - Wait for a port status register
 *      @ap: Port to wait for.
 *      @bits: bits that must be clear
 *      @max: number of 10uS waits to perform
 *
 *      Waits up to max*10 microseconds for the selected bits in the port's
 *      status register to be cleared.
 *      Returns final value of status register.
 *
 *      LOCKING:
 *      Inherited from caller.
 */
static inline u8 bfin_ata_busy_wait(struct ata_port *ap, unsigned int bits,
				unsigned int max, u8 usealtstatus)
{
	u8 status;

	do {
		udelay(10);
		if (usealtstatus)
			status = bfin_check_altstatus(ap);
		else
			status = bfin_check_status(ap);
		max--;
	} while (status != 0xff && (status & bits) && (max > 0));

	return status;
}

/**
 *	bfin_ata_busy_sleep - sleep until BSY clears, or timeout
 *	@ap: port containing status register to be polled
 *	@tmout_pat: impatience timeout in msecs
 *	@tmout: overall timeout in msecs
 *
 *	Sleep until ATA Status register bit BSY clears,
 *	or a timeout occurs.
 *
 *	RETURNS:
 *	0 on success, -errno otherwise.
 */
static int bfin_ata_busy_sleep(struct ata_port *ap,
		       long tmout_pat, unsigned long tmout)
{
	u8 status;

	status = bfin_ata_busy_wait(ap, ATA_BUSY, 300, 0);
	while (status != 0xff && (status & ATA_BUSY) && tmout_pat > 0) {
		msleep(50);
		tmout_pat -= 50;
		status = bfin_ata_busy_wait(ap, ATA_BUSY, 3, 0);
	}

	if (status != 0xff && (status & ATA_BUSY))
		printf("port is slow to respond, please be patient "
				"(Status 0x%x)\n", status);

	while (status != 0xff && (status & ATA_BUSY) && tmout_pat > 0) {
		msleep(50);
		tmout_pat -= 50;
		status = bfin_check_status(ap);
	}

	if (status == 0xff)
		return -ENODEV;

	if (status & ATA_BUSY) {
		printf("port failed to respond "
				"(%lu secs, Status 0x%x)\n",
				DIV_ROUND_UP(tmout, 1000), status);
		return -EBUSY;
	}

	return 0;
}

/**
 *	bfin_dev_select - Select device 0/1 on ATA bus
 *	@ap: ATA channel to manipulate
 *	@device: ATA device (numbered from zero) to select
 *
 *	Note: Original code is ata_sff_dev_select().
 */

static void bfin_dev_select(struct ata_port *ap, unsigned int device)
{
	void __iomem *base = (void __iomem *)ap->ioaddr.ctl_addr;
	u8 tmp;


	if (device == 0)
		tmp = ATA_DEVICE_OBS;
	else
		tmp = ATA_DEVICE_OBS | ATA_DEV1;

	write_atapi_register(base, ATA_REG_DEVICE, tmp);
	udelay(1);
}

/**
 *	bfin_devchk - PATA device presence detection
 *	@ap: ATA channel to examine
 *	@device: Device to examine (starting at zero)
 *
 *	Note: Original code is ata_devchk().
 */

static unsigned int bfin_devchk(struct ata_port *ap,
				unsigned int device)
{
	void __iomem *base = (void __iomem *)ap->ioaddr.ctl_addr;
	u8 nsect, lbal;

	bfin_dev_select(ap, device);

	write_atapi_register(base, ATA_REG_NSECT, 0x55);
	write_atapi_register(base, ATA_REG_LBAL, 0xaa);

	write_atapi_register(base, ATA_REG_NSECT, 0xaa);
	write_atapi_register(base, ATA_REG_LBAL, 0x55);

	write_atapi_register(base, ATA_REG_NSECT, 0x55);
	write_atapi_register(base, ATA_REG_LBAL, 0xaa);

	nsect = read_atapi_register(base, ATA_REG_NSECT);
	lbal = read_atapi_register(base, ATA_REG_LBAL);

	if ((nsect == 0x55) && (lbal == 0xaa))
		return 1;	/* we found a device */

	return 0;		/* nothing found */
}

/**
 *	bfin_bus_post_reset - PATA device post reset
 *
 *	Note: Original code is ata_bus_post_reset().
 */

static void bfin_bus_post_reset(struct ata_port *ap, unsigned int devmask)
{
	void __iomem *base = (void __iomem *)ap->ioaddr.ctl_addr;
	unsigned int dev0 = devmask & (1 << 0);
	unsigned int dev1 = devmask & (1 << 1);
	long deadline;

	/* if device 0 was found in ata_devchk, wait for its
	 * BSY bit to clear
	 */
	if (dev0)
		bfin_ata_busy_sleep(ap, ATA_TMOUT_BOOT_QUICK, ATA_TMOUT_BOOT);

	/* if device 1 was found in ata_devchk, wait for
	 * register access, then wait for BSY to clear
	 */
	deadline = ATA_TMOUT_BOOT;
	while (dev1) {
		u8 nsect, lbal;

		bfin_dev_select(ap, 1);
		nsect = read_atapi_register(base, ATA_REG_NSECT);
		lbal = read_atapi_register(base, ATA_REG_LBAL);
		if ((nsect == 1) && (lbal == 1))
			break;
		if (deadline <= 0) {
			dev1 = 0;
			break;
		}
		msleep(50);	/* give drive a breather */
		deadline -= 50;
	}
	if (dev1)
		bfin_ata_busy_sleep(ap, ATA_TMOUT_BOOT_QUICK, ATA_TMOUT_BOOT);

	/* is all this really necessary? */
	bfin_dev_select(ap, 0);
	if (dev1)
		bfin_dev_select(ap, 1);
	if (dev0)
		bfin_dev_select(ap, 0);
}

/**
 *	bfin_bus_softreset - PATA device software reset
 *
 *	Note: Original code is ata_bus_softreset().
 */

static unsigned int bfin_bus_softreset(struct ata_port *ap,
				       unsigned int devmask)
{
	void __iomem *base = (void __iomem *)ap->ioaddr.ctl_addr;

	/* software reset.  causes dev0 to be selected */
	write_atapi_register(base, ATA_REG_CTRL, ap->ctl_reg);
	udelay(20);
	write_atapi_register(base, ATA_REG_CTRL, ap->ctl_reg | ATA_SRST);
	udelay(20);
	write_atapi_register(base, ATA_REG_CTRL, ap->ctl_reg);

	/* spec mandates ">= 2ms" before checking status.
	 * We wait 150ms, because that was the magic delay used for
	 * ATAPI devices in Hale Landis's ATADRVR, for the period of time
	 * between when the ATA command register is written, and then
	 * status is checked.  Because waiting for "a while" before
	 * checking status is fine, post SRST, we perform this magic
	 * delay here as well.
	 *
	 * Old drivers/ide uses the 2mS rule and then waits for ready
	 */
	msleep(150);

	/* Before we perform post reset processing we want to see if
	 * the bus shows 0xFF because the odd clown forgets the D7
	 * pulldown resistor.
	 */
	if (bfin_check_status(ap) == 0xFF)
		return 0;

	bfin_bus_post_reset(ap, devmask);

	return 0;
}

/**
 *	bfin_softreset - reset host port via ATA SRST
 *	@ap: port to reset
 *
 *	Note: Original code is ata_sff_softreset().
 */

static int bfin_softreset(struct ata_port *ap)
{
	unsigned int err_mask;

	ap->dev_mask = 0;

	/* determine if device 0/1 are present.
	 * only one device is supported on one port by now.
	*/
	if (bfin_devchk(ap, 0))
		ap->dev_mask |= (1 << 0);
	else if (bfin_devchk(ap, 1))
		ap->dev_mask |= (1 << 1);
	else
		return -ENODEV;

	/* select device 0 again */
	bfin_dev_select(ap, 0);

	/* issue bus reset */
	err_mask = bfin_bus_softreset(ap, ap->dev_mask);
	if (err_mask) {
		printf("SRST failed (err_mask=0x%x)\n",
				err_mask);
		ap->dev_mask = 0;
		return -EIO;
	}

	return 0;
}

/**
 *	bfin_irq_clear - Clear ATAPI interrupt.
 *	@ap: Port associated with this ATA transaction.
 *
 *	Note: Original code is ata_sff_irq_clear().
 */

static void bfin_irq_clear(struct ata_port *ap)
{
	void __iomem *base = (void __iomem *)ap->ioaddr.ctl_addr;

	ATAPI_SET_INT_STATUS(base, ATAPI_GET_INT_STATUS(base)|ATAPI_DEV_INT
		| MULTI_DONE_INT | UDMAIN_DONE_INT | UDMAOUT_DONE_INT
		| MULTI_TERM_INT | UDMAIN_TERM_INT | UDMAOUT_TERM_INT);
}

static u8 bfin_wait_for_irq(struct ata_port *ap, unsigned int max)
{
	void __iomem *base = (void __iomem *)ap->ioaddr.ctl_addr;

	do {
		if (ATAPI_GET_INT_STATUS(base) & (ATAPI_DEV_INT
		| MULTI_DONE_INT | UDMAIN_DONE_INT | UDMAOUT_DONE_INT
		| MULTI_TERM_INT | UDMAIN_TERM_INT | UDMAOUT_TERM_INT)) {
			break;
		}
		udelay(1000);
		max--;
	} while ((max > 0));

	return max == 0;
}

/**
 *	bfin_ata_reset_port - initialize BFIN ATAPI port.
 */

static int bfin_ata_reset_port(struct ata_port *ap)
{
	void __iomem *base = (void __iomem *)ap->ioaddr.ctl_addr;
	int count;
	unsigned short status;

	/* Disable all ATAPI interrupts */
	ATAPI_SET_INT_MASK(base, 0);
	SSYNC();

	/* Assert the RESET signal 25us*/
	ATAPI_SET_CONTROL(base, ATAPI_GET_CONTROL(base) | DEV_RST);
	udelay(30);

	/* Negate the RESET signal for 2ms*/
	ATAPI_SET_CONTROL(base, ATAPI_GET_CONTROL(base) & ~DEV_RST);
	msleep(2);

	/* Wait on Busy flag to clear */
	count = 10000000;
	do {
		status = read_atapi_register(base, ATA_REG_STATUS);
	} while (--count && (status & ATA_BUSY));

	/* Enable only ATAPI Device interrupt */
	ATAPI_SET_INT_MASK(base, 1);
	SSYNC();

	return !count;
}

/**
 *
 *	Function:       bfin_config_atapi_gpio
 *
 *	Description:    Configures the ATAPI pins for use
 *
 */
static int bfin_config_atapi_gpio(struct ata_port *ap)
{
	const unsigned short pins[] = {
		P_ATAPI_RESET, P_ATAPI_DIOR, P_ATAPI_DIOW, P_ATAPI_CS0,
		P_ATAPI_CS1, P_ATAPI_DMACK, P_ATAPI_DMARQ, P_ATAPI_INTRQ,
		P_ATAPI_IORDY, P_ATAPI_D0A, P_ATAPI_D1A, P_ATAPI_D2A,
		P_ATAPI_D3A, P_ATAPI_D4A, P_ATAPI_D5A, P_ATAPI_D6A,
		P_ATAPI_D7A, P_ATAPI_D8A, P_ATAPI_D9A, P_ATAPI_D10A,
		P_ATAPI_D11A, P_ATAPI_D12A, P_ATAPI_D13A, P_ATAPI_D14A,
		P_ATAPI_D15A, P_ATAPI_A0A, P_ATAPI_A1A, P_ATAPI_A2A, 0,
	};

	peripheral_request_list(pins, "pata_bfin");

	return 0;
}

/**
 *	bfin_atapi_probe	-	attach a bfin atapi interface
 *	@pdev: platform device
 *
 *	Register a bfin atapi interface.
 *
 *
 *	Platform devices are expected to contain 2 resources per port:
 *
 *		- I/O Base (IORESOURCE_IO)
 *		- IRQ	   (IORESOURCE_IRQ)
 *
 */
static int bfin_ata_probe_port(struct ata_port *ap)
{
	if (bfin_config_atapi_gpio(ap)) {
		printf("Requesting Peripherals faild\n");
		return -EFAULT;
	}

	if (bfin_ata_reset_port(ap)) {
		printf("Fail to reset ATAPI device\n");
		return -EFAULT;
	}

	if (ap->ata_mode >= XFER_PIO_0 && ap->ata_mode <= XFER_PIO_4)
		bfin_set_piomode(ap, ap->ata_mode);
	else {
		printf("Given ATA data transfer mode is not supported.\n");
		return -EFAULT;
	}

	return 0;
}

#define ATA_SECTOR_WORDS (ATA_SECT_SIZE/2)

static void bfin_ata_identify(struct ata_port *ap, int dev)
{
	void __iomem *base = (void __iomem *)ap->ioaddr.ctl_addr;
	u8 status = 0;
	static u16 iobuf[ATA_SECTOR_WORDS];
	u64 n_sectors = 0;
	hd_driveid_t *iop = (hd_driveid_t *)iobuf;

	memset(iobuf, 0, sizeof(iobuf));

	if (!(ap->dev_mask & (1 << dev)))
		return;

	debug("port=%d dev=%d\n", ap->port_no, dev);

	bfin_dev_select(ap, dev);

	status = 0;
	/* Device Identify Command */
	write_atapi_register(base, ATA_REG_CMD, ATA_CMD_ID_ATA);
	bfin_check_altstatus(ap);
	udelay(10);

	status = bfin_ata_busy_wait(ap, ATA_BUSY, 1000, 0);
	if (status & ATA_ERR) {
		printf("\ndevice not responding\n");
		ap->dev_mask &= ~(1 << dev);
		return;
	}

	read_atapi_data(base, ATA_SECTOR_WORDS, iobuf);

	ata_swap_buf_le16(iobuf, ATA_SECTOR_WORDS);

	/* we require LBA and DMA support (bits 8 & 9 of word 49) */
	if (!ata_id_has_dma(iobuf) || !ata_id_has_lba(iobuf))
		printf("ata%u: no dma/lba\n", ap->port_no);

#ifdef DEBUG
	ata_dump_id(iobuf);
#endif

	n_sectors = ata_id_n_sectors(iobuf);

	if (n_sectors == 0) {
		ap->dev_mask &= ~(1 << dev);
		return;
	}

	ata_id_c_string(iobuf, (unsigned char *)sata_dev_desc[ap->port_no].revision,
			 ATA_ID_FW_REV, sizeof(sata_dev_desc[ap->port_no].revision));
	ata_id_c_string(iobuf, (unsigned char *)sata_dev_desc[ap->port_no].vendor,
			 ATA_ID_PROD, sizeof(sata_dev_desc[ap->port_no].vendor));
	ata_id_c_string(iobuf, (unsigned char *)sata_dev_desc[ap->port_no].product,
			 ATA_ID_SERNO, sizeof(sata_dev_desc[ap->port_no].product));

	if ((iop->config & 0x0080) == 0x0080)
		sata_dev_desc[ap->port_no].removable = 1;
	else
		sata_dev_desc[ap->port_no].removable = 0;

	sata_dev_desc[ap->port_no].lba = (u32) n_sectors;
	debug("lba=0x%lx\n", sata_dev_desc[ap->port_no].lba);

#ifdef CONFIG_LBA48
	if (iop->command_set_2 & 0x0400)
		sata_dev_desc[ap->port_no].lba48 = 1;
	else
		sata_dev_desc[ap->port_no].lba48 = 0;
#endif

	/* assuming HD */
	sata_dev_desc[ap->port_no].type = DEV_TYPE_HARDDISK;
	sata_dev_desc[ap->port_no].blksz = ATA_SECT_SIZE;
	sata_dev_desc[ap->port_no].lun = 0;	/* just to fill something in... */

	printf("PATA device#%d %s is found on ata port#%d.\n",
		ap->port_no%PATA_DEV_NUM_PER_PORT,
		sata_dev_desc[ap->port_no].vendor,
		ap->port_no/PATA_DEV_NUM_PER_PORT);
}

static void bfin_ata_set_Feature_cmd(struct ata_port *ap, int dev)
{
	void __iomem *base = (void __iomem *)ap->ioaddr.ctl_addr;
	u8 status = 0;

	if (!(ap->dev_mask & (1 << dev)))
		return;

	bfin_dev_select(ap, dev);

	write_atapi_register(base, ATA_REG_FEATURE, SETFEATURES_XFER);
	write_atapi_register(base, ATA_REG_NSECT, ap->ata_mode);
	write_atapi_register(base, ATA_REG_LBAL, 0);
	write_atapi_register(base, ATA_REG_LBAM, 0);
	write_atapi_register(base, ATA_REG_LBAH, 0);

	write_atapi_register(base, ATA_REG_DEVICE, ATA_DEVICE_OBS);
	write_atapi_register(base, ATA_REG_CMD, ATA_CMD_SET_FEATURES);

	udelay(50);
	msleep(150);

	status = bfin_ata_busy_wait(ap, ATA_BUSY, 5000, 0);
	if ((status & (ATA_BUSY | ATA_ERR))) {
		printf("Error  : status 0x%02x\n", status);
		ap->dev_mask &= ~(1 << dev);
	}
}

int scan_sata(int dev)
{
	/* dev is the index of each ata device in the system. one PATA port
	 * contains 2 devices. one element in scan_done array indicates one
	 * PATA port. device connected to one PATA port is selected by
	 * bfin_dev_select() before access.
	 */
	struct ata_port *ap = &port[dev];
	static int scan_done[(CONFIG_SYS_SATA_MAX_DEVICE+1)/PATA_DEV_NUM_PER_PORT];

	if (scan_done[dev/PATA_DEV_NUM_PER_PORT])
		return 0;

	/* Check for attached device */
	if (!bfin_ata_probe_port(ap)) {
		if (bfin_softreset(ap)) {
			/* soft reset failed, try a hard one */
			bfin_ata_reset_port(ap);
			if (bfin_softreset(ap))
				scan_done[dev/PATA_DEV_NUM_PER_PORT] = 1;
		} else {
			scan_done[dev/PATA_DEV_NUM_PER_PORT] = 1;
		}
	}
	if (scan_done[dev/PATA_DEV_NUM_PER_PORT]) {
		/* Probe device and set xfer mode */
		bfin_ata_identify(ap, dev%PATA_DEV_NUM_PER_PORT);
		bfin_ata_set_Feature_cmd(ap, dev%PATA_DEV_NUM_PER_PORT);
		init_part(&sata_dev_desc[dev]);
		return 0;
	}

	printf("PATA device#%d is not present on ATA port#%d.\n",
		ap->port_no%PATA_DEV_NUM_PER_PORT,
		ap->port_no/PATA_DEV_NUM_PER_PORT);

	return -1;
}

int init_sata(int dev)
{
	struct ata_port *ap = &port[dev];
	static u8 init_done;
	int res = 1;

	if (init_done)
		return res;

	init_done = 1;

	switch (dev/PATA_DEV_NUM_PER_PORT) {
	case 0:
		ap->ioaddr.ctl_addr = ATAPI_CONTROL;
		ap->ata_mode = CONFIG_BFIN_ATA_MODE;
		break;
	default:
		printf("Tried to scan unknown port %d.\n", dev);
		return res;
	}

	if (ap->ata_mode < XFER_PIO_0 || ap->ata_mode > XFER_PIO_4) {
		ap->ata_mode = XFER_PIO_4;
		printf("DMA mode is not supported. Set to PIO mode 4.\n");
	}

	ap->port_no = dev;
	ap->ctl_reg = 0x8;	/*Default value of control reg */

	res = 0;
	return res;
}

/* Read up to 255 sectors
 *
 * Returns sectors read
*/
static u8 do_one_read(struct ata_port *ap, u64 blknr, u8 blkcnt, u16 *buffer,
			uchar lba48)
{
	void __iomem *base = (void __iomem *)ap->ioaddr.ctl_addr;
	u8 sr = 0;
	u8 status;
	u16 err = 0;

	if (!(bfin_check_status(ap) & ATA_DRDY)) {
		printf("Device ata%d not ready\n", ap->port_no);
		return 0;
	}

	/* Set up transfer */
#ifdef CONFIG_LBA48
	if (lba48) {
		/* write high bits */
		write_atapi_register(base, ATA_REG_NSECT, 0);
		write_atapi_register(base, ATA_REG_LBAL, (blknr >> 24) & 0xFF);
		write_atapi_register(base, ATA_REG_LBAM, (blknr >> 32) & 0xFF);
		write_atapi_register(base, ATA_REG_LBAH, (blknr >> 40) & 0xFF);
	}
#endif
	write_atapi_register(base, ATA_REG_NSECT, blkcnt);
	write_atapi_register(base, ATA_REG_LBAL, (blknr >> 0) & 0xFF);
	write_atapi_register(base, ATA_REG_LBAM, (blknr >> 8) & 0xFF);
	write_atapi_register(base, ATA_REG_LBAH, (blknr >> 16) & 0xFF);

#ifdef CONFIG_LBA48
	if (lba48) {
		write_atapi_register(base, ATA_REG_DEVICE, ATA_LBA);
		write_atapi_register(base, ATA_REG_CMD, ATA_CMD_PIO_READ_EXT);
	} else
#endif
	{
		write_atapi_register(base, ATA_REG_DEVICE, ATA_LBA | ((blknr >> 24) & 0xF));
		write_atapi_register(base, ATA_REG_CMD, ATA_CMD_PIO_READ);
	}
	status = bfin_ata_busy_wait(ap, ATA_BUSY, 500000, 1);

	if (status & (ATA_BUSY | ATA_ERR)) {
		printf("Device %d not responding status 0x%x.\n", ap->port_no, status);
		err = read_atapi_register(base, ATA_REG_ERR);
		printf("Error reg = 0x%x\n", err);
		return sr;
	}

	while (blkcnt--) {
		if (bfin_wait_for_irq(ap, 500)) {
			printf("ata%u irq failed\n", ap->port_no);
			return sr;
		}

		status = bfin_check_status(ap);
		if (status & ATA_ERR) {
			err = read_atapi_register(base, ATA_REG_ERR);
			printf("ata%u error %d\n", ap->port_no, err);
			return sr;
		}
		bfin_irq_clear(ap);

		/* Read one sector */
		read_atapi_data(base, ATA_SECTOR_WORDS, buffer);
		buffer += ATA_SECTOR_WORDS;
		sr++;
	}

	return sr;
}

ulong sata_read(int dev, ulong block, lbaint_t blkcnt, void *buff)
{
	struct ata_port *ap = &port[dev];
	ulong n = 0, sread;
	u16 *buffer = (u16 *) buff;
	u8 status = 0;
	u64 blknr = (u64) block;
	unsigned char lba48 = 0;

#ifdef CONFIG_LBA48
	if (blknr > 0xfffffff) {
		if (!sata_dev_desc[dev].lba48) {
			printf("Drive doesn't support 48-bit addressing\n");
			return 0;
		}
		/* more than 28 bits used, use 48bit mode */
		lba48 = 1;
	}
#endif
	bfin_dev_select(ap, dev%PATA_DEV_NUM_PER_PORT);

	while (blkcnt > 0) {

		if (blkcnt > 255)
			sread = 255;
		else
			sread = blkcnt;

		status = do_one_read(ap, blknr, sread, buffer, lba48);
		if (status != sread) {
			printf("Read failed\n");
			return n;
		}

		blkcnt -= sread;
		blknr += sread;
		n += sread;
		buffer += sread * ATA_SECTOR_WORDS;
	}
	return n;
}

ulong sata_write(int dev, ulong block, lbaint_t blkcnt, const void *buff)
{
	struct ata_port *ap = &port[dev];
	void __iomem *base = (void __iomem *)ap->ioaddr.ctl_addr;
	ulong n = 0;
	u16 *buffer = (u16 *) buff;
	unsigned char status = 0;
	u64 blknr = (u64) block;
#ifdef CONFIG_LBA48
	unsigned char lba48 = 0;

	if (blknr > 0xfffffff) {
		if (!sata_dev_desc[dev].lba48) {
			printf("Drive doesn't support 48-bit addressing\n");
			return 0;
		}
		/* more than 28 bits used, use 48bit mode */
		lba48 = 1;
	}
#endif

	bfin_dev_select(ap, dev%PATA_DEV_NUM_PER_PORT);

	while (blkcnt-- > 0) {
		status = bfin_ata_busy_wait(ap, ATA_BUSY, 50000, 0);
		if (status & ATA_BUSY) {
			printf("ata%u failed to respond\n", ap->port_no);
			return n;
		}
#ifdef CONFIG_LBA48
		if (lba48) {
			/* write high bits */
			write_atapi_register(base, ATA_REG_NSECT, 0);
			write_atapi_register(base, ATA_REG_LBAL,
				(blknr >> 24) & 0xFF);
			write_atapi_register(base, ATA_REG_LBAM,
				(blknr >> 32) & 0xFF);
			write_atapi_register(base, ATA_REG_LBAH,
				(blknr >> 40) & 0xFF);
		}
#endif
		write_atapi_register(base, ATA_REG_NSECT, 1);
		write_atapi_register(base, ATA_REG_LBAL, (blknr >> 0) & 0xFF);
		write_atapi_register(base, ATA_REG_LBAM, (blknr >> 8) & 0xFF);
		write_atapi_register(base, ATA_REG_LBAH, (blknr >> 16) & 0xFF);
#ifdef CONFIG_LBA48
		if (lba48) {
			write_atapi_register(base, ATA_REG_DEVICE, ATA_LBA);
			write_atapi_register(base, ATA_REG_CMD,
				ATA_CMD_PIO_WRITE_EXT);
		} else
#endif
		{
			write_atapi_register(base, ATA_REG_DEVICE,
				ATA_LBA | ((blknr >> 24) & 0xF));
			write_atapi_register(base, ATA_REG_CMD,
				ATA_CMD_PIO_WRITE);
		}

		/*may take up to 5 sec */
		status = bfin_ata_busy_wait(ap, ATA_BUSY, 50000, 0);
		if ((status & (ATA_DRQ | ATA_BUSY | ATA_ERR)) != ATA_DRQ) {
			printf("Error no DRQ dev %d blk %ld: sts 0x%02x\n",
				ap->port_no, (ulong) blknr, status);
			return n;
		}

		write_atapi_data(base, ATA_SECTOR_WORDS, buffer);
		bfin_check_altstatus(ap);
		udelay(1);

		++n;
		++blknr;
		buffer += ATA_SECTOR_WORDS;
	}
	return n;
}
