/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <inttypes.h>
#include <pci.h>
#include <scsi.h>

#ifdef CONFIG_SCSI_DEV_LIST
#define SCSI_DEV_LIST CONFIG_SCSI_DEV_LIST
#else
#ifdef CONFIG_SCSI_SYM53C8XX
#define SCSI_VEND_ID	0x1000
#ifndef CONFIG_SCSI_DEV_ID
#define SCSI_DEV_ID		0x0001
#else
#define SCSI_DEV_ID		CONFIG_SCSI_DEV_ID
#endif
#elif defined CONFIG_SATA_ULI5288

#define SCSI_VEND_ID 0x10b9
#define SCSI_DEV_ID  0x5288

#elif !defined(CONFIG_SCSI_AHCI_PLAT)
#error no scsi device defined
#endif
#define SCSI_DEV_LIST {SCSI_VEND_ID, SCSI_DEV_ID}
#endif

#if defined(CONFIG_PCI) && !defined(CONFIG_SCSI_AHCI_PLAT)
const struct pci_device_id scsi_device_list[] = { SCSI_DEV_LIST };
#endif
static ccb tempccb;	/* temporary scsi command buffer */

static unsigned char tempbuff[512]; /* temporary data buffer */

static int scsi_max_devs; /* number of highest available scsi device */

static int scsi_curr_dev; /* current device */

static struct blk_desc scsi_dev_desc[CONFIG_SYS_SCSI_MAX_DEVICE];

/* almost the maximum amount of the scsi_ext command.. */
#define SCSI_MAX_READ_BLK 0xFFFF
#define SCSI_LBA48_READ	0xFFFFFFF

#ifdef CONFIG_SYS_64BIT_LBA
void scsi_setup_read16(ccb *pccb, lbaint_t start, unsigned long blocks)
{
	pccb->cmd[0] = SCSI_READ16;
	pccb->cmd[1] = pccb->lun << 5;
	pccb->cmd[2] = (unsigned char)(start >> 56) & 0xff;
	pccb->cmd[3] = (unsigned char)(start >> 48) & 0xff;
	pccb->cmd[4] = (unsigned char)(start >> 40) & 0xff;
	pccb->cmd[5] = (unsigned char)(start >> 32) & 0xff;
	pccb->cmd[6] = (unsigned char)(start >> 24) & 0xff;
	pccb->cmd[7] = (unsigned char)(start >> 16) & 0xff;
	pccb->cmd[8] = (unsigned char)(start >> 8) & 0xff;
	pccb->cmd[9] = (unsigned char)start & 0xff;
	pccb->cmd[10] = 0;
	pccb->cmd[11] = (unsigned char)(blocks >> 24) & 0xff;
	pccb->cmd[12] = (unsigned char)(blocks >> 16) & 0xff;
	pccb->cmd[13] = (unsigned char)(blocks >> 8) & 0xff;
	pccb->cmd[14] = (unsigned char)blocks & 0xff;
	pccb->cmd[15] = 0;
	pccb->cmdlen = 16;
	pccb->msgout[0] = SCSI_IDENTIFY; /* NOT USED */
	debug("scsi_setup_read16: cmd: %02X %02X startblk %02X%02X%02X%02X%02X%02X%02X%02X blccnt %02X%02X%02X%02X\n",
	      pccb->cmd[0], pccb->cmd[1],
	      pccb->cmd[2], pccb->cmd[3], pccb->cmd[4], pccb->cmd[5],
	      pccb->cmd[6], pccb->cmd[7], pccb->cmd[8], pccb->cmd[9],
	      pccb->cmd[11], pccb->cmd[12], pccb->cmd[13], pccb->cmd[14]);
}
#endif

