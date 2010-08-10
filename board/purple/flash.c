/*
 * (C) Copyright 2003
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
#include <asm/inca-ip.h>

flash_info_t	flash_info[CONFIG_SYS_MAX_FLASH_BANKS]; /* info for FLASH chips	*/

typedef unsigned long FLASH_PORT_WIDTH;
typedef volatile unsigned long FLASH_PORT_WIDTHV;

#define	FLASH_ID_MASK	0xFFFFFFFF

#define FPW	FLASH_PORT_WIDTH
#define FPWV	FLASH_PORT_WIDTHV

#define ORMASK(size) ((-size) & OR_AM_MSK)

#define FLASH29_REG_ADRS(reg) ((FPWV *)PHYS_FLASH_1 + (reg))

/* FLASH29 command register addresses */

#define FLASH29_REG_FIRST_CYCLE		FLASH29_REG_ADRS (0x1555)
#define FLASH29_REG_SECOND_CYCLE	FLASH29_REG_ADRS (0x2aaa)
#define FLASH29_REG_THIRD_CYCLE		FLASH29_REG_ADRS (0x3555)
#define	FLASH29_REG_FOURTH_CYCLE	FLASH29_REG_ADRS (0x4555)
#define	FLASH29_REG_FIFTH_CYCLE		FLASH29_REG_ADRS (0x5aaa)
#define	FLASH29_REG_SIXTH_CYCLE		FLASH29_REG_ADRS (0x6555)

/* FLASH29 command definitions */

#define	FLASH29_CMD_FIRST		0xaaaaaaaa
#define	FLASH29_CMD_SECOND		0x55555555
#define	FLASH29_CMD_FOURTH		0xaaaaaaaa
#define	FLASH29_CMD_FIFTH		0x55555555
#define	FLASH29_CMD_SIXTH		0x10101010

#define	FLASH29_CMD_SECTOR		0x30303030
#define	FLASH29_CMD_PROGRAM		0xa0a0a0a0
#define	FLASH29_CMD_CHIP_ERASE		0x80808080
#define	FLASH29_CMD_READ_RESET		0xf0f0f0f0
#define	FLASH29_CMD_AUTOSELECT		0x90909090
#define FLASH29_CMD_READ		0x70707070

#define IN_RAM_CMD_READ		0x1
#define IN_RAM_CMD_WRITE	0x2

#define FLASH_WRITE_CMD	((ulong)(flash_write_cmd) & 0x7)+0xbf008000
#define FLASH_READ_CMD  ((ulong)(flash_read_cmd) & 0x7)+0xbf008000

typedef void (*FUNCPTR_CP)(ulong *source, ulong *destination, ulong nlongs);
typedef void (*FUNCPTR_RD)(int cmd, FPWV * pFA, char * string, int strLen);
typedef void (*FUNCPTR_WR)(int cmd, FPWV * pFA, FPW value);

static ulong flash_get_size(FPWV *addr, flash_info_t *info);
static int write_word(flash_info_t *info, FPWV *dest, FPW data);
static void flash_get_offsets(ulong base, flash_info_t *info);
static flash_info_t *flash_get_info(ulong base);

static void load_cmd(ulong cmd);
static ulong in_ram_cmd = 0;


/******************************************************************************
*
* Don't change the program architecture
* This architecture assure the program
* can be relocated to scratch ram
*/
static void flash_read_cmd(int cmd, FPWV * pFA, char * string, int strLen)
{
	int i,j;
	FPW temp,temp1;
	FPWV *str;

	str = (FPWV *)string;

	j=  strLen/4;

	if(cmd == FLASH29_CMD_AUTOSELECT)
	   {
	    *(FLASH29_REG_FIRST_CYCLE)  = FLASH29_CMD_FIRST;
	    *(FLASH29_REG_SECOND_CYCLE) = FLASH29_CMD_SECOND;
	    *(FLASH29_REG_THIRD_CYCLE)  = FLASH29_CMD_AUTOSELECT;
	   }

	if(cmd == FLASH29_CMD_READ)
	   {
	    i = 0;
	    while(i<j)
	    {
		temp = *pFA++;
		temp1 = *(int *)0xa0000000;
		*(int *)0xbf0081f8 = temp1 + temp;
		*str++ = temp;
		i++;
	    }
	   }

	 if(cmd == FLASH29_CMD_READ_RESET)
	 {
	    *(FLASH29_REG_FIRST_CYCLE)  = FLASH29_CMD_FIRST;
	    *(FLASH29_REG_SECOND_CYCLE) = FLASH29_CMD_SECOND;
	    *(FLASH29_REG_THIRD_CYCLE)  = FLASH29_CMD_READ_RESET;
	 }

	*(int *)0xbf0081f8 = *(int *)0xa0000000;	/* dummy read switch back to sdram interface */
}

