// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000-2011
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#define LOG_CATEGORY UCLASS_IDE

#include <common.h>
#include <ata.h>
#include <blk.h>
#include <dm.h>
#include <ide.h>
#include <log.h>
#include <part.h>
#include <watchdog.h>
#include <asm/io.h>
#include <linux/delay.h>

#ifdef __PPC__
# define EIEIO		__asm__ volatile ("eieio")
# define SYNC		__asm__ volatile ("sync")
#else
# define EIEIO		/* nothing */
# define SYNC		/* nothing */
#endif

/* Current offset for IDE0 / IDE1 bus access	*/
ulong ide_bus_offset[CONFIG_SYS_IDE_MAXBUS] = {
#if defined(CONFIG_SYS_ATA_IDE0_OFFSET)
	CONFIG_SYS_ATA_IDE0_OFFSET,
#endif
#if defined(CONFIG_SYS_ATA_IDE1_OFFSET) && (CONFIG_SYS_IDE_MAXBUS > 1)
	CONFIG_SYS_ATA_IDE1_OFFSET,
#endif
};

static int ide_bus_ok[CONFIG_SYS_IDE_MAXBUS];

struct blk_desc ide_dev_desc[CONFIG_SYS_IDE_MAXDEVICE];

#define IDE_TIME_OUT	2000	/* 2 sec timeout */

#define ATAPI_TIME_OUT	7000	/* 7 sec timeout (5 sec seems to work...) */

#define IDE_SPIN_UP_TIME_OUT 5000 /* 5 sec spin-up timeout */

#ifdef CONFIG_IDE_RESET
extern void ide_set_reset(int idereset);

static void ide_reset(void)
{
	int i;

	for (i = 0; i < CONFIG_SYS_IDE_MAXBUS; ++i)
		ide_bus_ok[i] = 0;
	for (i = 0; i < CONFIG_SYS_IDE_MAXDEVICE; ++i)
		ide_dev_desc[i].type = DEV_TYPE_UNKNOWN;

	ide_set_reset(1);	/* assert reset */

	/* the reset signal shall be asserted for et least 25 us */
	udelay(25);

	WATCHDOG_RESET();

	/* de-assert RESET signal */
	ide_set_reset(0);

	/* wait 250 ms */
	for (i = 0; i < 250; ++i)
		udelay(1000);
}
#else
#define ide_reset()	/* dummy */
#endif /* CONFIG_IDE_RESET */

/*
 * Wait until Busy bit is off, or timeout (in ms)
 * Return last status
 */
static uchar ide_wait(int dev, ulong t)
{
	ulong delay = 10 * t;	/* poll every 100 us */
	uchar c;

	while ((c = ide_inb(dev, ATA_STATUS)) & ATA_STAT_BUSY) {
		udelay(100);
		if (delay-- == 0)
			break;
	}
	return c;
}

/*
 * copy src to dest, skipping leading and trailing blanks and null
 * terminate the string
 * "len" is the size of available memory including the terminating '\0'
 */
static void ident_cpy(unsigned char *dst, unsigned char *src,
		      unsigned int len)
{
	unsigned char *end, *last;

	last = dst;
	end = src + len - 1;

	/* reserve space for '\0' */
	if (len < 2)
		goto OUT;

	/* skip leading white space */
	while ((*src) && (src < end) && (*src == ' '))
		++src;

	/* copy string, omitting trailing white space */
	while ((*src) && (src < end)) {
		*dst++ = *src;
		if (*src++ != ' ')
			last = dst;
	}
OUT:
	*last = '\0';
}

#ifdef CONFIG_ATAPI
/****************************************************************************
 * ATAPI Support
 */

/* since ATAPI may use commands with not 4 bytes alligned length
 * we have our own transfer functions, 2 bytes alligned */
__weak void ide_output_data_shorts(int dev, ushort *sect_buf, int shorts)
{
	uintptr_t paddr = (ATA_CURR_BASE(dev) + ATA_DATA_REG);
	ushort *dbuf;

	dbuf = (ushort *)sect_buf;

	debug("in output data shorts base for read is %p\n", (void *)paddr);

	while (shorts--) {
		EIEIO;
		outw(cpu_to_le16(*dbuf++), paddr);
	}
}

__weak void ide_input_data_shorts(int dev, ushort *sect_buf, int shorts)
{
	uintptr_t paddr = (ATA_CURR_BASE(dev) + ATA_DATA_REG);
	ushort *dbuf;

	dbuf = (ushort *)sect_buf;

	debug("in input data shorts base for read is %p\n", (void *)paddr);

	while (shorts--) {
		EIEIO;
		*dbuf++ = le16_to_cpu(inw(paddr));
	}
}

/*
 * Wait until (Status & mask) == res, or timeout (in ms)
 * Return last status
 * This is used since some ATAPI CD ROMs clears their Busy Bit first
 * and then they set their DRQ Bit
 */
