/*
 * (C) Copyright 2005
 * Greg Ungerer, OpenGear Inc, greg.ungerer@opengear.com
 *
 * (C) Copyright 2001
 * Kyle Harris, Nexus Technologies, Inc. kharris@nexus-tech.net
 *
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/byteorder/swab.h>
#include <asm/sections.h>


flash_info_t flash_info[CONFIG_SYS_MAX_FLASH_BANKS];	/* info for FLASH chips */

#define mb() __asm__ __volatile__ ("" : : : "memory")

/*-----------------------------------------------------------------------
 * Functions
 */
static ulong flash_get_size (unsigned char * addr, flash_info_t * info);
static int write_data (flash_info_t * info, ulong dest, unsigned char data);
static void flash_get_offsets (ulong base, flash_info_t * info);
void inline spin_wheel (void);

/*-----------------------------------------------------------------------
 */

unsigned long flash_init (void)
{
	int i;
	ulong size = 0;

	for (i = 0; i < CONFIG_SYS_MAX_FLASH_BANKS; i++) {
		switch (i) {
		case 0:
			flash_get_size ((unsigned char *) PHYS_FLASH_1, &flash_info[i]);
			flash_get_offsets (PHYS_FLASH_1, &flash_info[i]);
			break;
		case 1:
			/* ignore for now */
			flash_info[i].flash_id = FLASH_UNKNOWN;
			break;
		default:
			panic ("configured too many flash banks!\n");
			break;
		}
		size += flash_info[i].size;
	}

	/* Protect monitor and environment sectors
	 */
	flash_protect (FLAG_PROTECT_SET,
		       CONFIG_SYS_FLASH_BASE,
		       CONFIG_SYS_FLASH_BASE + (__bss_end - __bss_start),
		       &flash_info[0]);

	return size;
}

/*-----------------------------------------------------------------------
 */
static void flash_get_offsets (ulong base, flash_info_t * info)
{
	int i;

	if (info->flash_id == FLASH_UNKNOWN)
		return;

	if ((info->flash_id & FLASH_VENDMASK) == FLASH_MAN_INTEL) {
		for (i = 0; i < info->sector_count; i++) {
			info->start[i] = base + (i * PHYS_FLASH_SECT_SIZE);
			info->protect[i] = 0;
		}
	}
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
	case FLASH_MAN_INTEL:
		printf ("INTEL ");
		break;
	default:
		printf ("Unknown Vendor ");
		break;
	}

	switch (info->flash_id & FLASH_TYPEMASK) {
	case FLASH_28F128J3A:
		printf ("28F128J3A\n");
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
			info->start[i], info->protect[i] ? " (RO)" : "     ");
	}
	printf ("\n");
	return;
}

/*
 * The following code cannot be run from FLASH!
 */
static ulong flash_get_size (unsigned char * addr, flash_info_t * info)
{
	volatile unsigned char value;

	/* Write auto select command: read Manufacturer ID */
	addr[0x5555] = 0xAA;
	addr[0x2AAA] = 0x55;
	addr[0x5555] = 0x90;

	mb ();
	value = addr[0];

	switch (value) {

	case (unsigned char)INTEL_MANUFACT:
		info->flash_id = FLASH_MAN_INTEL;
		break;

	default:
		info->flash_id = FLASH_UNKNOWN;
		info->sector_count = 0;
		info->size = 0;
		addr[0] = 0xFF;	/* restore read mode */
		return (0);	/* no or unknown flash  */
	}

	mb ();
	value = addr[2];	/* device ID            */

	switch (value) {

	case (unsigned char)INTEL_ID_28F640J3A:
		info->flash_id += FLASH_28F640J3A;
		info->sector_count = 64;
		info->size = 0x00800000;
		break;		/* => 8 MB     */

	case (unsigned char)INTEL_ID_28F128J3A:
		info->flash_id += FLASH_28F128J3A;
		info->sector_count = 128;
		info->size = 0x01000000;
		break;		/* => 16 MB     */

	default:
		info->flash_id = FLASH_UNKNOWN;
		break;
	}

	if (info->sector_count > CONFIG_SYS_MAX_FLASH_SECT) {
		printf ("** ERROR: sector count %d > max (%d) **\n",
			info->sector_count, CONFIG_SYS_MAX_FLASH_SECT);
		info->sector_count = CONFIG_SYS_MAX_FLASH_SECT;
	}

	addr[0] = 0xFF;	/* restore read mode */

	return (info->size);
}


/*-----------------------------------------------------------------------
 */

