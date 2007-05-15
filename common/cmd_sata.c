/*
 * Copyright (C) Procsys. All rights reserved.
 * Author: Mushtaq Khan <mushtaq_k@procsys.com>
 *			<mushtaqk_921@yahoo.co.in>
 *
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
 *
 * with the reference to libata in kernel 2.4.32
 *
 */

/*
 * File contains SATA read-write and other utility functions.
 */
#include <common.h>
#include <asm/io.h>
#include <pci.h>
#include <command.h>
#include <config.h>
#include <ide.h>
#include <ata.h>

#ifdef CFG_SATA_SUPPORTED
/*For debug prints set macro DEBUG_SATA to 1 */
#define DEBUG_SATA 0
/*Macro for SATA library specific declarations */
#define SATA_DECL
#include <sata.h>
#undef SATA_DECL

static u8 __inline__
sata_inb (unsigned long ioaddr)
{
	return inb (ioaddr);
}

static void __inline__
sata_outb (unsigned char val, unsigned long ioaddr)
{
	outb (val, ioaddr);
}

static void
output_data (struct sata_ioports *ioaddr, ulong * sect_buf, int words)
{
	outsw (ioaddr->data_addr, sect_buf, words << 1);
}

static int
input_data (struct sata_ioports *ioaddr, ulong * sect_buf, int words)
{
	insw (ioaddr->data_addr, sect_buf, words << 1);
	return 0;
}

static void
sata_cpy (unsigned char *dst, unsigned char *src, unsigned int len)
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

int
sata_bus_softreset (int num)
{
	u8 dev = 0, status = 0, i;

	port[num].dev_mask = 0;

	for (i = 0; i < CFG_SATA_DEVS_PER_BUS; i++) {
		if (!(sata_devchk (&port[num].ioaddr, i))) {
			PRINTF ("dev_chk failed for dev#%d\n", i);
		} else {
			port[num].dev_mask |= (1 << i);
			PRINTF ("dev_chk passed for dev#%d\n", i);
		}
	}

	if (!(port[num].dev_mask)) {
		printf ("no devices on port%d\n", num);
		return 1;
	}

	dev_select (&port[num].ioaddr, dev);

	port[num].ctl_reg = 0x08;	/*Default value of control reg */
	sata_outb (port[num].ctl_reg, port[num].ioaddr.ctl_addr);
	udelay (10);
	sata_outb (port[num].ctl_reg | ATA_SRST, port[num].ioaddr.ctl_addr);
	udelay (10);
	sata_outb (port[num].ctl_reg, port[num].ioaddr.ctl_addr);

	/* spec mandates ">= 2ms" before checking status.
	 * We wait 150ms, because that was the magic delay used for
	 * ATAPI devices in Hale Landis's ATADRVR, for the period of time
	 * between when the ATA command register is written, and then
	 * status is checked.  Because waiting for "a while" before
	 * checking status is fine, post SRST, we perform this magic
	 * delay here as well.
	 */
	msleep (150);
	status = sata_busy_wait (&port[num].ioaddr, ATA_BUSY, 300);
	while ((status & ATA_BUSY)) {
		msleep (100);
		status = sata_busy_wait (&port[num].ioaddr, ATA_BUSY, 3);
	}

	if (status & ATA_BUSY)
		printf ("ata%u is slow to respond,plz be patient\n", port);

	while ((status & ATA_BUSY)) {
		msleep (100);
		status = sata_chk_status (&port[num].ioaddr);
	}

	if (status & ATA_BUSY) {
		printf ("ata%u failed to respond : ", port);
		printf ("bus reset failed\n");
		return 1;
	}
	return 0;
}