static uchar atapi_wait_mask(int dev, ulong t, uchar mask, uchar res)
{
	ulong delay = 10 * t;	/* poll every 100 us */
	uchar c;

	/* prevents to read the status before valid */
	c = ide_inb(dev, ATA_DEV_CTL);

	while (((c = ide_inb(dev, ATA_STATUS)) & mask) != res) {
		/* break if error occurs (doesn't make sense to wait more) */
		if ((c & ATA_STAT_ERR) == ATA_STAT_ERR)
			break;
		udelay(100);
		if (delay-- == 0)
			break;
	}
	return c;
}

/*
 * issue an atapi command
 */
unsigned char atapi_issue(int device, unsigned char *ccb, int ccblen,
			  unsigned char *buffer, int buflen)
{
	unsigned char c, err, mask, res;
	int n;

	/* Select device
	 */
	mask = ATA_STAT_BUSY | ATA_STAT_DRQ;
	res = 0;
	ide_outb(device, ATA_DEV_HD, ATA_LBA | ATA_DEVICE(device));
	c = atapi_wait_mask(device, ATAPI_TIME_OUT, mask, res);
	if ((c & mask) != res) {
		printf("ATAPI_ISSUE: device %d not ready status %X\n", device,
		       c);
		err = 0xFF;
		goto AI_OUT;
	}
	/* write taskfile */
	ide_outb(device, ATA_ERROR_REG, 0);	/* no DMA, no overlaped */
	ide_outb(device, ATA_SECT_CNT, 0);
	ide_outb(device, ATA_SECT_NUM, 0);
	ide_outb(device, ATA_CYL_LOW, (unsigned char) (buflen & 0xFF));
	ide_outb(device, ATA_CYL_HIGH,
		 (unsigned char) ((buflen >> 8) & 0xFF));
	ide_outb(device, ATA_DEV_HD, ATA_LBA | ATA_DEVICE(device));

	ide_outb(device, ATA_COMMAND, ATA_CMD_PACKET);
	udelay(50);

	mask = ATA_STAT_DRQ | ATA_STAT_BUSY | ATA_STAT_ERR;
	res = ATA_STAT_DRQ;
	c = atapi_wait_mask(device, ATAPI_TIME_OUT, mask, res);

	if ((c & mask) != res) {	/* DRQ must be 1, BSY 0 */
		printf("ATAPI_ISSUE: Error (no IRQ) before sending ccb dev %d status 0x%02x\n",
		       device, c);
		err = 0xFF;
		goto AI_OUT;
	}

	/* write command block */
	ide_output_data_shorts(device, (unsigned short *)ccb, ccblen / 2);

	/* ATAPI Command written wait for completition */
	udelay(5000);		/* device must set bsy */

	mask = ATA_STAT_DRQ | ATA_STAT_BUSY | ATA_STAT_ERR;
	/*
	 * if no data wait for DRQ = 0 BSY = 0
	 * if data wait for DRQ = 1 BSY = 0
	 */
	res = 0;
	if (buflen)
		res = ATA_STAT_DRQ;
	c = atapi_wait_mask(device, ATAPI_TIME_OUT, mask, res);
	if ((c & mask) != res) {
		if (c & ATA_STAT_ERR) {
			err = (ide_inb(device, ATA_ERROR_REG)) >> 4;
			debug("atapi_issue 1 returned sense key %X status %02X\n",
			      err, c);
		} else {
			printf("ATAPI_ISSUE: (no DRQ) after sending ccb (%x)  status 0x%02x\n",
			       ccb[0], c);
			err = 0xFF;
		}
		goto AI_OUT;
	}
	n = ide_inb(device, ATA_CYL_HIGH);
	n <<= 8;
	n += ide_inb(device, ATA_CYL_LOW);
	if (n > buflen) {
		printf("ERROR, transfer bytes %d requested only %d\n", n,
		       buflen);
		err = 0xff;
		goto AI_OUT;
	}
	if ((n == 0) && (buflen < 0)) {
		printf("ERROR, transfer bytes %d requested %d\n", n, buflen);
		err = 0xff;
		goto AI_OUT;
	}
	if (n != buflen) {
		debug("WARNING, transfer bytes %d not equal with requested %d\n",
		      n, buflen);
	}
	if (n != 0) {		/* data transfer */
		debug("ATAPI_ISSUE: %d Bytes to transfer\n", n);
		/* we transfer shorts */
		n >>= 1;
		/* ok now decide if it is an in or output */
		if ((ide_inb(device, ATA_SECT_CNT) & 0x02) == 0) {
			debug("Write to device\n");
			ide_output_data_shorts(device, (unsigned short *)buffer,
					       n);
		} else {
			debug("Read from device @ %p shorts %d\n", buffer, n);
			ide_input_data_shorts(device, (unsigned short *)buffer,
					      n);
		}
	}
	udelay(5000);		/* seems that some CD ROMs need this... */
	mask = ATA_STAT_BUSY | ATA_STAT_ERR;
	res = 0;
	c = atapi_wait_mask(device, ATAPI_TIME_OUT, mask, res);
	if ((c & ATA_STAT_ERR) == ATA_STAT_ERR) {
		err = (ide_inb(device, ATA_ERROR_REG) >> 4);
		debug("atapi_issue 2 returned sense key %X status %X\n", err,
		      c);
	} else {
		err = 0;
	}
AI_OUT:
	return err;
}