int flash_erase (flash_info_t * info, int s_first, int s_last)
{
	int prot, sect;
	ulong type;
	int rcode = 0;
	ulong start;

	if ((s_first < 0) || (s_first > s_last)) {
		if (info->flash_id == FLASH_UNKNOWN) {
			printf ("- missing\n");
		} else {
			printf ("- no sectors to erase\n");
		}
		return 1;
	}

	type = (info->flash_id & FLASH_VENDMASK);
	if ((type != FLASH_MAN_INTEL)) {
		printf ("Can't erase unknown flash type %08lx - aborted\n",
			info->flash_id);
		return 1;
	}

	prot = 0;
	for (sect = s_first; sect <= s_last; ++sect) {
		if (info->protect[sect]) {
			prot++;
		}
	}

	if (prot)
		printf ("- Warning: %d protected sectors will not be erased!\n", prot);
	else
		printf ("\n");

	/* Disable interrupts which might cause a timeout here */
	disable_interrupts();

	/* Start erase on unprotected sectors */
	for (sect = s_first; sect <= s_last; sect++) {
		if (info->protect[sect] == 0) {	/* not protected */
			volatile unsigned char *addr;
			unsigned char status;

			printf ("Erasing sector %2d ... ", sect);

			/* arm simple, non interrupt dependent timer */
			start = get_timer(0);

			addr = (volatile unsigned char *) (info->start[sect]);
			*addr = 0x50;	/* clear status register */
			*addr = 0x20;	/* erase setup */
			*addr = 0xD0;	/* erase confirm */

			while (((status = *addr) & 0x80) != 0x80) {
				if (get_timer(start) >
				    CONFIG_SYS_FLASH_ERASE_TOUT) {
					printf ("Timeout\n");
					*addr = 0xB0;	/* suspend erase */
					*addr = 0xFF;	/* reset to read mode */
					rcode = 1;
					break;
				}
			}

			*addr = 0x50;	/* clear status register cmd */
			*addr = 0xFF;	/* resest to read mode */

			printf (" done\n");
		}
	}
	return rcode;
}

/*-----------------------------------------------------------------------
 * Copy memory to flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 * 4 - Flash not identified
 */

int write_buff (flash_info_t * info, uchar * src, ulong addr, ulong cnt)
{
	ulong cp, wp;
	unsigned char data;
	int count, i, l, rc, port_width;

	if (info->flash_id == FLASH_UNKNOWN)
		return 4;

	wp = addr;
	port_width = 1;

	/*
	 * handle unaligned start bytes
	 */
	if ((l = addr - wp) != 0) {
		data = 0;
		for (i = 0, cp = wp; i < l; ++i, ++cp) {
			data = (data << 8) | (*(uchar *) cp);
		}
		for (; i < port_width && cnt > 0; ++i) {
			data = (data << 8) | *src++;
			--cnt;
			++cp;
		}
		for (; cnt == 0 && i < port_width; ++i, ++cp) {
			data = (data << 8) | (*(uchar *) cp);
		}

		if ((rc = write_data (info, wp, data)) != 0) {
			return (rc);
		}
		wp += port_width;
	}

	/*
	 * handle word aligned part
	 */
	count = 0;
	while (cnt >= port_width) {
		data = 0;
		for (i = 0; i < port_width; ++i) {
			data = (data << 8) | *src++;
		}
		if ((rc = write_data (info, wp, data)) != 0) {
			return (rc);
		}
		wp += port_width;
		cnt -= port_width;
		if (count++ > 0x800) {
			spin_wheel ();
			count = 0;
		}
	}

	if (cnt == 0) {
		return (0);
	}

	/*
	 * handle unaligned tail bytes
	 */
	data = 0;
	for (i = 0, cp = wp; i < port_width && cnt > 0; ++i, ++cp) {
		data = (data << 8) | *src++;
		--cnt;
	}
	for (; i < port_width; ++i, ++cp) {
		data = (data << 8) | (*(uchar *) cp);
	}

	return (write_data (info, wp, data));
}

/*-----------------------------------------------------------------------
 * Write a word or halfword to Flash, returns:
 * 0 - OK
 * 1 - write timeout
 * 2 - Flash not erased
 */
static int write_data (flash_info_t * info, ulong dest, unsigned char data)
{
	volatile unsigned char *addr = (volatile unsigned char *) dest;
	ulong status;
	ulong start;

	/* Check if Flash is (sufficiently) erased */
	if ((*addr & data) != data) {
		printf ("not erased at %08lx (%lx)\n", (ulong) addr,
			(ulong) * addr);
		return (2);
	}
	/* Disable interrupts which might cause a timeout here */
	disable_interrupts();

	*addr = 0x40;	/* write setup */
	*addr = data;

	/* arm simple, non interrupt dependent timer */
	start = get_timer(0);

	/* wait while polling the status register */
	while (((status = *addr) & 0x80) != 0x80) {
		if (get_timer(start) > CONFIG_SYS_FLASH_WRITE_TOUT) {
			*addr = 0xFF;	/* restore read mode */
			return (1);
		}
	}

	*addr = 0xFF;	/* restore read mode */

	return (0);
}

void inline spin_wheel (void)
{
	static int p = 0;
	static char w[] = "\\/-";

	printf ("\010%c", w[p]);
	(++p == 3) ? (p = 0) : 0;
}
