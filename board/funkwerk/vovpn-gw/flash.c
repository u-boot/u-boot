/*
 * (C) Copyright 2004
 * Elmeg Communications Systems GmbH, Juergen Selent (j.selent@elmeg.de)
 *
 * Support for the Elmeg VoVPN Gateway Module
 * ------------------------------------------
 * This is a signle bank flashdriver for INTEL 28F320J3, 28F640J3
 * and 28F128J3A flashs working in 8 Bit mode.
 *
 * Most of this code is taken from existing u-boot source code.
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

flash_info_t				flash_info[CFG_MAX_FLASH_BANKS];

#define FLASH_CMD_READ_ID		0x90
#define FLASH_CMD_READ_STATUS		0x70
#define FLASH_CMD_RESET			0xff
#define FLASH_CMD_BLOCK_ERASE		0x20
#define FLASH_CMD_ERASE_CONFIRM		0xd0
#define FLASH_CMD_CLEAR_STATUS		0x50
#define FLASH_CMD_SUSPEND_ERASE		0xb0
#define FLASH_CMD_WRITE			0x40
#define FLASH_CMD_WRITE_BUFF		0xe8
#define FLASH_CMD_PROG_RESUME		0xd0
#define FLASH_CMD_PROTECT		0x60
#define FLASH_CMD_PROTECT_SET		0x01
#define FLASH_CMD_PROTECT_CLEAR		0xd0
#define FLASH_STATUS_DONE		0x80

#define FLASH_WRITE_BUFFER_SIZE		32

#ifdef CFG_FLASH_16BIT
#define FLASH_WORD_SIZE			unsigned short
#define FLASH_ID_MASK			0xffff
#define FLASH_CMD_ADDR_SHIFT		0
#else
#define FLASH_WORD_SIZE			unsigned char
#define FLASH_ID_MASK			0xff
/* A0 is not used in either 8x or 16x for READ ID */
#define FLASH_CMD_ADDR_SHIFT		1
#endif


static unsigned long
flash_get(volatile FLASH_WORD_SIZE *addr, flash_info_t *info)
{
	volatile FLASH_WORD_SIZE *p;
	FLASH_WORD_SIZE value;
	int i;

	addr[0] = FLASH_CMD_READ_ID;

	/* manufactor */
	value = addr[0 << FLASH_CMD_ADDR_SHIFT];
	switch (value) {
	case (INTEL_MANUFACT & FLASH_ID_MASK):
		info->flash_id = FLASH_MAN_INTEL;
		break;
	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		*addr = FLASH_CMD_RESET;
		return (0);

	}

	/* device */
	value = addr[1 << FLASH_CMD_ADDR_SHIFT];
	switch (value) {
	case (INTEL_ID_28F320J3A & FLASH_ID_MASK):
		info->flash_id += FLASH_28F320J3A;
		info->sector_count = 32;
		info->size = 0x00400000;
		break;
	case (INTEL_ID_28F640J3A & FLASH_ID_MASK):
		info->flash_id += FLASH_28F640J3A;
		info->sector_count = 64;
		info->size = 0x00800000;
		break;
	case (INTEL_ID_28F128J3A & FLASH_ID_MASK):
		info->flash_id += FLASH_28F128J3A;
		info->sector_count = 128;
		info->size = 0x01000000;
		break;
	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		*addr = FLASH_CMD_RESET;
		return (0);
	}

	/* setup sectors */
	for (i = 0; i < info->sector_count; i++) {
		info->start[i] = (unsigned long)addr + (i * info->size/info->sector_count);
	}

	/* check protected sectors */
	for (i = 0; i < info->sector_count; i++) {
		p = (volatile FLASH_WORD_SIZE *)(info->start[i]);
		info->protect[i] = p[2 << FLASH_CMD_ADDR_SHIFT] & 1;
	}

	/* reset bank */
	*addr = FLASH_CMD_RESET;
	return (info->size);
}