/*
 * sending the command to atapi_issue. If an status other than good
 * returns, an request_sense will be issued
 */

#define ATAPI_DRIVE_NOT_READY	100
#define ATAPI_UNIT_ATTN		10

unsigned char atapi_issue_autoreq(int device,
				  unsigned char *ccb,
				  int ccblen,
				  unsigned char *buffer, int buflen)
{
	unsigned char sense_data[18], sense_ccb[12];
	unsigned char res, key, asc, ascq;
	int notready, unitattn;

	unitattn = ATAPI_UNIT_ATTN;
	notready = ATAPI_DRIVE_NOT_READY;

retry:
	res = atapi_issue(device, ccb, ccblen, buffer, buflen);
	if (res == 0)
		return 0;	/* Ok */

	if (res == 0xFF)
		return 0xFF;	/* error */

	debug("(auto_req)atapi_issue returned sense key %X\n", res);

	memset(sense_ccb, 0, sizeof(sense_ccb));
	memset(sense_data, 0, sizeof(sense_data));
	sense_ccb[0] = ATAPI_CMD_REQ_SENSE;
	sense_ccb[4] = 18;	/* allocation Length */

	res = atapi_issue(device, sense_ccb, 12, sense_data, 18);
	key = (sense_data[2] & 0xF);
	asc = (sense_data[12]);
	ascq = (sense_data[13]);

	debug("ATAPI_CMD_REQ_SENSE returned %x\n", res);
	debug(" Sense page: %02X key %02X ASC %02X ASCQ %02X\n",
	      sense_data[0], key, asc, ascq);

	if ((key == 0))
		return 0;	/* ok device ready */

	if ((key == 6) || (asc == 0x29) || (asc == 0x28)) { /* Unit Attention */
		if (unitattn-- > 0) {
			udelay(200 * 1000);
			goto retry;
		}
		printf("Unit Attention, tried %d\n", ATAPI_UNIT_ATTN);
		goto error;
	}
	if ((asc == 0x4) && (ascq == 0x1)) {
		/* not ready, but will be ready soon */
		if (notready-- > 0) {
			udelay(200 * 1000);
			goto retry;
		}
		printf("Drive not ready, tried %d times\n",
		       ATAPI_DRIVE_NOT_READY);
		goto error;
	}
	if (asc == 0x3a) {
		debug("Media not present\n");
		goto error;
	}

	printf("ERROR: Unknown Sense key %02X ASC %02X ASCQ %02X\n", key, asc,
	       ascq);
error:
	debug("ERROR Sense key %02X ASC %02X ASCQ %02X\n", key, asc, ascq);
	return 0xFF;
}

/*
 * atapi_read:
 * we transfer only one block per command, since the multiple DRQ per
 * command is not yet implemented
 */
#define ATAPI_READ_MAX_BYTES	2048	/* we read max 2kbytes */
#define ATAPI_READ_BLOCK_SIZE	2048	/* assuming CD part */
#define ATAPI_READ_MAX_BLOCK	(ATAPI_READ_MAX_BYTES/ATAPI_READ_BLOCK_SIZE)

ulong atapi_read(struct blk_desc *block_dev, lbaint_t blknr, lbaint_t blkcnt,
		 void *buffer)
{
	int device = block_dev->devnum;
	ulong n = 0;
	unsigned char ccb[12];	/* Command descriptor block */
	ulong cnt;

	debug("atapi_read dev %d start " LBAF " blocks " LBAF
	      " buffer at %lX\n", device, blknr, blkcnt, (ulong) buffer);

	do {
		if (blkcnt > ATAPI_READ_MAX_BLOCK)
			cnt = ATAPI_READ_MAX_BLOCK;
		else
			cnt = blkcnt;

		ccb[0] = ATAPI_CMD_READ_12;
		ccb[1] = 0;	/* reserved */
		ccb[2] = (unsigned char) (blknr >> 24) & 0xFF;	/* MSB Block */
		ccb[3] = (unsigned char) (blknr >> 16) & 0xFF;	/*  */
		ccb[4] = (unsigned char) (blknr >> 8) & 0xFF;
		ccb[5] = (unsigned char) blknr & 0xFF;	/* LSB Block */
		ccb[6] = (unsigned char) (cnt >> 24) & 0xFF; /* MSB Block cnt */
		ccb[7] = (unsigned char) (cnt >> 16) & 0xFF;
		ccb[8] = (unsigned char) (cnt >> 8) & 0xFF;
		ccb[9] = (unsigned char) cnt & 0xFF;	/* LSB Block */
		ccb[10] = 0;	/* reserved */
		ccb[11] = 0;	/* reserved */

		if (atapi_issue_autoreq(device, ccb, 12,
					(unsigned char *)buffer,
					cnt * ATAPI_READ_BLOCK_SIZE)
		    == 0xFF) {
			return n;
		}
		n += cnt;
		blkcnt -= cnt;
		blknr += cnt;
		buffer += (cnt * ATAPI_READ_BLOCK_SIZE);
	} while (blkcnt > 0);
	return n;
}

