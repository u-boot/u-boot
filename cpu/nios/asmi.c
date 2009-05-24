/*
 * (C) Copyright 2003, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
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

#if defined(CONFIG_NIOS_ASMI)
#include <command.h>
#include <nios-io.h>

#if !defined(CONFIG_SYS_NIOS_ASMIBASE)
#error "*** CONFIG_SYS_NIOS_ASMIBASE not defined ***"
#endif

/*-----------------------------------------------------------------------*/
#define SHORT_HELP\
	"asmi    - read/write Cyclone ASMI configuration device.\n"

#define LONG_HELP\
	"\n"\
	"asmi erase start [end]\n"\
	"    - erase sector start or sectors start through end.\n"\
	"asmi info\n"\
	"    - display ASMI device information.\n"\
	"asmi protect on | off\n"\
	"    - turn device protection on or off.\n"\
	"asmi read addr offset count\n"\
	"    - read count bytes from offset to addr.\n"\
	"asmi write addr offset count\n"\
	"    - write count bytes to offset from addr.\n"\
	"asmi verify addr offset count\n"\
	"    - verify count bytes at offset from addr."


/*-----------------------------------------------------------------------*/
/* Operation codes for serial configuration devices
 */
#define ASMI_WRITE_ENA		0x06	/* Write enable */
#define ASMI_WRITE_DIS		0x04	/* Write disable */
#define ASMI_READ_STAT		0x05	/* Read status */
#define ASMI_READ_BYTES		0x03	/* Read bytes */
#define ASMI_READ_ID		0xab	/* Read silicon id */
#define ASMI_WRITE_STAT		0x01	/* Write status */
#define ASMI_WRITE_BYTES	0x02	/* Write bytes */
#define ASMI_ERASE_BULK		0xc7	/* Erase entire device */
#define ASMI_ERASE_SECT		0xd8	/* Erase sector */

/* Device status register bits
 */
#define ASMI_STATUS_WIP		(1<<0)	/* Write in progress */
#define ASMI_STATUS_WEL		(1<<1)	/* Write enable latch */

static nios_asmi_t *asmi = (nios_asmi_t *)CONFIG_SYS_NIOS_ASMIBASE;

/***********************************************************************
 * Device access
 ***********************************************************************/
static void asmi_cs (int assert)
{
	if (assert) {
		asmi->control |= NIOS_ASMI_SSO;
	} else {
		/* Let all bits shift out */
		while ((asmi->status & NIOS_ASMI_TMT) == 0)
			;
		asmi->control &= ~NIOS_ASMI_SSO;
	}
}

static void asmi_tx (unsigned char c)
{
	while ((asmi->status & NIOS_ASMI_TRDY) == 0)
		;
	asmi->txdata = c;
}

static int asmi_rx (void)
{
	while ((asmi->status & NIOS_ASMI_RRDY) == 0)
		;
	return (asmi->rxdata);
}

static unsigned char bitrev[] = {
	0x00, 0x08, 0x04, 0x0c, 0x02, 0x0a, 0x06, 0x0e,
	0x01, 0x09, 0x05, 0x0d, 0x03, 0x0b, 0x07, 0x0f
};

static unsigned char asmi_bitrev( unsigned char c )
{
	unsigned char val;

	val  = bitrev[c>>4];
	val |= bitrev[c & 0x0f]<<4;
	return (val);
}

static void asmi_rcv (unsigned char *dst, int len)
{
	while (len--) {
		asmi_tx (0);
		*dst++ = asmi_rx ();
	}
}

static void asmi_rrcv (unsigned char *dst, int len)
{
	while (len--) {
		asmi_tx (0);
		*dst++ = asmi_bitrev (asmi_rx ());
	}
}

static void asmi_snd (unsigned char *src, int len)
{
	while (len--) {
		asmi_tx (*src++);
		asmi_rx ();
	}
}

