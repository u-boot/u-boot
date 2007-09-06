/*
 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

/*
 * Boot support
 */
#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <image.h>
#include <malloc.h>
#include <zlib.h>
#include <bzlib.h>
#include <environment.h>
#include <asm/byteorder.h>

#if defined(CONFIG_OF_LIBFDT)
#include <fdt.h>
#include <libfdt.h>
#include <fdt_support.h>
#endif
#if defined(CONFIG_OF_FLAT_TREE)
#include <ft_build.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

/*cmd_boot.c*/
extern int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

#if defined(CONFIG_TIMESTAMP) || defined(CONFIG_CMD_DATE)
#include <rtc.h>
#endif

#ifdef CFG_HUSH_PARSER
#include <hush.h>
#endif

#ifdef CFG_INIT_RAM_LOCK
#include <asm/cache.h>
#endif

#ifdef CONFIG_LOGBUFFER
#include <logbuff.h>
#endif

#ifdef CONFIG_HAS_DATAFLASH
#include <dataflash.h>
#endif

/*
 * Some systems (for example LWMON) have very short watchdog periods;
 * we must make sure to split long operations like memmove() or
 * crc32() into reasonable chunks.
 */
#if defined(CONFIG_HW_WATCHDOG) || defined(CONFIG_WATCHDOG)
# define CHUNKSZ (64 * 1024)
#endif

int  gunzip (void *, int, unsigned char *, unsigned long *);

static void *zalloc(void *, unsigned, unsigned);
static void zfree(void *, void *, unsigned);

#if defined(CONFIG_CMD_IMI)
static int image_info (unsigned long addr);
#endif

#if defined(CONFIG_CMD_IMLS)
#include <flash.h>
extern flash_info_t flash_info[]; /* info for FLASH chips */
static int do_imls (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
#endif

static void print_type (image_header_t *hdr);

#ifdef __I386__
image_header_t *fake_header(image_header_t *hdr, void *ptr, int size);
#endif

/*
 *  Continue booting an OS image; caller already has:
 *  - copied image header to global variable `header'
 *  - checked header magic number, checksums (both header & image),
 *  - verified image architecture (PPC) and type (KERNEL or MULTI),
 *  - loaded (first part of) image to header load address,
 *  - disabled interrupts.
 */
typedef void boot_os_Fcn (cmd_tbl_t *cmdtp, int flag,
			  int	argc, char *argv[],
			  ulong	addr,		/* of image to boot */
			  ulong	*len_ptr,	/* multi-file image length table */
			  int	verify);	/* getenv("verify")[0] != 'n' */

#ifdef	DEBUG
extern int do_bdinfo ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
#endif

#ifdef CONFIG_PPC
static boot_os_Fcn do_bootm_linux;
#else
extern boot_os_Fcn do_bootm_linux;
#endif
#ifdef CONFIG_SILENT_CONSOLE
static void fixup_silent_linux (void);
#endif
static boot_os_Fcn do_bootm_netbsd;
static boot_os_Fcn do_bootm_rtems;
#if defined(CONFIG_CMD_ELF)
static boot_os_Fcn do_bootm_vxworks;
static boot_os_Fcn do_bootm_qnxelf;
int do_bootvx ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[] );
int do_bootelf (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[] );
#endif
#if defined(CONFIG_ARTOS) && defined(CONFIG_PPC)
static boot_os_Fcn do_bootm_artos;
#endif
#ifdef CONFIG_LYNXKDI
static boot_os_Fcn do_bootm_lynxkdi;
extern void lynxkdi_boot( image_header_t * );
#endif

#ifndef CFG_BOOTM_LEN
#define CFG_BOOTM_LEN	0x800000	/* use 8MByte as default max gunzip size */
#endif

image_header_t header;

ulong load_addr = CFG_LOAD_ADDR;		/* Default Load Address */

