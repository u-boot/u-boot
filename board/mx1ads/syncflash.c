/*
 * board/mx1ads/syncflash.c
 *
 * (c) Copyright 2004
 * Techware Information Technology, Inc.
 * http://www.techware.com.tw/
 *
 * Ming-Len Wu <minglen_wu@techware.com.tw>
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
/*#include <mc9328.h>*/
#include <asm/arch/imx-regs.h>

typedef unsigned long * p_u32;

/* 4Mx16x2 IAM=0 CSD1 */

flash_info_t flash_info[CFG_MAX_FLASH_BANKS];	/* info for FLASH chips    */

/*  Following Setting is for CSD1	*/
#define SFCTL			0x00221004
#define reg_SFCTL		__REG(SFCTL)

#define SYNCFLASH_A10		(0x00100000)

#define CMD_NORMAL		(0x81020300)			/* Normal Mode			*/
#define CMD_PREC		(CMD_NORMAL + 0x10000000) 	/* Precharge Command		*/
#define CMD_AUTO		(CMD_NORMAL + 0x20000000) 	/* Auto Refresh Command		*/
#define CMD_LMR			(CMD_NORMAL + 0x30000000) 	/* Load Mode Register Command 	*/
#define CMD_LCR			(CMD_NORMAL + 0x60000000) 	/* LCR Command			*/
#define CMD_PROGRAM		(CMD_NORMAL + 0x70000000)

#define MODE_REG_VAL		(CFG_FLASH_BASE+0x0008CC00) 	/* Cas Latency 3		*/

/* LCR Command */
#define LCR_READSTATUS		(0x0001C000)			/* 0x70				*/
#define LCR_ERASE_CONFIRM	(0x00008000)			/* 0x20				*/
#define LCR_ERASE_NVMODE	(0x0000C000)			/* 0x30				*/
#define LCR_PROG_NVMODE		(0x00028000)			/* 0xA0				*/
#define LCR_SR_CLEAR		(0x00014000)			/* 0x50				*/

/* Get Status register 			*/
u32 SF_SR(void) {
	u32 tmp,tmp1;

	reg_SFCTL	= CMD_PROGRAM;
	tmp 		= __REG(CFG_FLASH_BASE);

	reg_SFCTL	= CMD_NORMAL;

	reg_SFCTL	= CMD_LCR;			/* Activate LCR Mode 		*/
	tmp1 		= __REG(CFG_FLASH_BASE + LCR_SR_CLEAR);

	return tmp;
}

/* check if SyncFlash is ready 		*/
u8 SF_Ready(void) {
	u32 tmp;

	tmp	= SF_SR();

	if ((tmp & 0x00800000) && (tmp & 0x001C0000)) {
		printf ("SyncFlash Error code %08x\n",tmp);
	};

	if ((tmp & 0x00000080) && (tmp & 0x0000001C)) {
		printf ("SyncFlash Error code %08x\n",tmp);
	};

	if (tmp == 0x00800080) 		/* Test Bit 7 of SR	*/
		return 1;
	else
		return 0;
}

/* Issue the precharge all command 		*/
void SF_PrechargeAll(void) {

	u32 tmp;

	reg_SFCTL	= CMD_PREC;			/* Set Precharge Command 	*/
	tmp 		= __REG(CFG_FLASH_BASE + SYNCFLASH_A10); /* Issue Precharge All Command */
}

/* set SyncFlash to normal mode			*/
void SF_Normal(void) {

	SF_PrechargeAll();

	reg_SFCTL	= CMD_NORMAL;
}

/* Erase SyncFlash 				*/
void SF_Erase(u32 RowAddress) {
	u32 tmp;

	reg_SFCTL	= CMD_NORMAL;
	tmp 		= __REG(RowAddress);

	reg_SFCTL	= CMD_PREC;
	tmp 		= __REG(RowAddress);

	reg_SFCTL 	= CMD_LCR;			/* Set LCR mode 		*/
	__REG(RowAddress + LCR_ERASE_CONFIRM)	= 0;	/* Issue Erase Setup Command 	*/

	reg_SFCTL	= CMD_NORMAL;			/* return to Normal mode 	*/
	__REG(RowAddress)	= 0xD0D0D0D0; 		/* Confirm			*/

	while(!SF_Ready());
}

void SF_NvmodeErase(void) {
	SF_PrechargeAll();

	reg_SFCTL	= CMD_LCR;			/* Set to LCR mode		*/
	__REG(CFG_FLASH_BASE + LCR_ERASE_NVMODE)  = 0;	/* Issue Erase Nvmode Reg Command */

	reg_SFCTL	= CMD_NORMAL;			/* Return to Normal mode 	*/
	__REG(CFG_FLASH_BASE + LCR_ERASE_NVMODE) = 0xC0C0C0C0;	/* Confirm 		*/

	while(!SF_Ready());
}

