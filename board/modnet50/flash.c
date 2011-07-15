/*
 * (C) Copyright 2002
 * MAZeT GmbH <www.mazet.de>
 * Stephan Linz <linz@mazet.de>, <linz@li-pro.net>
 *
 * The most stuff comes from PPCBoot and Linux.
 *
 * IMMS gGmbH <www.imms.de>
 * Thomas Elste <info@elste.org>
 *
 * Modifications for ModNET50 Board
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
#include <asm/arch/netarm_registers.h>

#define SCR   (*(volatile unsigned int *)(NETARM_GEN_MODULE_BASE + NETARM_GEN_SYSTEM_CONTROL))

#define ALIGN_ABORT_OFF		SCR = SCR & ~NETARM_GEN_SYS_CFG_ALIGN_ABORT
#define ALIGN_ABORT_ON		SCR = SCR |  NETARM_GEN_SYS_CFG_ALIGN_ABORT

#define PROG_ADDR		(0x555*2)
#define SETUP_ADDR		(0x555*2)
#define ID_ADDR			(0x555*2)
#define UNLOCK_ADDR1		(0x555*2)
#define UNLOCK_ADDR2		(0x2AA*2)

#define UNLOCK_CMD1		(0xAA)
#define UNLOCK_CMD2		(0x55)
#define ERASE_SUSPEND_CMD	(0xB0)
#define ERASE_RESUME_CMD	(0x30)
#define RESET_CMD		(0xF0)
#define ID_CMD			(0x90)
#define SECERASE_CMD		(0x30)
#define CHIPERASE_CMD		(0x10)
#define PROG_CMD		(0xa0)
#define SETUP_CMD		(0x80)

#define DQ2			(0x04)
#define DQ3			(DQ2*2)
#define DQ5			(DQ3*4)
#define DQ6			(DQ5*2)

#define WRITE_UNLOCK(addr) { \
  *(volatile __u16*)(addr + UNLOCK_ADDR1) = (__u16)UNLOCK_CMD1; \
  *(volatile __u16*)(addr + UNLOCK_ADDR2) = (__u16)UNLOCK_CMD2; \
}

#define CONFIG_AM29_RESERVED	(0)
#define K			(1024)
#define MB			(4)

#define CELL_SIZE		(64*K)
#define DEVICE_SIZE		(MB*K*K)
#define CELLS_PER_DEVICE	(DEVICE_SIZE/CELL_SIZE)
#define RESERVED_CELLS		(CONFIG_AM29_RESERVED*K)/CELL_SIZE
#define MAX_FLASH_DEVICES	(1)
#define AVAIL_SIZE		(DEVICE_SIZE*MAX_FLASH_DEVICES - RESERVED_CELLS*CELL_SIZE)


flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];
static __u16 toggling_bits;


/*-----------------------------------------------------------------------
 */