static void atapi_inquiry(struct blk_desc *dev_desc)
{
	unsigned char ccb[12];	/* Command descriptor block */
	unsigned char iobuf[64];	/* temp buf */
	unsigned char c;
	int device;

	device = dev_desc->devnum;
	dev_desc->type = DEV_TYPE_UNKNOWN;	/* not yet valid */
#ifndef CONFIG_BLK
	dev_desc->block_read = atapi_read;
#endif

	memset(ccb, 0, sizeof(ccb));
	memset(iobuf, 0, sizeof(iobuf));

	ccb[0] = ATAPI_CMD_INQUIRY;
	ccb[4] = 40;		/* allocation Legnth */
	c = atapi_issue_autoreq(device, ccb, 12, (unsigned char *)iobuf, 40);

	debug("ATAPI_CMD_INQUIRY returned %x\n", c);
	if (c != 0)
		return;

	/* copy device ident strings */
	ident_cpy((unsigned char *)dev_desc->vendor, &iobuf[8], 8);
	ident_cpy((unsigned char *)dev_desc->product, &iobuf[16], 16);
	ident_cpy((unsigned char *)dev_desc->revision, &iobuf[32], 5);

	dev_desc->lun = 0;
	dev_desc->lba = 0;
	dev_desc->blksz = 0;
	dev_desc->log2blksz = LOG2_INVALID(typeof(dev_desc->log2blksz));
	dev_desc->type = iobuf[0] & 0x1f;

	if ((iobuf[1] & 0x80) == 0x80)
		dev_desc->removable = 1;
	else
		dev_desc->removable = 0;

	memset(ccb, 0, sizeof(ccb));
	memset(iobuf, 0, sizeof(iobuf));
	ccb[0] = ATAPI_CMD_START_STOP;
	ccb[4] = 0x03;		/* start */

	c = atapi_issue_autoreq(device, ccb, 12, (unsigned char *)iobuf, 0);

	debug("ATAPI_CMD_START_STOP returned %x\n", c);
	if (c != 0)
		return;

	memset(ccb, 0, sizeof(ccb));
	memset(iobuf, 0, sizeof(iobuf));
	c = atapi_issue_autoreq(device, ccb, 12, (unsigned char *)iobuf, 0);

	debug("ATAPI_CMD_UNIT_TEST_READY returned %x\n", c);
	if (c != 0)
		return;

	memset(ccb, 0, sizeof(ccb));
	memset(iobuf, 0, sizeof(iobuf));
	ccb[0] = ATAPI_CMD_READ_CAP;
	c = atapi_issue_autoreq(device, ccb, 12, (unsigned char *)iobuf, 8);
	debug("ATAPI_CMD_READ_CAP returned %x\n", c);
	if (c != 0)
		return;

	debug("Read Cap: LBA %02X%02X%02X%02X blksize %02X%02X%02X%02X\n",
	      iobuf[0], iobuf[1], iobuf[2], iobuf[3],
	      iobuf[4], iobuf[5], iobuf[6], iobuf[7]);

	dev_desc->lba = ((unsigned long) iobuf[0] << 24) +
		((unsigned long) iobuf[1] << 16) +
		((unsigned long) iobuf[2] << 8) + ((unsigned long) iobuf[3]);
	dev_desc->blksz = ((unsigned long) iobuf[4] << 24) +
		((unsigned long) iobuf[5] << 16) +
		((unsigned long) iobuf[6] << 8) + ((unsigned long) iobuf[7]);
	dev_desc->log2blksz = LOG2(dev_desc->blksz);
#ifdef CONFIG_LBA48
	/* ATAPI devices cannot use 48bit addressing (ATA/ATAPI v7) */
	dev_desc->lba48 = 0;
#endif
	return;
}

#endif /* CONFIG_ATAPI */

