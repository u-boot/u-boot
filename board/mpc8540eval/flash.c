/*
 * (C) Copyright 2003 Motorola Inc.
 *  Xianghua Xiao,(X.Xiao@motorola.com)
 *
 * (C) Copyright 2000, 2001
 *  Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2001, Stuart Hughes, Lineo Inc, stuarth@lineo.com
 * Add support the Sharp chips on the mpc8260ads.
 * I started with board/ip860/flash.c and made changes I found in
 * the MTD project by David Schleef.
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

#if !defined(CONFIG_SYS_NO_FLASH)

flash_info_t	flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips	*/

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

/*
 * The variable should be in the flash info structure. Since it
 * is only used in this board specific file it is declared here.
 * In the future I think an endian flag should be part of the
 * flash_info_t structure. (Ron Alder)
 */
static ulong big_endian = 0;

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size (vu_long *addr, flash_info_t *info);
static int write_block (flash_info_t *info, uchar * src, ulong dest, ulong cnt);
static int write_short (flash_info_t *info, ulong dest, ushort data);
static int write_word (flash_info_t *info, ulong dest, ulong data);
static int clear_block_lock_bit(flash_info_t *info, vu_long * addr);
/*-----------------------------------------------------------------------
 */

unsigned long flash_init (void)
{
	unsigned long size;
	int i;

	/* Init: enable write,
	 * or we cannot even write flash commands
	 */
	for (i=0; i<CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;

		/* set the default sector offset */
	}

	/* Static FLASH Bank configuration here - FIXME XXX */

	size = flash_get_size((vu_long *)CONFIG_SYS_FLASH_BASE, &flash_info[0]);

	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH on Bank 0 - Size = 0x%08lx = %ld MB\n",
			size, size<<20);
	}

	/* Re-do sizing to get full correct info */
	size = flash_get_size((vu_long *)CONFIG_SYS_FLASH_BASE, &flash_info[0]);

	flash_info[0].size = size;

#if !defined(CONFIG_RAM_AS_FLASH)
#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE
	/* monitor protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_SYS_MONITOR_BASE,
		      CONFIG_SYS_MONITOR_BASE+monitor_flash_len-1,
		      &flash_info[0]);
#endif

#ifdef	CONFIG_ENV_IS_IN_FLASH
	/* ENV protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_ENV_ADDR,
		      CONFIG_ENV_ADDR+CONFIG_ENV_SECT_SIZE-1,
		      &flash_info[0]);
#endif
#endif
	return (size);
}

/*-----------------------------------------------------------------------
 */
void flash_print_info  (flash_info_t *info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_INTEL:	printf ("Intel ");		break;
	case FLASH_MAN_SHARP:   printf ("Sharp ");		break;
	default:		printf ("Unknown Vendor ");	break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_28F016SV:	printf ("28F016SV (16 Mbit, 32 x 64k)\n");
				break;
	case FLASH_28F160S3:	printf ("28F160S3 (16 Mbit, 32 x 512K)\n");
				break;
	case FLASH_28F320S3:	printf ("28F320S3 (32 Mbit, 64 x 512K)\n");
				break;
	case FLASH_LH28F016SCT: printf ("28F016SC (16 Mbit, 32 x 64K)\n");
				break;
	case FLASH_28F640J3A:   printf ("28F640J3A (64 Mbit, 64 x 128K)\n");
				break;
	default:		printf ("Unknown Chip Type\n");
				break;
	}

	printf ("  Size: %ld MB in %d Sectors\n",
		info->size >> 20, info->sector_count);

	printf ("  Sector Start Addresses:");
	for (i=0; i<info->sector_count; ++i) {
		if ((i % 5) == 0)
			printf ("\n   ");
		printf (" %08lX%s",
			info->start[i],
			info->protect[i] ? " (RO)" : "     "
		);
	}
	printf ("\n");
}

 /* only deal with 16 bit and 32 bit port width, 16bit chip */