ulong flash_get_size (ulong baseaddr, flash_info_t * info)
{
	short i;
	__u16 flashtest;

	/* Write auto select command sequence and test FLASH answer */
	WRITE_UNLOCK (baseaddr);
	*(volatile __u16 *) (baseaddr + ID_ADDR) = (__u16) ID_CMD;
	flashtest /* manufacturer ID */  = *(volatile __u16 *) (baseaddr);
	*(volatile __u16 *) (baseaddr + ID_ADDR) = (__u16) RESET_CMD;

	switch ((__u32) ((flashtest << 16) + flashtest)) {
	case AMD_MANUFACT:
		info->flash_id = FLASH_MAN_AMD & FLASH_VENDMASK;
		break;
	case FUJ_MANUFACT:
		info->flash_id = FLASH_MAN_FUJ & FLASH_VENDMASK;
		break;
	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		return (0);	/* no or unknown flash */
	}

	/* Write auto select command sequence and test FLASH answer */
	WRITE_UNLOCK (baseaddr);
	*(volatile __u16 *) (baseaddr + ID_ADDR) = (__u16) ID_CMD;
	flashtest /* device ID */  = *(volatile __u16 *) (baseaddr + 2);
	*(volatile __u16 *) (baseaddr + ID_ADDR) = (__u16) RESET_CMD;

	/* toggling_bits = (flashtest == TOSHIBA)?(DQ6):(DQ2|DQ6); */
	toggling_bits = (DQ2 | DQ6);

	switch ((__u32) ((flashtest << 16) + flashtest)) {
	case AMD_ID_LV160B:
		info->flash_id +=
			(FLASH_AM160LV | FLASH_AM160B) & FLASH_TYPEMASK;
		info->sector_count = CONFIG_SYS_MAX_FLASH_SECT;
		info->size = CONFIG_SYS_FLASH_SIZE;
		/* 1*16K Boot Block
		   2*8K Parameter Block
		   1*32K Small Main Block */
		info->start[0] = baseaddr;
		info->start[1] = baseaddr + 0x4000;
		info->start[2] = baseaddr + 0x6000;
		info->start[3] = baseaddr + 0x8000;
		for (i = 1; i < info->sector_count; i++)
			info->start[3 + i] = baseaddr + i * CONFIG_SYS_MAIN_SECT_SIZE;
		break;
	default:
		info->flash_id = FLASH_UNKNOWN;
		return (0);	/* no or unknown flash */
	}

	for (i = 0; i < info->sector_count; i++) {
		/* Write auto select command sequence and test FLASH answer */
		WRITE_UNLOCK (info->start[i]);
		*(volatile __u16 *) (info->start[i] + ID_ADDR) = (__u16) ID_CMD;
		flashtest /* protected verify */  = *(volatile __u16 *) (info->start[i] + 4);
		*(volatile __u16 *) (info->start[i] + ID_ADDR) = (__u16) RESET_CMD;
		if (flashtest & 0x0001) {
			info->protect[i] = 1;	/* D0 = 1 if protected */
		} else {
			info->protect[i] = 0;
		}
	}
	return (info->size);
}

/*-----------------------------------------------------------------------
 */
ulong flash_init (void)
{
	ulong size = 0;
	int i;

	/* Init: no FLASHes known */
	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}

	/* Static FLASH Bank configuration here (only one bank) */
	size = flash_get_size (CONFIG_SYS_FLASH_BASE, &flash_info[0]);

	if (flash_info[0].flash_id == FLASH_UNKNOWN || size == 0) {
		printf ("## Unknown FLASH on Bank 0 - Size = 0x%08lx = %ld MB\n",
			size, size >> 20);
	}

	/*
	 * protect monitor and environment sectors
	 */
	flash_protect (FLAG_PROTECT_SET,
		       CONFIG_SYS_FLASH_BASE,
		       CONFIG_SYS_FLASH_BASE + monitor_flash_len - 1,
		       &flash_info[0]);

	flash_protect (FLAG_PROTECT_SET,
		       CONFIG_ENV_ADDR,
		       CONFIG_ENV_ADDR + CONFIG_ENV_SIZE - 1, &flash_info[0]);

	return size;
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
		printf ("Fujitsu ");
		break;
	default:
		printf ("Unknown Vendor ");
		break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AMDL323B:
		printf ("29DL323B (32 M, bottom sector)\n");
		break;
	case (FLASH_AM160LV | FLASH_AM160B):
		printf ("29LV160BE (1M x 16, bottom sector)\n");
		break;
	default:
		printf ("Unknown Chip Type\n");
		break;
	}

	printf ("  Size: %ld MB in %d Sectors\n",
		info->size >> 20, info->sector_count);
	printf ("  Sector Start Addresses:");
	for (i = 0; i < info->sector_count; i++) {
		if ((i % 4) == 0)
			printf ("\n   ");
		printf (" S%02d @ 0x%08lX%s", i,
			info->start[i], info->protect[i] ? " !" : "  ");
	}
	printf ("\n");
	return;
}

/*-----------------------------------------------------------------------
 */
int flash_check_protection (flash_info_t * info, int s_first, int s_last)
{
	int sect, prot = 0;

	for (sect = s_first; sect <= s_last; sect++)
		if (info->protect[sect])
			prot++;
	if (prot)
		printf ("- can't erase %d protected sectors\n", prot);
	return prot;
}

/*-----------------------------------------------------------------------
 */
