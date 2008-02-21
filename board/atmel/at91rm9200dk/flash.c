/*
 * (C) Copyright 2002
 * Lineo, Inc. <www.lineo.com>
 * Bernhard Kuhn <bkuhn@lineo.com>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
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

ulong myflush(void);


/* Flash Organization Structure */
typedef struct OrgDef
{
	unsigned int sector_number;
	unsigned int sector_size;
} OrgDef;


/* Flash Organizations */
OrgDef OrgAT49BV16x4[] =
{
	{  8,  8*1024 },	/*   8 *  8 kBytes sectors */
	{  2, 32*1024 },	/*   2 * 32 kBytes sectors */
	{ 30, 64*1024 },	/*  30 * 64 kBytes sectors */
};

OrgDef OrgAT49BV16x4A[] =
{
	{  8,  8*1024 },	/*   8 *  8 kBytes sectors */
	{ 31, 64*1024 },	/*  31 * 64 kBytes sectors */
};

OrgDef OrgAT49BV6416[] =
{
	{   8,  8*1024 },	/*   8 *  8 kBytes sectors */
	{ 127, 64*1024 },	/* 127 * 64 kBytes sectors */
};

flash_info_t    flash_info[CFG_MAX_FLASH_BANKS];

/* AT49BV1614A Codes */
#define FLASH_CODE1		0xAA
#define FLASH_CODE2		0x55
#define ID_IN_CODE		0x90
#define ID_OUT_CODE		0xF0


#define CMD_READ_ARRAY		0x00F0
#define CMD_UNLOCK1		0x00AA
#define CMD_UNLOCK2		0x0055
#define CMD_ERASE_SETUP		0x0080
#define CMD_ERASE_CONFIRM	0x0030
#define CMD_PROGRAM		0x00A0
#define CMD_UNLOCK_BYPASS	0x0020
#define CMD_SECTOR_UNLOCK	0x0070

#define MEM_FLASH_ADDR1		(*(volatile u16 *)(CFG_FLASH_BASE + (0x00005555<<1)))
#define MEM_FLASH_ADDR2		(*(volatile u16 *)(CFG_FLASH_BASE + (0x00002AAA<<1)))

#define BIT_ERASE_DONE		0x0080
#define BIT_RDY_MASK		0x0080
#define BIT_PROGRAM_ERROR	0x0020
#define BIT_TIMEOUT		0x80000000 /* our flag */

#define READY 1
#define ERR   2
#define TMO   4

/*-----------------------------------------------------------------------
 */
void flash_identification (flash_info_t * info)
{
	volatile u16 manuf_code, device_code, add_device_code;

	MEM_FLASH_ADDR1 = FLASH_CODE1;
	MEM_FLASH_ADDR2 = FLASH_CODE2;
	MEM_FLASH_ADDR1 = ID_IN_CODE;

	manuf_code = *(volatile u16 *) CFG_FLASH_BASE;
	device_code = *(volatile u16 *) (CFG_FLASH_BASE + 2);
	add_device_code = *(volatile u16 *) (CFG_FLASH_BASE + (3 << 1));

	MEM_FLASH_ADDR1 = FLASH_CODE1;
	MEM_FLASH_ADDR2 = FLASH_CODE2;
	MEM_FLASH_ADDR1 = ID_OUT_CODE;

	/* Vendor type */
	info->flash_id = ATM_MANUFACT & FLASH_VENDMASK;
	printf ("Atmel: ");

	if ((device_code & FLASH_TYPEMASK) == (ATM_ID_BV1614 & FLASH_TYPEMASK)) {

		if ((add_device_code & FLASH_TYPEMASK) ==
			(ATM_ID_BV1614A & FLASH_TYPEMASK)) {
			info->flash_id |= ATM_ID_BV1614A & FLASH_TYPEMASK;
			printf ("AT49BV1614A (16Mbit)\n");
		} else {				/* AT49BV1614 Flash */
			info->flash_id |= ATM_ID_BV1614 & FLASH_TYPEMASK;
			printf ("AT49BV1614 (16Mbit)\n");
		}

	} else if ((device_code & FLASH_TYPEMASK) == (ATM_ID_BV6416 & FLASH_TYPEMASK)) {
		info->flash_id |= ATM_ID_BV6416 & FLASH_TYPEMASK;
		printf ("AT49BV6416 (64Mbit)\n");
	}
}

ushort flash_number_sector(OrgDef *pOrgDef, unsigned int nb_blocks)
{
	int i, nb_sectors = 0;

	for (i=0; i<nb_blocks; i++){
		nb_sectors += pOrgDef[i].sector_number;
	}

	return nb_sectors;
}

void flash_unlock_sector(flash_info_t * info, unsigned int sector)
{
	volatile u16 *addr = (volatile u16 *) (info->start[sector]);

	MEM_FLASH_ADDR1 = CMD_UNLOCK1;
	*addr = CMD_SECTOR_UNLOCK;
}