static void ide_ident(struct blk_desc *dev_desc)
{
	unsigned char c;
	hd_driveid_t iop;

#ifdef CONFIG_ATAPI
	int retries = 0;
#endif
	int device;

	device = dev_desc->devnum;
	printf("  Device %d: ", device);

	/* Select device
	 */
	ide_outb(device, ATA_DEV_HD, ATA_LBA | ATA_DEVICE(device));
	dev_desc->if_type = IF_TYPE_IDE;
#ifdef CONFIG_ATAPI

	retries = 0;

	/* Warning: This will be tricky to read */
	while (retries <= 1) {
		/* check signature */
		if ((ide_inb(device, ATA_SECT_CNT) == 0x01) &&
		    (ide_inb(device, ATA_SECT_NUM) == 0x01) &&
		    (ide_inb(device, ATA_CYL_LOW) == 0x14) &&
		    (ide_inb(device, ATA_CYL_HIGH) == 0xEB)) {
			/* ATAPI Signature found */
			dev_desc->if_type = IF_TYPE_ATAPI;
			/*
			 * Start Ident Command
			 */
			ide_outb(device, ATA_COMMAND, ATA_CMD_ID_ATAPI);
			/*
			 * Wait for completion - ATAPI devices need more time
			 * to become ready
			 */
			c = ide_wait(device, ATAPI_TIME_OUT);
		} else
#endif
		{
			/*
			 * Start Ident Command
			 */
			ide_outb(device, ATA_COMMAND, ATA_CMD_ID_ATA);

			/*
			 * Wait for completion
			 */
			c = ide_wait(device, IDE_TIME_OUT);
		}

		if (((c & ATA_STAT_DRQ) == 0) ||
		    ((c & (ATA_STAT_FAULT | ATA_STAT_ERR)) != 0)) {
#ifdef CONFIG_ATAPI
			{
				/*
				 * Need to soft reset the device
				 * in case it's an ATAPI...
				 */
				debug("Retrying...\n");
				ide_outb(device, ATA_DEV_HD,
					 ATA_LBA | ATA_DEVICE(device));
				udelay(100000);
				ide_outb(device, ATA_COMMAND, 0x08);
				udelay(500000);	/* 500 ms */
			}
			/*
			 * Select device
			 */
			ide_outb(device, ATA_DEV_HD,
				 ATA_LBA | ATA_DEVICE(device));
			retries++;
#else
			return;
#endif
		}
#ifdef CONFIG_ATAPI
		else
			break;
	}			/* see above - ugly to read */

	if (retries == 2)	/* Not found */
		return;
#endif

	ide_input_swap_data(device, (ulong *)&iop, ATA_SECTORWORDS);

	ident_cpy((unsigned char *)dev_desc->revision, iop.fw_rev,
		  sizeof(dev_desc->revision));
	ident_cpy((unsigned char *)dev_desc->vendor, iop.model,
		  sizeof(dev_desc->vendor));
	ident_cpy((unsigned char *)dev_desc->product, iop.serial_no,
		  sizeof(dev_desc->product));

	if ((iop.config & 0x0080) == 0x0080)
		dev_desc->removable = 1;
	else
		dev_desc->removable = 0;

#ifdef CONFIG_ATAPI
	if (dev_desc->if_type == IF_TYPE_ATAPI) {
		atapi_inquiry(dev_desc);
		return;
	}
#endif /* CONFIG_ATAPI */

	iop.lba_capacity[0] = be16_to_cpu(iop.lba_capacity[0]);
	iop.lba_capacity[1] = be16_to_cpu(iop.lba_capacity[1]);
	dev_desc->lba =
			((unsigned long)iop.lba_capacity[0]) |
			((unsigned long)iop.lba_capacity[1] << 16);

#ifdef CONFIG_LBA48
	if (iop.command_set_2 & 0x0400) {	/* LBA 48 support */
		dev_desc->lba48 = 1;
		for (int i = 0; i < 4; i++)
			iop.lba48_capacity[i] = be16_to_cpu(iop.lba48_capacity[i]);
		dev_desc->lba =
			((unsigned long long)iop.lba48_capacity[0] |
			((unsigned long long)iop.lba48_capacity[1] << 16) |
			((unsigned long long)iop.lba48_capacity[2] << 32) |
			((unsigned long long)iop.lba48_capacity[3] << 48));
	} else {
		dev_desc->lba48 = 0;
	}
#endif /* CONFIG_LBA48 */
	/* assuming HD */
	dev_desc->type = DEV_TYPE_HARDDISK;
	dev_desc->blksz = ATA_BLOCKSIZE;
	dev_desc->log2blksz = LOG2(dev_desc->blksz);
	dev_desc->lun = 0;	/* just to fill something in... */

#if 0				/* only used to test the powersaving mode,
				 * if enabled, the drive goes after 5 sec
				 * in standby mode */
	ide_outb(device, ATA_DEV_HD, ATA_LBA | ATA_DEVICE(device));
	c = ide_wait(device, IDE_TIME_OUT);
	ide_outb(device, ATA_SECT_CNT, 1);
	ide_outb(device, ATA_LBA_LOW, 0);
	ide_outb(device, ATA_LBA_MID, 0);
	ide_outb(device, ATA_LBA_HIGH, 0);
	ide_outb(device, ATA_DEV_HD, ATA_LBA | ATA_DEVICE(device));
	ide_outb(device, ATA_COMMAND, 0xe3);
	udelay(50);
	c = ide_wait(device, IDE_TIME_OUT);	/* can't take over 500 ms */
#endif
}

__weak void ide_outb(int dev, int port, unsigned char val)
{
	debug("ide_outb (dev= %d, port= 0x%x, val= 0x%02x) : @ 0x%08lx\n",
	      dev, port, val, ATA_CURR_BASE(dev) + port);

	outb(val, ATA_CURR_BASE(dev) + port);
}

__weak unsigned char ide_inb(int dev, int port)
{
	uchar val;

	val = inb(ATA_CURR_BASE(dev) + port);

	debug("ide_inb (dev= %d, port= 0x%x) : @ 0x%08lx -> 0x%02x\n",
	      dev, port, ATA_CURR_BASE(dev) + port, val);
	return val;
}