static ulong flash_get_size (vu_long *addr, flash_info_t *info)
{
	short i;
	ulong value,va,vb,vc,vd;
	ulong base = (ulong)addr;
	ulong sector_offset;

#ifdef DEBUG
	printf("Check flash at 0x%08x\n",(uint)addr);
#endif
	/* Write "Intelligent Identifier" command: read Manufacturer ID */
	*addr = 0x90909090;
	udelay(20);
	asm("sync");

#ifndef CONFIG_SYS_FLASH_CFI
	printf("Not define CONFIG_SYS_FLASH_CFI\n");
	return (0);
#else
	value = addr[0];
	va=(value & 0xFF000000)>>24;
	vb=(value & 0x00FF0000)>>16;
	vc=(value & 0x0000FF00)>>8;
	vd=(value & 0x000000FF);
	if ((va==0) && (vb==0)) {
		printf("cannot identify Flash\n");
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		return (0);			/* no or unknown flash	*/
	}
	else if ((va==0) && (vb!=0)) {
		big_endian = 1;
		info->chipwidth = FLASH_CFI_BY16;
		if(vb == vd) info->portwidth = FLASH_CFI_32BIT;
		else info->portwidth = FLASH_CFI_16BIT;
	}
	else if ((va!=0) && (vb==0)) {
		big_endian = 0;
		info->chipwidth = FLASH_CFI_BY16;
		if(va == vc) info->portwidth = FLASH_CFI_32BIT;
		else info->portwidth = FLASH_CFI_16BIT;
	}
	else if ((va!=0) && (vb!=0)) {
		big_endian = 1;		/* no meaning for 8bit chip */
		info->chipwidth = FLASH_CFI_BY8;
		if(va == vb) info->portwidth = FLASH_CFI_16BIT;
		else info->portwidth = FLASH_CFI_8BIT;
	}
#ifdef DEBUG
	switch (info->portwidth) {
		case FLASH_CFI_8BIT:
			printf("port width is 8 bit.\n");
			break;
		case FLASH_CFI_16BIT:
			printf("port width is 16 bit, ");
			break;
		case FLASH_CFI_32BIT:
			printf("port width is 32 bit, ");
			break;
	}
	switch (info->chipwidth) {
		case FLASH_CFI_BY16:
			printf("chip width is 16 bit, ");
			switch (big_endian) {
				case 0:
					printf("Little Endian.\n");
					break;
				case 1:
					printf("Big Endian.\n");
					break;
			}
			break;
	}
#endif
#endif		/*#ifdef CONFIG_SYS_FLASH_CFI*/

	if (big_endian==0) value = (addr[0] & 0xFF000000) >>8;
	else value = (addr[0] & 0x00FF0000);
#ifdef DEBUG
	printf("manufacturer=0x%x\n",(uint)(value>>16));
#endif
	switch (value) {
	case MT_MANUFACT & 0xFFFF0000:	/* SHARP, MT or => Intel */
	case INTEL_ALT_MANU & 0xFFFF0000:
		info->flash_id = FLASH_MAN_INTEL;
		break;
	default:
		printf("unknown manufacturer: %x\n", (unsigned int)value);
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		return (0);			/* no or unknown flash	*/
	}

	if (info->portwidth==FLASH_CFI_16BIT) {
		switch (big_endian) {
			case 0:
				value = (addr[0] & 0x0000FF00)>>8;
				break;
			case 1:
				value = (addr[0] & 0x000000FF);
				break;
		}
	}
	else if (info->portwidth == FLASH_CFI_32BIT) {
		switch (big_endian) {
			case 0:
				value = (addr[1] & 0x0000FF00)>>8;
				break;
			case 1:
				value = (addr[1] & 0x000000FF);
				break;
		}
	}

#ifdef DEBUG
	printf("deviceID=0x%x\n",(uint)value);
#endif
	switch (value) {
	case (INTEL_ID_28F016S & 0x0000FFFF):
		info->flash_id += FLASH_28F016SV;
		info->sector_count = 32;
		sector_offset = 0x10000;
		break;				/* => 2 MB		*/

	case (INTEL_ID_28F160S3 & 0x0000FFFF):
		info->flash_id += FLASH_28F160S3;
		info->sector_count = 32;
		sector_offset = 0x10000;
		break;				/* => 2 MB		*/

	case (INTEL_ID_28F320S3 & 0x0000FFFF):
		info->flash_id += FLASH_28F320S3;
		info->sector_count = 64;
		sector_offset = 0x10000;
		break;				/* => 4 MB		*/

	case (INTEL_ID_28F640J3A & 0x0000FFFF):
		info->flash_id += FLASH_28F640J3A;
		info->sector_count = 64;
		sector_offset = 0x20000;
		break;                          /* => 8 MB             */

	case SHARP_ID_28F016SCL & 0x0000FFFF:
	case SHARP_ID_28F016SCZ & 0x0000FFFF:
		info->flash_id      = FLASH_MAN_SHARP | FLASH_LH28F016SCT;
		info->sector_count  = 32;
		sector_offset = 0x10000;
		break;				/* => 2 MB		*/


	default:
		info->flash_id = FLASH_UNKNOWN;
		return (0);			/* => no or unknown flash */

	}

	sector_offset = sector_offset * (info->portwidth / info->chipwidth);
	info->size = info->sector_count * sector_offset;

	/* set up sector start address table */
	for (i = 0; i < info->sector_count; i++) {
		info->start[i] = base;
		base += sector_offset;
		/* don't know how to check sector protection */
		info->protect[i] = 0;
	}

	/*
	 * Prevent writes to uninitialized FLASH.
	 */
	if (info->flash_id != FLASH_UNKNOWN) {
		addr = (vu_long *)info->start[0];
		*addr = 0xFFFFFF;	/* reset bank to read array mode */
		asm("sync");
	}

	return (info->size);
}