unsigned long
flash_init(void)
{
	unsigned long	size;
	int		i;

	for (i=0; i<CFG_MAX_FLASH_BANKS; ++i) {
		flash_info[i].flash_id = FLASH_UNKNOWN;
	}
	size = flash_get((volatile FLASH_WORD_SIZE *)CFG_FLASH_BASE, &flash_info[0]);
	if (flash_info[0].flash_id == FLASH_UNKNOWN) {
		printf ("## Unknown FLASH Size=0x%08lx\n", size);
		return (0);
	}

	/* always protect 1 sector containing the HRCW */
	flash_protect(FLAG_PROTECT_SET,
		      flash_info[0].start[0],
		      flash_info[0].start[1] - 1,
		      &flash_info[0]);

#if CFG_MONITOR_BASE >= CFG_FLASH_BASE
	flash_protect(FLAG_PROTECT_SET,
		      CFG_MONITOR_FLASH,
		      CFG_MONITOR_FLASH+CFG_MONITOR_LEN-1,
		      &flash_info[0]);
#endif
#ifdef	CFG_ENV_IS_IN_FLASH
	flash_protect(FLAG_PROTECT_SET,
		      CFG_ENV_ADDR,
		      CFG_ENV_ADDR+CFG_ENV_SECT_SIZE-1,
		      &flash_info[0]);
#endif
	return (size);
}

void
flash_print_info(flash_info_t *info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("missing or unknown FLASH type\n");
		return;
	}

	switch (info->flash_id & FLASH_VENDMASK) {
	case FLASH_MAN_INTEL:	printf ("INTEL ");		break;
	default:		printf ("Unknown Vendor ");	break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_28F320J3A:	printf ("28F320JA3 (32 Mbit)\n");
				break;
	case FLASH_28F640J3A:	printf ("28F640JA3 (64 Mbit)\n");
				break;
	case FLASH_28F128J3A:	printf ("28F128JA3 (128 Mbit)\n");
				break;
	default:		printf ("Unknown Chip Type");
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

int
flash_erase(flash_info_t *info, int s_first, int s_last)
{
	unsigned long start, now, last;
	int flag, prot, sect;
	volatile FLASH_WORD_SIZE *addr;
	FLASH_WORD_SIZE status;

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf ("- missing\n");
		} else {
			printf ("- no sectors to erase\n");
		}
		return (1);
	}

	if (info->flash_id == FLASH_UNKNOWN) {
		printf ("Cannot erase unknown flash - aborted\n");
		return (1);
	}

	prot = 0;
	for (sect=s_first; sect<=s_last; ++sect) {
		if (info->protect[sect]) {
			prot++;
		}
	}

	if (prot) {
		printf ("- Warning: %d protected sectors will not be erased!\n", prot);
	} else {
		printf ("\n");
	}

	start = get_timer (0);
	last  = start;

	for (sect = s_first; sect<=s_last; sect++) {
		if (info->protect[sect]) {
			continue;
		}

		addr = (volatile FLASH_WORD_SIZE *)(info->start[sect]);
		/* Disable interrupts which might cause a timeout here */
		flag = disable_interrupts();

#ifdef DEBUG
		printf("Erase sector %d at start addr 0x%08X", sect, (unsigned int)info->start[sect]);
#endif

		*addr = FLASH_CMD_CLEAR_STATUS;
		*addr = FLASH_CMD_BLOCK_ERASE;
		*addr = FLASH_CMD_ERASE_CONFIRM;

		/* re-enable interrupts if necessary */
		if (flag) {
			enable_interrupts();
		}

		/* wait at least 80us - let's wait 1 ms */
		udelay (1000);

		while (((status = *addr) & FLASH_STATUS_DONE) != FLASH_STATUS_DONE) {
			if ((now=get_timer(start)) > CFG_FLASH_ERASE_TOUT) {
				printf("Flash erase timeout at address %lx\n", info->start[sect]);
				*addr = FLASH_CMD_SUSPEND_ERASE;
				*addr = FLASH_CMD_RESET;
				return (1);
			}

			/* show that we're waiting */
			if ((now - last) > 1000) {	/* every second */
				putc ('.');
				last = now;
			}
		}
		*addr = FLASH_CMD_RESET;
	}
	printf (" done\n");
	return (0);
}