static void asmi_rsnd (unsigned char *src, int len)
{
	while (len--) {
		asmi_tx (asmi_bitrev (*src++));
		asmi_rx ();
	}
}

static void asmi_wr_enable (void)
{
	asmi_cs (1);
	asmi_tx (ASMI_WRITE_ENA);
	asmi_rx ();
	asmi_cs (0);
}

static unsigned char asmi_status_rd (void)
{
	unsigned char status;

	asmi_cs (1);
	asmi_tx (ASMI_READ_STAT);
	asmi_rx ();
	asmi_tx (0);
	status = asmi_rx ();
	asmi_cs (0);
	return (status);
}

static void asmi_status_wr (unsigned char status)
{
	asmi_wr_enable ();
	asmi_cs (1);
	asmi_tx (ASMI_WRITE_STAT);
	asmi_rx ();
	asmi_tx (status);
	asmi_rx ();
	asmi_cs (0);
	return;
}

/***********************************************************************
 * Device information
 ***********************************************************************/
typedef struct asmi_devinfo_t {
	const char	*name;		/* Device name */
	unsigned char	id;		/* Device silicon id */
	unsigned char	size;		/* Total size log2(bytes)*/
	unsigned char	num_sects;	/* Number of sectors */
	unsigned char	sz_sect;	/* Sector size log2(bytes) */
	unsigned char	sz_page;	/* Page size log2(bytes) */
	unsigned char   prot_mask;	/* Protection mask */
}asmi_devinfo_t;

static struct asmi_devinfo_t devinfo[] = {
	{ "EPCS1 ", 0x10, 17, 4, 15, 8, 0x0c },
	{ "EPCS4 ", 0x12, 19, 8, 16, 8, 0x1c },
	{ 0, 0, 0, 0, 0, 0 }
};

static asmi_devinfo_t *asmi_dev_find (void)
{
	unsigned char buf[4];
	unsigned char id;
	int i;
	struct asmi_devinfo_t *dev = NULL;

	/* Read silicon id requires 3 "dummy bytes" before it's put
	 * on the wire.
	 */
	buf[0] = ASMI_READ_ID;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 0;

	asmi_cs (1);
	asmi_snd (buf,4);
	asmi_rcv (buf,1);
	asmi_cs (0);
	id = buf[0];

	/* Find the info struct */
	i = 0;
	while (devinfo[i].name) {
		if (id == devinfo[i].id) {
			dev = &devinfo[i];
			break;
		}
		i++;
	}

	return (dev);
}

/***********************************************************************
 * Misc Utilities
 ***********************************************************************/
static unsigned asmi_cfgsz (void)
{
	unsigned sz = 0;
	unsigned char buf[128];
	unsigned char *p;

	/* Read in the first 128 bytes of the device */
	buf[0] = ASMI_READ_BYTES;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 0;

	asmi_cs (1);
	asmi_snd (buf,4);
	asmi_rrcv (buf, sizeof(buf));
	asmi_cs (0);

	/* Search for the starting 0x6a which is followed by the
	 * 4-byte 'register' and 4-byte bit-count.
	 */
	p = buf;
	while (p < buf + sizeof(buf)-8) {
		if ( *p == 0x6a ) {
			/* Point to bit count and extract */
			p += 5;
			sz = *p++;
			sz |= *p++ << 8;
			sz |= *p++ << 16;
			sz |= *p++ << 24;
			/* Convert to byte count */
			sz += 7;
			sz >>= 3;
		} else if (*p == 0xff) {
			/* 0xff is ok ... just skip */
			p++;
			continue;
		} else {
			/* Not 0xff or 0x6a ... something's not
			 * right ... report 'unknown' (sz=0).
			 */
			break;
		}
	}
	return (sz);
}