/******************************************************************************
*
* Don't change the program architecture
* This architecture assure the program
* can be relocated to scratch ram
*/
static void flash_write_cmd(int cmd, FPWV * pFA, FPW value)
{
	*(FLASH29_REG_FIRST_CYCLE)  = FLASH29_CMD_FIRST;
	*(FLASH29_REG_SECOND_CYCLE) = FLASH29_CMD_SECOND;

	if (cmd == FLASH29_CMD_SECTOR)
	   {
	    *(FLASH29_REG_THIRD_CYCLE)  = FLASH29_CMD_CHIP_ERASE;
	    *(FLASH29_REG_FOURTH_CYCLE) = FLASH29_CMD_FOURTH;
	    *(FLASH29_REG_FIFTH_CYCLE)  = FLASH29_CMD_FIFTH;
	    *pFA		        = FLASH29_CMD_SECTOR;
	   }

	if (cmd == FLASH29_CMD_SIXTH)
	   {
	    *(FLASH29_REG_THIRD_CYCLE)  = FLASH29_CMD_CHIP_ERASE;
	    *(FLASH29_REG_FOURTH_CYCLE) = FLASH29_CMD_FOURTH;
	    *(FLASH29_REG_FIFTH_CYCLE)  = FLASH29_CMD_FIFTH;
	    *(FLASH29_REG_SIXTH_CYCLE)  = FLASH29_CMD_SIXTH;
	   }

	if (cmd == FLASH29_CMD_PROGRAM)
	   {
	    *(FLASH29_REG_THIRD_CYCLE)  = FLASH29_CMD_PROGRAM;
	    *pFA = value;
	   }

	if (cmd == FLASH29_CMD_READ_RESET)
	   {
	    *(FLASH29_REG_THIRD_CYCLE)  = FLASH29_CMD_READ_RESET;
	   }

	*(int *)0xbf0081f8 = *(int *)0xa0000000;	/* dummy read switch back to sdram interface */
}

static void load_cmd(ulong cmd)
{
	ulong *src;
	ulong *dst;
	FUNCPTR_CP absEntry;
	ulong func;

	if (in_ram_cmd & cmd) return;

	if (cmd == IN_RAM_CMD_READ)
	{
		func = (ulong)flash_read_cmd;
	}
	else
	{
		func = (ulong)flash_write_cmd;
	}

	src = (ulong *)(func & 0xfffffff8);
	dst = (ulong *)0xbf008000;
	absEntry = (FUNCPTR_CP)(0xbf0081d0);
	absEntry(src,dst,0x38);

	in_ram_cmd = cmd;
}

/*-----------------------------------------------------------------------
 * flash_init()
 *
 * sets up flash_info and returns size of FLASH (bytes)
 */
unsigned long flash_init (void)
{
	unsigned long size = 0;
	int i;

	load_cmd(IN_RAM_CMD_READ);

	/* Init: no FLASHes known */
	for (i=0; i < CONFIG_SYS_MAX_FLASH_BANKS; ++i) {
		ulong flashbase = PHYS_FLASH_1;
		ulong * buscon = (ulong *) INCA_IP_EBU_EBU_BUSCON0;

		/* Disable write protection */
		*buscon &= ~INCA_IP_EBU_EBU_BUSCON1_WRDIS;

#if 1
		memset(&flash_info[i], 0, sizeof(flash_info_t));
#endif

		flash_info[i].size =
			flash_get_size((FPW *)flashbase, &flash_info[i]);

		if (flash_info[i].flash_id == FLASH_UNKNOWN) {
			printf ("## Unknown FLASH on Bank %d - Size = 0x%08lx\n",
			i, flash_info[i].size);
		}

		size += flash_info[i].size;
	}

#if CONFIG_SYS_MONITOR_BASE >= CONFIG_SYS_FLASH_BASE
	/* monitor protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_SYS_MONITOR_BASE,
		      CONFIG_SYS_MONITOR_BASE+monitor_flash_len-1,
		      flash_get_info(CONFIG_SYS_MONITOR_BASE));
#endif

#ifdef	CONFIG_ENV_IS_IN_FLASH
	/* ENV protection ON by default */
	flash_protect(FLAG_PROTECT_SET,
		      CONFIG_ENV_ADDR,
		      CONFIG_ENV_ADDR+CONFIG_ENV_SIZE-1,
		      flash_get_info(CONFIG_ENV_ADDR));
#endif

	return size;
}

