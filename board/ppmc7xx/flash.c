/*
 * flash.c
 * -------
 *
 * Flash programming routines for the Wind River PPMC 74xx/7xx
 * based on flash.c from the TQM8260 board.
 *
 * By Richard Danter (richard.danter@windriver.com)
 * Copyright (C) 2005 Wind River Systems
 */

#include <common.h>
#include <asm/processor.h>
#include <74xx_7xx.h>

#define DWORD unsigned long long

/* Local function prototypes */
static int	write_dword (flash_info_t* info, ulong dest, unsigned char *pdata);
static void	write_via_fpu (volatile DWORD* addr, DWORD* data);

flash_info_t flash_info[CFG_MAX_FLASH_BANKS];

/*-----------------------------------------------------------------------
 */
void flash_reset (void)
{
	unsigned long msr;
	DWORD cmd_reset = 0x00F000F000F000F0LL;

	if (flash_info[0].flash_id != FLASH_UNKNOWN) {
		msr = get_msr ();
		set_msr (msr | MSR_FP);

		write_via_fpu ((DWORD*)flash_info[0].start[0], &cmd_reset );

		set_msr (msr);
	}
}

/*-----------------------------------------------------------------------
 */
ulong flash_get_size (ulong baseaddr, flash_info_t * info)
{
	int i;
	unsigned long msr;
	DWORD flashtest;
	DWORD cmd_select[3] = { 0x00AA00AA00AA00AALL, 0x0055005500550055LL,
							0x0090009000900090LL };

	/* Enable FPU */
	msr = get_msr ();
	set_msr (msr | MSR_FP);

	/* Write auto-select command sequence */
	write_via_fpu ((DWORD*)(baseaddr + (0x0555 << 3)), &cmd_select[0] );
	write_via_fpu ((DWORD*)(baseaddr + (0x02AA << 3)), &cmd_select[1] );
	write_via_fpu ((DWORD*)(baseaddr + (0x0555 << 3)), &cmd_select[2] );

	/* Restore FPU */
	set_msr (msr);

	/* Read manufacturer ID */
	flashtest = *(volatile DWORD*)baseaddr;
	switch ((int)flashtest) {
	case AMD_MANUFACT:
		info->flash_id = FLASH_MAN_AMD;
		break;
	case FUJ_MANUFACT:
		info->flash_id = FLASH_MAN_FUJ;
		break;
	default:
		/* No, faulty or unknown flash */
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		return (0);
	}

	/* Read device ID */
	flashtest = *(volatile DWORD*)(baseaddr + 8);
	switch ((long)flashtest) {
	case AMD_ID_LV800T:
		info->flash_id += FLASH_AM800T;
		info->sector_count = 19;
		info->size = 0x00400000;
		break;
	case AMD_ID_LV800B:
		info->flash_id += FLASH_AM800B;
		info->sector_count = 19;
		info->size = 0x00400000;
		break;
	case AMD_ID_LV160T:
		info->flash_id += FLASH_AM160T;
		info->sector_count = 35;
		info->size = 0x00800000;
		break;
	case AMD_ID_LV160B:
		info->flash_id += FLASH_AM160B;
		info->sector_count = 35;
		info->size = 0x00800000;
		break;
	case AMD_ID_DL322T:
		info->flash_id += FLASH_AMDL322T;
		info->sector_count = 71;
		info->size = 0x01000000;
		break;
	case AMD_ID_DL322B:
		info->flash_id += FLASH_AMDL322B;
		info->sector_count = 71;
		info->size = 0x01000000;
		break;
	case AMD_ID_DL323T:
		info->flash_id += FLASH_AMDL323T;
		info->sector_count = 71;
		info->size = 0x01000000;
		break;
	case AMD_ID_DL323B:
		info->flash_id += FLASH_AMDL323B;
		info->sector_count = 71;
		info->size = 0x01000000;
		break;
	case AMD_ID_LV640U:
		info->flash_id += FLASH_AM640U;
		info->sector_count = 128;
		info->size = 0x02000000;
		break;
	default:
		/* Unknown flash type */
		info->flash_id = FLASH_UNKNOWN;
		return (0);
	}

	if ((long)flashtest == AMD_ID_LV640U) {
		/* set up sector start adress table (uniform sector type) */
		for (i = 0; i < info->sector_count; i++)
			info->start[i] = baseaddr + (i * 0x00040000);
	} else if (info->flash_id & FLASH_BTYPE) {
		/* set up sector start adress table (bottom sector type) */
		info->start[0] = baseaddr + 0x00000000;
		info->start[1] = baseaddr + 0x00010000;
		info->start[2] = baseaddr + 0x00018000;
		info->start[3] = baseaddr + 0x00020000;
		for (i = 4; i < info->sector_count; i++) {
			info->start[i] = baseaddr + (i * 0x00040000) - 0x000C0000;
		}
	} else {
		/* set up sector start adress table (top sector type) */
		i = info->sector_count - 1;
		info->start[i--] = baseaddr + info->size - 0x00010000;
		info->start[i--] = baseaddr + info->size - 0x00018000;
		info->start[i--] = baseaddr + info->size - 0x00020000;
		for (; i >= 0; i--) {
			info->start[i] = baseaddr + i * 0x00040000;
		}
	}

	/* check for protected sectors */
	for (i = 0; i < info->sector_count; i++) {
		/* read sector protection at sector address, (A7 .. A0) = 0x02 */
		if (*(volatile DWORD*)(info->start[i] + 16) & 0x0001000100010001LL) {
			info->protect[i] = 1;	/* D0 = 1 if protected */
		} else {
			info->protect[i] = 0;
		}
	}

	flash_reset ();
	return (info->size);
}

