/*
 * (C) Copyright 2002
 * Brad Kemp, Seranoa Networks, Brad.Kemp@seranoa.com
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/processor.h>

#undef  DEBUG_FLASH
/*
 * This file implements a Common Flash Interface (CFI) driver for ppcboot.
 * The width of the port and the width of the chips are determined at initialization.
 * These widths are used to calculate the address for access CFI data structures.
 * It has been tested on an Intel Strataflash implementation.
 *
 * References
 * JEDEC Standard JESD68 - Common Flash Interface (CFI)
 * JEDEC Standard JEP137-A Common Flash Interface (CFI) ID Codes
 * Intel Application Note 646 Common Flash Interface (CFI) and Command Sets
 * Intel 290667-008 3 Volt Intel StrataFlash Memory datasheet
 *
 * TODO
 * Use Primary Extended Query table (PRI) and Alternate Algorithm Query Table (ALT) to determine if protection is available
 * Add support for other command sets Use the PRI and ALT to determine command set
 * Verify erase and program timeouts.
 */

#define FLASH_CMD_CFI			0x98
#define FLASH_CMD_READ_ID		0x90
#define FLASH_CMD_RESET			0xff
#define FLASH_CMD_BLOCK_ERASE		0x20
#define FLASH_CMD_ERASE_CONFIRM		0xD0
#define FLASH_CMD_WRITE			0x40
#define FLASH_CMD_PROTECT		0x60
#define FLASH_CMD_PROTECT_SET		0x01
#define FLASH_CMD_PROTECT_CLEAR		0xD0
#define FLASH_CMD_CLEAR_STATUS		0x50
#define FLASH_CMD_WRITE_TO_BUFFER       0xE8
#define FLASH_CMD_WRITE_BUFFER_CONFIRM  0xD0

#define FLASH_STATUS_DONE		0x80
#define FLASH_STATUS_ESS		0x40
#define FLASH_STATUS_ECLBS		0x20
#define FLASH_STATUS_PSLBS		0x10
#define FLASH_STATUS_VPENS		0x08
#define FLASH_STATUS_PSS		0x04
#define FLASH_STATUS_DPS		0x02
#define FLASH_STATUS_R			0x01
#define FLASH_STATUS_PROTECT		0x01

#define FLASH_OFFSET_CFI		0x55
#define FLASH_OFFSET_CFI_RESP		0x10
#define FLASH_OFFSET_WTOUT		0x1F
#define FLASH_OFFSET_WBTOUT             0x20
#define FLASH_OFFSET_ETOUT		0x21
#define FLASH_OFFSET_CETOUT             0x22
#define FLASH_OFFSET_WMAX_TOUT		0x23
#define FLASH_OFFSET_WBMAX_TOUT         0x24
#define FLASH_OFFSET_EMAX_TOUT		0x25
#define FLASH_OFFSET_CEMAX_TOUT         0x26
#define FLASH_OFFSET_SIZE		0x27
#define FLASH_OFFSET_INTERFACE          0x28
#define FLASH_OFFSET_BUFFER_SIZE        0x2A
#define FLASH_OFFSET_NUM_ERASE_REGIONS	0x2C
#define FLASH_OFFSET_ERASE_REGIONS	0x2D
#define FLASH_OFFSET_PROTECT		0x02
#define FLASH_OFFSET_USER_PROTECTION    0x85
#define FLASH_OFFSET_INTEL_PROTECTION   0x81


#define FLASH_MAN_CFI			0x01000000


typedef union {
	unsigned char c;
	unsigned short w;
	unsigned long l;
} cfiword_t;

typedef union {
	unsigned char * cp;
	unsigned short *wp;
	unsigned long *lp;
} cfiptr_t;

#define NUM_ERASE_REGIONS 4

flash_info_t	flash_info[CFG_MAX_FLASH_BANKS]; /* info for FLASH chips	*/


