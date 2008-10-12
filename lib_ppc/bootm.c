/*
 * (C) Copyright 2008 Semihalf
 *
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

#ifdef CFG_INIT_RAM_LOCK
#include <asm/cache.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

extern ulong get_effective_memsize(void);
static ulong get_sp (void);
static void set_clocks_in_mhz (bd_t *kbd);

#ifndef CFG_LINUX_LOWMEM_MAX_SIZE
#define CFG_LINUX_LOWMEM_MAX_SIZE	(768*1024*1024)
#endif

__attribute__((noinline))
int do_bootm_linux(int flag, int argc, char *argv[], bootm_headers_t *images)
{
	ulong	sp;

	ulong	initrd_start, initrd_end;
	ulong	rd_len;
	ulong	size;
	phys_size_t bootm_size;

	ulong	cmd_start, cmd_end, bootmap_base;
	bd_t	*kbd;
	void	(*kernel)(bd_t *, ulong r4, ulong r5, ulong r6,
			  ulong r7, ulong r8, ulong r9);
	int	ret;
	ulong	of_size = images->ft_len;
	struct lmb *lmb = &images->lmb;

#if defined(CONFIG_OF_LIBFDT)
	char	*of_flat_tree = images->ft_addr;
#endif

	kernel = (void (*)(bd_t *, ulong, ulong, ulong,
			   ulong, ulong, ulong))images->ep;

	bootmap_base = getenv_bootm_low();
	bootm_size = getenv_bootm_size();

#ifdef DEBUG
	if (((u64)bootmap_base + bootm_size) >
	    (CFG_SDRAM_BASE + (u64)gd->ram_size))
		puts("WARNING: bootm_low + bootm_size exceed total memory\n");
	if ((bootmap_base + bootm_size) > get_effective_memsize())
		puts("WARNING: bootm_low + bootm_size exceed eff. memory\n");
#endif

	size = min(bootm_size, get_effective_memsize());
	size = min(size, CFG_LINUX_LOWMEM_MAX_SIZE);

	if (size < bootm_size) {
		ulong base = bootmap_base + size;
		printf("WARNING: adjusting available memory to %lx\n", size);
		lmb_reserve(lmb, base, bootm_size - size);
	}

	/*
	 * Booting a (Linux) kernel image
	 *
	 * Allocate space for command line and board info - the
	 * address should be as high as possible within the reach of
	 * the kernel (see CFG_BOOTMAPSZ settings), but in unused
	 * memory, which means far enough below the current stack
	 * pointer.
	 */
	sp = get_sp();
	debug ("## Current stack ends at 0x%08lx\n", sp);

	/* adjust sp by 1K to be safe */
	sp -= 1024;
	lmb_reserve(lmb, sp, (CFG_SDRAM_BASE + get_effective_memsize() - sp));

	if (!of_size) {
		/* allocate space and init command line */
		ret = boot_get_cmdline (lmb, &cmd_start, &cmd_end, bootmap_base);
		if (ret) {
			puts("ERROR with allocation of cmdline\n");
			goto error;
		}

		/* allocate space for kernel copy of board info */
		ret = boot_get_kbd (lmb, &kbd, bootmap_base);
		if (ret) {
			puts("ERROR with allocation of kernel bd\n");
			goto error;
		}
		set_clocks_in_mhz(kbd);
	}

	rd_len = images->rd_end - images->rd_start;

#if defined(CONFIG_OF_LIBFDT)
	ret = boot_relocate_fdt(lmb, bootmap_base, &of_flat_tree, &of_size);
	if (ret)
		goto error;

	/*
	 * Add the chosen node if it doesn't exist, add the env and bd_t
	 * if the user wants it (the logic is in the subroutines).
	 */
	if (of_size) {
		if (fdt_chosen(of_flat_tree, 1) < 0) {
			puts ("ERROR: ");
			puts ("/chosen node create failed");
			puts (" - must RESET the board to recover.\n");
			goto error;
		}
#ifdef CONFIG_OF_BOARD_SETUP
		/* Call the board-specific fixup routine */
		ft_board_setup(of_flat_tree, gd->bd);
#endif
	}

	/* Fixup the fdt memreserve now that we know how big it is */
	if (of_flat_tree) {
		/* Delete the old LMB reservation */
		lmb_free(lmb, (phys_addr_t)(u32)of_flat_tree,
				(phys_size_t)fdt_totalsize(of_flat_tree));

		ret = fdt_resize(of_flat_tree);
		if (ret < 0)
			goto error;
		of_size = ret;

		if ((of_flat_tree) && (initrd_start && initrd_end))
			of_size += FDT_RAMDISK_OVERHEAD;
		/* Create a new LMB reservation */
		lmb_reserve(lmb, (ulong)of_flat_tree, of_size);
	}
#endif	/* CONFIG_OF_LIBFDT */

	ret = boot_ramdisk_high (lmb, images->rd_start, rd_len, &initrd_start, &initrd_end);
	if (ret)
		goto error;

#if defined(CONFIG_OF_LIBFDT)
	/* fixup the initrd now that we know where it should be */
	if ((of_flat_tree) && (initrd_start && initrd_end))
		fdt_initrd(of_flat_tree, initrd_start, initrd_end, 1);
#endif
	debug ("## Transferring control to Linux (at address %08lx) ...\n",
		(ulong)kernel);

	show_boot_progress (15);

#if defined(CFG_INIT_RAM_LOCK) && !defined(CONFIG_E500)
	unlock_ram_in_cache();
#endif

#if defined(CONFIG_OF_LIBFDT)
	if (of_flat_tree) {	/* device tree; boot new style */
		/*
		 * Linux Kernel Parameters (passing device tree):
		 *   r3: pointer to the fdt
		 *   r4: 0
		 *   r5: 0
		 *   r6: epapr magic
		 *   r7: size of IMA in bytes
		 *   r8: 0
		 *   r9: 0
		 */
#if defined(CONFIG_85xx) || defined(CONFIG_440)
 #define EPAPR_MAGIC	(0x45504150)
#else
 #define EPAPR_MAGIC	(0x65504150)
#endif

		debug ("   Booting using OF flat tree...\n");
		(*kernel) ((bd_t *)of_flat_tree, 0, 0, EPAPR_MAGIC,
			   CFG_BOOTMAPSZ, 0, 0);
		/* does not return */
	} else
#endif
	{
		/*
		 * Linux Kernel Parameters (passing board info data):
		 *   r3: ptr to board info data
		 *   r4: initrd_start or 0 if no initrd
		 *   r5: initrd_end - unused if r4 is 0
		 *   r6: Start of command line string
		 *   r7: End   of command line string
		 *   r8: 0
		 *   r9: 0
		 */
		debug ("   Booting using board info...\n");
		(*kernel) (kbd, initrd_start, initrd_end,
			   cmd_start, cmd_end, 0, 0);
		/* does not return */
	}
	return 1;

error:
	return 1;
}

static ulong get_sp (void)
{
	ulong sp;

	asm( "mr %0,1": "=r"(sp) : );
	return sp;
}

static void set_clocks_in_mhz (bd_t *kbd)
{
	char	*s;

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
		kbd->bi_vco	/= 1000000L;
#endif
#if defined(CONFIG_MPC5xxx)
		kbd->bi_ipbfreq /= 1000000L;
		kbd->bi_pcifreq /= 1000000L;
#endif /* CONFIG_MPC5xxx */
	}
}