/*-----------------------------------------------------------------------
 */
static void flash_get_offsets (ulong base, flash_info_t *info)
{
	int i;

	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_AMD
		 && (info->flash_id & FLASH_TYPEMASK) == FLASH_AM160B) {

		int bootsect_size[4];	/* number of bytes/boot sector  */
		int sect_size;		/* number of bytes/regular sector */

		bootsect_size[0] = 0x00008000;
		bootsect_size[1] = 0x00004000;
		bootsect_size[2] = 0x00004000;
		bootsect_size[3] = 0x00010000;
		sect_size =        0x00020000;

		/* set sector offsets for bottom boot block type	*/
		for (i = 0; i < info->sector_count; i++) {
			info->start[i] = base;
			base += i < 4 ? bootsect_size[i] : sect_size;
		}
	}
}

/*-----------------------------------------------------------------------
 */

static flash_info_t *flash_get_info(ulong base)
{
	int i;
	flash_info_t * info;

	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i ++) {
		info = & flash_info[i];
		if (info->start[0] <= base && base < info->start[0] + info->size)
			break;
	}

	return i == CONFIG_SYS_MAX_FLASH_BANKS ? 0 : info;
}

/*-----------------------------------------------------------------------
 */

void flash_print_info (flash_info_t *info)
{
	int i;
	uchar *boottype;
	uchar *bootletter;
	char *fmt;
	uchar botbootletter[] = "B";
	uchar topbootletter[] = "T";
	uchar botboottype[] = "bottom boot sector";
	uchar topboottype[] = "top boot sector";

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_AMD:	printf ("AMD ");		break;
	case FLASH_MAN_BM:	printf ("BRIGHT MICRO ");	break;
	case FLASH_MAN_FUJ:	printf ("FUJITSU ");		break;
	case FLASH_MAN_SST:	printf ("SST ");		break;
	case FLASH_MAN_STM:	printf ("STM ");		break;
	case FLASH_MAN_INTEL:	printf ("INTEL ");		break;
	default:		printf ("Unknown Vendor ");	break;
	}

	/* check for top or bottom boot, if it applies */
	if (info->flash_id & FLASH_BTYPE) {
		boottype = botboottype;
		bootletter = botbootletter;
	}
	else {
		boottype = topboottype;
		bootletter = topbootletter;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AM160B:
		fmt = "29LV160B%s (16 Mbit, %s)\n";
		break;
	case FLASH_28F800C3B:
	case FLASH_28F800C3T:
		fmt = "28F800C3%s (8 Mbit, %s)\n";
		break;
	case FLASH_INTEL800B:
	case FLASH_INTEL800T:
		fmt = "28F800B3%s (8 Mbit, %s)\n";
		break;
	case FLASH_28F160C3B:
	case FLASH_28F160C3T:
		fmt = "28F160C3%s (16 Mbit, %s)\n";
		break;
	case FLASH_INTEL160B:
	case FLASH_INTEL160T:
		fmt = "28F160B3%s (16 Mbit, %s)\n";
		break;
	case FLASH_28F320C3B:
	case FLASH_28F320C3T:
		fmt = "28F320C3%s (32 Mbit, %s)\n";
		break;
	case FLASH_INTEL320B:
	case FLASH_INTEL320T:
		fmt = "28F320B3%s (32 Mbit, %s)\n";
		break;
	case FLASH_28F640C3B:
	case FLASH_28F640C3T:
		fmt = "28F640C3%s (64 Mbit, %s)\n";
		break;
	case FLASH_INTEL640B:
	case FLASH_INTEL640T:
		fmt = "28F640B3%s (64 Mbit, %s)\n";
		break;
	default:
		fmt = "Unknown Chip Type\n";
		break;
	}

	printf (fmt, bootletter, boottype);

	printf ("  Size: %ld MB in %d Sectors\n",
		info->size >> 20,
		info->sector_count);

	printf ("  Sector Start Addresses:");

	for (i=0; i<info->sector_count; ++i) {
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

/*
 * The following code cannot be run from FLASH!
 */

ulong flash_get_size (FPWV *addr, flash_info_t *info)
{
	FUNCPTR_RD absEntry;
	FPW retValue;
	int flag;

	load_cmd(IN_RAM_CMD_READ);
	absEntry = (FUNCPTR_RD)FLASH_READ_CMD;

	flag = disable_interrupts();
	absEntry(FLASH29_CMD_AUTOSELECT,0,0,0);
	if (flag) enable_interrupts();

	udelay(100);

	flag = disable_interrupts();
	absEntry(FLASH29_CMD_READ, addr + 1, (char *)&retValue, sizeof(retValue));
	absEntry(FLASH29_CMD_READ_RESET,0,0,0);
	if (flag) enable_interrupts();

	udelay(100);

	switch (retValue) {

	case (FPW)AMD_ID_LV160B:
		info->flash_id += FLASH_AM160B;
		info->sector_count = 35;
		info->size = 0x00400000;
		break;				/* => 8 or 16 MB	*/

	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		return (0);			/* => no or unknown flash */
	}

	flash_get_offsets((ulong)addr, info);

	return (info->size);
}

/*-----------------------------------------------------------------------
 */

int	flash_erase (flash_info_t *info, int s_first, int s_last)
{
	FPWV *addr;
	int flag, prot, sect;
	ulong start, now, last;
	FUNCPTR_WR absEntry;

	load_cmd(IN_RAM_CMD_WRITE);
	absEntry = (FUNCPTR_WR)FLASH_WRITE_CMD;

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf ("- missing\n");
		} else {
			printf ("- no sectors to erase\n");
		}
		return 1;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_AM160B:
		break;
	case FLASH_UNKNOWN:
	default:
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

	last  = get_timer(0);

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect<=s_last; sect++) {

		if (info->protect[sect] != 0)	/* protected, skip it */
			continue;

		/* Disable interrupts which might cause a timeout here */
		flag = disable_interrupts();

		addr = (FPWV *)(info->start[sect]);
		absEntry(FLASH29_CMD_SECTOR, addr, 0);

		/* re-enable interrupts if necessary */
		if (flag)
			enable_interrupts();

		start = get_timer(0);

		while ((now = get_timer(start)) <= CONFIG_SYS_FLASH_ERASE_TOUT) {

			/* show that we're waiting */
			if ((get_timer(last)) > CONFIG_SYS_HZ) {/* every second */
				putc ('.');
				last = get_timer(0);
			}
		}

		flag = disable_interrupts();
		absEntry(FLASH29_CMD_READ_RESET,0,0);
		if (flag)
			enable_interrupts();
	}

	printf (" done\n");
	return 0;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