/*-----------------------------------------------------------------------
 */
unsigned long flash_init (void)
{
	unsigned long size_b0 = 0;
	int i;

	/* Init: no FLASHes known */
	for (i = 0; i < CFG_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	/* Static FLASH Bank configuration here (only one bank) */
	size_b0 = flash_get_size (CFG_FLASH_BASE, &flash_info[0]);
	if (flash_info[0].flash_id == FLASH_UNKNOWN || size_b0 == 0) {
		printf ("## Unknown FLASH on Bank 0 - Size = 0x%08lx = %ld MB\n",
				size_b0, size_b0 >> 20);
	}

	/*
	 * protect monitor and environment sectors
	 */
#if CFG_MONITOR_BASE >= CFG_FLASH_BASE
	flash_protect (FLAG_PROTECT_SET,
		       CFG_MONITOR_BASE,
		       CFG_MONITOR_BASE + monitor_flash_len - 1, &flash_info[0]);
#endif

#if (CFG_ENV_IS_IN_FLASH == 1) && defined(CFG_ENV_ADDR)
# ifndef  CFG_ENV_SIZE
#  define CFG_ENV_SIZE	CFG_ENV_SECT_SIZE
# endif
	flash_protect (FLAG_PROTECT_SET,
		       CFG_ENV_ADDR,
		       CFG_ENV_ADDR + CFG_ENV_SIZE - 1, &flash_info[0]);
#endif

	return (size_b0);
}

/*-----------------------------------------------------------------------
 */
void flash_print_info (flash_info_t * info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_AMD:
		printf ("AMD ");
		break;
	case FLASH_MAN_FUJ:
		printf ("FUJITSU ");
		break;
	default:
		printf ("Unknown Vendor ");
		break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AM800T:
		printf ("29LV800T (8 M, top sector)\n");
		break;
	case FLASH_AM800B:
		printf ("29LV800T (8 M, bottom sector)\n");
		break;
	case FLASH_AM160T:
		printf ("29LV160T (16 M, top sector)\n");
		break;
	case FLASH_AM160B:
		printf ("29LV160B (16 M, bottom sector)\n");
		break;
	case FLASH_AMDL322T:
		printf ("29DL322T (32 M, top sector)\n");
		break;
	case FLASH_AMDL322B:
		printf ("29DL322B (32 M, bottom sector)\n");
		break;
	case FLASH_AMDL323T:
		printf ("29DL323T (32 M, top sector)\n");
		break;
	case FLASH_AMDL323B:
		printf ("29DL323B (32 M, bottom sector)\n");
		break;
	case FLASH_AM640U:
		printf ("29LV640D (64 M, uniform sector)\n");
		break;
	default:
		printf ("Unknown Chip Type\n");
		break;
	}

	printf ("  Size: %ld MB in %d Sectors\n",
			info->size >> 20, info->sector_count);

	printf ("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; ++i) {
		if ((i % 5) == 0)
			printf ("\n   ");
		printf (" %08lX%s",
			info->start[i],
			info->protect[i] ? " (RO)" : "     "
		);
	}
	printf ("\n");
	return;
}

/*-----------------------------------------------------------------------
 */