static void scsi_setup_read_ext(ccb *pccb, lbaint_t start,
				unsigned short blocks)
{
	pccb->cmd[0] = SCSI_READ10;
	pccb->cmd[1] = pccb->lun << 5;
	pccb->cmd[2] = (unsigned char)(start >> 24) & 0xff;
	pccb->cmd[3] = (unsigned char)(start >> 16) & 0xff;
	pccb->cmd[4] = (unsigned char)(start >> 8) & 0xff;
	pccb->cmd[5] = (unsigned char)start & 0xff;
	pccb->cmd[6] = 0;
	pccb->cmd[7] = (unsigned char)(blocks >> 8) & 0xff;
	pccb->cmd[8] = (unsigned char)blocks & 0xff;
	pccb->cmd[6] = 0;
	pccb->cmdlen = 10;
	pccb->msgout[0] = SCSI_IDENTIFY; /* NOT USED */
	debug("scsi_setup_read_ext: cmd: %02X %02X startblk %02X%02X%02X%02X blccnt %02X%02X\n",
	      pccb->cmd[0], pccb->cmd[1],
	      pccb->cmd[2], pccb->cmd[3], pccb->cmd[4], pccb->cmd[5],
	      pccb->cmd[7], pccb->cmd[8]);
}

static void scsi_setup_write_ext(ccb *pccb, lbaint_t start,
				 unsigned short blocks)
{
	pccb->cmd[0] = SCSI_WRITE10;
	pccb->cmd[1] = pccb->lun << 5;
	pccb->cmd[2] = (unsigned char)(start >> 24) & 0xff;
	pccb->cmd[3] = (unsigned char)(start >> 16) & 0xff;
	pccb->cmd[4] = (unsigned char)(start >> 8) & 0xff;
	pccb->cmd[5] = (unsigned char)start & 0xff;
	pccb->cmd[6] = 0;
	pccb->cmd[7] = ((unsigned char)(blocks >> 8)) & 0xff;
	pccb->cmd[8] = (unsigned char)blocks & 0xff;
	pccb->cmd[9] = 0;
	pccb->cmdlen = 10;
	pccb->msgout[0] = SCSI_IDENTIFY;  /* NOT USED */
	debug("%s: cmd: %02X %02X startblk %02X%02X%02X%02X blccnt %02X%02X\n",
	      __func__,
	      pccb->cmd[0], pccb->cmd[1],
	      pccb->cmd[2], pccb->cmd[3], pccb->cmd[4], pccb->cmd[5],
	      pccb->cmd[7], pccb->cmd[8]);
}

static void scsi_setup_inquiry(ccb *pccb)
{
	pccb->cmd[0] = SCSI_INQUIRY;
	pccb->cmd[1] = pccb->lun << 5;
	pccb->cmd[2] = 0;
	pccb->cmd[3] = 0;
	if (pccb->datalen > 255)
		pccb->cmd[4] = 255;
	else
		pccb->cmd[4] = (unsigned char)pccb->datalen;
	pccb->cmd[5] = 0;
	pccb->cmdlen = 6;
	pccb->msgout[0] = SCSI_IDENTIFY; /* NOT USED */
}

#ifdef CONFIG_BLK
static ulong scsi_read(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt,
		       void *buffer)
#else
static ulong scsi_read(struct blk_desc *block_dev, lbaint_t blknr,
		       lbaint_t blkcnt, void *buffer)