int do_bootm (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong	iflag;
	ulong	addr;
	ulong	data, len, checksum;
	ulong  *len_ptr;
	uint	unc_len = CFG_BOOTM_LEN;
	int	i, verify;
	char	*name, *s;
	int	(*appl)(int, char *[]);
	image_header_t *hdr = &header;

	s = getenv ("verify");
	verify = (s && (*s == 'n')) ? 0 : 1;

	if (argc < 2) {
		addr = load_addr;
	} else {
		addr = simple_strtoul(argv[1], NULL, 16);
	}

	show_boot_progress (1);
	printf ("## Booting image at %08lx ...\n", addr);

	/* Copy header so we can blank CRC field for re-calculation */
#ifdef CONFIG_HAS_DATAFLASH
	if (addr_dataflash(addr)){
		read_dataflash(addr, sizeof(image_header_t), (char *)&header);
	} else
#endif
	memmove (&header, (char *)addr, sizeof(image_header_t));

	if (ntohl(hdr->ih_magic) != IH_MAGIC) {
#ifdef __I386__	/* correct image format not implemented yet - fake it */
		if (fake_header(hdr, (void*)addr, -1) != NULL) {
			/* to compensate for the addition below */
			addr -= sizeof(image_header_t);
			/* turnof verify,
			 * fake_header() does not fake the data crc
			 */
			verify = 0;
		} else
#endif	/* __I386__ */
	    {
		puts ("Bad Magic Number\n");
		show_boot_progress (-1);
		return 1;
	    }
	}
	show_boot_progress (2);

	data = (ulong)&header;
	len  = sizeof(image_header_t);

	checksum = ntohl(hdr->ih_hcrc);
	hdr->ih_hcrc = 0;

	if (crc32 (0, (uchar *)data, len) != checksum) {
		puts ("Bad Header Checksum\n");
		show_boot_progress (-2);
		return 1;
	}
	show_boot_progress (3);

#ifdef CONFIG_HAS_DATAFLASH
	if (addr_dataflash(addr)){
		len  = ntohl(hdr->ih_size) + sizeof(image_header_t);
		read_dataflash(addr, len, (char *)CFG_LOAD_ADDR);
		addr = CFG_LOAD_ADDR;
	}
#endif


	/* for multi-file images we need the data part, too */
	print_image_hdr ((image_header_t *)addr);

	data = addr + sizeof(image_header_t);
	len  = ntohl(hdr->ih_size);

	if (verify) {
		puts ("   Verifying Checksum ... ");
		if (crc32 (0, (uchar *)data, len) != ntohl(hdr->ih_dcrc)) {
			printf ("Bad Data CRC\n");
			show_boot_progress (-3);
			return 1;
		}
		puts ("OK\n");
	}
	show_boot_progress (4);

	len_ptr = (ulong *)data;

#if defined(__ARM__)
	if (hdr->ih_arch != IH_CPU_ARM)
#elif defined(__avr32__)
	if (hdr->ih_arch != IH_CPU_AVR32)
#elif defined(__bfin__)
	if (hdr->ih_arch != IH_CPU_BLACKFIN)
#elif defined(__I386__)
	if (hdr->ih_arch != IH_CPU_I386)
#elif defined(__M68K__)
	if (hdr->ih_arch != IH_CPU_M68K)
#elif defined(__microblaze__)
	if (hdr->ih_arch != IH_CPU_MICROBLAZE)
#elif defined(__mips__)
	if (hdr->ih_arch != IH_CPU_MIPS)
#elif defined(__nios__)
	if (hdr->ih_arch != IH_CPU_NIOS)
#elif defined(__nios2__)
	if (hdr->ih_arch != IH_CPU_NIOS2)
#elif defined(__PPC__)
	if (hdr->ih_arch != IH_CPU_PPC)
#else
# error Unknown CPU type
#endif
	{
		printf ("Unsupported Architecture 0x%x\n", hdr->ih_arch);
		show_boot_progress (-4);
		return 1;
	}
	show_boot_progress (5);

	switch (hdr->ih_type) {
	case IH_TYPE_STANDALONE:
		name = "Standalone Application";
		/* A second argument overwrites the load address */
		if (argc > 2) {
			hdr->ih_load = htonl(simple_strtoul(argv[2], NULL, 16));
		}
		break;
	case IH_TYPE_KERNEL:
		name = "Kernel Image";
		break;
	case IH_TYPE_MULTI:
		name = "Multi-File Image";
		len  = ntohl(len_ptr[0]);
		/* OS kernel is always the first image */
		data += 8; /* kernel_len + terminator */
		for (i=1; len_ptr[i]; ++i)
			data += 4;
		break;
	default: printf ("Wrong Image Type for %s command\n", cmdtp->name);
		show_boot_progress (-5);
		return 1;
	}
	show_boot_progress (6);

	/*
	 * We have reached the point of no return: we are going to
	 * overwrite all exception vector code, so we cannot easily
	 * recover from any failures any more...
	 */

	iflag = disable_interrupts();

#ifdef CONFIG_AMIGAONEG3SE
	/*
	 * We've possible left the caches enabled during
	 * bios emulation, so turn them off again
	 */
	icache_disable();
	invalidate_l1_instruction_cache();
	flush_data_cache();
	dcache_disable();
#endif

	switch (hdr->ih_comp) {
	case IH_COMP_NONE:
		if(ntohl(hdr->ih_load) == addr) {
			printf ("   XIP %s ... ", name);
		} else {
#if defined(CONFIG_HW_WATCHDOG) || defined(CONFIG_WATCHDOG)
			size_t l = len;
			void *to = (void *)ntohl(hdr->ih_load);
			void *from = (void *)data;

			printf ("   Loading %s ... ", name);

			while (l > 0) {
				size_t tail = (l > CHUNKSZ) ? CHUNKSZ : l;
				WATCHDOG_RESET();
				memmove (to, from, tail);
				to += tail;
				from += tail;
				l -= tail;
			}
#else	/* !(CONFIG_HW_WATCHDOG || CONFIG_WATCHDOG) */
			memmove ((void *) ntohl(hdr->ih_load), (uchar *)data, len);
#endif	/* CONFIG_HW_WATCHDOG || CONFIG_WATCHDOG */
		}
		break;
	case IH_COMP_GZIP:
		printf ("   Uncompressing %s ... ", name);
		if (gunzip ((void *)ntohl(hdr->ih_load), unc_len,
			    (uchar *)data, &len) != 0) {
			puts ("GUNZIP ERROR - must RESET board to recover\n");
			show_boot_progress (-6);
			do_reset (cmdtp, flag, argc, argv);
		}
		break;
#ifdef CONFIG_BZIP2
	case IH_COMP_BZIP2:
		printf ("   Uncompressing %s ... ", name);
		/*
		 * If we've got less than 4 MB of malloc() space,
		 * use slower decompression algorithm which requires
		 * at most 2300 KB of memory.
		 */
		i = BZ2_bzBuffToBuffDecompress ((char*)ntohl(hdr->ih_load),
						&unc_len, (char *)data, len,
						CFG_MALLOC_LEN < (4096 * 1024), 0);
		if (i != BZ_OK) {
			printf ("BUNZIP2 ERROR %d - must RESET board to recover\n", i);
			show_boot_progress (-6);
			do_reset (cmdtp, flag, argc, argv);
		}
		break;
#endif /* CONFIG_BZIP2 */
	default:
		if (iflag)
			enable_interrupts();
		printf ("Unimplemented compression type %d\n", hdr->ih_comp);
		show_boot_progress (-7);
		return 1;
	}
	puts ("OK\n");
	show_boot_progress (7);

	switch (hdr->ih_type) {
	case IH_TYPE_STANDALONE:
		if (iflag)
			enable_interrupts();

		/* load (and uncompress), but don't start if "autostart"
		 * is set to "no"
		 */
		if (((s = getenv("autostart")) != NULL) && (strcmp(s,"no") == 0)) {
			char buf[32];
			sprintf(buf, "%lX", len);
			setenv("filesize", buf);
			return 0;
		}
		appl = (int (*)(int, char *[]))ntohl(hdr->ih_ep);
		(*appl)(argc-1, &argv[1]);
		return 0;
	case IH_TYPE_KERNEL:
	case IH_TYPE_MULTI:
		/* handled below */
		break;
	default:
		if (iflag)
			enable_interrupts();
		printf ("Can't boot image type %d\n", hdr->ih_type);
		show_boot_progress (-8);
		return 1;
	}
	show_boot_progress (8);

	switch (hdr->ih_os) {
	default:			/* handled by (original) Linux case */
	case IH_OS_LINUX:
#ifdef CONFIG_SILENT_CONSOLE
	    fixup_silent_linux();
#endif
	    do_bootm_linux  (cmdtp, flag, argc, argv,
			     addr, len_ptr, verify);
	    break;
	case IH_OS_NETBSD:
	    do_bootm_netbsd (cmdtp, flag, argc, argv,
			     addr, len_ptr, verify);
	    break;

#ifdef CONFIG_LYNXKDI
	case IH_OS_LYNXOS:
	    do_bootm_lynxkdi (cmdtp, flag, argc, argv,
			     addr, len_ptr, verify);
	    break;
#endif

	case IH_OS_RTEMS:
	    do_bootm_rtems (cmdtp, flag, argc, argv,
			     addr, len_ptr, verify);
	    break;

#if defined(CONFIG_CMD_ELF)
	case IH_OS_VXWORKS:
	    do_bootm_vxworks (cmdtp, flag, argc, argv,
			      addr, len_ptr, verify);
	    break;
	case IH_OS_QNX:
	    do_bootm_qnxelf (cmdtp, flag, argc, argv,
			      addr, len_ptr, verify);
	    break;
#endif
#ifdef CONFIG_ARTOS
	case IH_OS_ARTOS:
	    do_bootm_artos  (cmdtp, flag, argc, argv,
			     addr, len_ptr, verify);
	    break;
#endif
	}

	show_boot_progress (-9);
#ifdef DEBUG
	puts ("\n## Control returned to monitor - resetting...\n");
	do_reset (cmdtp, flag, argc, argv);
#endif
	return 1;
}