void
sata_identify (int num, int dev)
{
	u8 cmd = 0, status = 0, devno = num * CFG_SATA_DEVS_PER_BUS + dev;
	u16 iobuf[ATA_SECT_SIZE];
	u64 n_sectors = 0;
	u8 mask = 0;

	memset (iobuf, 0, sizeof (iobuf));
	hd_driveid_t *iop = (hd_driveid_t *) iobuf;

	if (dev == 0)
		mask = 0x01;
	else
		mask = 0x02;

	if (!(port[num].dev_mask & mask)) {
		printf ("dev%d is not present on port#%d\n", dev, num);
		return;
	}

	printf ("port=%d dev=%d\n", num, dev);

	dev_select (&port[num].ioaddr, dev);

	status = 0;
	cmd = ATA_CMD_IDENT;	/*Device Identify Command */
	sata_outb (cmd, port[num].ioaddr.command_addr);
	sata_inb (port[num].ioaddr.altstatus_addr);
	udelay (10);

	status = sata_busy_wait (&port[num].ioaddr, ATA_BUSY, 1000);
	if (status & ATA_ERR) {
		printf ("\ndevice not responding\n");
		port[num].dev_mask &= ~mask;
		return;
	}

	input_data (&port[num].ioaddr, (ulong *) iobuf, ATA_SECTORWORDS);

	PRINTF ("\nata%u: dev %u cfg 49:%04x 82:%04x 83:%04x 84:%04x85:%04x"
		"86:%04x" "87:%04x 88:%04x\n", num, dev, iobuf[49],
		iobuf[82], iobuf[83], iobuf[84], iobuf[85], iobuf[86],
		iobuf[87], iobuf[88]);

	/* we require LBA and DMA support (bits 8 & 9 of word 49) */
	if (!ata_id_has_dma (iobuf) || !ata_id_has_lba (iobuf)) {
		PRINTF ("ata%u: no dma/lba\n", num);
	}
	ata_dump_id (iobuf);

	if (ata_id_has_lba48 (iobuf)) {
		n_sectors = ata_id_u64 (iobuf, 100);
	} else {
		n_sectors = ata_id_u32 (iobuf, 60);
	}
	PRINTF ("no. of sectors %u\n", ata_id_u64 (iobuf, 100));
	PRINTF ("no. of sectors %u\n", ata_id_u32 (iobuf, 60));

	if (n_sectors == 0) {
		port[num].dev_mask &= ~mask;
		return;
	}

	sata_cpy (sata_dev_desc[devno].revision, iop->fw_rev,
		  sizeof (sata_dev_desc[devno].revision));
	sata_cpy (sata_dev_desc[devno].vendor, iop->model,
		  sizeof (sata_dev_desc[devno].vendor));
	sata_cpy (sata_dev_desc[devno].product, iop->serial_no,
		  sizeof (sata_dev_desc[devno].product));
	strswab (sata_dev_desc[devno].revision);
	strswab (sata_dev_desc[devno].vendor);

	if ((iop->config & 0x0080) == 0x0080) {
		sata_dev_desc[devno].removable = 1;
	} else {
		sata_dev_desc[devno].removable = 0;
	}

	sata_dev_desc[devno].lba = iop->lba_capacity;
	PRINTF ("lba=0x%x", sata_dev_desc[devno].lba);

#ifdef CONFIG_LBA48
	if (iop->command_set_2 & 0x0400) {
		sata_dev_desc[devno].lba48 = 1;
		lba = (unsigned long long) iop->lba48_capacity[0] |
		    ((unsigned long long) iop->lba48_capacity[1] << 16) |
		    ((unsigned long long) iop->lba48_capacity[2] << 32) |
		    ((unsigned long long) iop->lba48_capacity[3] << 48);
	} else {
		sata_dev_desc[devno].lba48 = 0;
	}
#endif

	/* assuming HD */
	sata_dev_desc[devno].type = DEV_TYPE_HARDDISK;
	sata_dev_desc[devno].blksz = ATA_BLOCKSIZE;
	sata_dev_desc[devno].lun = 0;	/* just to fill something in... */
}