int flash_check_erase_amd (ulong start)
{
	__u16 v1, v2;

	v1 = *(volatile __u16 *) (start);
	v2 = *(volatile __u16 *) (start);

	if (((v1 ^ v2) & toggling_bits) == toggling_bits) {
		if (((v1 | v2) & DQ5) == DQ5) {
			printf ("[DQ5] ");
			/* OOPS: exceeded timing limits */

			v1 = *(volatile __u16 *) (start);
			v2 = *(volatile __u16 *) (start);

			if (((v1 ^ v2) & toggling_bits) == toggling_bits) {

				printf ("[%s] ",
					((toggling_bits & (DQ2 | DQ6)) ==
					 (DQ2 | DQ6)) ? "DQ2,DQ6" : "DQ6");

				/* OOPS: there is an erasure in progress,
				 *       try to reset chip */
				*(volatile __u16 *) (start) =
					(__u16) RESET_CMD;

				return 1;	/* still busy */
			}
		}
		return 1;	/* still busy */
	}
	return 0;		/* be free */
}

/*-----------------------------------------------------------------------
 */
int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	int flag, sect, setup_offset = 0;
	int rc = ERR_OK;
	ulong start;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("- missing\n");
		return ERR_UNKNOWN_FLASH_TYPE;
	}

	if ((s_first < 0) || (s_first > s_last)) {
		printf ("- no sectors to erase\n");
		return ERR_INVAL;
	}

	if (flash_check_protection (info, s_first, s_last))
		return ERR_PROTECTED;

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_FUJ:
	case FLASH_MAN_AMD:
		switch (info->flash_id & FLASH_TYPEMASK) {
		case (FLASH_AM160LV | FLASH_AM160B):
			setup_offset = UNLOCK_ADDR1;	/* just the adress for setup_cmd differs */
		case FLASH_AMDL323B:
			/*
			 * Disable interrupts which might cause a timeout
			 * here. Remember that our exception vectors are
			 * at address 0 in the flash, and we don't want a
			 * (ticker) exception to happen while the flash
			 * chip is in programming mode.
			 */
			flag = disable_interrupts ();
			/* Start erase on unprotected sectors */
			for (sect = s_first; sect <= s_last && !ctrlc ();
			     sect++) {
				printf ("Erasing sector %2d ... ", sect);

				if (info->protect[sect] == 0) {
					/* not protected */
					/* Write sector erase command sequence */
					WRITE_UNLOCK (info->start[0]);
					*(volatile __u16 *) (info->start[0] +
							     setup_offset) =
						(__u16) SETUP_CMD;
					WRITE_UNLOCK (info->start[0]);
					*(volatile __u16 *) (info->
							     start[sect]) =
						(__u16) SECERASE_CMD;

					/* wait some time */
					start = get_timer(0);
					while (get_timer(start) < 1000) {
					}

					/* arm simple, non interrupt dependent timer */
					start = get_timer(0);
					while (flash_check_erase_amd (info->start[sect])) {
						if (get_timer(start) > CONFIG_SYS_FLASH_ERASE_TOUT) {
							printf ("timeout!\n");
							/* OOPS: reach timeout,
							 * try to reset chip
							 */
							*(volatile __u16 *) (info-> start[sect]) = (__u16) RESET_CMD;
							rc = ERR_TIMOUT;
							goto outahere_323B;
						}
					}
					printf ("ok.\n");
				} else {
					printf ("protected!\n");
				}
			}
			if (ctrlc ())
				printf ("User Interrupt!\n");
outahere_323B:
			/* allow flash to settle - wait 10 ms */
			udelay_masked (10000);
			if (flag)
				enable_interrupts ();
			return rc;
		default:
			printf ("- unknown chip type\n");
			return ERR_UNKNOWN_FLASH_TYPE;
		}
		break;
	default:
		printf ("- unknown vendor ");
		return ERR_UNKNOWN_FLASH_VENDOR;
	}
}

/*-----------------------------------------------------------------------
 */
