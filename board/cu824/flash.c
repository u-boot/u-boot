/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <mpc824x.h>
#include <asm/processor.h>

#if defined(CONFIG_ENV_IS_IN_FLASH)
# ifndef  CONFIG_ENV_ADDR
#  define CONFIG_ENV_ADDR	(CONFIG_SYS_FLASH_BASE + CONFIG_ENV_OFFSET)
# endif
# ifndef  CONFIG_ENV_SIZE
#  define CONFIG_ENV_SIZE	CONFIG_ENV_SECT_SIZE
# endif
# ifndef  CONFIG_ENV_SECT_SIZE
#  define CONFIG_ENV_SECT_SIZE  CONFIG_ENV_SIZE
# endif
#endif

#define FLASH_BANK_SIZE 0x800000
#define MAIN_SECT_SIZE  0x40000
#define PARAM_SECT_SIZE 0x8000

#define BOARD_CTRL_REG 0xFE800013

flash_info_t    flash_info[CONFIG_SYS_MAX_FLASH_BANKS];

static int write_data (flash_info_t *info, ulong dest, ulong *data);
static void write_via_fpu(vu_long *addr, ulong *data);
static __inline__ unsigned long get_msr(void);
static __inline__ void set_msr(unsigned long msr);

/*---------------------------------------------------------------------*/
#undef	DEBUG_FLASH

/*---------------------------------------------------------------------*/
#ifdef DEBUG_FLASH
#define DEBUGF(fmt,args...) printf(fmt ,##args)
#else
#define DEBUGF(fmt,args...)
#endif
/*---------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
 */

unsigned long flash_init(void)
{
    int i, j;
    ulong size = 0;
    volatile unsigned char *bcr = (volatile unsigned char *)(BOARD_CTRL_REG);

    DEBUGF("Write protect was: 0x%02X\n", *bcr);
    *bcr &= 0x1;	/* FWPT must be 0  */
    *bcr |= 0x6;	/* FWP0 = FWP1 = 1 */
    DEBUGF("Write protect is:  0x%02X\n", *bcr);

    for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i++) {
	vu_long *addr = (vu_long *)(CONFIG_SYS_FLASH_BASE + i * FLASH_BANK_SIZE);

	addr[0] = 0x00900090;

	DEBUGF ("Flash bank # %d:\n"
		"\tManuf. ID @ 0x%08lX: 0x%08lX\n"
		"\tDevice ID @ 0x%08lX: 0x%08lX\n",
		i,
		(ulong)(&addr[0]), addr[0],
		(ulong)(&addr[2]), addr[2]);

	if ((addr[0] == addr[1]) && (addr[0] == INTEL_MANUFACT) &&
	    (addr[2] == addr[3]) && (addr[2] == INTEL_ID_28F160F3B))
	{
	    flash_info[i].flash_id = (FLASH_MAN_INTEL & FLASH_VENDMASK) |
				     (INTEL_ID_28F160F3B & FLASH_TYPEMASK);
	} else {
	    flash_info[i].flash_id = FLASH_UNKNOWN;
	    addr[0] = 0xFFFFFFFF;
	    goto Done;
	}

	DEBUGF ("flash_id = 0x%08lX\n", flash_info[i].flash_id);

	addr[0] = 0xFFFFFFFF;

	flash_info[i].size = FLASH_BANK_SIZE;
	flash_info[i].sector_count = CONFIG_SYS_MAX_FLASH_SECT;
	memset(flash_info[i].protect, 0, CONFIG_SYS_MAX_FLASH_SECT);
	for (j = 0; j < flash_info[i].sector_count; j++) {
		if (j <= 7) {
			flash_info[i].start[j] = CONFIG_SYS_FLASH_BASE +
						 i * FLASH_BANK_SIZE +
						 j * PARAM_SECT_SIZE;
		} else {
			flash_info[i].start[j] = CONFIG_SYS_FLASH_BASE +
						 i * FLASH_BANK_SIZE +
						 (j - 7)*MAIN_SECT_SIZE;
		}
	}
	size += flash_info[i].size;
    }

    /* Protect monitor and environment sectors
     */
#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE
#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE + FLASH_BANK_SIZE
    flash_protect(FLAG_PROTECT_SET,
	      CONFIG_SYS_MONITOR_BASE,
	      CONFIG_SYS_MONITOR_BASE + monitor_flash_len - 1,
	      &flash_info[1]);
#else
    flash_protect(FLAG_PROTECT_SET,
	      CONFIG_SYS_MONITOR_BASE,
	      CONFIG_SYS_MONITOR_BASE + monitor_flash_len - 1,
	      &flash_info[0]);