/*-----------------------------------------------------------------------
 * Functions
 */


static void flash_add_byte(flash_info_t *info, cfiword_t * cword, uchar c);
static void flash_make_cmd(flash_info_t * info, uchar cmd, void * cmdbuf);
static void flash_write_cmd(flash_info_t * info, int sect, uchar offset, uchar cmd);
static int flash_isequal(flash_info_t * info, int sect, uchar offset, uchar cmd);
static int flash_isset(flash_info_t * info, int sect, uchar offset, uchar cmd);
static int flash_detect_cfi(flash_info_t * info);
static ulong flash_get_size (ulong base, int banknum);
static int flash_write_cfiword (flash_info_t *info, ulong dest, cfiword_t cword);
static int flash_full_status_check(flash_info_t * info, ulong sector, ulong tout, char * prompt);
#ifdef CFG_FLASH_USE_BUFFER_WRITE
static int flash_write_cfibuffer(flash_info_t * info, ulong dest, uchar * cp, int len);
#endif
/*-----------------------------------------------------------------------
 * create an address based on the offset and the port width
 */
inline uchar * flash_make_addr(flash_info_t * info, int sect, int offset)
{
	return ((uchar *)(info->start[sect] + (offset * info->portwidth)));
}
/*-----------------------------------------------------------------------
 * read a character at a port width address
 */
inline uchar flash_read_uchar(flash_info_t * info, uchar offset)
{
	uchar *cp;
	cp = flash_make_addr(info, 0, offset);
	return (cp[info->portwidth - 1]);
}

/*-----------------------------------------------------------------------
 * read a short word by swapping for ppc format.
 */
ushort flash_read_ushort(flash_info_t * info, int sect,  uchar offset)
{
    uchar * addr;

    addr = flash_make_addr(info, sect, offset);
    return ((addr[(2*info->portwidth) - 1] << 8) | addr[info->portwidth - 1]);

}

/*-----------------------------------------------------------------------
 * read a long word by picking the least significant byte of each maiximum
 * port size word. Swap for ppc format.
 */
ulong flash_read_long(flash_info_t * info, int sect,  uchar offset)
{
    uchar * addr;

    addr = flash_make_addr(info, sect, offset);
    return ( (addr[(2*info->portwidth) - 1] << 24 ) | (addr[(info->portwidth) -1] << 16) |
	    (addr[(4*info->portwidth) - 1] << 8) | addr[(3*info->portwidth) - 1]);

}

/*-----------------------------------------------------------------------
 */
unsigned long flash_init (void)
{
	unsigned long size;
	int i;
	unsigned long  address;


	/* The flash is positioned back to back, with the demultiplexing of the chip
	 * based on the A24 address line.
	 *
	 */

	address = CFG_FLASH_BASE;
	size = 0;

	/* Init: no FLASHes known */
	for (i=0; i<CFG_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
		size += flash_info[i].size = flash_get_size(address, i);
		address += CFG_FLASH_INCREMENT;
		if (flash_info[0].flash_id == FLASH_UNKNOWN) {
			printf ("## Unknown FLASH on Bank %d - Size = 0x%08lx = %ld MB\n",i,
				flash_info[0].size, flash_info[i].size<<20);
		}
	}

#if 0 /* test-only */
	/* Monitor protection ON by default */
#if (CFG_MONITOR_BASE >= CFG_FLASH_BASE)
	for(i=0; flash_info[0].start[i] < CFG_MONITOR_BASE+CFG_MONITOR_LEN-1; i++)
		(void)flash_real_protect(&flash_info[0], i, 1);
#endif
#else
	/* monitor protection ON by default */
	flash_protect (FLAG_PROTECT_SET,
		       - CFG_MONITOR_LEN,
		       - 1, &flash_info[1]);
#endif

	return (size);
}

/*-----------------------------------------------------------------------
 */