#endif
{
#ifdef CONFIG_BLK
	struct blk_desc *block_dev = dev_get_uclass_platdata(dev);
#endif
	lbaint_t start, blks;
	uintptr_t buf_addr;
	unsigned short smallblks = 0;
	ccb *pccb = (ccb *)&tempccb;

	/* Setup device */
	pccb->target = block_dev->target;
	pccb->lun = block_dev->lun;
	buf_addr = (unsigned long)buffer;
	start = blknr;
	blks = blkcnt;
	debug("\nscsi_read: dev %d startblk " LBAF
	      ", blccnt " LBAF " buffer %lx\n",
	      block_dev->devnum, start, blks, (unsigned long)buffer);
	do {
		pccb->pdata = (unsigned char *)buf_addr;
#ifdef CONFIG_SYS_64BIT_LBA
		if (start > SCSI_LBA48_READ) {
			unsigned long blocks;
			blocks = min_t(lbaint_t, blks, SCSI_MAX_READ_BLK);
			pccb->datalen = block_dev->blksz * blocks;
			scsi_setup_read16(pccb, start, blocks);
			start += blocks;
			blks -= blocks;
		} else
#endif
		if (blks > SCSI_MAX_READ_BLK) {
			pccb->datalen = block_dev->blksz *
				SCSI_MAX_READ_BLK;
			smallblks = SCSI_MAX_READ_BLK;
			scsi_setup_read_ext(pccb, start, smallblks);
			start += SCSI_MAX_READ_BLK;
			blks -= SCSI_MAX_READ_BLK;
		} else {
			pccb->datalen = block_dev->blksz * blks;
			smallblks = (unsigned short)blks;
			scsi_setup_read_ext(pccb, start, smallblks);
			start += blks;
			blks = 0;
		}
		debug("scsi_read_ext: startblk " LBAF
		      ", blccnt %x buffer %" PRIXPTR "\n",
		      start, smallblks, buf_addr);
		if (scsi_exec(pccb) != true) {
			scsi_print_error(pccb);
			blkcnt -= blks;
			break;
		}
		buf_addr += pccb->datalen;
	} while (blks != 0);
	debug("scsi_read_ext: end startblk " LBAF
	      ", blccnt %x buffer %" PRIXPTR "\n", start, smallblks, buf_addr);
	return blkcnt;
}

/*******************************************************************************
 * scsi_write
 */

/* Almost the maximum amount of the scsi_ext command.. */
#define SCSI_MAX_WRITE_BLK 0xFFFF

#ifdef CONFIG_BLK
static ulong scsi_write(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt,
			const void *buffer)
#else
static ulong scsi_write(struct blk_desc *block_dev, lbaint_t blknr,
			lbaint_t blkcnt, const void *buffer)
#endif
{
#ifdef CONFIG_BLK
	struct blk_desc *block_dev = dev_get_uclass_platdata(dev);
#endif
	lbaint_t start, blks;
	uintptr_t buf_addr;
	unsigned short smallblks;
	ccb *pccb = (ccb *)&tempccb;

	/* Setup device */
	pccb->target = block_dev->target;
	pccb->lun = block_dev->lun;
	buf_addr = (unsigned long)buffer;
	start = blknr;
	blks = blkcnt;
	debug("\n%s: dev %d startblk " LBAF ", blccnt " LBAF " buffer %lx\n",
	      __func__, block_dev->devnum, start, blks, (unsigned long)buffer);
	do {
		pccb->pdata = (unsigned char *)buf_addr;
		if (blks > SCSI_MAX_WRITE_BLK) {
			pccb->datalen = (block_dev->blksz *
					 SCSI_MAX_WRITE_BLK);
			smallblks = SCSI_MAX_WRITE_BLK;
			scsi_setup_write_ext(pccb, start, smallblks);
			start += SCSI_MAX_WRITE_BLK;
			blks -= SCSI_MAX_WRITE_BLK;
		} else {
			pccb->datalen = block_dev->blksz * blks;
			smallblks = (unsigned short)blks;
			scsi_setup_write_ext(pccb, start, smallblks);
			start += blks;
			blks = 0;
		}
		debug("%s: startblk " LBAF ", blccnt %x buffer %" PRIXPTR "\n",
		      __func__, start, smallblks, buf_addr);
		if (scsi_exec(pccb) != true) {
			scsi_print_error(pccb);
			blkcnt -= blks;
			break;
		}
		buf_addr += pccb->datalen;
	} while (blks != 0);
	debug("%s: end startblk " LBAF ", blccnt %x buffer %" PRIXPTR "\n",
	      __func__, start, smallblks, buf_addr);
	return blkcnt;
}