/*-----------------------------------------------------------------------
 */

int	flash_erase (flash_info_t *info, int s_first, int s_last)
{
	int flag, prot, sect;
	ulong start, now, last, ready, erase_err_status;

	if (big_endian == 1) {
		ready = 0x0080;
		erase_err_status = 0x00a0;
	}
	else {
		ready = 0x8000;
		erase_err_status = 0xa000;
	}
	if ((info->portwidth / info->chipwidth)==2) {
		ready += (ready <<16);
		erase_err_status += (erase_err_status <<16);
	}

#ifdef DEBUG
	printf ("\nReady flag is 0x%lx\nErase error flag is 0x%lx", ready, erase_err_status);
#endif

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf ("- missing\n");
		} else {
			printf ("- no sectors to erase\n");
		}
		return 1;
	}

	if (    ((info->flash_id & FLASH_VENDMASK) != FLASH_MAN_INTEL)
	     && ((info->flash_id & FLASH_VENDMASK) != FLASH_MAN_SHARP) ) {
		printf ("Can't erase unknown flash type %08lx - aborted\n",
			info->flash_id);
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

#ifdef DEBUG
	printf("\nFlash Erase:\n");
#endif
	/* Make Sure Block Lock Bit is not set. */
	if(clear_block_lock_bit(info, (vu_long *)(info->start[s_first]))){
		return 1;
	}

	/* Start erase on unprotected sectors */
#if defined(DEBUG)
	printf("Begin to erase now,s_first=0x%x s_last=0x%x...\n",s_first,s_last);
#endif
	for (sect = s_first; sect<=s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			vu_short *addr16 = (vu_short *)(info->start[sect]);
			vu_long *addr   = (vu_long *)(info->start[sect]);
			printf(".");
			switch (info->portwidth) {
				case FLASH_CFI_16BIT:
					asm("sync");
					last = start = get_timer (0);
					/* Disable interrupts which might cause a timeout here */
					flag = disable_interrupts();
					/* Reset Array */
					*addr16 = 0xffff;
					asm("sync");
					/* Clear Status Register */
					*addr16 = 0x5050;
					asm("sync");
					/* Single Block Erase Command */
					*addr16 = 0x2020;
					asm("sync");
					/* Confirm */
					*addr16 = 0xD0D0;
					asm("sync");
					if((info->flash_id & FLASH_TYPEMASK) != FLASH_LH28F016SCT) {
					    /* Resume Command, as per errata update */
					    *addr16 = 0xD0D0;
					    asm("sync");
					}
					/* re-enable interrupts if necessary */
					if (flag)
						enable_interrupts();
					/* wait at least 80us - let's wait 1 ms */
					*addr16 = 0x7070;
					udelay (1000);
					while ((*addr16 & ready) != ready) {
						if((*addr16 & erase_err_status)== erase_err_status){
							printf("Error in Block Erase - Lock Bit may be set!\n");
							printf("Status Register = 0x%X\n", (uint)*addr16);
							*addr16 = 0xFFFF;	/* reset bank */
							asm("sync");
							return 1;
						}
						if ((now=get_timer(start)) > CONFIG_SYS_FLASH_ERASE_TOUT) {
							printf ("Timeout\n");
							*addr16 = 0xFFFF;	/* reset bank */
							asm("sync");
							return 1;
						}
						/* show that we're waiting */
						if ((now - last) > 1000) {	/* every second */
							putc ('.');
							last = now;
						}
					}
					/* reset to read mode */
					*addr16 = 0xFFFF;
					asm("sync");
					break;
				case FLASH_CFI_32BIT:
					asm("sync");
					last = start = get_timer (0);
					/* Disable interrupts which might cause a timeout here */
					flag = disable_interrupts();
					/* Reset Array */
					*addr = 0xffffffff;
					asm("sync");
					/* Clear Status Register */
					*addr = 0x50505050;
					asm("sync");
					/* Single Block Erase Command */
					*addr = 0x20202020;
					asm("sync");
					/* Confirm */
					*addr = 0xD0D0D0D0;
					asm("sync");
					if((info->flash_id & FLASH_TYPEMASK) != FLASH_LH28F016SCT) {
					    /* Resume Command, as per errata update */
					    *addr = 0xD0D0D0D0;
					    asm("sync");
					}
					/* re-enable interrupts if necessary */
					if (flag)
						enable_interrupts();
					/* wait at least 80us - let's wait 1 ms */
					*addr = 0x70707070;
					udelay (1000);
					while ((*addr & ready) != ready) {
						if((*addr & erase_err_status)==erase_err_status){
							printf("Error in Block Erase - Lock Bit may be set!\n");
							printf("Status Register = 0x%X\n", (uint)*addr);
							*addr = 0xFFFFFFFF;	/* reset bank */
							asm("sync");
							return 1;
						}
						if ((now=get_timer(start)) > CONFIG_SYS_FLASH_ERASE_TOUT) {
							printf ("Timeout\n");
							*addr = 0xFFFFFFFF;	/* reset bank */
							asm("sync");
							return 1;
						}
						/* show that we're waiting */
						if ((now - last) > 1000) {	/* every second */
							putc ('.');
							last = now;
						}
					}
					/* reset to read mode */
					*addr = 0xFFFFFFFF;
					asm("sync");
					break;
			}	/* end switch */
		}		/* end if */
	}			/* end for */

	printf ("flash erase done\n");
	return 0;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */

#define FLASH_BLOCK_SIZE 32

int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	ulong cp, wp, data, count, temp;
/*	ulong temp[FLASH_BLOCK_SIZE/4];*/
	int i, l, rc;

	count = cnt;
	wp = (addr & ~3);	/* get lower word aligned address */

	/*
	 * handle unaligned start bytes
	 */
	if ((l = addr - wp) != 0) {
		data = 0;
		for (i=0, cp=wp; i<l; ++i, ++cp) {
			data = (data << 8) | (*(uchar *)cp);
		}
		for (; i<4 && cnt>0; ++i) {
			data = (data << 8) | *src++;
			--cnt;
			++cp;
		}
		for (; cnt==0 && i<4; ++i, ++cp) {
			data = (data << 8) | (*(uchar *)cp);
		}

		if ((rc = write_word(info, wp, data)) != 0) {
			return (rc);
		}
		wp += 4;
	}

	cp = wp;
	/* handle unaligned block bytes , flash block size = 16bytes */
	wp = (cp+FLASH_BLOCK_SIZE-1) & ~(FLASH_BLOCK_SIZE-1);
	if ((wp-cp)>=cnt) {
		if ((rc = write_block(info,src,cp,wp-cp)) !=0)
			return (rc);
		src += wp-cp;
		cnt -= wp-cp;
	}
	/* handle aligned block bytes */
	temp = 0;
	printf("\n");
	while ( cnt >= FLASH_BLOCK_SIZE) {
		if ((rc = write_block(info,src,cp,FLASH_BLOCK_SIZE)) !=0) {
			return (rc);
		}
		src += FLASH_BLOCK_SIZE;
		cp += FLASH_BLOCK_SIZE;
		cnt -= FLASH_BLOCK_SIZE;
		if (((count-cnt)>>10)>temp) {
			temp=(count-cnt)>>10;
			printf("\r%lu KB",temp);
		}
	}
	printf("\n");
	wp = cp;
	/*
	 * handle word aligned part
	 */
	while (cnt >= 4) {
		data = 0;
		for (i=0; i<4; ++i) {
			data = (data << 8) | *src++;
		}
		if ((rc = write_word(info, wp, data)) != 0) {
			return (rc);
		}
		wp  += 4;
		cnt -= 4;
	}

	if (cnt == 0) {
		return (0);
	}

	/*
	 * handle unaligned tail bytes
	 */
	data = 0;
	for (i=0, cp=wp; i<4 && cnt>0; ++i, ++cp) {
		data = (data << 8) | *src++;
		--cnt;
	}
	for (; i<4; ++i, ++cp) {
		data = (data << 8) | (*(uchar *)cp);
	}

	return (write_word(info, wp, data));
}
#undef FLASH_BLOCK_SIZE