void
set_Feature_cmd (int num, int dev)
{
	u8 mask = 0x00, status = 0;

	if (dev == 0)
		mask = 0x01;
	else
		mask = 0x02;

	if (!(port[num].dev_mask & mask)) {
		PRINTF ("dev%d is not present on port#%d\n", dev, num);
		return;
	}

	dev_select (&port[num].ioaddr, dev);

	sata_outb (SETFEATURES_XFER, port[num].ioaddr.feature_addr);
	sata_outb (XFER_PIO_4, port[num].ioaddr.nsect_addr);
	sata_outb (0, port[num].ioaddr.lbal_addr);
	sata_outb (0, port[num].ioaddr.lbam_addr);
	sata_outb (0, port[num].ioaddr.lbah_addr);

	sata_outb (ATA_DEVICE_OBS, port[num].ioaddr.device_addr);
	sata_outb (ATA_CMD_SETF, port[num].ioaddr.command_addr);

	udelay (50);
	msleep (150);

	status = sata_busy_wait (&port[num].ioaddr, ATA_BUSY, 5000);
	if ((status & (ATA_STAT_BUSY | ATA_STAT_ERR))) {
		printf ("Error  : status 0x%02x\n", status);
		port[num].dev_mask &= ~mask;
	}
}

void
sata_port (struct sata_ioports *ioport)
{
	ioport->data_addr = ioport->cmd_addr + ATA_REG_DATA;
	ioport->error_addr = ioport->cmd_addr + ATA_REG_ERR;
	ioport->feature_addr = ioport->cmd_addr + ATA_REG_FEATURE;
	ioport->nsect_addr = ioport->cmd_addr + ATA_REG_NSECT;
	ioport->lbal_addr = ioport->cmd_addr + ATA_REG_LBAL;
	ioport->lbam_addr = ioport->cmd_addr + ATA_REG_LBAM;
	ioport->lbah_addr = ioport->cmd_addr + ATA_REG_LBAH;
	ioport->device_addr = ioport->cmd_addr + ATA_REG_DEVICE;
	ioport->status_addr = ioport->cmd_addr + ATA_REG_STATUS;
	ioport->command_addr = ioport->cmd_addr + ATA_REG_CMD;
}

int
sata_devchk (struct sata_ioports *ioaddr, int dev)
{
	u8 nsect, lbal;

	dev_select (ioaddr, dev);

	sata_outb (0x55, ioaddr->nsect_addr);
	sata_outb (0xaa, ioaddr->lbal_addr);

	sata_outb (0xaa, ioaddr->nsect_addr);
	sata_outb (0x55, ioaddr->lbal_addr);

	sata_outb (0x55, ioaddr->nsect_addr);
	sata_outb (0xaa, ioaddr->lbal_addr);

	nsect = sata_inb (ioaddr->nsect_addr);
	lbal = sata_inb (ioaddr->lbal_addr);

	if ((nsect == 0x55) && (lbal == 0xaa))
		return 1;	/* we found a device */
	else
		return 0;	/* nothing found */
}

void
dev_select (struct sata_ioports *ioaddr, int dev)
{
	u8 tmp = 0;

	if (dev == 0)
		tmp = ATA_DEVICE_OBS;
	else
		tmp = ATA_DEVICE_OBS | ATA_DEV1;

	sata_outb (tmp, ioaddr->device_addr);
	sata_inb (ioaddr->altstatus_addr);
	udelay (5);
}

u8
sata_busy_wait (struct sata_ioports *ioaddr, int bits, unsigned int max)
{
	u8 status;

	do {
		udelay (1000);
		status = sata_chk_status (ioaddr);
		max--;
	} while ((status & bits) && (max > 0));

	return status;
}

u8
sata_chk_status (struct sata_ioports * ioaddr)
{
	return sata_inb (ioaddr->status_addr);
}

void
msleep (int count)
{
	int i;

	for (i = 0; i < count; i++)
		udelay (1000);
}