ulong flash_init (void)
{
	int i, j, k;
	unsigned int flash_nb_blocks, sector;
	unsigned int start_address;
	OrgDef *pOrgDef;

	ulong size = 0;

	for (i = 0; i < CFG_MAX_FLASH_BANKS; i++) {
		ulong flashbase = 0;

		flash_identification (&flash_info[i]);

		if ((flash_info[i].flash_id & FLASH_TYPEMASK) ==
			(ATM_ID_BV1614 & FLASH_TYPEMASK)) {

			pOrgDef = OrgAT49BV16x4;
			flash_nb_blocks = sizeof (OrgAT49BV16x4) / sizeof (OrgDef);
		} else if ((flash_info[i].flash_id & FLASH_TYPEMASK) ==
			(ATM_ID_BV1614A & FLASH_TYPEMASK)){	/* AT49BV1614A Flash */

			pOrgDef = OrgAT49BV16x4A;
			flash_nb_blocks = sizeof (OrgAT49BV16x4A) / sizeof (OrgDef);
		} else if ((flash_info[i].flash_id & FLASH_TYPEMASK) ==
			(ATM_ID_BV6416 & FLASH_TYPEMASK)){	/* AT49BV6416 Flash */

			pOrgDef = OrgAT49BV6416;
			flash_nb_blocks = sizeof (OrgAT49BV6416) / sizeof (OrgDef);
		} else {
			flash_nb_blocks = 0;
			pOrgDef = OrgAT49BV16x4;
		}

		flash_info[i].sector_count = flash_number_sector(pOrgDef, flash_nb_blocks);
		memset (flash_info[i].protect, 0, flash_info[i].sector_count);

		if (i == 0)
			flashbase = PHYS_FLASH_1;
		else
			panic ("configured too many flash banks!\n");

		sector = 0;
		start_address = flashbase;
		flash_info[i].size = 0;

		for (j = 0; j < flash_nb_blocks; j++) {
			for (k = 0; k < pOrgDef[j].sector_number; k++) {
				flash_info[i].start[sector++] = start_address;
				start_address += pOrgDef[j].sector_size;
				flash_info[i].size += pOrgDef[j].sector_size;
			}
		}

		size += flash_info[i].size;

		if ((flash_info[i].flash_id & FLASH_TYPEMASK) ==
			(ATM_ID_BV6416 & FLASH_TYPEMASK)){	/* AT49BV6416 Flash */

			/* Unlock all sectors at reset */
			for (j=0; j<flash_info[i].sector_count; j++){
				flash_unlock_sector(&flash_info[i], j);
			}
		}
	}

	/* Protect binary boot image */
	flash_protect (FLAG_PROTECT_SET,
		       CFG_FLASH_BASE,
		       CFG_FLASH_BASE + CFG_BOOT_SIZE - 1, &flash_info[0]);

	/* Protect environment variables */
	flash_protect (FLAG_PROTECT_SET,
		       CFG_ENV_ADDR,
		       CFG_ENV_ADDR + CFG_ENV_SIZE - 1, &flash_info[0]);

	/* Protect U-Boot gzipped image */
	flash_protect (FLAG_PROTECT_SET,
		       CFG_U_BOOT_BASE,
		       CFG_U_BOOT_BASE + CFG_U_BOOT_SIZE - 1, &flash_info[0]);

	return size;
}

/*-----------------------------------------------------------------------
 */
