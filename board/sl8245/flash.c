/*
 * (C) Copyright 2001 - 2003
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
#include <mpc824x.h>
#include <asm/processor.h>

#if defined(CFG_ENV_IS_IN_FLASH)
# ifndef  CFG_ENV_ADDR
#  define CFG_ENV_ADDR	(CFG_FLASH_BASE + CFG_ENV_OFFSET)
# endif
# ifndef  CFG_ENV_SIZE
#  define CFG_ENV_SIZE	CFG_ENV_SECT_SIZE
# endif
# ifndef  CFG_ENV_SECT_SIZE
#  define CFG_ENV_SECT_SIZE  CFG_ENV_SIZE
# endif
#endif

#define FLASH_BANK_SIZE 0x800000
#define MAIN_SECT_SIZE  0x40000
#define PARAM_SECT1_SIZE 0x20000
#define PARAM_SECT23_SIZE 0x8000
#define PARAM_SECT4_SIZE 0x10000

flash_info_t    flash_info[CFG_MAX_FLASH_BANKS];

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

#define __align__ __attribute__ ((aligned (8)))
static __align__ ulong precmd0[2]   = { 0x00aa00aa, 0x00aa00aa };
static __align__ ulong precmd1[2]   = { 0x00550055, 0x00550055 };
static __align__ ulong cmdid[2]     = { 0x00900090, 0x00900090 };
static __align__ ulong cmderase[2]  = { 0x00800080, 0x00800080 };
static __align__ ulong cmdersusp[2] = { 0x00b000b0, 0x00b000b0 };
static __align__ ulong cmdsecter[2] = { 0x00300030, 0x00300030 };
static __align__ ulong cmdprog[2]   = { 0x00a000a0, 0x00a000a0 };
static __align__ ulong cmdres[2]    = { 0x00f000f0, 0x00f000f0 };

/*-----------------------------------------------------------------------
 */