void ide_init(void)
{
	unsigned char c;
	int i, bus;

	WATCHDOG_RESET();

	/* ATAPI Drives seems to need a proper IDE Reset */
	ide_reset();

	/*
	 * Wait for IDE to get ready.
	 * According to spec, this can take up to 31 seconds!
	 */
	for (bus = 0; bus < CONFIG_SYS_IDE_MAXBUS; ++bus) {
		int dev =
			bus * (CONFIG_SYS_IDE_MAXDEVICE /
			       CONFIG_SYS_IDE_MAXBUS);

		printf("Bus %d: ", bus);

		ide_bus_ok[bus] = 0;

		/* Select device
		 */
		udelay(100000);	/* 100 ms */
		ide_outb(dev, ATA_DEV_HD, ATA_LBA | ATA_DEVICE(dev));
		udelay(100000);	/* 100 ms */
		i = 0;
		do {
			udelay(10000);	/* 10 ms */

			c = ide_inb(dev, ATA_STATUS);
			i++;
			if (i > (ATA_RESET_TIME * 100)) {
				puts("** Timeout **\n");
				return;
			}
			if ((i >= 100) && ((i % 100) == 0))
				putc('.');

		} while (c & ATA_STAT_BUSY);

		if (c & (ATA_STAT_BUSY | ATA_STAT_FAULT)) {
			puts("not available  ");
			debug("Status = 0x%02X ", c);
#ifndef CONFIG_ATAPI		/* ATAPI Devices do not set DRDY */
		} else if ((c & ATA_STAT_READY) == 0) {
			puts("not available  ");
			debug("Status = 0x%02X ", c);
#endif
		} else {
			puts("OK ");
			ide_bus_ok[bus] = 1;
		}
		WATCHDOG_RESET();
	}

	putc('\n');

	for (i = 0; i < CONFIG_SYS_IDE_MAXDEVICE; ++i) {
		ide_dev_desc[i].type = DEV_TYPE_UNKNOWN;
		ide_dev_desc[i].if_type = IF_TYPE_IDE;
		ide_dev_desc[i].devnum = i;
		ide_dev_desc[i].part_type = PART_TYPE_UNKNOWN;
		ide_dev_desc[i].blksz = 0;
		ide_dev_desc[i].log2blksz =
			LOG2_INVALID(typeof(ide_dev_desc[i].log2blksz));
		ide_dev_desc[i].lba = 0;
#ifndef CONFIG_BLK
		ide_dev_desc[i].block_read = ide_read;
		ide_dev_desc[i].block_write = ide_write;
#endif
		if (!ide_bus_ok[IDE_BUS(i)])
			continue;
		ide_ident(&ide_dev_desc[i]);
		dev_print(&ide_dev_desc[i]);

#ifndef CONFIG_BLK
		if ((ide_dev_desc[i].lba > 0) && (ide_dev_desc[i].blksz > 0)) {
			/* initialize partition type */
			part_init(&ide_dev_desc[i]);
		}
#endif
	}
	WATCHDOG_RESET();

#ifdef CONFIG_BLK
	struct udevice *dev;

	uclass_first_device(UCLASS_IDE, &dev);
#endif
}

__weak void ide_input_swap_data(int dev, ulong *sect_buf, int words)
{
	uintptr_t paddr = (ATA_CURR_BASE(dev) + ATA_DATA_REG);
	ushort *dbuf = (ushort *)sect_buf;

	debug("in input swap data base for read is %p\n", (void *)paddr);

	while (words--) {
		EIEIO;
		*dbuf++ = be16_to_cpu(inw(paddr));
		EIEIO;
		*dbuf++ = be16_to_cpu(inw(paddr));
	}
}

__weak void ide_output_data(int dev, const ulong *sect_buf, int words)
{
	uintptr_t paddr = (ATA_CURR_BASE(dev) + ATA_DATA_REG);
	ushort *dbuf;

	dbuf = (ushort *)sect_buf;
	while (words--) {
		EIEIO;
		outw(cpu_to_le16(*dbuf++), paddr);
		EIEIO;
		outw(cpu_to_le16(*dbuf++), paddr);
	}
}

__weak void ide_input_data(int dev, ulong *sect_buf, int words)
{
	uintptr_t paddr = (ATA_CURR_BASE(dev) + ATA_DATA_REG);
	ushort *dbuf;

	dbuf = (ushort *)sect_buf;

	debug("in input data base for read is %p\n", (void *)paddr);

	while (words--) {
		EIEIO;
		*dbuf++ = le16_to_cpu(inw(paddr));
		EIEIO;
		*dbuf++ = le16_to_cpu(inw(paddr));
	}
}

#ifdef CONFIG_BLK
ulong ide_read(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt,
	       void *buffer)
#else
ulong ide_read(struct blk_desc *block_dev, lbaint_t blknr, lbaint_t blkcnt,
	       void *buffer)