ulong
sata_read (int device, lbaint_t blknr, ulong blkcnt, ulong * buffer)
{
	ulong n = 0;
	u8 dev = 0, num = 0, mask = 0, status = 0;

#ifdef CONFIG_LBA48
	unsigned char lba48 = 0;

	if (blknr & 0x0000fffff0000000) {
		if (!sata_dev_desc[devno].lba48) {
			printf ("Drive doesn't support 48-bit addressing\n");
			return 0;
		}
		/* more than 28 bits used, use 48bit mode */
		lba48 = 1;
	}
#endif
	/*Port Number */
	num = device / CFG_SATA_DEVS_PER_BUS;
	/*dev on the port */
	if (device >= CFG_SATA_DEVS_PER_BUS)
		dev = device - CFG_SATA_DEVS_PER_BUS;
	else
		dev = device;

	if (dev == 0)
		mask = 0x01;
	else
		mask = 0x02;

	if (!(port[num].dev_mask & mask)) {
		printf ("dev%d is not present on port#%d\n", dev, num);
		return 0;
	}

	/* Select device */
	dev_select (&port[num].ioaddr, dev);

	status = sata_busy_wait (&port[num].ioaddr, ATA_BUSY, 500);
	if (status & ATA_BUSY) {
		printf ("ata%u failed to respond\n", port[num].port_no);
		return n;
	}
	while (blkcnt-- > 0) {
		status = sata_busy_wait (&port[num].ioaddr, ATA_BUSY, 500);
		if (status & ATA_BUSY) {
			printf ("ata%u failed to respond\n", 0);
			return n;
		}
#ifdef CONFIG_LBA48
		if (lba48) {
			/* write high bits */
			sata_outb (0, port[num].ioaddr.nsect_addr);
			sata_outb ((blknr >> 24) & 0xFF,
				   port[num].ioaddr.lbal_addr);
			sata_outb ((blknr >> 32) & 0xFF,
				   port[num].ioaddr.lbam_addr);
			sata_outb ((blknr >> 40) & 0xFF,
				   port[num].ioaddr.lbah_addr);
		}
#endif
		sata_outb (1, port[num].ioaddr.nsect_addr);
		sata_outb (((blknr) >> 0) & 0xFF,
			   port[num].ioaddr.lbal_addr);
		sata_outb ((blknr >> 8) & 0xFF, port[num].ioaddr.lbam_addr);
		sata_outb ((blknr >> 16) & 0xFF, port[num].ioaddr.lbah_addr);

#ifdef CONFIG_LBA48
		if (lba48) {
			sata_outb (ATA_LBA, port[num].ioaddr.device_addr);
			sata_outb (ATA_CMD_READ_EXT,
				   port[num].ioaddr.command_addr);
		} else
#endif
		{
			sata_outb (ATA_LBA | ((blknr >> 24) & 0xF),
				   port[num].ioaddr.device_addr);
			sata_outb (ATA_CMD_READ,
				   port[num].ioaddr.command_addr);
		}

		msleep (50);
		/*may take up to 4 sec */
		status = sata_busy_wait (&port[num].ioaddr, ATA_BUSY, 4000);

		if ((status & (ATA_STAT_DRQ | ATA_STAT_BUSY | ATA_STAT_ERR))
		    != ATA_STAT_DRQ) {
			u8 err = 0;

			printf ("Error no DRQ dev %d blk %ld: sts 0x%02x\n",
				device, (ulong) blknr, status);
			err = sata_inb (port[num].ioaddr.error_addr);
			printf ("Error reg = 0x%x\n", err);
			return (n);
		}
		input_data (&port[num].ioaddr, buffer, ATA_SECTORWORDS);
		sata_inb (port[num].ioaddr.altstatus_addr);
		udelay (50);

		++n;
		++blknr;
		buffer += ATA_SECTORWORDS;
	}
	return n;
}