unsigned long flash_init (void)
{
	int i, j;
	ulong size = 0;

	for (i = 0; i < CFG_MAX_FLASH_BANKS; i++) {
		vu_long *addr = (vu_long *) (CFG_FLASH_BASE + i * FLASH_BANK_SIZE);

		write_via_fpu (&addr[0xaaa], precmd0);
		write_via_fpu (&addr[0x554], precmd1);
		write_via_fpu (&addr[0xaaa], cmdid);

		DEBUGF ("Flash bank # %d:\n"
			"\tManuf. ID @ 0x%08lX: 0x%08lX\n"
			"\tDevice ID @ 0x%08lX: 0x%08lX\n",
			i,
			(ulong) (&addr[0]), addr[0],
			(ulong) (&addr[2]), addr[2]);

		if ((addr[0] == addr[1]) && (addr[0] == AMD_MANUFACT) &&
			(addr[2] == addr[3]) && (addr[2] == AMD_ID_LV160T)) {
			flash_info[i].flash_id = (FLASH_MAN_AMD & FLASH_VENDMASK) |
					(FLASH_AM160T & FLASH_TYPEMASK);
		} else {
			flash_info[i].flash_id = FLASH_UNKNOWN;
			write_via_fpu (addr, cmdres);
			goto Done;
		}

		DEBUGF ("flash_id = 0x%08lX\n", flash_info[i].flash_id);

		write_via_fpu (addr, cmdres);

		flash_info[i].size = FLASH_BANK_SIZE;
		flash_info[i].sector_count = CFG_MAX_FLASH_SECT;
		memset (flash_info[i].protect, 0, CFG_MAX_FLASH_SECT);
		for (j = 0; j < 32; j++) {
			flash_info[i].start[j] = CFG_FLASH_BASE +
					i * FLASH_BANK_SIZE + j * MAIN_SECT_SIZE;
		}
		flash_info[i].start[32] =
				flash_info[i].start[31] + PARAM_SECT1_SIZE;
		flash_info[i].start[33] =
				flash_info[i].start[32] + PARAM_SECT23_SIZE;
		flash_info[i].start[34] =
				flash_info[i].start[33] + PARAM_SECT23_SIZE;
		size += flash_info[i].size;
	}

	/* Protect monitor and environment sectors
	 */
#if CFG_MONITOR_BASE >= CFG_FLASH_BASE
#if CFG_MONITOR_BASE >= CFG_FLASH_BASE + FLASH_BANK_SIZE
	flash_protect ( FLAG_PROTECT_SET,
			CFG_MONITOR_BASE,
			CFG_MONITOR_BASE + monitor_flash_len - 1,
			&flash_info[1]);
#else
	flash_protect ( FLAG_PROTECT_SET,
			CFG_MONITOR_BASE,
			CFG_MONITOR_BASE + monitor_flash_len - 1,
			&flash_info[0]);
#endif
#endif

#if (CFG_ENV_IS_IN_FLASH == 1) && defined(CFG_ENV_ADDR)
#if CFG_ENV_ADDR >= CFG_FLASH_BASE + FLASH_BANK_SIZE
	flash_protect ( FLAG_PROTECT_SET,
			CFG_ENV_ADDR,
			CFG_ENV_ADDR + CFG_ENV_SIZE - 1, &flash_info[1]);
#else
	flash_protect ( FLAG_PROTECT_SET,
			CFG_ENV_ADDR,
			CFG_ENV_ADDR + CFG_ENV_SIZE - 1, &flash_info[0]);
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
	case (FLASH_MAN_AMD & FLASH_VENDMASK):
		printf ("Intel: ");
		break;
	default:
		printf ("Unknown Vendor 0x%04x ", i);
		break;
	}

	switch ((i = info->flash_id & FLASH_TYPEMASK)) {
	case (FLASH_AM160T & FLASH_TYPEMASK):
		printf ("AM29LV160BT (16Mbit)\n");
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

int flash_erase (flash_info_t * info, int s_first, int s_last)
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
		(FLASH_MAN_AMD & FLASH_VENDMASK)) {
		printf ("Can erase only AMD flash types - aborted\n");
		return 1;
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect) {
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
	last = start;
	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			vu_long *addr = (vu_long *) (info->start[sect]);

			DEBUGF ("Erase sect %d @ 0x%08lX\n", sect, (ulong) addr);

			/* Disable interrupts which might cause a timeout
			 * here.
			 */
			flag = disable_interrupts ();

			write_via_fpu (&addr[0xaaa], precmd0);
			write_via_fpu (&addr[0x554], precmd1);
			write_via_fpu (&addr[0xaaa], cmderase);
			write_via_fpu (&addr[0xaaa], precmd0);
			write_via_fpu (&addr[0x554], precmd1);
			write_via_fpu (&addr[0xaaa], cmdsecter);

			/* re-enable interrupts if necessary */
			if (flag)
				enable_interrupts ();

			/* wait at least 80us - let's wait 1 ms */
			udelay (1000);

			while (((addr[0] & 0x00800080) != 0x00800080) ||
				   ((addr[1] & 0x00800080) != 0x00800080)) {
				if ((now = get_timer (start)) > CFG_FLASH_ERASE_TOUT) {
					printf ("Timeout\n");
					write_via_fpu (addr, cmdersusp);
					write_via_fpu (addr, cmdres);
					return 1;
				}

				/* show that we're waiting */
				if ((now - last) > 1000) {	/* every second  */
					putc ('.');
					last = now;
				}
			}

			write_via_fpu (addr, cmdres);
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

#define	FLASH_WIDTH	8		/* flash bus width in bytes */

int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	ulong wp, cp, msr;
	int l, rc, i;
	ulong data[2];
	ulong *datah = &data[0];
	ulong *datal = &data[1];

	DEBUGF ("Flash write_buff: @ 0x%08lx, src 0x%08lx len %ld\n",
		addr, (ulong) src, cnt);

	if (info->flash_id == FLASH_UNKNOWN) {
		return 4;
	}

	msr = get_msr ();
	set_msr (msr | MSR_FP);

	wp = (addr & ~(FLASH_WIDTH - 1));	/* get lower aligned address */

	/*
	 * handle unaligned start bytes
	 */
	if ((l = addr - wp) != 0) {
		*datah = *datal = 0;

		for (i = 0, cp = wp; i < l; i++, cp++) {
			if (i >= 4) {
				*datah = (*datah << 8) | ((*datal & 0xFF000000) >> 24);
			}

			*datal = (*datal << 8) | (*(uchar *) cp);
		}
		for (; i < FLASH_WIDTH && cnt > 0; ++i) {
			char tmp;

			tmp = *src;

			src++;

			if (i >= 4) {
				*datah = (*datah << 8) | ((*datal & 0xFF000000) >> 24);
			}

			*datal = (*datal << 8) | tmp;

			--cnt;
			++cp;
		}

		for (; cnt == 0 && i < FLASH_WIDTH; ++i, ++cp) {
			if (i >= 4) {
				*datah = (*datah << 8) | ((*datal & 0xFF000000) >> 24);
			}

			*datal = (*datah << 8) | (*(uchar *) cp);
		}

		if ((rc = write_data (info, wp, data)) != 0) {
			set_msr (msr);
			return (rc);
		}

		wp += FLASH_WIDTH;
	}

	/*
	 * handle FLASH_WIDTH aligned part
	 */
	while (cnt >= FLASH_WIDTH) {
		*datah = *(ulong *) src;
		*datal = *(ulong *) (src + 4);
		if ((rc = write_data (info, wp, data)) != 0) {
			set_msr (msr);
			return (rc);
		}
		wp += FLASH_WIDTH;
		cnt -= FLASH_WIDTH;
		src += FLASH_WIDTH;
	}

	if (cnt == 0) {
		set_msr (msr);
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

		*datal = (*datal << 8) | (*(uchar *) cp);
	}

	rc = write_data (info, wp, data);
	set_msr (msr);

	return (rc);
}