#if defined(CONFIG_PCI) && !defined(CONFIG_SCSI_AHCI_PLAT)
void scsi_init(void)
{
	int busdevfunc = -1;
	int i;
	/*
	 * Find a device from the list, this driver will support a single
	 * controller.
	 */
	for (i = 0; i < ARRAY_SIZE(scsi_device_list); i++) {
		/* get PCI Device ID */
#ifdef CONFIG_DM_PCI
		struct udevice *dev;
		int ret;

		ret = dm_pci_find_device(scsi_device_list[i].vendor,
					 scsi_device_list[i].device, 0, &dev);
		if (!ret) {
			busdevfunc = dm_pci_get_bdf(dev);
			break;
		}
#else
		busdevfunc = pci_find_device(scsi_device_list[i].vendor,
					     scsi_device_list[i].device,
					     0);
#endif
		if (busdevfunc != -1)
			break;
	}

	if (busdevfunc == -1) {
		printf("Error: SCSI Controller(s) ");
		for (i = 0; i < ARRAY_SIZE(scsi_device_list); i++) {
			printf("%04X:%04X ",
			       scsi_device_list[i].vendor,
			       scsi_device_list[i].device);
		}
		printf("not found\n");
		return;
	}
#ifdef DEBUG
	else {
		printf("SCSI Controller (%04X,%04X) found (%d:%d:%d)\n",
		       scsi_device_list[i].vendor,
		       scsi_device_list[i].device,
		       (busdevfunc >> 16) & 0xFF,
		       (busdevfunc >> 11) & 0x1F,
		       (busdevfunc >> 8) & 0x7);
	}
#endif
	bootstage_start(BOOTSTAGE_ID_ACCUM_SCSI, "ahci");
	scsi_low_level_init(busdevfunc);
	scsi_scan(1);
	bootstage_accum(BOOTSTAGE_ID_ACCUM_SCSI);
}
#endif

/* copy src to dest, skipping leading and trailing blanks
 * and null terminate the string
 */
static void scsi_ident_cpy(unsigned char *dest, unsigned char *src,
			   unsigned int len)
{
	int start, end;

	start = 0;
	while (start < len) {
		if (src[start] != ' ')
			break;
		start++;
	}
	end = len-1;
	while (end > start) {
		if (src[end] != ' ')
			break;
		end--;
	}
	for (; start <= end; start++)
		*dest ++= src[start];
	*dest = '\0';
}

static int scsi_read_capacity(ccb *pccb, lbaint_t *capacity,
			      unsigned long *blksz)
{
	*capacity = 0;

	memset(pccb->cmd, '\0', sizeof(pccb->cmd));
	pccb->cmd[0] = SCSI_RD_CAPAC10;
	pccb->cmd[1] = pccb->lun << 5;
	pccb->cmdlen = 10;
	pccb->msgout[0] = SCSI_IDENTIFY; /* NOT USED */

	pccb->datalen = 8;
	if (scsi_exec(pccb) != true)
		return 1;

	*capacity = ((lbaint_t)pccb->pdata[0] << 24) |
		    ((lbaint_t)pccb->pdata[1] << 16) |
		    ((lbaint_t)pccb->pdata[2] << 8)  |
		    ((lbaint_t)pccb->pdata[3]);

	if (*capacity != 0xffffffff) {
		/* Read capacity (10) was sufficient for this drive. */
		*blksz = ((unsigned long)pccb->pdata[4] << 24) |
			 ((unsigned long)pccb->pdata[5] << 16) |
			 ((unsigned long)pccb->pdata[6] << 8)  |
			 ((unsigned long)pccb->pdata[7]);
		return 0;
	}

	/* Read capacity (10) was insufficient. Use read capacity (16). */
	memset(pccb->cmd, '\0', sizeof(pccb->cmd));
	pccb->cmd[0] = SCSI_RD_CAPAC16;
	pccb->cmd[1] = 0x10;
	pccb->cmdlen = 16;
	pccb->msgout[0] = SCSI_IDENTIFY; /* NOT USED */

	pccb->datalen = 16;
	if (scsi_exec(pccb) != true)
		return 1;

	*capacity = ((uint64_t)pccb->pdata[0] << 56) |
		    ((uint64_t)pccb->pdata[1] << 48) |
		    ((uint64_t)pccb->pdata[2] << 40) |
		    ((uint64_t)pccb->pdata[3] << 32) |
		    ((uint64_t)pccb->pdata[4] << 24) |
		    ((uint64_t)pccb->pdata[5] << 16) |
		    ((uint64_t)pccb->pdata[6] << 8)  |
		    ((uint64_t)pccb->pdata[7]);

	*blksz = ((uint64_t)pccb->pdata[8]  << 56) |
		 ((uint64_t)pccb->pdata[9]  << 48) |
		 ((uint64_t)pccb->pdata[10] << 40) |
		 ((uint64_t)pccb->pdata[11] << 32) |
		 ((uint64_t)pccb->pdata[12] << 24) |
		 ((uint64_t)pccb->pdata[13] << 16) |
		 ((uint64_t)pccb->pdata[14] << 8)  |
		 ((uint64_t)pccb->pdata[15]);

	return 0;
}