U_BOOT_CMD(
 	bootm,	CFG_MAXARGS,	1,	do_bootm,
 	"bootm   - boot application image from memory\n",
 	"[addr [arg ...]]\n    - boot application image stored in memory\n"
 	"\tpassing arguments 'arg ...'; when booting a Linux kernel,\n"
 	"\t'arg' can be the address of an initrd image\n"
#if defined(CONFIG_OF_FLAT_TREE) || defined(CONFIG_OF_LIBFDT)
	"\tWhen booting a Linux kernel which requires a flat device-tree\n"
	"\ta third argument is required which is the address of the of the\n"
	"\tdevice-tree blob. To boot that kernel without an initrd image,\n"
	"\tuse a '-' for the second argument. If you do not pass a third\n"
	"\ta bd_info struct will be passed instead\n"
#endif
);

#ifdef CONFIG_SILENT_CONSOLE
static void
fixup_silent_linux ()
{
	char buf[256], *start, *end;
	char *cmdline = getenv ("bootargs");

	/* Only fix cmdline when requested */
	if (!(gd->flags & GD_FLG_SILENT))
		return;

	debug ("before silent fix-up: %s\n", cmdline);
	if (cmdline) {
		if ((start = strstr (cmdline, "console=")) != NULL) {
			end = strchr (start, ' ');
			strncpy (buf, cmdline, (start - cmdline + 8));
			if (end)
				strcpy (buf + (start - cmdline + 8), end);
			else
				buf[start - cmdline + 8] = '\0';
		} else {
			strcpy (buf, cmdline);
			strcat (buf, " console=");
		}
	} else {
		strcpy (buf, "console=");
	}

	setenv ("bootargs", buf);
	debug ("after silent fix-up: %s\n", buf);
}
#endif /* CONFIG_SILENT_CONSOLE */