static int asmi_erase (unsigned start, unsigned end)
{
	unsigned off, sectsz;
	unsigned char buf[4];
	struct asmi_devinfo_t *dev = asmi_dev_find ();

	if (!dev || (start>end))
		return (-1);

	/* Erase the requested sectors. An address is required
	 * that lies within the requested sector -- we'll just
	 * use the first address in the sector.
	 */
	printf ("asmi erasing sector %d ", start);
	if (start != end)
		printf ("to %d ", end);
	sectsz = (1 << dev->sz_sect);
	while (start <= end) {
		off = start * sectsz;
		start++;

		buf[0] = ASMI_ERASE_SECT;
		buf[1] = off >> 16;
		buf[2] = off >> 8;
		buf[3] = off;

		asmi_wr_enable ();
		asmi_cs (1);
		asmi_snd (buf,4);
		asmi_cs (0);

		printf ("."); /* Some user feedback */

		/* Wait for erase to complete */
		while (asmi_status_rd() & ASMI_STATUS_WIP)
			;
	}
	printf (" done.\n");
	return (0);
}

static int asmi_read (ulong addr, ulong off, ulong cnt)
{
	unsigned char buf[4];

	buf[0] = ASMI_READ_BYTES;
	buf[1] = off >> 16;
	buf[2] = off >> 8;
	buf[3] = off;

	asmi_cs (1);
	asmi_snd (buf,4);
	asmi_rrcv ((unsigned char *)addr, cnt);
	asmi_cs (0);

	return (0);
}

static
int asmi_write (ulong addr, ulong off, ulong cnt)
{
	ulong wrcnt;
	unsigned pgsz;
	unsigned char buf[4];
	struct asmi_devinfo_t *dev = asmi_dev_find ();

	if (!dev)
		return (-1);

	pgsz = (1<<dev->sz_page);
	while (cnt) {
		if (off % pgsz)
			wrcnt = pgsz - (off % pgsz);
		else
			wrcnt = pgsz;
		wrcnt = (wrcnt > cnt) ? cnt : wrcnt;

		buf[0] = ASMI_WRITE_BYTES;
		buf[1] = off >> 16;
		buf[2] = off >> 8;
		buf[3] = off;

		asmi_wr_enable ();
		asmi_cs (1);
		asmi_snd (buf,4);
		asmi_rsnd ((unsigned char *)addr, wrcnt);
		asmi_cs (0);

		/* Wait for write to complete */
		while (asmi_status_rd() & ASMI_STATUS_WIP)
			;

		cnt -= wrcnt;
		off += wrcnt;
		addr += wrcnt;
	}

	return (0);
}

static
int asmi_verify (ulong addr, ulong off, ulong cnt, ulong *err)
{
	ulong rdcnt;
	unsigned char buf[256];
	unsigned char *start,*end;
	int i;

	start = end = (unsigned char *)addr;
	while (cnt) {
		rdcnt = (cnt>sizeof(buf)) ? sizeof(buf) : cnt;
		asmi_read ((ulong)buf, off, rdcnt);
		for (i=0; i<rdcnt; i++) {
			if (*end != buf[i]) {
				*err = end - start;
				return(-1);
			}
			end++;
		}
		cnt -= rdcnt;
		off += rdcnt;
	}
	return (0);
}

static int asmi_sect_erased (int sect, unsigned *offset,
		struct asmi_devinfo_t *dev)
{
	unsigned char buf[128];
	unsigned off, end;
	unsigned sectsz;
	int i;

	sectsz = (1 << dev->sz_sect);
	off = sectsz * sect;
	end = off + sectsz;

	while (off < end) {
		asmi_read ((ulong)buf, off, sizeof(buf));
		for (i=0; i < sizeof(buf); i++) {
			if (buf[i] != 0xff) {
				*offset = off + i;
				return (0);
			}
		}
		off += sizeof(buf);
	}
	return (1);
}


/***********************************************************************
 * Commands
 ***********************************************************************/