int flash_check_write_amd (ulong dest)
{
	__u16 v1, v2;

	v1 = *(volatile __u16 *) (dest);
	v2 = *(volatile __u16 *) (dest);

	/* DQ6 toggles during write */
	if (((v1 ^ v2) & DQ6) == DQ6) {
		if (((v1 | v2) & DQ5) == DQ5) {
			printf ("[DQ5] @ %08lX\n", dest);

			/* OOPS: exceeded timing limits,
			 *       try to reset chip */
			*(volatile __u16 *) (dest) = (__u16) RESET_CMD;
			return 0;	/* be free */
		}
		return 1;	/* still busy */
	}

	return 0;		/* be free */
}

/*-----------------------------------------------------------------------
 * Copy memory to flash
 */
static int write_word (flash_info_t * info, ulong dest, ushort data)
{
	int rc = ERR_OK;
	int flag;
	ulong start;

	/* Check if Flash is (sufficiently) erased */
	if ((*(__u16 *) (dest) & data) != data)
		return ERR_NOT_ERASED;

	/*
	 * Disable interrupts which might cause a timeout
	 * here. Remember that our exception vectors are
	 * at address 0 in the flash, and we don't want a
	 * (ticker) exception to happen while the flash
	 * chip is in programming mode.
	 */
	flag = disable_interrupts ();

	/* Write program command sequence */
	WRITE_UNLOCK (info->start[0]);

	/* Flash dependend program seqence */
	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_FUJ:
	case FLASH_MAN_AMD:
		switch (info->flash_id & FLASH_TYPEMASK) {
		case (FLASH_AM160LV | FLASH_AM160B):
			*(volatile __u16 *) (info->start[0] + UNLOCK_ADDR1) =
				(__u16) PROG_CMD;
			*(volatile __u16 *) (dest) = (__u16) data;
			break;
		case FLASH_AMDL323B:
			*(volatile __u16 *) (dest) = (__u16) PROG_CMD;
			*(volatile __u16 *) (dest) = (__u16) data;
			break;
		}
	}

	/* arm simple, non interrupt dependent timer */
	start = get_timer(0);

	while (flash_check_write_amd (dest)) {
		if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
			printf ("timeout! @ %08lX\n", dest);
			/* OOPS: reach timeout,
			 *       try to reset chip */
			*(volatile __u16 *) (dest) = (__u16) RESET_CMD;

			rc = ERR_TIMOUT;
			goto outahere_323B;
		}
	}

	/* Check if Flash was (accurately) written */
	if (*(__u16 *) (dest) != data)
		rc = ERR_PROG_ERROR;

outahere_323B:
	if (flag)
		enable_interrupts ();
	return rc;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash.
 */
int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	ulong cp, wp;
	ushort data;
	int l;
	int i, rc;

	wp = (addr & ~1);	/* get lower word aligned address */

	/*
	 * handle unaligned start bytes
	 */
	if ((l = addr - wp) != 0) {
		data = 0;
		for (i = 0, cp = wp; i < l; ++i, ++cp) {
			data = (data >> 8) | (*(uchar *) cp << 8);
		}
		for (; i < 2 && cnt > 0; ++i) {
			data = (data >> 8) | (*src++ << 8);
			--cnt;
			++cp;
		}
		for (; cnt == 0 && i < 2; ++i, ++cp) {
			data = (data >> 8) | (*(uchar *) cp << 8);
		}

		if ((rc = write_word (info, wp, data)) != 0) {
			return (rc);
		}
		wp += 2;
	}

	/*
	 * handle word aligned part
	 */
	while (cnt >= 2) {
		data = *((ushort *) src);
		if ((rc = write_word (info, wp, data)) != 0)
			return (rc);
		src += 2;
		wp += 2;
		cnt -= 2;
	}

	if (cnt == 0)
		return ERR_OK;

	/*
	 * handle unaligned tail bytes
	 */
	data = 0;
	for (i = 0, cp = wp; i < 2 && cnt > 0; ++i, ++cp) {
		data = (data >> 8) | (*src++ << 8);
		--cnt;
	}
	for (; i < 2; ++i, ++cp) {
		data = (data >> 8) | (*(uchar *) cp << 8);
	}

	return write_word (info, wp, data);
}