/*-----------------------------------------------------------------------
 * Write block to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 * -1  Error
 */
static int write_block(flash_info_t *info, uchar * src, ulong dest, ulong cnt)
{
	vu_short *baddr, *addr = (vu_short *)dest;
	ushort data;
	ulong start, now, xsr,csr, ready;
	int flag;

	if (cnt==0) return 0;
	else if(cnt != (cnt& ~1)) return -1;

	/* Check if Flash is (sufficiently) erased */
	data = * src;
	data = (data<<8) | *(src+1);
	if ((*addr & data) != data) {
		return (2);
	}
	if (big_endian == 1) {
		ready = 0x0080;
	}
	else {
		ready = 0x8000;
	}
	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

		do {
			/* Write Command */
			*addr = 0xe8e8;
			asm("sync");
			xsr = *addr;
			asm("sync");
		} while (!(xsr & ready));	/*wait until read */
		/*write count=BLOCK SIZE -1 */
		data=(cnt>>1)-1;
		data=(data<<8)|data;
		*addr = data;		/* word mode, cnt/2 */
		asm("sync");
		baddr = addr;
		while(cnt) {
			data = * src++;
			data = (data<<8) | *src++;
			asm("sync");
			*baddr = data;
			asm("sync");
			++baddr;
			cnt = cnt -2;
		}
		*addr = 0xd0d0;			/* confirm write */
		start = get_timer(0);
		asm("sync");
		if (flag)
			enable_interrupts();
		/* data polling for D7 */
		flag  = 0;
		while (((csr = *addr) & ready) != ready) {
			if ((now=get_timer(start)) > CONFIG_SYS_FLASH_WRITE_TOUT) {
				flag = 1;
				break;
			}
		}
		if (csr & 0x4040) {
			printf ("CSR indicates write error (%04lx) at %08lx\n",
				csr, (ulong)addr);
			flag = 1;
		}
		/* Clear Status Registers Command */
		*addr = 0x5050;
		asm("sync");
		/* Reset to read array mode */
		*addr = 0xFFFF;
		asm("sync");
	return (flag);
}