/*
 * Some setup (fill-in) routines
 */
static void scsi_setup_test_unit_ready(ccb *pccb)
{
	pccb->cmd[0] = SCSI_TST_U_RDY;
	pccb->cmd[1] = pccb->lun << 5;
	pccb->cmd[2] = 0;
	pccb->cmd[3] = 0;
	pccb->cmd[4] = 0;
	pccb->cmd[5] = 0;
	pccb->cmdlen = 6;
	pccb->msgout[0] = SCSI_IDENTIFY; /* NOT USED */
}

/**
 * scsi_init_dev_desc_priv - initialize only SCSI specific blk_desc properties
 *
 * @dev_desc: Block device description pointer
 */
static void scsi_init_dev_desc_priv(struct blk_desc *dev_desc)
{
	dev_desc->target = 0xff;
	dev_desc->lun = 0xff;
	dev_desc->log2blksz =
		LOG2_INVALID(typeof(dev_desc->log2blksz));
	dev_desc->type = DEV_TYPE_UNKNOWN;
	dev_desc->vendor[0] = 0;
	dev_desc->product[0] = 0;
	dev_desc->revision[0] = 0;
	dev_desc->removable = false;
#ifndef CONFIG_BLK
	dev_desc->block_read = scsi_read;
	dev_desc->block_write = scsi_write;
#endif
}

/**
 * scsi_init_dev_desc - initialize all SCSI specific blk_desc properties
 *
 * @dev_desc: Block device description pointer
 * @devnum: Device number
 */
static void scsi_init_dev_desc(struct blk_desc *dev_desc, int devnum)
{
	dev_desc->lba = 0;
	dev_desc->blksz = 0;
	dev_desc->if_type = IF_TYPE_SCSI;
	dev_desc->devnum = devnum;
	dev_desc->part_type = PART_TYPE_UNKNOWN;

	scsi_init_dev_desc_priv(dev_desc);
}

/**
 * scsi_detect_dev - Detect scsi device
 *
 * @target: target id
 * @dev_desc: block device description
 *
 * The scsi_detect_dev detects and fills a dev_desc structure when the device is
 * detected. The LUN number is taken from the struct blk_desc *dev_desc.
 *
 * Return: 0 on success, error value otherwise
 */