#endif
#endif

#if defined(CONFIG_ENV_IS_IN_FLASH) && defined(CONFIG_ENV_ADDR)
#if CONFIG_ENV_ADDR >= CONFIG_SYS_FLASH_BASE + FLASH_BANK_SIZE
    flash_protect(FLAG_PROTECT_SET,
	      CONFIG_ENV_ADDR,
	      CONFIG_ENV_ADDR + CONFIG_ENV_SIZE - 1,
	      &flash_info[1]);
#else
    flash_protect(FLAG_PROTECT_SET,
	      CONFIG_ENV_ADDR,
	      CONFIG_ENV_ADDR + CONFIG_ENV_SIZE - 1,
	      &flash_info[0]);
#endif
#endif

Done:
    return size;
}

/*-----------------------------------------------------------------------
 */
void flash_print_info (flash_info_t * info)
{
	int i;

	switch ((i = info->flash_id & FLASH_VENDMASK)) {
	case (FLASH_MAN_INTEL & FLASH_VENDMASK):
		printf ("Intel: ");
		break;
	default:
		printf ("Unknown Vendor 0x%04x ", i);
		break;
	}

	switch ((i = info->flash_id & FLASH_TYPEMASK)) {
	case (INTEL_ID_28F160F3B & FLASH_TYPEMASK):
		printf ("28F160F3B (16Mbit)\n");
		break;
	default:
		printf ("Unknown Chip Type 0x%04x\n", i);
		goto Done;
		break;
	}

	printf ("  Size: %ld MB in %d Sectors\n",
			info->size >> 20, info->sector_count);

	printf ("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; i++) {
		if ((i % 5) == 0) {
			printf ("\n   ");
		}
		printf (" %08lX%s", info->start[i],
				info->protect[i] ? " (RO)" : "     ");
	}
	printf ("\n");

Done:
	return;
}

/*-----------------------------------------------------------------------
 */

int	flash_erase (flash_info_t *info, int s_first, int s_last)
{
	int flag, prot, sect;
	ulong start, now, last;

	DEBUGF ("Erase flash bank %d sect %d ... %d\n",
		info - &flash_info[0], s_first, s_last);

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf ("- missing\n");
		} else {
			printf ("- no sectors to erase\n");
		}
		return 1;
	}

	if ((info->flash_id & FLASH_VENDMASK) !=
	    (FLASH_MAN_INTEL & FLASH_VENDMASK)) {
		printf ("Can erase only Intel flash types - aborted\n");
		return 1;
	}

	prot = 0;
	for (sect=s_first; sect<=s_last; ++sect) {
		if (info->protect[sect]) {
			prot++;
		}
	}

	if (prot) {
		printf ("- Warning: %d protected sectors will not be erased!\n",
			prot);
	} else {
		printf ("\n");
	}

	start = get_timer (0);
	last  = start;
	/* Start erase on unprotected sectors */
	for (sect = s_first; sect<=s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			vu_long *addr = (vu_long *)(info->start[sect]);

			DEBUGF ("Erase sect %d @ 0x%08lX\n",
				sect, (ulong)addr);

			/* Disable interrupts which might cause a timeout
			 * here.
			 */
			flag = disable_interrupts();

			addr[0] = 0x00500050;	/* clear status register */
			addr[0] = 0x00200020;	/* erase setup */
			addr[0] = 0x00D000D0;	/* erase confirm */

			addr[1] = 0x00500050;	/* clear status register */
			addr[1] = 0x00200020;	/* erase setup */
			addr[1] = 0x00D000D0;	/* erase confirm */

			/* re-enable interrupts if necessary */
			if (flag)
				enable_interrupts();

			/* wait at least 80us - let's wait 1 ms */
			udelay (1000);

			while (((addr[0] & 0x00800080) != 0x00800080) ||
			       ((addr[1] & 0x00800080) != 0x00800080) ) {
				if ((now=get_timer(start)) >
					   CONFIG_SYS_FLASH_ERASE_TOUT) {
					printf ("Timeout\n");
					addr[0] = 0x00B000B0; /* suspend erase */
					addr[0] = 0x00FF00FF; /* to read mode  */
					return 1;
				}

				/* show that we're waiting */
				if ((now - last) > 1000) {  /* every second  */
					putc ('.');
					last = now;
				}
			}

			addr[0] = 0x00FF00FF;
		}
	}
	printf (" done\n");
	return 0;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 * 4 - Flash not identified
 */

#define	FLASH_WIDTH	8	/* flash bus width in bytes */