void SF_NvmodeWrite(void) {
	SF_PrechargeAll();

	reg_SFCTL 	= CMD_LCR;			/* Set to LCR mode 		*/
	__REG(CFG_FLASH_BASE+LCR_PROG_NVMODE) = 0;	/* Issue Program Nvmode reg command */

	reg_SFCTL	= CMD_NORMAL;			/* Return to Normal mode 	*/
	__REG(CFG_FLASH_BASE+LCR_PROG_NVMODE) = 0xC0C0C0C0; 	/* Confirm not needed 	*/
}

/****************************************************************************************/

ulong flash_init(void) {
	int i, j;
	u32 tmp;

/* Turn on CSD1 for negating RESETSF of SyncFLash */

	reg_SFCTL 	|= 0x80000000;		/* enable CSD1 for SyncFlash 		*/
	udelay(200);

	reg_SFCTL 	= CMD_LMR;		/* Set Load Mode Register Command 	*/
	tmp 		= __REG(MODE_REG_VAL);	/* Issue Load Mode Register Command 	*/

	SF_Normal();

	i = 0;

	flash_info[i].flash_id 	=  FLASH_MAN_MT | FLASH_MT28S4M16LC;

	flash_info[i].size 	= FLASH_BANK_SIZE;
	flash_info[i].sector_count = CFG_MAX_FLASH_SECT;

	memset(flash_info[i].protect, 0, CFG_MAX_FLASH_SECT);

	for (j = 0; j < flash_info[i].sector_count; j++) {
		flash_info[i].start[j] = CFG_FLASH_BASE + j * 0x00100000;
	}

	flash_protect(FLAG_PROTECT_SET,
		CFG_FLASH_BASE,
		CFG_FLASH_BASE + monitor_flash_len - 1,
		&flash_info[0]);

	flash_protect(FLAG_PROTECT_SET,
		CFG_ENV_ADDR,
		CFG_ENV_ADDR + CFG_ENV_SIZE - 1,
		&flash_info[0]);

	return FLASH_BANK_SIZE;
}

void flash_print_info (flash_info_t *info) {

	int i;

	switch (info->flash_id & FLASH_VENDMASK) {
		case (FLASH_MAN_MT & FLASH_VENDMASK):
			printf("Micron: ");
			break;
		default:
			printf("Unknown Vendor ");
			break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
		case (FLASH_MT28S4M16LC & FLASH_TYPEMASK):
			printf("2x FLASH_MT28S4M16LC (16MB Total)\n");
			break;
		default:
			printf("Unknown Chip Type\n");
			return;
			break;
	}

	printf("  Size: %ld MB in %d Sectors\n",
		info->size >> 20, info->sector_count);

	printf("  Sector Start Addresses: ");

	for (i = 0; i < info->sector_count; i++) {
		if ((i % 5) == 0)
			printf ("\n   ");

		printf (" %08lX%s", info->start[i],
			info->protect[i] ? " (RO)" : "     ");
	}

	printf ("\n");
}

/*-----------------------------------------------------------------------*/

int flash_erase (flash_info_t *info, int s_first, int s_last) {
	int iflag, cflag, prot, sect;
	int rc = ERR_OK;

/* first look for protection bits */

	if (info->flash_id == FLASH_UNKNOWN)
		return ERR_UNKNOWN_FLASH_TYPE;

	if ((s_first < 0) || (s_first > s_last))
		return ERR_INVAL;

	if ((info->flash_id & FLASH_VENDMASK) != (FLASH_MAN_MT & FLASH_VENDMASK))
		return ERR_UNKNOWN_FLASH_VENDOR;

	prot = 0;

	for (sect = s_first; sect <= s_last; ++sect) {
		if (info->protect[sect])
			prot++;
	}

	if (prot) {
		printf("protected!\n");
		return ERR_PROTECTED;
	}
/*
 * Disable interrupts which might cause a timeout
 * here. Remember that our exception vectors are
 * at address 0 in the flash, and we don't want a
 * (ticker) exception to happen while the flash
 * chip is in programming mode.
 */

	cflag = icache_status();
	icache_disable();
	iflag = disable_interrupts();

/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last && !ctrlc(); sect++) {

		printf("Erasing sector %2d ... ", sect);

/* arm simple, non interrupt dependent timer */

		reset_timer_masked();

		SF_NvmodeErase();
		SF_NvmodeWrite();

		SF_Erase(CFG_FLASH_BASE + (0x0100000 * sect));
		SF_Normal();

		printf("ok.\n");
	}

	if (ctrlc())
		printf("User Interrupt!\n");

	if (iflag)
		enable_interrupts();

	if (cflag)
		icache_enable();

	return rc;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash.
 */

int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt) {
	int i;

	for(i = 0; i < cnt; i += 4) {

		SF_PrechargeAll();

		reg_SFCTL	= CMD_PROGRAM;		/* Enter SyncFlash Program mode */
		__REG(addr + i) = __REG((u32)src  + i);

		while(!SF_Ready());
	}

	SF_Normal();

	return ERR_OK;
}