/*-----------------------------------------------------------------------
 * Write a short word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_short (flash_info_t *info, ulong dest, ushort data)
{
	vu_short *addr = (vu_short *)dest;
	ulong start, now, csr, ready;
	int flag;

	/* Check if Flash is (sufficiently) erased */
	if ((*addr & data) != data) {
		return (2);
	}
	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

		/* Write Command */
		*addr = 0x1010;
		start = get_timer (0);
		asm("sync");
		/* Write Data */
		*addr = data;
		asm("sync");
		/* re-enable interrupts if necessary */
		if (flag)
			enable_interrupts();
		if (big_endian == 1) {
			ready = 0x0080;
		}
		else {
			ready = 0x8000;
		}
		/* data polling for D7 */
		flag  = 0;
		while (((csr = *addr) & ready) != ready) {
			if ((now=get_timer(start)) > CONFIG_SYS_FLASH_WRITE_TOUT) {
				flag = 1;
				break;
			}
		}
		if (csr & 0x4040) {
			printf ("CSR indicates write error (%04lx) at %08lx\n",
				csr, (ulong)addr);
			flag = 1;
		}
		/* Clear Status Registers Command */
		*addr = 0x5050;
		asm("sync");
		/* Reset to read array mode */
		*addr = 0xFFFF;
		asm("sync");
	return (flag);
}

/*-----------------------------------------------------------------------
 * Write a word to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_word (flash_info_t *info, ulong dest, ulong data)
{
	vu_long *addr = (vu_long *)dest;
	ulong start, csr, ready;
	int flag=0;

	switch (info->portwidth) {
	case FLASH_CFI_32BIT:
		/* Check if Flash is (sufficiently) erased */
		if ((*addr & data) != data) {
			return (2);
		}
		/* Disable interrupts which might cause a timeout here */
		flag = disable_interrupts();

		if (big_endian == 1) {
			ready = 0x0080;
		}
		else {
			ready = 0x8000;
		}
		if ((info->portwidth / info->chipwidth)==2) {
			ready += (ready <<16);
		}
		else {
			ready = ready << 16;
		}
		/* Write Command */
		*addr = 0x10101010;
		asm("sync");
		/* Write Data */
		*addr = data;
		/* re-enable interrupts if necessary */
		if (flag)
			enable_interrupts();
		/* data polling for D7 */
		start = get_timer (0);
		flag  = 0;
		while (((csr = *addr) & ready) != ready) {
			if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
				flag = 1;
				break;
			}
		}
		if (csr & 0x40404040) {
			printf ("CSR indicates write error (%08lx) at %08lx\n", csr, (ulong)addr);
			flag = 1;
		}
		/* Clear Status Registers Command */
		*addr = 0x50505050;
		asm("sync");
		/* Reset to read array mode */
		*addr = 0xFFFFFFFF;
		asm("sync");
		break;
	case FLASH_CFI_16BIT:
		flag = write_short (info, dest,  (unsigned short) (data>>16));
		if (flag == 0)
			flag = write_short (info, dest+2,  (unsigned short) (data));
		break;
	}
	return (flag);
}

/*-----------------------------------------------------------------------
 * Clear Block Lock Bit, returns:
 * 0 - OK
 * 1 - Timeout
 */

static int clear_block_lock_bit(flash_info_t * info, vu_long  * addr)
{
	ulong start, now, ready;

	/* Reset Array */
	*addr = 0xffffffff;
	asm("sync");
	/* Clear Status Register */
	*addr = 0x50505050;
	asm("sync");

	*addr = 0x60606060;
	asm("sync");
	*addr = 0xd0d0d0d0;
	asm("sync");


	if (big_endian == 1) {
		ready = 0x0080;
	}
	else {
		ready = 0x8000;
	}
	if ((info->portwidth / info->chipwidth)==2) {
		ready += (ready <<16);
	}
	else {
		ready = ready << 16;
	}
#ifdef DEBUG
	printf ("%s: Ready flag is 0x%8lx\n", __FUNCTION__, ready);
#endif
	*addr = 0x70707070;	/* read status */
	start = get_timer (0);
	while((*addr & ready) != ready){
		if ((now=get_timer(start)) > CONFIG_SYS_FLASH_ERASE_TOUT) {
			printf ("Timeout on clearing Block Lock Bit\n");
			*addr = 0xFFFFFFFF;	/* reset bank */
			asm("sync");
			return 1;
		}
	}
	return 0;
}

#endif /* !CONFIG_SYS_NO_FLASH */