int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	ulong wp, cp, msr;
	int l, rc, i;
	ulong data[2];
	ulong *datah = &data[0];
	ulong *datal = &data[1];

	DEBUGF ("Flash write_buff: @ 0x%08lx, src 0x%08lx len %ld\n",
		addr, (ulong)src, cnt);

	if (info->flash_id == FLASH_UNKNOWN) {
		return 4;
	}

	msr = get_msr();
	set_msr(msr | MSR_FP);

	wp = (addr & ~(FLASH_WIDTH-1));	/* get lower aligned address */

	/*
	 * handle unaligned start bytes
	 */
	if ((l = addr - wp) != 0) {
		*datah = *datal = 0;

		for (i = 0, cp = wp; i < l; i++, cp++) {
			if (i >= 4) {
				*datah = (*datah << 8) |
						((*datal & 0xFF000000) >> 24);
			}

			*datal = (*datal << 8) | (*(uchar *)cp);
		}
		for (; i < FLASH_WIDTH && cnt > 0; ++i) {
			char tmp;

			tmp = *src;

			src++;

			if (i >= 4) {
				*datah = (*datah << 8) |
						((*datal & 0xFF000000) >> 24);
			}

			*datal = (*datal << 8) | tmp;

			--cnt; ++cp;
		}

		for (; cnt == 0 && i < FLASH_WIDTH; ++i, ++cp) {
			if (i >= 4) {
				*datah = (*datah << 8) |
						((*datal & 0xFF000000) >> 24);
			}

			*datal = (*datah << 8) | (*(uchar *)cp);
		}

		if ((rc = write_data(info, wp, data)) != 0) {
			set_msr(msr);
			return (rc);
		}

		wp += FLASH_WIDTH;
	}

	/*
	 * handle FLASH_WIDTH aligned part
	 */
	while (cnt >= FLASH_WIDTH) {
		*datah = *(ulong *)src;
		*datal = *(ulong *)(src + 4);
		if ((rc = write_data(info, wp, data)) != 0) {
			set_msr(msr);
			return (rc);
		}
		wp  += FLASH_WIDTH;
		cnt -= FLASH_WIDTH;
		src += FLASH_WIDTH;
	}

	if (cnt == 0) {
		set_msr(msr);
		return (0);
	}

	/*
	 * handle unaligned tail bytes
	 */
	*datah = *datal = 0;
	for (i = 0, cp = wp; i < FLASH_WIDTH && cnt > 0; ++i, ++cp) {
		char tmp;

		tmp = *src;

		src++;

		if (i >= 4) {
			*datah = (*datah << 8) | ((*datal & 0xFF000000) >> 24);
		}

		*datal = (*datal << 8) | tmp;

		--cnt;
	}

	for (; i < FLASH_WIDTH; ++i, ++cp) {
		if (i >= 4) {
			*datah = (*datah << 8) | ((*datal & 0xFF000000) >> 24);
		}

		*datal = (*datal << 8) | (*(uchar *)cp);
	}

	rc = write_data(info, wp, data);
	set_msr(msr);

	return (rc);
}

/*-----------------------------------------------------------------------
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_data (flash_info_t *info, ulong dest, ulong *data)
{
	vu_long *addr = (vu_long *)dest;
	ulong start;
	int flag;

	/* Check if Flash is (sufficiently) erased */
	if (((addr[0] & data[0]) != data[0]) ||
	    ((addr[1] & data[1]) != data[1]) ) {
		return (2);
	}
	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	addr[0] = 0x00400040;		/* write setup */
	write_via_fpu(addr, data);

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts();

	start = get_timer (0);

	while (((addr[0] & 0x00800080) != 0x00800080) ||
	       ((addr[1] & 0x00800080) != 0x00800080) ) {
		if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
			addr[0] = 0x00FF00FF;	/* restore read mode */
			return (1);
		}
	}

	addr[0] = 0x00FF00FF;	/* restore read mode */

	return (0);
}

/*-----------------------------------------------------------------------
 */
static void write_via_fpu(vu_long *addr, ulong *data)
{
	__asm__ __volatile__ ("lfd  1, 0(%0)" : : "r" (data));
	__asm__ __volatile__ ("stfd 1, 0(%0)" : : "r" (addr));
}
/*-----------------------------------------------------------------------
 */
static __inline__ unsigned long get_msr(void)
{
    unsigned long msr;

    __asm__ __volatile__ ("mfmsr %0" : "=r" (msr) :);
    return msr;
}

static __inline__ void set_msr(unsigned long msr)
{
    __asm__ __volatile__ ("mtmsr %0" : : "r" (msr));
}