static
void do_asmi_info (struct asmi_devinfo_t *dev, int argc, char *argv[])
{
	int i;
	unsigned char stat;
	unsigned tmp;
	int erased;

	/* Basic device info */
	printf ("%s: %d kbytes (%d sectors x %d kbytes,"
		" %d bytes/page)\n",
		dev->name, 1 << (dev->size-10),
		dev->num_sects, 1 << (dev->sz_sect-10),
		1 << dev->sz_page );

	/* Status -- for now protection is all-or-nothing */
	stat = asmi_status_rd();
	printf ("status: 0x%02x (WIP:%d, WEL:%d, PROT:%s)\n",
		stat,
		(stat & ASMI_STATUS_WIP) ? 1 : 0,
	        (stat & ASMI_STATUS_WEL) ? 1 : 0,
		(stat & dev->prot_mask) ? "on" : "off" );

	/* Configuration  */
	tmp = asmi_cfgsz ();
	if (tmp) {
		printf ("config: 0x%06x (%d) bytes\n", tmp, tmp );
	} else {
		printf ("config: unknown\n" );
	}

	/* Sector info */
	for (i=0; i<dev->num_sects; i++) {
		erased = asmi_sect_erased (i, &tmp, dev);
		printf ("     %d: %06x ",
			i, i*(1<<dev->sz_sect) );
		if (erased)
			printf ("erased\n");
		else
			printf ("data @ 0x%06x\n", tmp);
	}

	return;
}

static
void do_asmi_erase (struct asmi_devinfo_t *dev, int argc, char *argv[])
{
	unsigned start,end;

	if ((argc < 3) || (argc > 4)) {
		printf ("USAGE: asmi erase sect [end]\n");
		return;
	}
	if ((asmi_status_rd() & dev->prot_mask) != 0) {
		printf ( "asmi: device protected.\n");
		return;
	}

	start = simple_strtoul (argv[2], NULL, 10);
	if (argc > 3)
		end = simple_strtoul (argv[3], NULL, 10);
	else
		end = start;
	if ((start >= dev->num_sects) || (start > end)) {
		printf ("asmi: invalid sector range: [%d:%d]\n",
			start, end );
		return;
	}

	asmi_erase (start, end);

	return;
}

static
void do_asmi_protect (struct asmi_devinfo_t *dev, int argc, char *argv[])
{
	unsigned char stat;

	/* For now protection is all-or-nothing to keep things
	 * simple. The protection bits don't map in a linear
	 * fashion ... and we would rather protect the bottom
	 * of the device since it contains the config data and
	 * leave the top unprotected for app use. But unfortunately
	 * protection works from top-to-bottom so it does
	 * really help very much from a software app point-of-view.
	 */
	if (argc < 3) {
		printf ("USAGE: asmi protect on | off\n");
		return;
	}
	if (!dev)
		return;

	/* Protection on/off is just a matter of setting/clearing
	 * all protection bits in the status register.
	 */
	stat = asmi_status_rd ();
	if (strcmp ("on", argv[2]) == 0) {
		stat |= dev->prot_mask;
	} else if (strcmp ("off", argv[2]) == 0 ) {
		stat &= ~dev->prot_mask;
	} else {
		printf ("asmi: unknown protection: %s\n", argv[2]);
		return;
	}
	asmi_status_wr (stat);
	return;
}

static
void do_asmi_read (struct asmi_devinfo_t *dev, int argc, char *argv[])
{
	ulong addr,off,cnt;
	ulong sz;

	if (argc < 5) {
		printf ("USAGE: asmi read addr offset count\n");
		return;
	}

	sz = 1 << dev->size;
	addr = simple_strtoul (argv[2], NULL, 16);
	off  = simple_strtoul (argv[3], NULL, 16);
	cnt  = simple_strtoul (argv[4], NULL, 16);
	if (off > sz) {
		printf ("offset is greater than device size"
			"... aborting.\n");
		return;
	}
	if ((off + cnt) > sz) {
		printf ("request exceeds device size"
			"... truncating.\n");
		cnt = sz - off;
	}
	printf ("asmi: read %08lx <- %06lx (0x%lx bytes)\n",
			addr, off, cnt);
	asmi_read (addr, off, cnt);

	return;
}