static int scsi_detect_dev(int target, struct blk_desc *dev_desc)
{
	unsigned char perq, modi;
	lbaint_t capacity;
	unsigned long blksz;
	ccb *pccb = (ccb *)&tempccb;

	pccb->target = target;
	pccb->lun = dev_desc->lun;
	pccb->pdata = (unsigned char *)&tempbuff;
	pccb->datalen = 512;
	scsi_setup_inquiry(pccb);
	if (scsi_exec(pccb) != true) {
		if (pccb->contr_stat == SCSI_SEL_TIME_OUT) {
			/*
			  * selection timeout => assuming no
			  * device present
			  */
			debug("Selection timeout ID %d\n",
			      pccb->target);
			return -ETIMEDOUT;
		}
		scsi_print_error(pccb);
		return -ENODEV;
	}
	perq = tempbuff[0];
	modi = tempbuff[1];
	if ((perq & 0x1f) == 0x1f)
		return -ENODEV; /* skip unknown devices */
	if ((modi & 0x80) == 0x80) /* drive is removable */
		dev_desc->removable = true;
	/* get info for this device */
	scsi_ident_cpy((unsigned char *)dev_desc->vendor,
		       &tempbuff[8], 8);
	scsi_ident_cpy((unsigned char *)dev_desc->product,
		       &tempbuff[16], 16);
	scsi_ident_cpy((unsigned char *)dev_desc->revision,
		       &tempbuff[32], 4);
	dev_desc->target = pccb->target;
	dev_desc->lun = pccb->lun;

	pccb->datalen = 0;
	scsi_setup_test_unit_ready(pccb);
	if (scsi_exec(pccb) != true) {
		if (dev_desc->removable) {
			dev_desc->type = perq;
			goto removable;
		}
		scsi_print_error(pccb);
		return -EINVAL;
	}
	if (scsi_read_capacity(pccb, &capacity, &blksz)) {
		scsi_print_error(pccb);
		return -EINVAL;
	}
	dev_desc->lba = capacity;
	dev_desc->blksz = blksz;
	dev_desc->log2blksz = LOG2(dev_desc->blksz);
	dev_desc->type = perq;
	part_init(&dev_desc[0]);
removable:
	return 0;
}

/*
 * (re)-scan the scsi bus and reports scsi device info
 * to the user if mode = 1
 */
int scsi_scan(int mode)
{
	unsigned char i, lun;
	int ret;

	if (mode == 1)
		printf("scanning bus for devices...\n");
	for (i = 0; i < CONFIG_SYS_SCSI_MAX_DEVICE; i++)
		scsi_init_dev_desc(&scsi_dev_desc[i], i);

	scsi_max_devs = 0;
	for (i = 0; i < CONFIG_SYS_SCSI_MAX_SCSI_ID; i++) {
		for (lun = 0; lun < CONFIG_SYS_SCSI_MAX_LUN; lun++) {
			scsi_dev_desc[scsi_max_devs].lun = lun;
			ret = scsi_detect_dev(i, &scsi_dev_desc[scsi_max_devs]);
			if (ret)
				continue;

			if (mode == 1) {
				printf("  Device %d: ", 0);
				dev_print(&scsi_dev_desc[scsi_max_devs]);
			} /* if mode */
			scsi_max_devs++;
		} /* next LUN */
	}
	if (scsi_max_devs > 0)
		scsi_curr_dev = 0;
	else
		scsi_curr_dev = -1;

	printf("Found %d device(s).\n", scsi_max_devs);
#ifndef CONFIG_SPL_BUILD
	setenv_ulong("scsidevs", scsi_max_devs);
#endif
	return 0;
}

#ifdef CONFIG_BLK
static const struct blk_ops scsi_blk_ops = {
	.read	= scsi_read,
	.write	= scsi_write,
};

U_BOOT_DRIVER(scsi_blk) = {
	.name		= "scsi_blk",
	.id		= UCLASS_BLK,
	.ops		= &scsi_blk_ops,
};
#else
U_BOOT_LEGACY_BLK(scsi) = {
	.if_typename	= "scsi",
	.if_type	= IF_TYPE_SCSI,
	.max_devs	= CONFIG_SYS_SCSI_MAX_DEVICE,
	.desc		= scsi_dev_desc,
};
#endif
