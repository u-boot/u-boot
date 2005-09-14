#include <common.h>

#if (CONFIG_COMMANDS & CFG_CMD_NAND) && defined CONFIG_NEW_NAND_CODE

#include <command.h>
#include <watchdog.h>
#include <malloc.h>
#include <asm/byteorder.h>

#ifdef CONFIG_SHOW_BOOT_PROGRESS
# include <status_led.h>
# define SHOW_BOOT_PROGRESS(arg)	show_boot_progress(arg)
#else
# define SHOW_BOOT_PROGRESS(arg)
#endif

#include <jffs2/jffs2.h>
#include <nand.h>

extern nand_info_t nand_info[];       /* info for NAND chips */

static int nand_dump_oob(nand_info_t *nand, ulong off)
{
	return 0;
}

static int nand_dump(nand_info_t *nand, ulong off)
{
	int i;
	u_char *buf, *p;

	buf = malloc(nand->oobblock + nand->oobsize);
	if (!buf) {
		puts("No memory for page buffer\n");
		return 1;
	}
	off &= ~(nand->oobblock - 1);
	i = nand_read_raw(nand, buf, off, nand->oobblock, nand->oobsize);
	if (i < 0) {
		printf("Error (%d) reading page %08x\n", i, off);
		free(buf);
		return 1;
	}
	printf("Page %08x dump:\n", off);
	i = nand->oobblock >> 4; p = buf;
	while (i--) {
		printf( "\t%02x %02x %02x %02x %02x %02x %02x %02x"
			"  %02x %02x %02x %02x %02x %02x %02x %02x\n",
			p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7],
			p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
		p += 16;
	}
	puts("OOB:\n");
	i = nand->oobsize >> 3;
	while (i--) {
		printf( "\t%02x %02x %02x %02x %02x %02x %02x %02x\n",
			p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
		p += 8;
	}
	free(buf);

	return 0;
}

/* ------------------------------------------------------------------------- */

static void
arg_off_size(int argc, char *argv[], ulong *off, ulong *size, ulong totsize)
{
	*off = 0;
	*size = 0;

#if defined(CONFIG_JFFS2_NAND) && defined(CFG_JFFS_CUSTOM_PART)
	if (argc >= 1 && strcmp(argv[0], "partition") == 0) {
		int part_num;
		struct part_info *part;
		const char *partstr;

		if (argc >= 2)
			partstr = argv[1];
		else
			partstr = getenv("partition");

		if (partstr)
			part_num = (int)simple_strtoul(partstr, NULL, 10);
		else
			part_num = 0;

		part = jffs2_part_info(part_num);
		if (part == NULL) {
			printf("\nInvalid partition %d\n", part_num);
			return;
		}
		*size = part->size;
		*off = (ulong)part->offset;
	} else
#endif
	{
		if (argc >= 1)
			*off = (ulong)simple_strtoul(argv[0], NULL, 16);
		else
			*off = 0;

		if (argc >= 2)
			*size = (ulong)simple_strtoul(argv[1], NULL, 16);
		else
			*size = totsize - *off;

	}

}

int do_nand(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	int i, dev, ret;
	ulong addr, off, size;
	char *cmd, *s;
	nand_info_t *nand;

	/* at least two arguments please */
	if (argc < 2)
		goto usage;

	cmd = argv[1];

	if (strcmp(cmd, "info") == 0) {

		putc('\n');
		for (i = 0; i < CFG_MAX_NAND_DEVICE; i++) {
			if (nand_info[i].name)
				printf("Device %d: %s\n", i, nand_info[i].name);
		}
		return 0;
	}

	if (strcmp(cmd, "device") == 0) {

		if (argc < 3) {
			if ((nand_curr_device < 0) ||
			    (nand_curr_device >= CFG_MAX_NAND_DEVICE))
				puts("\nno devices available\n");
			else
				printf("\nDevice %d: %s\n", nand_curr_device,
					nand_info[nand_curr_device].name);
			return 0;
		}
		dev = (int)simple_strtoul(argv[2], NULL, 10);
		if (dev < 0 || dev >= CFG_MAX_NAND_DEVICE || !nand_info[dev].name) {
			puts("No such device\n");
			return 1;
		}
		printf("Device %d: %s", dev, nand_info[dev].name);
		puts("... is now current device\n");
		nand_curr_device = dev;
		return 0;
	}

	if (strcmp(cmd, "bad") != 0 && strcmp(cmd, "erase") != 0 &&
	    strncmp(cmd, "dump", 4) != 0 &&
	    strncmp(cmd, "read", 4) != 0 && strncmp(cmd, "write", 5) != 0)
		goto usage;

	/* the following commands operate on the current device */
	if (nand_curr_device < 0 || nand_curr_device >= CFG_MAX_NAND_DEVICE ||
	    !nand_info[nand_curr_device].name) {
		puts("\nno devices available\n");
		return 1;
	}
	nand = &nand_info[nand_curr_device];

	if (strcmp(cmd, "bad") == 0) {
		printf("\nDevice %d bad blocks:\n", nand_curr_device);
		for (off = 0; off < nand->size; off += nand->erasesize)
			if (nand_block_isbad(nand, off))
				printf("  %08x\n", off);
		return 0;
	}

	if (strcmp(cmd, "erase") == 0) {
		arg_off_size(argc - 2, argv + 2, &off, &size, nand->size);
		if (off == 0 && size == 0)
			return 1;

		printf("\nNAND erase: device %d offset 0x%x, size 0x%x ",
		       nand_curr_device, off, size);
		ret = nand_erase(nand, off, size);
		printf("%s\n", ret ? "ERROR" : "OK");

		return ret == 0 ? 0 : 1;
	}

	if (strncmp(cmd, "dump", 4) == 0) {
		if (argc < 3)
			goto usage;

		s = strchr(cmd, '.');
		off = (int)simple_strtoul(argv[2], NULL, 16);

		if (s != NULL && strcmp(s, ".oob") == 0)
			ret = nand_dump_oob(nand, off);
		else
			ret = nand_dump(nand, off);

		return ret == 0 ? 1 : 0;

	}

	/* read write */
	if (strncmp(cmd, "read", 4) == 0 || strncmp(cmd, "write", 5) == 0) {
		if (argc < 4)
			goto usage;
/*
		s = strchr(cmd, '.');
		clean = CLEAN_NONE;
		if (s != NULL) {
			if (strcmp(s, ".jffs2") == 0 || strcmp(s, ".e") == 0
			    || strcmp(s, ".i"))
				clean = CLEAN_JFFS2;
		}
*/
		addr = (ulong)simple_strtoul(argv[2], NULL, 16);

		arg_off_size(argc - 3, argv + 3, &off, &size, nand->size);
		if (off == 0 && size == 0)
			return 1;

		i = strncmp(cmd, "read", 4) == 0;	/* 1 = read, 0 = write */
		printf("\nNAND %s: device %d offset %u, size %u ... ",
		       i ? "read" : "write", nand_curr_device, off, size);

		if (i)
			ret = nand_read(nand, off, &size, (u_char *)addr);
		else
			ret = nand_write(nand, off, &size, (u_char *)addr);

		printf(" %d bytes %s: %s\n", size,
		       i ? "read" : "written", ret ? "ERROR" : "OK");

		return ret == 0 ? 0 : 1;
	}
usage:
	printf("Usage:\n%s\n", cmdtp->usage);
	return 1;
}

U_BOOT_CMD(nand, 5, 1, do_nand,
	"nand    - NAND sub-system\n",
	"info                  - show available NAND devices\n"
	"nand device [dev]     - show or set current device\n"
	"nand read[.jffs2]     - addr off size\n"
	"nand write[.jffs2]    - addr off size - read/write `size' bytes starting\n"
	"    at offset `off' to/from memory address `addr'\n"
	"nand erase [clean] [off size] - erase `size' bytes from\n"
	"    offset `off' (entire device if not specified)\n"
	"nand bad - show bad blocks\n"
	"nand dump[.oob] off - dump page\n"
	"nand scrub - really clean NAND erasing bad blocks (UNSAFE)\n"
	"nand markbad off - mark bad block at offset (UNSAFE)\n"
	"nand biterr off - make a bit error at offset (UNSAFE)\n");

int do_nandboot(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	char *boot_device = NULL;
	char *ep;
	int dev;
	int r;
	ulong addr, cnt, offset = 0;
	image_header_t *hdr;
	nand_info_t *nand;

	switch (argc) {
	case 1:
		addr = CFG_LOAD_ADDR;
		boot_device = getenv("bootdevice");
		break;
	case 2:
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_device = getenv("bootdevice");
		break;
	case 3:
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_device = argv[2];
		break;
	case 4:
		addr = simple_strtoul(argv[1], NULL, 16);
		boot_device = argv[2];
		offset = simple_strtoul(argv[3], NULL, 16);
		break;
	default:
		printf("Usage:\n%s\n", cmdtp->usage);
		SHOW_BOOT_PROGRESS(-1);
		return 1;
	}

	if (!boot_device) {
		puts("\n** No boot device **\n");
		SHOW_BOOT_PROGRESS(-1);
		return 1;
	}

	dev = simple_strtoul(boot_device, &ep, 16);

	if (dev < 0 || dev >= CFG_MAX_NAND_DEVICE || !nand_info[dev].name) {
		printf("\n** Device %d not available\n", dev);
		SHOW_BOOT_PROGRESS(-1);
		return 1;
	}

	nand = &nand_info[dev];
	printf("\nLoading from device %d: %s (offset 0x%lx)\n",
	       dev, nand->name, offset);

	cnt = nand->oobblock;
	r = nand_read(nand, offset, &cnt, (u_char *) addr);
	if (r) {
		printf("** Read error on %d\n", dev);
		SHOW_BOOT_PROGRESS(-1);
		return 1;
	}

	hdr = (image_header_t *) addr;

	if (ntohl(hdr->ih_magic) != IH_MAGIC) {
		printf("\n** Bad Magic Number 0x%x **\n", hdr->ih_magic);
		SHOW_BOOT_PROGRESS(-1);
		return 1;
	}

	print_image_hdr(hdr);

	cnt = (ntohl(hdr->ih_size) + sizeof (image_header_t));

	r = nand_read(nand, offset, &cnt, (u_char *) addr);
	if (r) {
		printf("** Read error on %d\n", dev);
		SHOW_BOOT_PROGRESS(-1);
		return 1;
	}

	/* Loading ok, update default load address */

	load_addr = addr;

	/* Check if we should attempt an auto-start */
	if (((ep = getenv("autostart")) != NULL) && (strcmp(ep, "yes") == 0)) {
		char *local_args[2];
		extern int do_bootm(cmd_tbl_t *, int, int, char *[]);

		local_args[0] = argv[0];
		local_args[1] = NULL;

		printf("Automatic boot of image at addr 0x%08lx ...\n", addr);

		do_bootm(cmdtp, 0, 1, local_args);
		return 1;
	}
	return 0;
}

U_BOOT_CMD(nboot, 4, 1, do_nandboot,
	"nboot   - boot from NAND device\n", "loadAddr dev\n");


#endif				/* (CONFIG_COMMANDS & CFG_CMD_NAND) */
