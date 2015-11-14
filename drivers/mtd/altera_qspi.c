/*
 * Copyright (C) 2015 Thomas Chou <thomas@wytron.com.tw>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdt_support.h>
#include <flash.h>
#include <mtd.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * The QUADSPI_MEM_OP register is used to do memory protect and erase operations
 */
#define QUADSPI_MEM_OP_BULK_ERASE		0x00000001
#define QUADSPI_MEM_OP_SECTOR_ERASE		0x00000002
#define QUADSPI_MEM_OP_SECTOR_PROTECT		0x00000003

/*
 * The QUADSPI_ISR register is used to determine whether an invalid write or
 * erase operation trigerred an interrupt
 */
#define QUADSPI_ISR_ILLEGAL_ERASE		BIT(0)
#define QUADSPI_ISR_ILLEGAL_WRITE		BIT(1)

struct altera_qspi_regs {
	u32	rd_status;
	u32	rd_sid;
	u32	rd_rdid;
	u32	mem_op;
	u32	isr;
	u32	imr;
	u32	chip_select;
};

struct altera_qspi_platdata {
	struct altera_qspi_regs *regs;
	void *base;
	unsigned long size;
};

flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];	/* FLASH chips info */

void flash_print_info(flash_info_t *info)
{
	printf("Altera QSPI flash  Size: %ld MB in %d Sectors\n",
	       info->size >> 20, info->sector_count);
}

int flash_erase(flash_info_t *info, int s_first, int s_last)
{
	struct mtd_info *mtd = info->mtd;
	struct erase_info instr;
	int ret;

	memset(&instr, 0, sizeof(instr));
	instr.addr = mtd->erasesize * s_first;
	instr.len = mtd->erasesize * (s_last + 1 - s_first);
	ret = mtd_erase(mtd, &instr);
	if (ret)
		return ERR_NOT_ERASED;

	return 0;
}

int write_buff(flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	struct mtd_info *mtd = info->mtd;
	struct udevice *dev = mtd->dev;
	struct altera_qspi_platdata *pdata = dev_get_platdata(dev);
	ulong base = (ulong)pdata->base;
	loff_t to = addr - base;
	size_t retlen;
	int ret;

	ret = mtd_write(mtd, to, cnt, &retlen, src);
	if (ret)
		return ERR_NOT_ERASED;

	return 0;
}

unsigned long flash_init(void)
{
	struct udevice *dev;

	/* probe every MTD device */
	for (uclass_first_device(UCLASS_MTD, &dev);
	     dev;
	     uclass_next_device(&dev)) {
	}

	return flash_info[0].size;
}

static int altera_qspi_erase(struct mtd_info *mtd, struct erase_info *instr)
{
	struct udevice *dev = mtd->dev;
	struct altera_qspi_platdata *pdata = dev_get_platdata(dev);
	struct altera_qspi_regs *regs = pdata->regs;
	size_t addr = instr->addr;
	size_t len = instr->len;
	size_t end = addr + len;
	u32 sect;
	u32 stat;

	instr->state = MTD_ERASING;
	addr &= ~(mtd->erasesize - 1); /* get lower aligned address */
	while (addr < end) {
		sect = addr / mtd->erasesize;
		sect <<= 8;
		sect |= QUADSPI_MEM_OP_SECTOR_ERASE;
		debug("erase %08x\n", sect);
		writel(sect, &regs->mem_op);
		stat = readl(&regs->isr);
		if (stat & QUADSPI_ISR_ILLEGAL_ERASE) {
			/* erase failed, sector might be protected */
			debug("erase %08x fail %x\n", sect, stat);
			writel(stat, &regs->isr); /* clear isr */
			instr->state = MTD_ERASE_FAILED;
			return -EIO;
		}
		addr += mtd->erasesize;
	}
	instr->state = MTD_ERASE_DONE;
	mtd_erase_callback(instr);

	return 0;
}

static int altera_qspi_read(struct mtd_info *mtd, loff_t from, size_t len,
			    size_t *retlen, u_char *buf)
{
	struct udevice *dev = mtd->dev;
	struct altera_qspi_platdata *pdata = dev_get_platdata(dev);

	memcpy_fromio(buf, pdata->base + from, len);
	*retlen = len;

	return 0;
}