#endif
{
#ifdef CONFIG_BLK
	struct blk_desc *block_dev = dev_get_uclass_plat(dev);
#endif
	int device = block_dev->devnum;
	ulong n = 0;
	unsigned char c;
	unsigned char pwrsave = 0;	/* power save */

#ifdef CONFIG_LBA48
	unsigned char lba48 = 0;

	if (blknr & 0x0000fffff0000000ULL) {
		/* more than 28 bits used, use 48bit mode */
		lba48 = 1;
	}
#endif
	debug("ide_read dev %d start " LBAF ", blocks " LBAF " buffer at %lX\n",
	      device, blknr, blkcnt, (ulong) buffer);

	/* Select device
	 */
	ide_outb(device, ATA_DEV_HD, ATA_LBA | ATA_DEVICE(device));
	c = ide_wait(device, IDE_TIME_OUT);

	if (c & ATA_STAT_BUSY) {
		printf("IDE read: device %d not ready\n", device);
		goto IDE_READ_E;
	}

	/* first check if the drive is in Powersaving mode, if yes,
	 * increase the timeout value */
	ide_outb(device, ATA_COMMAND, ATA_CMD_CHK_POWER);
	udelay(50);

	c = ide_wait(device, IDE_TIME_OUT);	/* can't take over 500 ms */

	if (c & ATA_STAT_BUSY) {
		printf("IDE read: device %d not ready\n", device);
		goto IDE_READ_E;
	}
	if ((c & ATA_STAT_ERR) == ATA_STAT_ERR) {
		printf("No Powersaving mode %X\n", c);
	} else {
		c = ide_inb(device, ATA_SECT_CNT);
		debug("Powersaving %02X\n", c);
		if (c == 0)
			pwrsave = 1;
	}


	while (blkcnt-- > 0) {
		c = ide_wait(device, IDE_TIME_OUT);

		if (c & ATA_STAT_BUSY) {
			printf("IDE read: device %d not ready\n", device);
			break;
		}
#ifdef CONFIG_LBA48
		if (lba48) {
			/* write high bits */
			ide_outb(device, ATA_SECT_CNT, 0);
			ide_outb(device, ATA_LBA_LOW, (blknr >> 24) & 0xFF);
#ifdef CONFIG_SYS_64BIT_LBA
			ide_outb(device, ATA_LBA_MID, (blknr >> 32) & 0xFF);
			ide_outb(device, ATA_LBA_HIGH, (blknr >> 40) & 0xFF);
#else
			ide_outb(device, ATA_LBA_MID, 0);
			ide_outb(device, ATA_LBA_HIGH, 0);
#endif
		}
#endif
		ide_outb(device, ATA_SECT_CNT, 1);
		ide_outb(device, ATA_LBA_LOW, (blknr >> 0) & 0xFF);
		ide_outb(device, ATA_LBA_MID, (blknr >> 8) & 0xFF);
		ide_outb(device, ATA_LBA_HIGH, (blknr >> 16) & 0xFF);

#ifdef CONFIG_LBA48
		if (lba48) {
			ide_outb(device, ATA_DEV_HD,
				 ATA_LBA | ATA_DEVICE(device));
			ide_outb(device, ATA_COMMAND, ATA_CMD_PIO_READ_EXT);

		} else
#endif
		{
			ide_outb(device, ATA_DEV_HD, ATA_LBA |
				 ATA_DEVICE(device) | ((blknr >> 24) & 0xF));
			ide_outb(device, ATA_COMMAND, ATA_CMD_PIO_READ);
		}

		udelay(50);

		if (pwrsave) {
			/* may take up to 4 sec */
			c = ide_wait(device, IDE_SPIN_UP_TIME_OUT);
			pwrsave = 0;
		} else {
			/* can't take over 500 ms */
			c = ide_wait(device, IDE_TIME_OUT);
		}

		if ((c & (ATA_STAT_DRQ | ATA_STAT_BUSY | ATA_STAT_ERR)) !=
		    ATA_STAT_DRQ) {
			printf("Error (no IRQ) dev %d blk " LBAF
			       ": status %#02x\n", device, blknr, c);
			break;
		}

		ide_input_data(device, buffer, ATA_SECTORWORDS);
		(void) ide_inb(device, ATA_STATUS);	/* clear IRQ */

		++n;
		++blknr;
		buffer += ATA_BLOCKSIZE;
	}
IDE_READ_E:
	return n;
}

#ifdef CONFIG_BLK
ulong ide_write(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt,
		const void *buffer)
#else
ulong ide_write(struct blk_desc *block_dev, lbaint_t blknr, lbaint_t blkcnt,
		const void *buffer)