int write_buff (flash_info_t *info, uchar *src, ulong addr, ulong cnt)
{
    FPW data = 0; /* 16 or 32 bit word, matches flash bus width on MPC8XX */
    int bytes;	  /* number of bytes to program in current word		*/
    int left;	  /* number of bytes left to program			*/
    int i, res;

    for (left = cnt, res = 0;
	 left > 0 && res == 0;
	 addr += sizeof(data), left -= sizeof(data) - bytes) {

	bytes = addr & (sizeof(data) - 1);
	addr &= ~(sizeof(data) - 1);

	/* combine source and destination data so can program
	 * an entire word of 16 or 32 bits
	 */
	for (i = 0; i < sizeof(data); i++) {
	    data <<= 8;
	    if (i < bytes || i - bytes >= left )
		data += *((uchar *)addr + i);
	    else
		data += *src++;
	}

	res = write_word(info, (FPWV *)addr, data);
    }

    return (res);
}

static int write_word (flash_info_t *info, FPWV *dest, FPW data)
{
    int res = 0;	/* result, assume success	*/
    FUNCPTR_WR absEntry;
    int flag;

    /* Check if Flash is (sufficiently) erased */
    if ((*dest & data) != data) {
	return (2);
    }

    if (info->start[0] != PHYS_FLASH_1)
    {
	return (3);
    }

    load_cmd(IN_RAM_CMD_WRITE);
    absEntry = (FUNCPTR_WR)FLASH_WRITE_CMD;

    flag = disable_interrupts();
    absEntry(FLASH29_CMD_PROGRAM,dest,data);
    if (flag) enable_interrupts();

    udelay(100);

    flag = disable_interrupts();
    absEntry(FLASH29_CMD_READ_RESET,0,0);
    if (flag) enable_interrupts();

    return (res);
}