int flash_erase (flash_info_t *info, int s_first, int s_last)
{
	int rcode = 0;
	int prot;
	int sect;

	if( info->flash_id != FLASH_MAN_CFI) {
		printf ("Can't erase unknown flash type - aborted\n");
		return 1;
	}
	if ((s_first < 0) || (s_first > s_last)) {
		printf ("- no sectors to erase\n");
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


	for (sect = s_first; sect<=s_last; sect++) {
		if (info->protect[sect] == 0) { /* not protected */
			flash_write_cmd(info, sect, 0, FLASH_CMD_CLEAR_STATUS);
			flash_write_cmd(info, sect, 0, FLASH_CMD_BLOCK_ERASE);
			flash_write_cmd(info, sect, 0, FLASH_CMD_ERASE_CONFIRM);

			if(flash_full_status_check(info, sect, info->erase_blk_tout, "erase")) {
				rcode = 1;
			} else
				printf(".");
		}
	}
	printf (" done\n");
	return rcode;
}

/*-----------------------------------------------------------------------
 */
void flash_print_info  (flash_info_t *info)
{
	int i;

	if (info->flash_id != FLASH_MAN_CFI) {
		printf ("missing or unknown FLASH type\n");
		return;
	}

	printf("CFI conformant FLASH (%d x %d)",
	       (info->portwidth	 << 3 ), (info->chipwidth  << 3 ));
	printf ("  Size: %ld MB in %d Sectors\n",
		info->size >> 20, info->sector_count);
	printf(" Erase timeout %ld ms, write timeout %ld ms, buffer write timeout %ld ms, buffer size %d\n",
	       info->erase_blk_tout, info->write_tout, info->buffer_write_tout, info->buffer_size);

	printf ("  Sector Start Addresses:");
	for (i=0; i<info->sector_count; ++i) {
#ifdef CFG_FLASH_EMPTY_INFO
		int k;
		int size;
		int erased;
		volatile unsigned long *flash;

		/*
		 * Check if whole sector is erased
		 */
		if (i != (info->sector_count-1))
		  size = info->start[i+1] - info->start[i];
		else
		  size = info->start[0] + info->size - info->start[i];
		erased = 1;
		flash = (volatile unsigned long *)info->start[i];
		size = size >> 2;        /* divide by 4 for longword access */
		for (k=0; k<size; k++)
		  {
		    if (*flash++ != 0xffffffff)
		      {
			erased = 0;
			break;
		      }
		  }

		if ((i % 5) == 0)
			printf ("\n   ");
		/* print empty and read-only info */
		printf (" %08lX%s%s",
			info->start[i],
			erased ? " E" : "  ",
			info->protect[i] ? "RO " : "   ");
#else
		if ((i % 5) == 0)
			printf ("\n   ");
		printf (" %08lX%s",
			info->start[i],
			info->protect[i] ? " (RO)" : "     ");
#endif
	}
	printf ("\n");
	return;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
	ulong wp;
	ulong cp;
	int aln;
	cfiword_t cword;
	int i, rc;

	/* get lower aligned address */
	wp = (addr & ~(info->portwidth - 1));

	/* handle unaligned start */
	if((aln = addr - wp) != 0) {
		cword.l = 0;
		cp = wp;
		for(i=0;i<aln; ++i, ++cp)
			flash_add_byte(info, &cword, (*(uchar *)cp));

		for(; (i< info->portwidth) && (cnt > 0) ; i++) {
			flash_add_byte(info, &cword, *src++);
			cnt--;
			cp++;
		}
		for(; (cnt == 0) && (i < info->portwidth); ++i, ++cp)
			flash_add_byte(info, &cword, (*(uchar *)cp));
		if((rc = flash_write_cfiword(info, wp, cword)) != 0)
			return rc;
		wp = cp;
	}

#ifdef CFG_FLASH_USE_BUFFER_WRITE
	while(cnt >= info->portwidth) {
		i = info->buffer_size > cnt? cnt: info->buffer_size;
		if((rc = flash_write_cfibuffer(info, wp, src,i)) != ERR_OK)
			return rc;
		wp += i;
		src += i;
		cnt -=i;
	}
#else
	/* handle the aligned part */
	while(cnt >= info->portwidth) {
		cword.l = 0;
		for(i = 0; i < info->portwidth; i++) {
			flash_add_byte(info, &cword, *src++);
		}
		if((rc = flash_write_cfiword(info, wp, cword)) != 0)
			return rc;
		wp += info->portwidth;
		cnt -= info->portwidth;
	}
#endif /* CFG_FLASH_USE_BUFFER_WRITE */
	if (cnt == 0) {
		return (0);
	}

	/*
	 * handle unaligned tail bytes
	 */
	cword.l = 0;
	for (i=0, cp=wp; (i<info->portwidth) && (cnt>0); ++i, ++cp) {
		flash_add_byte(info, &cword, *src++);
		--cnt;
	}
	for (; i<info->portwidth; ++i, ++cp) {
		flash_add_byte(info, & cword, (*(uchar *)cp));
	}

	return flash_write_cfiword(info, wp, cword);
}

/*-----------------------------------------------------------------------
 */
int flash_real_protect(flash_info_t *info, long sector, int prot)
{
	int retcode = 0;

	flash_write_cmd(info, sector, 0, FLASH_CMD_CLEAR_STATUS);
	flash_write_cmd(info, sector, 0, FLASH_CMD_PROTECT);
	if(prot)
		flash_write_cmd(info, sector, 0, FLASH_CMD_PROTECT_SET);
	else
		flash_write_cmd(info, sector, 0, FLASH_CMD_PROTECT_CLEAR);

	if((retcode = flash_full_status_check(info, sector, info->erase_blk_tout,
					 prot?"protect":"unprotect")) == 0) {

		info->protect[sector] = prot;
		/* Intel's unprotect unprotects all locking */
		if(prot == 0) {
			int i;
			for(i = 0 ; i<info->sector_count; i++) {
				if(info->protect[i])
					flash_real_protect(info, i, 1);
			}
		}
	}

	return retcode;
}
/*-----------------------------------------------------------------------
 *  wait for XSR.7 to be set. Time out with an error if it does not.
 *  This routine does not set the flash to read-array mode.
 */
static int flash_status_check(flash_info_t * info, ulong sector, ulong tout, char * prompt)
{
	ulong start;

	/* Wait for command completion */
	start = get_timer (0);
	while(!flash_isset(info, sector, 0, FLASH_STATUS_DONE)) {
		if (get_timer(start) > info->erase_blk_tout) {
			printf("Flash %s timeout at address %lx\n", prompt, info->start[sector]);
			flash_write_cmd(info, sector, 0, FLASH_CMD_RESET);
			return ERR_TIMOUT;
		}
	}
	return ERR_OK;
}
/*-----------------------------------------------------------------------
 * Wait for XSR.7 to be set, if it times out print an error, otherwise do a full status check.
 * This routine sets the flash to read-array mode.
 */
static int flash_full_status_check(flash_info_t * info, ulong sector, ulong tout, char * prompt)
{
	int retcode;
	retcode = flash_status_check(info, sector, tout, prompt);
	if((retcode == ERR_OK) && !flash_isequal(info,sector, 0, FLASH_STATUS_DONE)) {
		retcode = ERR_INVAL;
		printf("Flash %s error at address %lx\n", prompt,info->start[sector]);
		if(flash_isset(info, sector, 0, FLASH_STATUS_ECLBS | FLASH_STATUS_PSLBS)){
			printf("Command Sequence Error.\n");
		} else if(flash_isset(info, sector, 0, FLASH_STATUS_ECLBS)){
			printf("Block Erase Error.\n");
			retcode = ERR_NOT_ERASED;
		} else if (flash_isset(info, sector, 0, FLASH_STATUS_PSLBS)) {
			printf("Locking Error\n");
		}
		if(flash_isset(info, sector, 0, FLASH_STATUS_DPS)){
			printf("Block locked.\n");
			retcode = ERR_PROTECTED;
		}
		if(flash_isset(info, sector, 0, FLASH_STATUS_VPENS))
			printf("Vpp Low Error.\n");
	}
	flash_write_cmd(info, sector, 0, FLASH_CMD_RESET);
	return retcode;
}
/*-----------------------------------------------------------------------
 */
static void flash_add_byte(flash_info_t *info, cfiword_t * cword, uchar c)
{
	switch(info->portwidth) {
	case FLASH_CFI_8BIT:
		cword->c = c;
		break;
	case FLASH_CFI_16BIT:
		cword->w = (cword->w << 8) | c;
		break;
	case FLASH_CFI_32BIT:
		cword->l = (cword->l << 8) | c;
	}
}


/*-----------------------------------------------------------------------
 * make a proper sized command based on the port and chip widths
 */
static void flash_make_cmd(flash_info_t * info, uchar cmd, void * cmdbuf)
{
	int i;
	uchar *cp = (uchar *)cmdbuf;
	for(i=0; i< info->portwidth; i++)
		*cp++ = ((i+1) % info->chipwidth) ? '\0':cmd;
}

/*
 * Write a proper sized command to the correct address
 */
static void flash_write_cmd(flash_info_t * info, int sect, uchar offset, uchar cmd)
{

	volatile cfiptr_t addr;
	cfiword_t cword;
	addr.cp = flash_make_addr(info, sect, offset);
	flash_make_cmd(info, cmd, &cword);
	switch(info->portwidth) {
	case FLASH_CFI_8BIT:
		*addr.cp = cword.c;
		break;
	case FLASH_CFI_16BIT:
		*addr.wp = cword.w;
		break;
	case FLASH_CFI_32BIT:
		*addr.lp = cword.l;
		break;
	}
}

/*-----------------------------------------------------------------------
 */
static int flash_isequal(flash_info_t * info, int sect, uchar offset, uchar cmd)
{
	cfiptr_t cptr;
	cfiword_t cword;
	int retval;
	cptr.cp = flash_make_addr(info, sect, offset);
	flash_make_cmd(info, cmd, &cword);
	switch(info->portwidth) {
	case FLASH_CFI_8BIT:
		retval = (cptr.cp[0] == cword.c);
		break;
	case FLASH_CFI_16BIT:
		retval = (cptr.wp[0] == cword.w);
		break;
	case FLASH_CFI_32BIT:
		retval = (cptr.lp[0] == cword.l);
		break;
	default:
		retval = 0;
		break;
	}
	return retval;
}
/*-----------------------------------------------------------------------
 */
static int flash_isset(flash_info_t * info, int sect, uchar offset, uchar cmd)
{
	cfiptr_t cptr;
	cfiword_t cword;
	int retval;
	cptr.cp = flash_make_addr(info, sect, offset);
	flash_make_cmd(info, cmd, &cword);
	switch(info->portwidth) {
	case FLASH_CFI_8BIT:
		retval = ((cptr.cp[0] & cword.c) == cword.c);
		break;
	case FLASH_CFI_16BIT:
		retval = ((cptr.wp[0] & cword.w) == cword.w);
		break;
	case FLASH_CFI_32BIT:
		retval = ((cptr.lp[0] & cword.l) == cword.l);
		break;
	default:
		retval = 0;
		break;
	}
	return retval;
}

/*-----------------------------------------------------------------------
 * detect if flash is compatible with the Common Flash Interface (CFI)
 * http://www.jedec.org/download/search/jesd68.pdf
 *
*/
static int flash_detect_cfi(flash_info_t * info)
{

	for(info->portwidth=FLASH_CFI_8BIT; info->portwidth <= FLASH_CFI_32BIT;
	    info->portwidth <<= 1) {
		for(info->chipwidth =FLASH_CFI_BY8;
		    info->chipwidth <= info->portwidth;
		    info->chipwidth <<= 1) {
			flash_write_cmd(info, 0, 0, FLASH_CMD_RESET);
			flash_write_cmd(info, 0, FLASH_OFFSET_CFI, FLASH_CMD_CFI);
			if(flash_isequal(info, 0, FLASH_OFFSET_CFI_RESP,'Q') &&
			   flash_isequal(info, 0, FLASH_OFFSET_CFI_RESP + 1, 'R') &&
			   flash_isequal(info, 0, FLASH_OFFSET_CFI_RESP + 2, 'Y'))
				return 1;
		}
	}
	return 0;
}
/*
 * The following code cannot be run from FLASH!
 *
 */
static ulong flash_get_size (ulong base, int banknum)
{
	flash_info_t * info = &flash_info[banknum];
	int i, j;
	int sect_cnt;
	unsigned long sector;
	unsigned long tmp;
	int size_ratio;
	uchar num_erase_regions;
	int  erase_region_size;
	int  erase_region_count;

	info->start[0] = base;

	if(flash_detect_cfi(info)){
#ifdef DEBUG_FLASH
		printf("portwidth=%d chipwidth=%d\n", info->portwidth, info->chipwidth); /* test-only */
#endif
		size_ratio = info->portwidth / info->chipwidth;
		num_erase_regions = flash_read_uchar(info, FLASH_OFFSET_NUM_ERASE_REGIONS);
#ifdef DEBUG_FLASH
		printf("found %d erase regions\n", num_erase_regions);
#endif
		sect_cnt = 0;
		sector = base;
		for(i = 0 ; i < num_erase_regions; i++) {
			if(i > NUM_ERASE_REGIONS) {
				printf("%d erase regions found, only %d used\n",
				       num_erase_regions, NUM_ERASE_REGIONS);
				break;
			}
			tmp = flash_read_long(info, 0, FLASH_OFFSET_ERASE_REGIONS);
			erase_region_size = (tmp & 0xffff)? ((tmp & 0xffff) * 256): 128;
			tmp >>= 16;
			erase_region_count = (tmp & 0xffff) +1;
			for(j = 0; j< erase_region_count; j++) {
				info->start[sect_cnt] = sector;
				sector += (erase_region_size * size_ratio);
				info->protect[sect_cnt] = flash_isset(info, sect_cnt, FLASH_OFFSET_PROTECT, FLASH_STATUS_PROTECT);
				sect_cnt++;
			}
		}

		info->sector_count = sect_cnt;
		/* multiply the size by the number of chips */
		info->size = (1 << flash_read_uchar(info, FLASH_OFFSET_SIZE)) * size_ratio;
		info->buffer_size = (1 << flash_read_ushort(info, 0, FLASH_OFFSET_BUFFER_SIZE));
		tmp = 1 << flash_read_uchar(info, FLASH_OFFSET_ETOUT);
		info->erase_blk_tout = (tmp * (1 << flash_read_uchar(info, FLASH_OFFSET_EMAX_TOUT)));
		tmp = 1 << flash_read_uchar(info, FLASH_OFFSET_WBTOUT);
		info->buffer_write_tout = (tmp * (1 << flash_read_uchar(info, FLASH_OFFSET_WBMAX_TOUT)));
		tmp = 1 << flash_read_uchar(info, FLASH_OFFSET_WTOUT);
		info->write_tout = (tmp * (1 << flash_read_uchar(info, FLASH_OFFSET_WMAX_TOUT)))/ 1000;
		info->flash_id = FLASH_MAN_CFI;
	}

	flash_write_cmd(info, 0, 0, FLASH_CMD_RESET);
	return(info->size);
}


/*-----------------------------------------------------------------------
 */
static int flash_write_cfiword (flash_info_t *info, ulong dest, cfiword_t cword)
{

	cfiptr_t ctladdr;
	cfiptr_t cptr;
	int flag;

	ctladdr.cp = flash_make_addr(info, 0, 0);
	cptr.cp = (uchar *)dest;


	/* Check if Flash is (sufficiently) erased */
	switch(info->portwidth) {
	case FLASH_CFI_8BIT:
		flag = ((cptr.cp[0] & cword.c) == cword.c);
		break;
	case FLASH_CFI_16BIT:
		flag = ((cptr.wp[0] & cword.w) == cword.w);
		break;
	case FLASH_CFI_32BIT:
		flag = ((cptr.lp[0] & cword.l)	== cword.l);
		break;
	default:
		return 2;
	}
	if(!flag)
		return 2;

	/* Disable interrupts which might cause a timeout here */
	flag = disable_interrupts();

	flash_write_cmd(info, 0, 0, FLASH_CMD_CLEAR_STATUS);
	flash_write_cmd(info, 0, 0, FLASH_CMD_WRITE);

	switch(info->portwidth) {
	case FLASH_CFI_8BIT:
		cptr.cp[0] = cword.c;
		break;
	case FLASH_CFI_16BIT:
		cptr.wp[0] = cword.w;
		break;
	case FLASH_CFI_32BIT:
		cptr.lp[0] = cword.l;
		break;
	}

	/* re-enable interrupts if necessary */
	if(flag)
		enable_interrupts();

	return flash_full_status_check(info, 0, info->write_tout, "write");
}

#ifdef CFG_FLASH_USE_BUFFER_WRITE

/* loop through the sectors from the highest address
 * when the passed address is greater or equal to the sector address
 * we have a match
 */
static int find_sector(flash_info_t *info, ulong addr)
{
	int sector;
	for(sector = info->sector_count - 1; sector >= 0; sector--) {
		if(addr >= info->start[sector])
			break;
	}
	return sector;
}

static int flash_write_cfibuffer(flash_info_t * info, ulong dest, uchar * cp, int len)
{

	int sector;
	int cnt;
	int retcode;
	volatile cfiptr_t src;
	volatile cfiptr_t dst;

	src.cp = cp;
	dst.cp = (uchar *)dest;
	sector = find_sector(info, dest);
	flash_write_cmd(info, sector, 0, FLASH_CMD_CLEAR_STATUS);
	flash_write_cmd(info, sector, 0, FLASH_CMD_WRITE_TO_BUFFER);
	if((retcode = flash_status_check(info, sector, info->buffer_write_tout,
					 "write to buffer")) == ERR_OK) {
		switch(info->portwidth) {
		case FLASH_CFI_8BIT:
			cnt = len;
			break;
		case FLASH_CFI_16BIT:
			cnt = len >> 1;
			break;
		case FLASH_CFI_32BIT:
			cnt = len >> 2;
			break;
		default:
			return ERR_INVAL;
			break;
		}
		flash_write_cmd(info, sector, 0, (uchar)cnt-1);
		while(cnt-- > 0) {
			switch(info->portwidth) {
			case FLASH_CFI_8BIT:
				*dst.cp++ = *src.cp++;
				break;
			case FLASH_CFI_16BIT:
				*dst.wp++ = *src.wp++;
				break;
			case FLASH_CFI_32BIT:
				*dst.lp++ = *src.lp++;
				break;
			default:
				return ERR_INVAL;
				break;
			}
		}
		flash_write_cmd(info, sector, 0, FLASH_CMD_WRITE_BUFFER_CONFIRM);
		retcode = flash_full_status_check(info, sector, info->buffer_write_tout,
					     "buffer write");
	}
	flash_write_cmd(info, sector, 0, FLASH_CMD_CLEAR_STATUS);
	return retcode;
}
#endif /* CFG_USE_FLASH_BUFFER_WRITE */