ulong
sata_write (int device, lbaint_t blknr, ulong blkcnt, ulong * buffer)
{
	ulong n = 0;
	unsigned char status = 0, num = 0, dev = 0, mask = 0;

#ifdef CONFIG_LBA48
	unsigned char lba48 = 0;

	if (blknr & 0x0000fffff0000000) {
		if (!sata_dev_desc[devno].lba48) {
			printf ("Drive doesn't support 48-bit addressing\n");
			return 0;
		}
		/* more than 28 bits used, use 48bit mode */
		lba48 = 1;
	}
#endif
	/*Port Number */
	num = device / CFG_SATA_DEVS_PER_BUS;
	/*dev on the Port */
	if (device >= CFG_SATA_DEVS_PER_BUS)
		dev = device - CFG_SATA_DEVS_PER_BUS;
	else
		dev = device;

	if (dev == 0)
		mask = 0x01;
	else
		mask = 0x02;

	/* Select device */
	dev_select (&port[num].ioaddr, dev);

	status = sata_busy_wait (&port[num].ioaddr, ATA_BUSY, 500);
	if (status & ATA_BUSY) {
		printf ("ata%u failed to respond\n", port[num].port_no);
		return n;
	}

	while (blkcnt-- > 0) {
		status = sata_busy_wait (&port[num].ioaddr, ATA_BUSY, 500);
		if (status & ATA_BUSY) {
			printf ("ata%u failed to respond\n",
				port[num].port_no);
			return n;
		}
#ifdef CONFIG_LBA48
		if (lba48) {
			/* write high bits */
			sata_outb (0, port[num].ioaddr.nsect_addr);
			sata_outb ((blknr >> 24) & 0xFF,
				   port[num].ioaddr.lbal_addr);
			sata_outb ((blknr >> 32) & 0xFF,
				   port[num].ioaddr.lbam_addr);
			sata_outb ((blknr >> 40) & 0xFF,
				   port[num].ioaddr.lbah_addr);
		}
#endif
		sata_outb (1, port[num].ioaddr.nsect_addr);
		sata_outb ((blknr >> 0) & 0xFF, port[num].ioaddr.lbal_addr);
		sata_outb ((blknr >> 8) & 0xFF, port[num].ioaddr.lbam_addr);
		sata_outb ((blknr >> 16) & 0xFF, port[num].ioaddr.lbah_addr);
#ifdef CONFIG_LBA48
		if (lba48) {
			sata_outb (ATA_LBA, port[num].ioaddr.device_addr);
			sata_outb (ATA_CMD_WRITE_EXT,
				   port[num].ioaddr.command_addr);
		} else
#endif
		{
			sata_outb (ATA_LBA | ((blknr >> 24) & 0xF),
				   port[num].ioaddr.device_addr);
			sata_outb (ATA_CMD_WRITE,
				   port[num].ioaddr.command_addr);
		}

		msleep (50);
		/*may take up to 4 sec */
		status = sata_busy_wait (&port[num].ioaddr, ATA_BUSY, 4000);
		if ((status & (ATA_STAT_DRQ | ATA_STAT_BUSY | ATA_STAT_ERR))
		    != ATA_STAT_DRQ) {
			printf ("Error no DRQ dev %d blk %ld: sts 0x%02x\n",
				device, (ulong) blknr, status);
			return (n);
		}

		output_data (&port[num].ioaddr, buffer, ATA_SECTORWORDS);
		sata_inb (port[num].ioaddr.altstatus_addr);
		udelay (50);

		++n;
		++blknr;
		buffer += ATA_SECTORWORDS;
	}
	return n;
}

block_dev_desc_t *sata_get_dev (int dev);

block_dev_desc_t *
sata_get_dev (int dev)
{
	return ((block_dev_desc_t *) & sata_dev_desc[dev]);
}