void flash_print_info (flash_info_t * info)
{
	int i;

	switch (info->flash_id & FLASH_VENDMASK) {
	case (ATM_MANUFACT & FLASH_VENDMASK):
		printf ("Atmel: ");
		break;
	default:
		printf ("Unknown Vendor ");
		break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case (ATM_ID_BV1614 & FLASH_TYPEMASK):
		printf ("AT49BV1614 (16Mbit)\n");
		break;
	case (ATM_ID_BV1614A & FLASH_TYPEMASK):
		printf ("AT49BV1614A (16Mbit)\n");
		break;
	case (ATM_ID_BV6416 & FLASH_TYPEMASK):
		printf ("AT49BV6416 (64Mbit)\n");
		break;
	default:
		printf ("Unknown Chip Type\n");
		return;
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
}

/*-----------------------------------------------------------------------
 */

int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	ulong result;
	int iflag, cflag, prot, sect;
	int rc = ERR_OK;
	int chip1;

	/* first look for protection bits */

	if (info->flash_id == FLASH_UNKNOWN)
		return ERR_UNKNOWN_FLASH_TYPE;

	if ((s_first < 0) || (s_first > s_last)) {
		return ERR_INVAL;
	}

	if ((info->flash_id & FLASH_VENDMASK) !=
		(ATM_MANUFACT & FLASH_VENDMASK)) {
		return ERR_UNKNOWN_FLASH_VENDOR;
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect) {
		if (info->protect[sect]) {
			prot++;
		}
	}
	if (prot)
		return ERR_PROTECTED;

	/*
	 * Disable interrupts which might cause a timeout
	 * here. Remember that our exception vectors are
	 * at address 0 in the flash, and we don't want a
	 * (ticker) exception to happen while the flash
	 * chip is in programming mode.
	 */
	cflag = icache_status ();
	icache_disable ();
	iflag = disable_interrupts ();

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last && !ctrlc (); sect++) {
		printf ("Erasing sector %2d ... ", sect);

		/* arm simple, non interrupt dependent timer */
		reset_timer_masked ();

		if (info->protect[sect] == 0) {	/* not protected */
			volatile u16 *addr = (volatile u16 *) (info->start[sect]);

			MEM_FLASH_ADDR1 = CMD_UNLOCK1;
			MEM_FLASH_ADDR2 = CMD_UNLOCK2;
			MEM_FLASH_ADDR1 = CMD_ERASE_SETUP;

			MEM_FLASH_ADDR1 = CMD_UNLOCK1;
			MEM_FLASH_ADDR2 = CMD_UNLOCK2;
			*addr = CMD_ERASE_CONFIRM;

			/* wait until flash is ready */
			chip1 = 0;

			do {
				result = *addr;

				/* check timeout */
				if (get_timer_masked () > CFG_FLASH_ERASE_TOUT) {
					MEM_FLASH_ADDR1 = CMD_READ_ARRAY;
					chip1 = TMO;
					break;
				}

				if (!chip1 && (result & 0xFFFF) & BIT_ERASE_DONE)
					chip1 = READY;

			} while (!chip1);

			MEM_FLASH_ADDR1 = CMD_READ_ARRAY;

			if (chip1 == ERR) {
				rc = ERR_PROG_ERROR;
				goto outahere;
			}
			if (chip1 == TMO) {
				rc = ERR_TIMOUT;
				goto outahere;
			}

			printf ("ok.\n");
		} else {			/* it was protected */
			printf ("protected!\n");
		}
	}

	if (ctrlc ())
		printf ("User Interrupt!\n");

outahere:
	/* allow flash to settle - wait 10 ms */
	udelay_masked (10000);

	if (iflag)
		enable_interrupts ();

	if (cflag)
		icache_enable ();

	return rc;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash
 */

static int write_word (flash_info_t * info, ulong dest, ulong data)
{
	volatile u16 *addr = (volatile u16 *) dest;
	ulong result;
	int rc = ERR_OK;
	int cflag, iflag;
	int chip1;

	/*
	 * Check if Flash is (sufficiently) erased
	 */
	result = *addr;
	if ((result & data) != data)
		return ERR_NOT_ERASED;

	/*
	 * Disable interrupts which might cause a timeout
	 * here. Remember that our exception vectors are
	 * at address 0 in the flash, and we don't want a
	 * (ticker) exception to happen while the flash
	 * chip is in programming mode.
	 */
	cflag = icache_status ();
	icache_disable ();
	iflag = disable_interrupts ();

	MEM_FLASH_ADDR1 = CMD_UNLOCK1;
	MEM_FLASH_ADDR2 = CMD_UNLOCK2;
	MEM_FLASH_ADDR1 = CMD_PROGRAM;
	*addr = data;

	/* arm simple, non interrupt dependent timer */
	reset_timer_masked ();

	/* wait until flash is ready */
	chip1 = 0;
	do {
		result = *addr;

		/* check timeout */
		if (get_timer_masked () > CFG_FLASH_ERASE_TOUT) {
			chip1 = ERR | TMO;
			break;
		}
		if (!chip1 && ((result & 0x80) == (data & 0x80)))
			chip1 = READY;

	} while (!chip1);

	*addr = CMD_READ_ARRAY;

	if (chip1 == ERR || *addr != data)
		rc = ERR_PROG_ERROR;

	if (iflag)
		enable_interrupts ();

	if (cflag)
		icache_enable ();

	return rc;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash.
 */

int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	ulong wp, data;
	int rc;

	if (addr & 1) {
		printf ("unaligned destination not supported\n");
		return ERR_ALIGN;
	};

	if ((int) src & 1) {
		printf ("unaligned source not supported\n");
		return ERR_ALIGN;
	};

	wp = addr;

	while (cnt >= 2) {
		data = *((volatile u16 *) src);
		if ((rc = write_word (info, wp, data)) != 0) {
			return (rc);
		}
		src += 2;
		wp += 2;
		cnt -= 2;
	}

	if (cnt == 1) {
		data = (*((volatile u8 *) src)) | (*((volatile u8 *) (wp + 1)) <<
										   8);
		if ((rc = write_word (info, wp, data)) != 0) {
			return (rc);
		}
		src += 1;
		wp += 1;
		cnt -= 1;
	};

	return ERR_OK;
}