static
void do_asmi_write (struct asmi_devinfo_t *dev, int argc, char *argv[])
{
	ulong addr,off,cnt;
	ulong sz;
	ulong err;

	if (argc < 5) {
		printf ("USAGE: asmi write addr offset count\n");
		return;
	}
	if ((asmi_status_rd() & dev->prot_mask) != 0) {
		printf ( "asmi: device protected.\n");
		return;
	}

	sz = 1 << dev->size;
	addr = simple_strtoul (argv[2], NULL, 16);
	off  = simple_strtoul (argv[3], NULL, 16);
	cnt  = simple_strtoul (argv[4], NULL, 16);
	if (off > sz) {
		printf ("offset is greater than device size"
			"... aborting.\n");
		return;
	}
	if ((off + cnt) > sz) {
		printf ("request exceeds device size"
			"... truncating.\n");
		cnt = sz - off;
	}
	printf ("asmi: write %08lx -> %06lx (0x%lx bytes)\n",
			addr, off, cnt);
	asmi_write (addr, off, cnt);
	if (asmi_verify (addr, off, cnt, &err) != 0)
		printf ("asmi: write error at offset %06lx\n", err);

	return;
}

static
void do_asmi_verify (struct asmi_devinfo_t *dev, int argc, char *argv[])
{
	ulong addr,off,cnt;
	ulong sz;
	ulong err;

	if (argc < 5) {
		printf ("USAGE: asmi verify addr offset count\n");
		return;
	}

	sz = 1 << dev->size;
	addr = simple_strtoul (argv[2], NULL, 16);
	off  = simple_strtoul (argv[3], NULL, 16);
	cnt  = simple_strtoul (argv[4], NULL, 16);
	if (off > sz) {
		printf ("offset is greater than device size"
			"... aborting.\n");
		return;
	}
	if ((off + cnt) > sz) {
		printf ("request exceeds device size"
			"... truncating.\n");
		cnt = sz - off;
	}
	printf ("asmi: verify %08lx -> %06lx (0x%lx bytes)\n",
			addr, off, cnt);
	if (asmi_verify (addr, off, cnt, &err) != 0)
		printf ("asmi: verify error at offset %06lx\n", err);

	return;
}

/*-----------------------------------------------------------------------*/
int do_asmi (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int len;
	struct asmi_devinfo_t *dev = asmi_dev_find ();

	if (argc < 2) {
		printf ("Usage:%s", LONG_HELP);
		return (0);
	}

	if (!dev) {
		printf ("asmi: device not found.\n");
		return (0);
	}

	len = strlen (argv[1]);
	if (strncmp ("info", argv[1], len) == 0) {
		do_asmi_info ( dev, argc, argv);
	} else if (strncmp ("erase", argv[1], len) == 0) {
		do_asmi_erase (dev, argc, argv);
	} else if (strncmp ("protect", argv[1], len) == 0) {
		do_asmi_protect (dev, argc, argv);
	} else if (strncmp ("read", argv[1], len) == 0) {
		do_asmi_read (dev, argc, argv);
	} else if (strncmp ("write", argv[1], len) == 0) {
		do_asmi_write (dev, argc, argv);
	} else if (strncmp ("verify", argv[1], len) == 0) {
		do_asmi_verify (dev, argc, argv);
	} else {
		printf ("asmi: unknown operation: %s\n", argv[1]);
	}

	return (0);
}

/*-----------------------------------------------------------------------*/


U_BOOT_CMD( asmi, 5, 0, do_asmi, SHORT_HELP, LONG_HELP );

#endif /* CONFIG_NIOS_ASMI */