static int altera_qspi_write(struct mtd_info *mtd, loff_t to, size_t len,
			     size_t *retlen, const u_char *buf)
{
	struct udevice *dev = mtd->dev;
	struct altera_qspi_platdata *pdata = dev_get_platdata(dev);
	struct altera_qspi_regs *regs = pdata->regs;
	u32 stat;

	memcpy_toio(pdata->base + to, buf, len);
	/* check whether write triggered a illegal write interrupt */
	stat = readl(&regs->isr);
	if (stat & QUADSPI_ISR_ILLEGAL_WRITE) {
		/* write failed, sector might be protected */
		debug("write fail %x\n", stat);
		writel(stat, &regs->isr); /* clear isr */
		return -EIO;
	}
	*retlen = len;

	return 0;
}

static void altera_qspi_sync(struct mtd_info *mtd)
{
}

static int altera_qspi_probe(struct udevice *dev)
{
	struct altera_qspi_platdata *pdata = dev_get_platdata(dev);
	struct altera_qspi_regs *regs = pdata->regs;
	unsigned long base = (unsigned long)pdata->base;
	struct mtd_info *mtd;
	flash_info_t *flash = &flash_info[0];
	u32 rdid;
	int i;

	rdid = readl(&regs->rd_rdid);
	debug("rdid %x\n", rdid);

	mtd = dev_get_uclass_priv(dev);
	mtd->dev = dev;
	mtd->name		= "nor0";
	mtd->type		= MTD_NORFLASH;
	mtd->flags		= MTD_CAP_NORFLASH;
	mtd->size		= 1 << ((rdid & 0xff) - 6);
	mtd->writesize		= 1;
	mtd->writebufsize	= mtd->writesize;
	mtd->_erase		= altera_qspi_erase;
	mtd->_read		= altera_qspi_read;
	mtd->_write		= altera_qspi_write;
	mtd->_sync		= altera_qspi_sync;
	mtd->numeraseregions = 0;
	mtd->erasesize = 0x10000;
	if (add_mtd_device(mtd))
		return -ENOMEM;

	flash->mtd = mtd;
	flash->size = mtd->size;
	flash->sector_count = mtd->size / mtd->erasesize;
	flash->flash_id = rdid;
	flash->start[0] = base;
	for (i = 1; i < flash->sector_count; i++)
		flash->start[i] = flash->start[i - 1] + mtd->erasesize;
	gd->bd->bi_flashstart = base;

	return 0;
}

static int altera_qspi_ofdata_to_platdata(struct udevice *dev)
{
	struct altera_qspi_platdata *pdata = dev_get_platdata(dev);
	void *blob = (void *)gd->fdt_blob;
	int node = dev->of_offset;
	const char *list, *end;
	const fdt32_t *cell;
	void *base;
	unsigned long addr, size;
	int parent, addrc, sizec;
	int len, idx;

	/*
	 * decode regs. there are multiple reg tuples, and they need to
	 * match with reg-names.
	 */
	parent = fdt_parent_offset(blob, node);
	of_bus_default_count_cells(blob, parent, &addrc, &sizec);
	list = fdt_getprop(blob, node, "reg-names", &len);
	if (!list)
		return -ENOENT;
	end = list + len;
	cell = fdt_getprop(blob, node, "reg", &len);
	if (!cell)
		return -ENOENT;
	idx = 0;
	while (list < end) {
		addr = fdt_translate_address((void *)blob,
					     node, cell + idx);
		size = fdt_addr_to_cpu(cell[idx + addrc]);
		base = map_physmem(addr, size, MAP_NOCACHE);
		len = strlen(list);
		if (strcmp(list, "avl_csr") == 0) {
			pdata->regs = base;
		} else if (strcmp(list, "avl_mem") == 0) {
			pdata->base = base;
			pdata->size = size;
		}
		idx += addrc + sizec;
		list += (len + 1);
	}

	return 0;
}

static const struct udevice_id altera_qspi_ids[] = {
	{ .compatible = "altr,quadspi-1.0" },
	{}
};

U_BOOT_DRIVER(altera_qspi) = {
	.name	= "altera_qspi",
	.id	= UCLASS_MTD,
	.of_match = altera_qspi_ids,
	.ofdata_to_platdata = altera_qspi_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct altera_qspi_platdata),
	.probe	= altera_qspi_probe,
};