int
do_sata (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{

	switch (argc) {
	case 0:
	case 1:
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	case 2:
		if (strncmp (argv[1], "init", 4) == 0) {
			int rcode = 0;

			rcode = init_sata ();
			if (rcode)
				printf ("Sata initialization Failed\n");
			return rcode;
		} else if (strncmp (argv[1], "inf", 3) == 0) {
			int i;

			putc ('\n');
			for (i = 0; i < CFG_SATA_MAXDEVICES; ++i) {
				/*List only known devices */
				if (sata_dev_desc[i].type ==
				    DEV_TYPE_UNKNOWN)
					continue;
				printf ("sata dev %d: ", i);
				dev_print (&sata_dev_desc[i]);
			}
			return 0;
		}
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	case 3:
		if (strcmp (argv[1], "dev") == 0) {
			int dev = (int) simple_strtoul (argv[2], NULL, 10);

			if (dev >= CFG_SATA_MAXDEVICES) {
				printf ("\nSata dev %d not available\n",
					dev);
				return 1;
			}
			printf ("\nSATA dev %d: ", dev);
			dev_print (&sata_dev_desc[dev]);
			if (sata_dev_desc[dev].type == DEV_TYPE_UNKNOWN)
				return 1;
			curr_dev = dev;
			return 0;
		} else if (strcmp (argv[1], "part") == 0) {
			int dev = (int) simple_strtoul (argv[2], NULL, 10);

			if (dev >= CFG_SATA_MAXDEVICES) {
				printf ("\nSata dev %d not available\n",
					dev);
				return 1;
			}
			PRINTF ("\nSATA dev %d: ", dev);
			if (sata_dev_desc[dev].part_type !=
			    PART_TYPE_UNKNOWN) {
				print_part (&sata_dev_desc[dev]);
			} else {
				printf ("\nSata dev %d partition type "
					"unknown\n", dev);
				return 1;
			}
			return 0;
		}
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	default:
		if (argc < 5) {
			printf ("Usage:\n%s\n", cmdtp->usage);
			return 1;
		}
		if (strcmp (argv[1], "read") == 0) {
			ulong addr = simple_strtoul (argv[2], NULL, 16);
			ulong cnt = simple_strtoul (argv[4], NULL, 16);
			ulong n;
			lbaint_t blk = simple_strtoul (argv[3], NULL, 16);

			memset ((int *) addr, 0, cnt * 512);
			printf ("\nSATA read: dev %d blk # %ld,"
				"count %ld ... ", curr_dev, blk, cnt);
			n = sata_read (curr_dev, blk, cnt, (ulong *) addr);
			/* flush cache after read */
			flush_cache (addr, cnt * 512);
			printf ("%ld blocks read: %s\n", n,
				(n == cnt) ? "OK" : "ERR");
			if (n == cnt)
				return 1;
			else
				return 0;
		} else if (strcmp (argv[1], "write") == 0) {
			ulong addr = simple_strtoul (argv[2], NULL, 16);
			ulong cnt = simple_strtoul (argv[4], NULL, 16);
			ulong n;
			lbaint_t blk = simple_strtoul (argv[3], NULL, 16);

			printf ("\nSata write: dev %d blk # %ld,"
				"count %ld ... ", curr_dev, blk, cnt);
			n = sata_write (curr_dev, blk, cnt, (ulong *) addr);
			printf ("%ld blocks written: %s\n", n,
				(n == cnt) ? "OK" : "ERR");
			if (n == cnt)
				return 1;
			else
				return 0;
		} else {
			printf ("Usage:\n%s\n", cmdtp->usage);
			return 1;
		}
	}			/*End OF SWITCH */
}

U_BOOT_CMD (sata, 5, 1, do_sata,
	    "sata init\n"
	    "sata info\n"
	    "sata part device\n"
	    "sata dev device\n"
	    "sata read  addr blk# cnt\n"
	    "sata write  addr blk# cnt\n", "cmd for init,rw and dev-info\n");

#endif