#endif
{
#ifdef CONFIG_BLK
	struct blk_desc *block_dev = dev_get_uclass_plat(dev);
#endif
	int device = block_dev->devnum;
	ulong n = 0;
	unsigned char c;

#ifdef CONFIG_LBA48
	unsigned char lba48 = 0;

	if (blknr & 0x0000fffff0000000ULL) {
		/* more than 28 bits used, use 48bit mode */
		lba48 = 1;
	}
#endif

	/* Select device
	 */
	ide_outb(device, ATA_DEV_HD, ATA_LBA | ATA_DEVICE(device));

	while (blkcnt-- > 0) {
		c = ide_wait(device, IDE_TIME_OUT);

		if (c & ATA_STAT_BUSY) {
			printf("IDE read: device %d not ready\n", device);
			goto WR_OUT;
		}
#ifdef CONFIG_LBA48
		if (lba48) {
			/* write high bits */
			ide_outb(device, ATA_SECT_CNT, 0);
			ide_outb(device, ATA_LBA_LOW, (blknr >> 24) & 0xFF);
#ifdef CONFIG_SYS_64BIT_LBA
			ide_outb(device, ATA_LBA_MID, (blknr >> 32) & 0xFF);
			ide_outb(device, ATA_LBA_HIGH, (blknr >> 40) & 0xFF);
#else
			ide_outb(device, ATA_LBA_MID, 0);
			ide_outb(device, ATA_LBA_HIGH, 0);
#endif
		}
#endif
		ide_outb(device, ATA_SECT_CNT, 1);
		ide_outb(device, ATA_LBA_LOW, (blknr >> 0) & 0xFF);
		ide_outb(device, ATA_LBA_MID, (blknr >> 8) & 0xFF);
		ide_outb(device, ATA_LBA_HIGH, (blknr >> 16) & 0xFF);

#ifdef CONFIG_LBA48
		if (lba48) {
			ide_outb(device, ATA_DEV_HD,
				 ATA_LBA | ATA_DEVICE(device));
			ide_outb(device, ATA_COMMAND, ATA_CMD_PIO_WRITE_EXT);

		} else
#endif
		{
			ide_outb(device, ATA_DEV_HD, ATA_LBA |
				 ATA_DEVICE(device) | ((blknr >> 24) & 0xF));
			ide_outb(device, ATA_COMMAND, ATA_CMD_PIO_WRITE);
		}

		udelay(50);

		/* can't take over 500 ms */
		c = ide_wait(device, IDE_TIME_OUT);

		if ((c & (ATA_STAT_DRQ | ATA_STAT_BUSY | ATA_STAT_ERR)) !=
		    ATA_STAT_DRQ) {
			printf("Error (no IRQ) dev %d blk " LBAF
			       ": status %#02x\n", device, blknr, c);
			goto WR_OUT;
		}

		ide_output_data(device, buffer, ATA_SECTORWORDS);
		c = ide_inb(device, ATA_STATUS);	/* clear IRQ */
		++n;
		++blknr;
		buffer += ATA_BLOCKSIZE;
	}
WR_OUT:
	return n;
}

#if defined(CONFIG_OF_IDE_FIXUP)
int ide_device_present(int dev)
{
	if (dev >= CONFIG_SYS_IDE_MAXBUS)
		return 0;
	return ide_dev_desc[dev].type == DEV_TYPE_UNKNOWN ? 0 : 1;
}
#endif

#ifdef CONFIG_BLK
static int ide_blk_probe(struct udevice *udev)
{
	struct blk_desc *desc = dev_get_uclass_plat(udev);

	/* fill in device vendor/product/rev strings */
	strncpy(desc->vendor, ide_dev_desc[desc->devnum].vendor,
		BLK_VEN_SIZE);
	desc->vendor[BLK_VEN_SIZE] = '\0';
	strncpy(desc->product, ide_dev_desc[desc->devnum].product,
		BLK_PRD_SIZE);
	desc->product[BLK_PRD_SIZE] = '\0';
	strncpy(desc->revision, ide_dev_desc[desc->devnum].revision,
		BLK_REV_SIZE);
	desc->revision[BLK_REV_SIZE] = '\0';

	return 0;
}

static const struct blk_ops ide_blk_ops = {
	.read	= ide_read,
	.write	= ide_write,
};

U_BOOT_DRIVER(ide_blk) = {
	.name		= "ide_blk",
	.id		= UCLASS_BLK,
	.ops		= &ide_blk_ops,
	.probe		= ide_blk_probe,
};

static int ide_probe(struct udevice *udev)
{
	struct udevice *blk_dev;
	char name[20];
	int blksz;
	lbaint_t size;
	int i;
	int ret;

	for (i = 0; i < CONFIG_SYS_IDE_MAXDEVICE; i++) {
		if (ide_dev_desc[i].type != DEV_TYPE_UNKNOWN) {
			sprintf(name, "blk#%d", i);

			blksz = ide_dev_desc[i].blksz;
			size = blksz * ide_dev_desc[i].lba;

			/*
			 * With CDROM, if there is no CD inserted, blksz will
			 * be zero, don't bother to create IDE block device.
			 */
			if (!blksz)
				continue;
			ret = blk_create_devicef(udev, "ide_blk", name,
						 IF_TYPE_IDE, i,
						 blksz, size, &blk_dev);
			if (ret)
				return ret;

			ret = blk_probe_or_unbind(blk_dev);
			if (ret)
				return ret;
		}
	}

	return 0;
}

U_BOOT_DRIVER(ide) = {
	.name		= "ide",
	.id		= UCLASS_IDE,
	.probe		= ide_probe,
};

struct pci_device_id ide_supported[] = {
	{ PCI_DEVICE_CLASS(PCI_CLASS_STORAGE_IDE << 8, 0xffff00) },
	{ }
};

U_BOOT_PCI_DEVICE(ide, ide_supported);

UCLASS_DRIVER(ide) = {
	.name		= "ide",
	.id		= UCLASS_IDE,
};
#else
U_BOOT_LEGACY_BLK(ide) = {
	.if_typename	= "ide",
	.if_type	= IF_TYPE_IDE,
	.max_devs	= CONFIG_SYS_IDE_MAXDEVICE,
	.desc		= ide_dev_desc,
};
#endif