int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	int flag, prot, sect, l_sect;
	ulong start, now, last;
	unsigned long msr;
	DWORD cmd_erase[6] = { 0x00AA00AA00AA00AALL, 0x0055005500550055LL,
						   0x0080008000800080LL, 0x00AA00AA00AA00AALL,
						   0x0055005500550055LL, 0x0030003000300030LL };

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf ("- missing\n");
		} else {
			printf ("- no sectors to erase\n");
		}
		return 1;
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect])
			prot++;
	}

	if (prot) {
		printf ("- Warning: %d protected sectors will not be erased!\n",
			prot);
	} else {
		printf ("\n");
	}

	l_sect = -1;

	/* Enable FPU */
	msr = get_msr();
	set_msr ( msr | MSR_FP );

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts ();

	write_via_fpu ((DWORD*)(info->start[0] + (0x0555 << 3)), &cmd_erase[0] );
	write_via_fpu ((DWORD*)(info->start[0] + (0x02AA << 3)), &cmd_erase[1] );
	write_via_fpu ((DWORD*)(info->start[0] + (0x0555 << 3)), &cmd_erase[2] );
	write_via_fpu ((DWORD*)(info->start[0] + (0x0555 << 3)), &cmd_erase[3] );
	write_via_fpu ((DWORD*)(info->start[0] + (0x02AA << 3)), &cmd_erase[4] );
	udelay (1000);

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			write_via_fpu ((DWORD*)info->start[sect], &cmd_erase[5] );
			l_sect = sect;
		}
	}

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts ();

	/* Restore FPU */
	set_msr (msr);

	/* wait at least 80us - let's wait 1 ms */
	udelay (1000);

	/*
	 * We wait for the last triggered sector
	 */
	if (l_sect < 0)
		goto DONE;

	start = get_timer (0);
	last = start;
	while ((*(volatile DWORD*)info->start[l_sect] & 0x0080008000800080LL )
				!= 0x0080008000800080LL )
	{
		if ((now = get_timer (start)) > CFG_FLASH_ERASE_TOUT) {
			printf ("Timeout\n");
			return 1;
		}
		/* show that we're waiting */
		if ((now - last) > 1000) {	/* every second */
			serial_putc ('.');
			last = now;
		}
	}

  DONE:
	/* reset to read mode */
	flash_reset ();

	printf (" done\n");
	return 0;
}


/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */

int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	ulong dp;
	static unsigned char bb[8];
	int i, l, rc, cc = cnt;

	dp = (addr & ~7);		/* get lower dword aligned address */

	/*
	 * handle unaligned start bytes
	 */
	if ((l = addr - dp) != 0) {
		for (i = 0; i < 8; i++)
			bb[i] = (i < l || (i - l) >= cc) ? *(char*)(dp + i) : *src++;
		if ((rc = write_dword (info, dp, bb)) != 0) {
			return (rc);
		}
		dp += 8;
		cc -= 8 - l;
	}

	/*
	 * handle word aligned part
	 */
	while (cc >= 8) {
		if ((rc = write_dword (info, dp, src)) != 0) {
			return (rc);
		}
		dp += 8;
		src += 8;
		cc -= 8;
	}

	if (cc <= 0) {
		return (0);
	}

	/*
	 * handle unaligned tail bytes
	 */
	for (i = 0; i < 8; i++) {
		bb[i] = (i < cc) ? *src++ : *(char*)(dp + i);
	}
	return (write_dword (info, dp, bb));
}

/*-----------------------------------------------------------------------
 * Write a dword to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_dword (flash_info_t * info, ulong dest, unsigned char *pdata)
{
	ulong start;
	unsigned long msr;
	int flag, i;
	DWORD data;
	DWORD cmd_write[3] = { 0x00AA00AA00AA00AALL, 0x0055005500550055LL,
						   0x00A000A000A000A0LL };

	for (data = 0, i = 0; i < 8; i++)
		data = (data << 8) + *pdata++;

	/* Check if Flash is (sufficiently) erased */
	if ((*(DWORD*)dest & data) != data) {
		return (2);
	}

	/* Enable FPU */
	msr = get_msr();
	set_msr( msr | MSR_FP );

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts ();

	write_via_fpu ((DWORD*)(info->start[0] + (0x0555 << 3)), &cmd_write[0] );
	write_via_fpu ((DWORD*)(info->start[0] + (0x02AA << 3)), &cmd_write[1] );
	write_via_fpu ((DWORD*)(info->start[0] + (0x0555 << 3)), &cmd_write[2] );
	write_via_fpu ((DWORD*)dest, &data );

	/* re-enable interrupts if necessary */
	if (flag)
		enable_interrupts ();

	/* Restore FPU */
	set_msr(msr);

	/* data polling for D7 */
	start = get_timer (0);
	while (*(volatile DWORD*)dest != data ) {
		if (get_timer (start) > CFG_FLASH_WRITE_TOUT) {
			return (1);
		}
	}
	return (0);
}

/*-----------------------------------------------------------------------
 */
static void write_via_fpu (volatile DWORD* addr, DWORD* data)
{
	__asm__ __volatile__ ("lfd  1, 0(%0)"::"r" (data));
	__asm__ __volatile__ ("stfd 1, 0(%0)"::"r" (addr));
	__asm__ __volatile__ ("eieio");
}