static int
write_buff2( volatile FLASH_WORD_SIZE *dst,
	     volatile FLASH_WORD_SIZE *src,
	     unsigned long cnt )
{
	unsigned long start;
	FLASH_WORD_SIZE status;
	int flag, i;

	start = get_timer (0);
	while (1) {
		/* Disable interrupts which might cause a timeout here */
		flag = disable_interrupts();
		dst[0] = FLASH_CMD_WRITE_BUFF;
		if ((status = *dst) & FLASH_STATUS_DONE) {
			break;
		}

		/* re-enable interrupts if necessary */
		if (flag) {
			enable_interrupts();
		}

		if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
			return (-1);
		}
	}
	dst[0] = (FLASH_WORD_SIZE)(cnt - 1);
	for (i=0; i<cnt; i++) {
		dst[i] = src[i];
	}
	dst[0] = FLASH_CMD_PROG_RESUME;

	if (flag) {
		enable_interrupts();
	}

	return( 0 );
}

static int
poll_status( volatile FLASH_WORD_SIZE *addr )
{
	unsigned long start;

	start = get_timer (0);
	/* wait for error or finish */
	while (1) {
		if (*addr == FLASH_STATUS_DONE) {
			if (*addr == FLASH_STATUS_DONE) {
				break;
			}
		}
		if (get_timer(start) > CFG_FLASH_WRITE_TOUT) {
			*addr = FLASH_CMD_RESET;
			return (-1);
		}
	}
	*addr = FLASH_CMD_RESET;
	return (0);
}

/*
 * write_buff return values:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 * 4 - Flash not identified
 */
int
write_buff(flash_info_t *info, uchar *src, ulong udst, ulong cnt)
{
	volatile FLASH_WORD_SIZE *addr, *dst;
	unsigned long bcnt;
	int flag, i;

	if (info->flash_id == FLASH_UNKNOWN) {
		return (4);
	}

	addr = (volatile FLASH_WORD_SIZE *)(info->start[0]);
	dst = (volatile FLASH_WORD_SIZE *) udst;

#ifdef CFG_FLASH_16BIT
#error NYI
#else
	while (cnt > 0) {
		/* Check if buffer write is possible */
		if (cnt > 1 && (((unsigned long)dst & (FLASH_WRITE_BUFFER_SIZE - 1)) == 0)) {
			bcnt = cnt > FLASH_WRITE_BUFFER_SIZE ? FLASH_WRITE_BUFFER_SIZE : cnt;
			/* Check if Flash is (sufficiently) erased */
			for (i=0; i<bcnt; i++) {
				if ((dst[i] & src[i]) != src[i]) {
					return (2);
				}
			}
			if (write_buff2( dst,src,bcnt ) != 0) {
				addr[0] = FLASH_CMD_READ_STATUS;
			}
			if (poll_status( dst ) != 0) {
				return (1);
			}
			cnt -= bcnt;
			dst += bcnt;
			src += bcnt;
			continue;
		}

		/* Check if Flash is (sufficiently) erased */
		if ((*dst & *src) != *src) {
			return (2);
		}

		/* Disable interrupts which might cause a timeout here */
		flag = disable_interrupts();
		addr[0] = FLASH_CMD_ERASE_CONFIRM;
		addr[0] = FLASH_CMD_WRITE;
		*dst++ = *src++;
		/* re-enable interrupts if necessary */
		if (flag) {
			enable_interrupts();
		}

		if (poll_status( dst ) != 0) {
			return (1);
		}
		cnt --;
	}
#endif
	return (0);
}

int
flash_real_protect(flash_info_t *info, long sector, int prot)
{
	volatile FLASH_WORD_SIZE *addr;
	unsigned long start;

	addr = (volatile FLASH_WORD_SIZE *)(info->start[sector]);
	*addr = FLASH_CMD_CLEAR_STATUS;
	*addr = FLASH_CMD_PROTECT;

	if(prot) {
		*addr = FLASH_CMD_PROTECT_SET;
	} else {
		*addr = FLASH_CMD_PROTECT_CLEAR;
	}

	/* wait for error or finish */
	start = get_timer (0);
	while(!(addr[0] & FLASH_STATUS_DONE)){
		if (get_timer(start) > CFG_FLASH_ERASE_TOUT) {
			printf("Flash protect timeout at address %lx\n",  info->start[sector]);
			addr[0] = FLASH_CMD_RESET;
			return (1);
		}
	}

	/* Set software protect flag */
	info->protect[sector] = prot;
	*addr = FLASH_CMD_RESET;
	return (0);
}