/*-----------------------------------------------------------------------
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_data (flash_info_t * info, ulong dest, ulong * data)
{
	vu_long *chip = (vu_long *) (info->start[0]);
	vu_long *addr = (vu_long *) dest;
	ulong start;
	int flag;

	/* Check if Flash is (sufficiently) erased */
	if (((addr[0] & data[0]) != data[0]) ||
		((addr[1] & data[1]) != data[1])) {
		return (2);
	}
	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts ();

	write_via_fpu (&chip[0xaaa], precmd0);
	write_via_fpu (&chip[0x554], precmd1);
	write_via_fpu (&chip[0xaaa], cmdprog);
	write_via_fpu (addr, data);

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts ();

	start = get_timer (0);

	while (((addr[0] & 0x00800080) != (data[0] & 0x00800080)) ||
	       ((addr[1] & 0x00800080) != (data[1] & 0x00800080))) {
		if (get_timer (start) > CFG_FLASH_WRITE_TOUT) {
			write_via_fpu (chip, cmdres);
			return (1);
		}
	}

	write_via_fpu (chip, cmdres);

	return (0);
}

/*-----------------------------------------------------------------------
 */
static void write_via_fpu (vu_long * addr, ulong * data)
{
	__asm__ __volatile__ ("lfd  1, 0(%0)"::"r" (data));
	__asm__ __volatile__ ("stfd 1, 0(%0)"::"r" (addr));
}

/*-----------------------------------------------------------------------
 */
static __inline__ unsigned long get_msr (void)
{
	unsigned long msr;

	__asm__ __volatile__ ("mfmsr %0":"=r" (msr):);

	return msr;
}

static __inline__ void set_msr (unsigned long msr)
{
	__asm__ __volatile__ ("mtmsr %0"::"r" (msr));
}