#ifdef CONFIG_PPC
static void  __attribute__((noinline))
do_bootm_linux (cmd_tbl_t *cmdtp, int flag,
		int	argc, char *argv[],
		ulong	addr,
		ulong	*len_ptr,
		int	verify)
{
	ulong	sp;
	ulong	len, checksum;
	ulong	initrd_start, initrd_end;
	ulong	cmd_start, cmd_end;
	ulong	initrd_high;
	ulong	data;
	int	initrd_copy_to_ram = 1;
	char    *cmdline;
	char	*s;
	bd_t	*kbd;
	void	(*kernel)(bd_t *, ulong, ulong, ulong, ulong);
	image_header_t *hdr = &header;
#if defined(CONFIG_OF_FLAT_TREE) || defined(CONFIG_OF_LIBFDT)
	char	*of_flat_tree = NULL;
	ulong	of_data = 0;
#endif

	if ((s = getenv ("initrd_high")) != NULL) {
		/* a value of "no" or a similar string will act like 0,
		 * turning the "load high" feature off. This is intentional.
		 */
		initrd_high = simple_strtoul(s, NULL, 16);
		if (initrd_high == ~0)
			initrd_copy_to_ram = 0;
	} else {	/* not set, no restrictions to load high */
		initrd_high = ~0;
	}

#ifdef CONFIG_LOGBUFFER
	kbd=gd->bd;
	/* Prevent initrd from overwriting logbuffer */
	if (initrd_high < (kbd->bi_memsize-LOGBUFF_LEN-LOGBUFF_OVERHEAD))
		initrd_high = kbd->bi_memsize-LOGBUFF_LEN-LOGBUFF_OVERHEAD;
	debug ("## Logbuffer at 0x%08lX ", kbd->bi_memsize-LOGBUFF_LEN);
#endif

	/*
	 * Booting a (Linux) kernel image
	 *
	 * Allocate space for command line and board info - the
	 * address should be as high as possible within the reach of
	 * the kernel (see CFG_BOOTMAPSZ settings), but in unused
	 * memory, which means far enough below the current stack
	 * pointer.
	 */

	asm( "mr %0,1": "=r"(sp) : );

	debug ("## Current stack ends at 0x%08lX ", sp);

	sp -= 2048;		/* just to be sure */
	if (sp > CFG_BOOTMAPSZ)
		sp = CFG_BOOTMAPSZ;
	sp &= ~0xF;

	debug ("=> set upper limit to 0x%08lX\n", sp);

	cmdline = (char *)((sp - CFG_BARGSIZE) & ~0xF);
	kbd = (bd_t *)(((ulong)cmdline - sizeof(bd_t)) & ~0xF);

	if ((s = getenv("bootargs")) == NULL)
		s = "";

	strcpy (cmdline, s);

	cmd_start    = (ulong)&cmdline[0];
	cmd_end      = cmd_start + strlen(cmdline);

	*kbd = *(gd->bd);

#ifdef	DEBUG
	printf ("## cmdline at 0x%08lX ... 0x%08lX\n", cmd_start, cmd_end);

	do_bdinfo (NULL, 0, 0, NULL);
#endif

	if ((s = getenv ("clocks_in_mhz")) != NULL) {
		/* convert all clock information to MHz */
		kbd->bi_intfreq /= 1000000L;
		kbd->bi_busfreq /= 1000000L;
#if defined(CONFIG_MPC8220)
	kbd->bi_inpfreq /= 1000000L;
	kbd->bi_pcifreq /= 1000000L;
	kbd->bi_pevfreq /= 1000000L;
	kbd->bi_flbfreq /= 1000000L;
	kbd->bi_vcofreq /= 1000000L;
#endif
#if defined(CONFIG_CPM2)
		kbd->bi_cpmfreq /= 1000000L;
		kbd->bi_brgfreq /= 1000000L;
		kbd->bi_sccfreq /= 1000000L;
		kbd->bi_vco     /= 1000000L;
#endif
#if defined(CONFIG_MPC5xxx)
		kbd->bi_ipbfreq /= 1000000L;
		kbd->bi_pcifreq /= 1000000L;
#endif /* CONFIG_MPC5xxx */
	}

	kernel = (void (*)(bd_t *, ulong, ulong, ulong, ulong)) ntohl(hdr->ih_ep);

	/*
	 * Check if there is an initrd image
	 */

#if defined(CONFIG_OF_FLAT_TREE) || defined(CONFIG_OF_LIBFDT)
	/* Look for a '-' which indicates to ignore the ramdisk argument */
	if (argc >= 3 && strcmp(argv[2], "-") ==  0) {
			debug ("Skipping initrd\n");
			len = data = 0;
		}
	else
#endif
	if (argc >= 3) {
		debug ("Not skipping initrd\n");
		show_boot_progress (9);

		addr = simple_strtoul(argv[2], NULL, 16);

		printf ("## Loading RAMDisk Image at %08lx ...\n", addr);

		/* Copy header so we can blank CRC field for re-calculation */
		memmove (&header, (char *)addr, sizeof(image_header_t));

		if (ntohl(hdr->ih_magic)  != IH_MAGIC) {
			puts ("Bad Magic Number\n");
			show_boot_progress (-10);
			do_reset (cmdtp, flag, argc, argv);
		}

		data = (ulong)&header;
		len  = sizeof(image_header_t);

		checksum = ntohl(hdr->ih_hcrc);
		hdr->ih_hcrc = 0;

		if (crc32 (0, (uchar *)data, len) != checksum) {
			puts ("Bad Header Checksum\n");
			show_boot_progress (-11);
			do_reset (cmdtp, flag, argc, argv);
		}

		show_boot_progress (10);

		print_image_hdr (hdr);

		data = addr + sizeof(image_header_t);
		len  = ntohl(hdr->ih_size);

		if (verify) {
			ulong csum = 0;
#if defined(CONFIG_HW_WATCHDOG) || defined(CONFIG_WATCHDOG)
			ulong cdata = data, edata = cdata + len;
#endif	/* CONFIG_HW_WATCHDOG || CONFIG_WATCHDOG */

			puts ("   Verifying Checksum ... ");

#if defined(CONFIG_HW_WATCHDOG) || defined(CONFIG_WATCHDOG)

			while (cdata < edata) {
				ulong chunk = edata - cdata;

				if (chunk > CHUNKSZ)
					chunk = CHUNKSZ;
				csum = crc32 (csum, (uchar *)cdata, chunk);
				cdata += chunk;

				WATCHDOG_RESET();
			}
#else	/* !(CONFIG_HW_WATCHDOG || CONFIG_WATCHDOG) */
			csum = crc32 (0, (uchar *)data, len);
#endif	/* CONFIG_HW_WATCHDOG || CONFIG_WATCHDOG */

			if (csum != ntohl(hdr->ih_dcrc)) {
				puts ("Bad Data CRC\n");
				show_boot_progress (-12);
				do_reset (cmdtp, flag, argc, argv);
			}
			puts ("OK\n");
		}

		show_boot_progress (11);

		if ((hdr->ih_os   != IH_OS_LINUX)	||
		    (hdr->ih_arch != IH_CPU_PPC)	||
		    (hdr->ih_type != IH_TYPE_RAMDISK)	) {
			puts ("No Linux PPC Ramdisk Image\n");
			show_boot_progress (-13);
			do_reset (cmdtp, flag, argc, argv);
		}

		/*
		 * Now check if we have a multifile image
		 */
	} else if ((hdr->ih_type==IH_TYPE_MULTI) && (len_ptr[1])) {
		u_long tail    = ntohl(len_ptr[0]) % 4;
		int i;

		show_boot_progress (13);

		/* skip kernel length and terminator */
		data = (ulong)(&len_ptr[2]);
		/* skip any additional image length fields */
		for (i=1; len_ptr[i]; ++i)
			data += 4;
		/* add kernel length, and align */
		data += ntohl(len_ptr[0]);
		if (tail) {
			data += 4 - tail;
		}

		len   = ntohl(len_ptr[1]);

	} else {
		/*
		 * no initrd image
		 */
		show_boot_progress (14);

		len = data = 0;
	}

#if defined(CONFIG_OF_FLAT_TREE) || defined(CONFIG_OF_LIBFDT)
	if(argc > 3) {
		of_flat_tree = (char *) simple_strtoul(argv[3], NULL, 16);
		hdr = (image_header_t *)of_flat_tree;
#if defined(CONFIG_OF_FLAT_TREE)
		if (*((ulong *)(of_flat_tree + sizeof(image_header_t))) != OF_DT_HEADER) {
#else
		if (fdt_check_header(of_flat_tree + sizeof(image_header_t)) != 0) {
#endif
#ifndef CFG_NO_FLASH
			if (addr2info((ulong)of_flat_tree) != NULL)
				of_data = (ulong)of_flat_tree;
#endif
		} else if (ntohl(hdr->ih_magic) == IH_MAGIC) {
			printf("## Flat Device Tree at %08lX\n", hdr);
			print_image_hdr(hdr);

			if ((ntohl(hdr->ih_load) <  ((unsigned long)hdr + ntohl(hdr->ih_size) + sizeof(hdr))) &&
			   ((ntohl(hdr->ih_load) + ntohl(hdr->ih_size)) > (unsigned long)hdr)) {
				puts ("ERROR: fdt overwritten - "
					"must RESET the board to recover.\n");
				do_reset (cmdtp, flag, argc, argv);
			}

			puts ("   Verifying Checksum ... ");
			memmove (&header, (char *)hdr, sizeof(image_header_t));
			checksum = ntohl(header.ih_hcrc);
			header.ih_hcrc = 0;

			if(checksum != crc32(0, (uchar *)&header, sizeof(image_header_t))) {
				puts ("ERROR: fdt header checksum invalid - "
					"must RESET the board to recover.\n");
				do_reset (cmdtp, flag, argc, argv);
			}

			checksum = ntohl(hdr->ih_dcrc);
			addr = (ulong)((uchar *)(hdr) + sizeof(image_header_t));

			if(checksum != crc32(0, (uchar *)addr, ntohl(hdr->ih_size))) {
				puts ("ERROR: fdt checksum invalid - "
					"must RESET the board to recover.\n");
				do_reset (cmdtp, flag, argc, argv);
			}
			puts ("OK\n");

			if (ntohl(hdr->ih_type) != IH_TYPE_FLATDT) {
				puts ("ERROR: uImage is not a fdt - "
					"must RESET the board to recover.\n");
				do_reset (cmdtp, flag, argc, argv);
			}
			if (ntohl(hdr->ih_comp) != IH_COMP_NONE) {
				puts ("ERROR: uImage is compressed - "
					"must RESET the board to recover.\n");
				do_reset (cmdtp, flag, argc, argv);
			}
#if defined(CONFIG_OF_FLAT_TREE)
			if (*((ulong *)(of_flat_tree + sizeof(image_header_t))) != OF_DT_HEADER) {
#else
			if (fdt_check_header(of_flat_tree + sizeof(image_header_t)) != 0) {
#endif
				puts ("ERROR: uImage data is not a fdt - "
					"must RESET the board to recover.\n");
				do_reset (cmdtp, flag, argc, argv);
			}

			memmove((void *)ntohl(hdr->ih_load),
		       		(void *)(of_flat_tree + sizeof(image_header_t)),
				ntohl(hdr->ih_size));
			of_flat_tree = (char *)ntohl(hdr->ih_load);
		} else {
			puts ("Did not find a flat Flat Device Tree.\n"
				"Must RESET the board to recover.\n");
			do_reset (cmdtp, flag, argc, argv);
		}
		printf ("   Booting using the fdt at 0x%x\n",
				of_flat_tree);
	} else if ((hdr->ih_type==IH_TYPE_MULTI) && (len_ptr[1]) && (len_ptr[2])) {
		u_long tail    = ntohl(len_ptr[0]) % 4;
		int i;

		/* skip kernel length, initrd length, and terminator */
		of_data = (ulong)(&len_ptr[3]);
		/* skip any additional image length fields */
		for (i=2; len_ptr[i]; ++i)
			of_data += 4;
		/* add kernel length, and align */
		of_data += ntohl(len_ptr[0]);
		if (tail) {
			of_data += 4 - tail;
		}

		/* add initrd length, and align */
		tail = ntohl(len_ptr[1]) % 4;
		of_data += ntohl(len_ptr[1]);
		if (tail) {
			of_data += 4 - tail;
		}

#if defined(CONFIG_OF_FLAT_TREE)
		if (*((ulong *)(of_flat_tree + sizeof(image_header_t))) != OF_DT_HEADER) {
#else
		if (fdt_check_header(of_flat_tree + sizeof(image_header_t)) != 0) {
#endif
			puts ("ERROR: image is not a fdt - "
				"must RESET the board to recover.\n");
			do_reset (cmdtp, flag, argc, argv);
		}

#if defined(CONFIG_OF_FLAT_TREE)
		if (((struct boot_param_header *)of_data)->totalsize != ntohl(len_ptr[2])) {
#else
		if (be32_to_cpu(fdt_totalsize(of_data)) !=  ntohl(len_ptr[2])) {
#endif
			puts ("ERROR: fdt size != image size - "
				"must RESET the board to recover.\n");
			do_reset (cmdtp, flag, argc, argv);
		}
	}
#endif
	if (!data) {
		debug ("No initrd\n");
	}

	if (data) {
	    if (!initrd_copy_to_ram) {	/* zero-copy ramdisk support */
		initrd_start = data;
		initrd_end = initrd_start + len;
	    } else {
		initrd_start  = (ulong)kbd - len;
		initrd_start &= ~(4096 - 1);	/* align on page */

		if (initrd_high) {
			ulong nsp;

			/*
			 * the inital ramdisk does not need to be within
			 * CFG_BOOTMAPSZ as it is not accessed until after
			 * the mm system is initialised.
			 *
			 * do the stack bottom calculation again and see if
			 * the initrd will fit just below the monitor stack
			 * bottom without overwriting the area allocated
			 * above for command line args and board info.
			 */
			asm( "mr %0,1": "=r"(nsp) : );
			nsp -= 2048;		/* just to be sure */
			nsp &= ~0xF;
			if (nsp > initrd_high)	/* limit as specified */
				nsp = initrd_high;
			nsp -= len;
			nsp &= ~(4096 - 1);	/* align on page */
			if (nsp >= sp)
				initrd_start = nsp;
		}

		show_boot_progress (12);

		debug ("## initrd at 0x%08lX ... 0x%08lX (len=%ld=0x%lX)\n",
			data, data + len - 1, len, len);

		initrd_end    = initrd_start + len;
		printf ("   Loading Ramdisk to %08lx, end %08lx ... ",
			initrd_start, initrd_end);
#if defined(CONFIG_HW_WATCHDOG) || defined(CONFIG_WATCHDOG)
		{
			size_t l = len;
			void *to = (void *)initrd_start;
			void *from = (void *)data;

			while (l > 0) {
				size_t tail = (l > CHUNKSZ) ? CHUNKSZ : l;
				WATCHDOG_RESET();
				memmove (to, from, tail);
				to += tail;
				from += tail;
				l -= tail;
			}
		}
#else	/* !(CONFIG_HW_WATCHDOG || CONFIG_WATCHDOG) */
		memmove ((void *)initrd_start, (void *)data, len);
#endif	/* CONFIG_HW_WATCHDOG || CONFIG_WATCHDOG */
		puts ("OK\n");
	    }
	} else {
		initrd_start = 0;
		initrd_end = 0;
	}

#if defined(CONFIG_OF_LIBFDT)

#ifdef CFG_BOOTMAPSZ
	/*
	 * The blob must be within CFG_BOOTMAPSZ,
	 * so we flag it to be copied if it is not.
	 */
	if (of_flat_tree >= (char *)CFG_BOOTMAPSZ)
		of_data = (ulong)of_flat_tree;
#endif

	/* move of_flat_tree if needed */
	if (of_data) {
		int err;
		ulong of_start, of_len;

		of_len = be32_to_cpu(fdt_totalsize(of_data));

		/* position on a 4K boundary before the kbd */
		of_start  = (ulong)kbd - of_len;
		of_start &= ~(4096 - 1);	/* align on page */
		debug ("## device tree at 0x%08lX ... 0x%08lX (len=%ld=0x%lX)\n",
			of_data, of_data + of_len - 1, of_len, of_len);

		of_flat_tree = (char *)of_start;
		printf ("   Loading Device Tree to %08lx, end %08lx ... ",
			of_start, of_start + of_len - 1);
		err = fdt_open_into((void *)of_data, (void *)of_start, of_len);
		if (err != 0) {
			puts ("ERROR: fdt move failed - "
				"must RESET the board to recover.\n");
			do_reset (cmdtp, flag, argc, argv);
		}
	}
	/*
	 * Add the chosen node if it doesn't exist, add the env and bd_t
	 * if the user wants it (the logic is in the subroutines).
	 */
	if (of_flat_tree) {
		if (fdt_chosen(of_flat_tree, initrd_start, initrd_end, 0) < 0) {
			puts ("ERROR: /chosen node create failed - "
				"must RESET the board to recover.\n");
			do_reset (cmdtp, flag, argc, argv);
		}
#ifdef CONFIG_OF_HAS_UBOOT_ENV
		if (fdt_env(of_flat_tree) < 0) {
			puts ("ERROR: /u-boot-env node create failed - "
				"must RESET the board to recover.\n");
			do_reset (cmdtp, flag, argc, argv);
		}
#endif
#ifdef CONFIG_OF_HAS_BD_T
		if (fdt_bd_t(of_flat_tree) < 0) {
			puts ("ERROR: /bd_t node create failed - "
				"must RESET the board to recover.\n");
			do_reset (cmdtp, flag, argc, argv);
		}
#endif
#ifdef CONFIG_OF_BOARD_SETUP
		/* Call the board-specific fixup routine */
		ft_board_setup(of_flat_tree, gd->bd);
#endif
	}
#endif /* CONFIG_OF_LIBFDT */
#if defined(CONFIG_OF_FLAT_TREE)
#ifdef CFG_BOOTMAPSZ
	/*
	 * The blob must be within CFG_BOOTMAPSZ,
	 * so we flag it to be copied if it is not.
	 */
	if (of_flat_tree >= (char *)CFG_BOOTMAPSZ)
		of_data = (ulong)of_flat_tree;
#endif

	/* move of_flat_tree if needed */
	if (of_data) {
		ulong of_start, of_len;
		of_len = ((struct boot_param_header *)of_data)->totalsize;

		/* provide extra 8k pad */
		of_start  = (ulong)kbd - of_len - 8192;
		of_start &= ~(4096 - 1);	/* align on page */
		debug ("## device tree at 0x%08lX ... 0x%08lX (len=%ld=0x%lX)\n",
			of_data, of_data + of_len - 1, of_len, of_len);

		of_flat_tree = (char *)of_start;
		printf ("   Loading Device Tree to %08lx, end %08lx ... ",
			of_start, of_start + of_len - 1);
		memmove ((void *)of_start, (void *)of_data, of_len);
	}
	/*
	 * Create the /chosen node and modify the blob with board specific
	 * values as needed.
	 */
	ft_setup(of_flat_tree, kbd, initrd_start, initrd_end);
	/* ft_dump_blob(of_flat_tree); */
#endif
	debug ("## Transferring control to Linux (at address %08lx) ...\n",
		(ulong)kernel);

	show_boot_progress (15);

#if defined(CFG_INIT_RAM_LOCK) && !defined(CONFIG_E500)
	unlock_ram_in_cache();
#endif

#if defined(CONFIG_OF_FLAT_TREE) || defined(CONFIG_OF_LIBFDT)
	if (of_flat_tree) {	/* device tree; boot new style */
		/*
		 * Linux Kernel Parameters (passing device tree):
		 *   r3: pointer to the fdt, followed by the board info data
		 *   r4: physical pointer to the kernel itself
		 *   r5: NULL
		 *   r6: NULL
		 *   r7: NULL
		 */
		(*kernel) ((bd_t *)of_flat_tree, (ulong)kernel, 0, 0, 0);
		/* does not return */
	}
#endif
	/*
	 * Linux Kernel Parameters (passing board info data):
	 *   r3: ptr to board info data
	 *   r4: initrd_start or 0 if no initrd
	 *   r5: initrd_end - unused if r4 is 0
	 *   r6: Start of command line string
	 *   r7: End   of command line string
	 */
	(*kernel) (kbd, initrd_start, initrd_end, cmd_start, cmd_end);
	/* does not return */
}
#endif /* CONFIG_PPC */

static void
do_bootm_netbsd (cmd_tbl_t *cmdtp, int flag,
		int	argc, char *argv[],
		ulong	addr,
		ulong	*len_ptr,
		int	verify)
{
	image_header_t *hdr = &header;

	void	(*loader)(bd_t *, image_header_t *, char *, char *);
	image_header_t *img_addr;
	char     *consdev;
	char     *cmdline;


	/*
	 * Booting a (NetBSD) kernel image
	 *
	 * This process is pretty similar to a standalone application:
	 * The (first part of an multi-) image must be a stage-2 loader,
	 * which in turn is responsible for loading & invoking the actual
	 * kernel.  The only differences are the parameters being passed:
	 * besides the board info strucure, the loader expects a command
	 * line, the name of the console device, and (optionally) the
	 * address of the original image header.
	 */

	img_addr = 0;
	if ((hdr->ih_type==IH_TYPE_MULTI) && (len_ptr[1]))
		img_addr = (image_header_t *) addr;


	consdev = "";
#if   defined (CONFIG_8xx_CONS_SMC1)
	consdev = "smc1";
#elif defined (CONFIG_8xx_CONS_SMC2)
	consdev = "smc2";
#elif defined (CONFIG_8xx_CONS_SCC2)
	consdev = "scc2";
#elif defined (CONFIG_8xx_CONS_SCC3)
	consdev = "scc3";
#endif

	if (argc > 2) {
		ulong len;
		int   i;

		for (i=2, len=0 ; i<argc ; i+=1)
			len += strlen (argv[i]) + 1;
		cmdline = malloc (len);

		for (i=2, len=0 ; i<argc ; i+=1) {
			if (i > 2)
				cmdline[len++] = ' ';
			strcpy (&cmdline[len], argv[i]);
			len += strlen (argv[i]);
		}
	} else if ((cmdline = getenv("bootargs")) == NULL) {
		cmdline = "";
	}

	loader = (void (*)(bd_t *, image_header_t *, char *, char *)) ntohl(hdr->ih_ep);

	printf ("## Transferring control to NetBSD stage-2 loader (at address %08lx) ...\n",
		(ulong)loader);

	show_boot_progress (15);

	/*
	 * NetBSD Stage-2 Loader Parameters:
	 *   r3: ptr to board info data
	 *   r4: image address
	 *   r5: console device
	 *   r6: boot args string
	 */
	(*loader) (gd->bd, img_addr, consdev, cmdline);
}

#if defined(CONFIG_ARTOS) && defined(CONFIG_PPC)

/* Function that returns a character from the environment */
extern uchar (*env_get_char)(int);

static void
do_bootm_artos (cmd_tbl_t *cmdtp, int flag,
		int	argc, char *argv[],
		ulong	addr,
		ulong	*len_ptr,
		int	verify)
{
	ulong top;
	char *s, *cmdline;
	char **fwenv, **ss;
	int i, j, nxt, len, envno, envsz;
	bd_t *kbd;
	void (*entry)(bd_t *bd, char *cmdline, char **fwenv, ulong top);
	image_header_t *hdr = &header;

	/*
	 * Booting an ARTOS kernel image + application
	 */

	/* this used to be the top of memory, but was wrong... */
#ifdef CONFIG_PPC
	/* get stack pointer */
	asm volatile ("mr %0,1" : "=r"(top) );
#endif
	debug ("## Current stack ends at 0x%08lX ", top);

	top -= 2048;		/* just to be sure */
	if (top > CFG_BOOTMAPSZ)
		top = CFG_BOOTMAPSZ;
	top &= ~0xF;

	debug ("=> set upper limit to 0x%08lX\n", top);

	/* first check the artos specific boot args, then the linux args*/
	if ((s = getenv("abootargs")) == NULL && (s = getenv("bootargs")) == NULL)
		s = "";

	/* get length of cmdline, and place it */
	len = strlen(s);
	top = (top - (len + 1)) & ~0xF;
	cmdline = (char *)top;
	debug ("## cmdline at 0x%08lX ", top);
	strcpy(cmdline, s);

	/* copy bdinfo */
	top = (top - sizeof(bd_t)) & ~0xF;
	debug ("## bd at 0x%08lX ", top);
	kbd = (bd_t *)top;
	memcpy(kbd, gd->bd, sizeof(bd_t));

	/* first find number of env entries, and their size */
	envno = 0;
	envsz = 0;
	for (i = 0; env_get_char(i) != '\0'; i = nxt + 1) {
		for (nxt = i; env_get_char(nxt) != '\0'; ++nxt)
			;
		envno++;
		envsz += (nxt - i) + 1;	/* plus trailing zero */
	}
	envno++;	/* plus the terminating zero */
	debug ("## %u envvars total size %u ", envno, envsz);

	top = (top - sizeof(char **)*envno) & ~0xF;
	fwenv = (char **)top;
	debug ("## fwenv at 0x%08lX ", top);

	top = (top - envsz) & ~0xF;
	s = (char *)top;
	ss = fwenv;

	/* now copy them */
	for (i = 0; env_get_char(i) != '\0'; i = nxt + 1) {
		for (nxt = i; env_get_char(nxt) != '\0'; ++nxt)
			;
		*ss++ = s;
		for (j = i; j < nxt; ++j)
			*s++ = env_get_char(j);
		*s++ = '\0';
	}
	*ss++ = NULL;	/* terminate */

	entry = (void (*)(bd_t *, char *, char **, ulong))ntohl(hdr->ih_ep);
	(*entry)(kbd, cmdline, fwenv, top);
}
#endif


#if defined(CONFIG_CMD_BOOTD)
int do_bootd (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int rcode = 0;
#ifndef CFG_HUSH_PARSER
	if (run_command (getenv ("bootcmd"), flag) < 0) rcode = 1;
#else
	if (parse_string_outer(getenv("bootcmd"),
		FLAG_PARSE_SEMICOLON | FLAG_EXIT_FROM_LOOP) != 0 ) rcode = 1;
#endif
	return rcode;
}

U_BOOT_CMD(
 	boot,	1,	1,	do_bootd,
 	"boot    - boot default, i.e., run 'bootcmd'\n",
	NULL
);

/* keep old command name "bootd" for backward compatibility */
U_BOOT_CMD(
 	bootd, 1,	1,	do_bootd,
 	"bootd   - boot default, i.e., run 'bootcmd'\n",
	NULL
);

#endif

#if defined(CONFIG_CMD_IMI)
int do_iminfo ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int	arg;
	ulong	addr;
	int     rcode=0;

	if (argc < 2) {
		return image_info (load_addr);
	}

	for (arg=1; arg <argc; ++arg) {
		addr = simple_strtoul(argv[arg], NULL, 16);
		if (image_info (addr) != 0) rcode = 1;
	}
	return rcode;
}

static int image_info (ulong addr)
{
	ulong	data, len, checksum;
	image_header_t *hdr = &header;

	printf ("\n## Checking Image at %08lx ...\n", addr);

	/* Copy header so we can blank CRC field for re-calculation */
	memmove (&header, (char *)addr, sizeof(image_header_t));

	if (ntohl(hdr->ih_magic) != IH_MAGIC) {
		puts ("   Bad Magic Number\n");
		return 1;
	}

	data = (ulong)&header;
	len  = sizeof(image_header_t);

	checksum = ntohl(hdr->ih_hcrc);
	hdr->ih_hcrc = 0;

	if (crc32 (0, (uchar *)data, len) != checksum) {
		puts ("   Bad Header Checksum\n");
		return 1;
	}

	/* for multi-file images we need the data part, too */
	print_image_hdr ((image_header_t *)addr);

	data = addr + sizeof(image_header_t);
	len  = ntohl(hdr->ih_size);

	puts ("   Verifying Checksum ... ");
	if (crc32 (0, (uchar *)data, len) != ntohl(hdr->ih_dcrc)) {
		puts ("   Bad Data CRC\n");
		return 1;
	}
	puts ("OK\n");
	return 0;
}

U_BOOT_CMD(
	iminfo,	CFG_MAXARGS,	1,	do_iminfo,
	"iminfo  - print header information for application image\n",
	"addr [addr ...]\n"
	"    - print header information for application image starting at\n"
	"      address 'addr' in memory; this includes verification of the\n"
	"      image contents (magic number, header and payload checksums)\n"
);

#endif

#if defined(CONFIG_CMD_IMLS)
/*-----------------------------------------------------------------------
 * List all images found in flash.
 */
int do_imls (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	flash_info_t *info;
	int i, j;
	image_header_t *hdr;
	ulong data, len, checksum;

	for (i=0, info=&flash_info[0]; i<CFG_MAX_FLASH_BANKS; ++i, ++info) {
		if (info->flash_id == FLASH_UNKNOWN)
			goto next_bank;
		for (j=0; j<info->sector_count; ++j) {

			if (!(hdr=(image_header_t *)info->start[j]) ||
			    (ntohl(hdr->ih_magic) != IH_MAGIC))
				goto next_sector;

			/* Copy header so we can blank CRC field for re-calculation */
			memmove (&header, (char *)hdr, sizeof(image_header_t));

			checksum = ntohl(header.ih_hcrc);
			header.ih_hcrc = 0;

			if (crc32 (0, (uchar *)&header, sizeof(image_header_t))
			    != checksum)
				goto next_sector;

			printf ("Image at %08lX:\n", (ulong)hdr);
			print_image_hdr( hdr );

			data = (ulong)hdr + sizeof(image_header_t);
			len  = ntohl(hdr->ih_size);

			puts ("   Verifying Checksum ... ");
			if (crc32 (0, (uchar *)data, len) != ntohl(hdr->ih_dcrc)) {
				puts ("   Bad Data CRC\n");
			}
			puts ("OK\n");
next_sector:		;
		}
next_bank:	;
	}

	return (0);
}

U_BOOT_CMD(
	imls,	1,		1,	do_imls,
	"imls    - list all images found in flash\n",
	"\n"
	"    - Prints information about all images found at sector\n"
	"      boundaries in flash.\n"
);
#endif

void
print_image_hdr (image_header_t *hdr)
{
#if defined(CONFIG_TIMESTAMP) || defined(CONFIG_CMD_DATE)
	time_t timestamp = (time_t)ntohl(hdr->ih_time);
	struct rtc_time tm;
#endif

	printf ("   Image Name:   %.*s\n", IH_NMLEN, hdr->ih_name);
#if defined(CONFIG_TIMESTAMP) || defined(CONFIG_CMD_DATE)
	to_tm (timestamp, &tm);
	printf ("   Created:      %4d-%02d-%02d  %2d:%02d:%02d UTC\n",
		tm.tm_year, tm.tm_mon, tm.tm_mday,
		tm.tm_hour, tm.tm_min, tm.tm_sec);
#endif
	puts ("   Image Type:   "); print_type(hdr);
	printf ("\n   Data Size:    %d Bytes = ", ntohl(hdr->ih_size));
	print_size (ntohl(hdr->ih_size), "\n");
	printf ("   Load Address: %08x\n"
		"   Entry Point:  %08x\n",
		 ntohl(hdr->ih_load), ntohl(hdr->ih_ep));

	if (hdr->ih_type == IH_TYPE_MULTI) {
		int i;
		ulong len;
		ulong *len_ptr = (ulong *)((ulong)hdr + sizeof(image_header_t));

		puts ("   Contents:\n");
		for (i=0; (len = ntohl(*len_ptr)); ++i, ++len_ptr) {
			printf ("   Image %d: %8ld Bytes = ", i, len);
			print_size (len, "\n");
		}
	}
}


static void
print_type (image_header_t *hdr)
{
	char *os, *arch, *type, *comp;

	switch (hdr->ih_os) {
	case IH_OS_INVALID:	os = "Invalid OS";		break;
	case IH_OS_NETBSD:	os = "NetBSD";			break;
	case IH_OS_LINUX:	os = "Linux";			break;
	case IH_OS_VXWORKS:	os = "VxWorks";			break;
	case IH_OS_QNX:		os = "QNX";			break;
	case IH_OS_U_BOOT:	os = "U-Boot";			break;
	case IH_OS_RTEMS:	os = "RTEMS";			break;
#ifdef CONFIG_ARTOS
	case IH_OS_ARTOS:	os = "ARTOS";			break;
#endif
#ifdef CONFIG_LYNXKDI
	case IH_OS_LYNXOS:	os = "LynxOS";			break;
#endif
	default:		os = "Unknown OS";		break;
	}

	switch (hdr->ih_arch) {
	case IH_CPU_INVALID:	arch = "Invalid CPU";		break;
	case IH_CPU_ALPHA:	arch = "Alpha";			break;
	case IH_CPU_ARM:	arch = "ARM";			break;
	case IH_CPU_AVR32:	arch = "AVR32";			break;
	case IH_CPU_BLACKFIN:	arch = "Blackfin";		break;
	case IH_CPU_I386:	arch = "Intel x86";		break;
	case IH_CPU_IA64:	arch = "IA64";			break;
	case IH_CPU_M68K:	arch = "M68K"; 			break;
	case IH_CPU_MICROBLAZE:	arch = "Microblaze"; 		break;
	case IH_CPU_MIPS64:	arch = "MIPS 64 Bit";		break;
	case IH_CPU_MIPS:	arch = "MIPS";			break;
	case IH_CPU_NIOS2:	arch = "Nios-II";		break;
	case IH_CPU_NIOS:	arch = "Nios";			break;
	case IH_CPU_PPC:	arch = "PowerPC";		break;
	case IH_CPU_S390:	arch = "IBM S390";		break;
	case IH_CPU_SH:		arch = "SuperH";		break;
	case IH_CPU_SPARC64:	arch = "SPARC 64 Bit";		break;
	case IH_CPU_SPARC:	arch = "SPARC";			break;
	default:		arch = "Unknown Architecture";	break;
	}

	switch (hdr->ih_type) {
	case IH_TYPE_INVALID:	type = "Invalid Image";		break;
	case IH_TYPE_STANDALONE:type = "Standalone Program";	break;
	case IH_TYPE_KERNEL:	type = "Kernel Image";		break;
	case IH_TYPE_RAMDISK:	type = "RAMDisk Image";		break;
	case IH_TYPE_MULTI:	type = "Multi-File Image";	break;
	case IH_TYPE_FIRMWARE:	type = "Firmware";		break;
	case IH_TYPE_SCRIPT:	type = "Script";		break;
	case IH_TYPE_FLATDT:	type = "Flat Device Tree";	break;
	default:		type = "Unknown Image";		break;
	}

	switch (hdr->ih_comp) {
	case IH_COMP_NONE:	comp = "uncompressed";		break;
	case IH_COMP_GZIP:	comp = "gzip compressed";	break;
	case IH_COMP_BZIP2:	comp = "bzip2 compressed";	break;
	default:		comp = "unknown compression";	break;
	}

	printf ("%s %s %s (%s)", arch, os, type, comp);
}

#define	ZALLOC_ALIGNMENT	16

static void *zalloc(void *x, unsigned items, unsigned size)
{
	void *p;

	size *= items;
	size = (size + ZALLOC_ALIGNMENT - 1) & ~(ZALLOC_ALIGNMENT - 1);

	p = malloc (size);

	return (p);
}

static void zfree(void *x, void *addr, unsigned nb)
{
	free (addr);
}

#define HEAD_CRC	2
#define EXTRA_FIELD	4
#define ORIG_NAME	8
#define COMMENT		0x10
#define RESERVED	0xe0

#define DEFLATED	8

int gunzip(void *dst, int dstlen, unsigned char *src, unsigned long *lenp)
{
	z_stream s;
	int r, i, flags;

	/* skip header */
	i = 10;
	flags = src[3];
	if (src[2] != DEFLATED || (flags & RESERVED) != 0) {
		puts ("Error: Bad gzipped data\n");
		return (-1);
	}
	if ((flags & EXTRA_FIELD) != 0)
		i = 12 + src[10] + (src[11] << 8);
	if ((flags & ORIG_NAME) != 0)
		while (src[i++] != 0)
			;
	if ((flags & COMMENT) != 0)
		while (src[i++] != 0)
			;
	if ((flags & HEAD_CRC) != 0)
		i += 2;
	if (i >= *lenp) {
		puts ("Error: gunzip out of data in header\n");
		return (-1);
	}

	s.zalloc = zalloc;
	s.zfree = zfree;
#if defined(CONFIG_HW_WATCHDOG) || defined(CONFIG_WATCHDOG)
	s.outcb = (cb_func)WATCHDOG_RESET;
#else
	s.outcb = Z_NULL;
#endif	/* CONFIG_HW_WATCHDOG */

	r = inflateInit2(&s, -MAX_WBITS);
	if (r != Z_OK) {
		printf ("Error: inflateInit2() returned %d\n", r);
		return (-1);
	}
	s.next_in = src + i;
	s.avail_in = *lenp - i;
	s.next_out = dst;
	s.avail_out = dstlen;
	r = inflate(&s, Z_FINISH);
	if (r != Z_OK && r != Z_STREAM_END) {
		printf ("Error: inflate() returned %d\n", r);
		return (-1);
	}
	*lenp = s.next_out - (unsigned char *) dst;
	inflateEnd(&s);

	return (0);
}

#ifdef CONFIG_BZIP2
void bz_internal_error(int errcode)
{
	printf ("BZIP2 internal error %d\n", errcode);
}
#endif /* CONFIG_BZIP2 */

static void
do_bootm_rtems (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[],
		ulong addr, ulong *len_ptr, int verify)
{
	image_header_t *hdr = &header;
	void	(*entry_point)(bd_t *);

	entry_point = (void (*)(bd_t *)) ntohl(hdr->ih_ep);

	printf ("## Transferring control to RTEMS (at address %08lx) ...\n",
		(ulong)entry_point);

	show_boot_progress (15);

	/*
	 * RTEMS Parameters:
	 *   r3: ptr to board info data
	 */

	(*entry_point ) ( gd->bd );
}

#if defined(CONFIG_CMD_ELF)
static void
do_bootm_vxworks (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[],
		  ulong addr, ulong *len_ptr, int verify)
{
	image_header_t *hdr = &header;
	char str[80];

	sprintf(str, "%x", ntohl(hdr->ih_ep)); /* write entry-point into string */
	setenv("loadaddr", str);
	do_bootvx(cmdtp, 0, 0, NULL);
}

static void
do_bootm_qnxelf (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[],
		 ulong addr, ulong *len_ptr, int verify)
{
	image_header_t *hdr = &header;
	char *local_args[2];
	char str[16];

	sprintf(str, "%x", ntohl(hdr->ih_ep)); /* write entry-point into string */
	local_args[0] = argv[0];
	local_args[1] = str;	/* and provide it via the arguments */
	do_bootelf(cmdtp, 0, 2, local_args);
}
#endif

#ifdef CONFIG_LYNXKDI
static void
do_bootm_lynxkdi (cmd_tbl_t *cmdtp, int flag,
		 int	argc, char *argv[],
		 ulong	addr,
		 ulong	*len_ptr,
		 int	verify)
{
	lynxkdi_boot( &header );
}

#endif /* CONFIG_LYNXKDI */
