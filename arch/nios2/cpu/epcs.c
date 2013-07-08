/*
 * (C) Copyright 2004, Psyent Corporation <www.psyent.com>
 * Scott McNutt <smcnutt@psyent.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>

#if defined(CONFIG_SYS_NIOS_EPCSBASE)
#include <command.h>
#include <asm/io.h>
#include <nios2-io.h>
#include <nios2-epcs.h>


/*-----------------------------------------------------------------------*/
#define SHORT_HELP\
	"epcs    - read/write Cyclone EPCS configuration device.\n"

#define LONG_HELP\
	"\n"\
	"epcs erase start [end]\n"\
	"    - erase sector start or sectors start through end.\n"\
	"epcs info\n"\
	"    - display EPCS device information.\n"\
	"epcs protect on | off\n"\
	"    - turn device protection on or off.\n"\
	"epcs read addr offset count\n"\
	"    - read count bytes from offset to addr.\n"\
	"epcs write addr offset count\n"\
	"    - write count bytes to offset from addr.\n"\
	"epcs verify addr offset count\n"\
	"    - verify count bytes at offset from addr."


/*-----------------------------------------------------------------------*/
/* Operation codes for serial configuration devices
 */
#define EPCS_WRITE_ENA		0x06	/* Write enable */
#define EPCS_WRITE_DIS		0x04	/* Write disable */
#define EPCS_READ_STAT		0x05	/* Read status */
#define EPCS_READ_BYTES		0x03	/* Read bytes */
#define EPCS_READ_ID		0xab	/* Read silicon id */
#define EPCS_WRITE_STAT		0x01	/* Write status */
#define EPCS_WRITE_BYTES	0x02	/* Write bytes */
#define EPCS_ERASE_BULK		0xc7	/* Erase entire device */
#define EPCS_ERASE_SECT		0xd8	/* Erase sector */

/* Device status register bits
 */
#define EPCS_STATUS_WIP		(1<<0)	/* Write in progress */
#define EPCS_STATUS_WEL		(1<<1)	/* Write enable latch */

/* Misc
 */
#define EPCS_TIMEOUT		100	/* 100 msec timeout */

static nios_spi_t *epcs = (nios_spi_t *)CONFIG_SYS_NIOS_EPCSBASE;

/***********************************************************************
 * Device access
 ***********************************************************************/
static int epcs_cs (int assert)
{
	ulong start;
	unsigned tmp;


	if (assert) {
		tmp = readl (&epcs->control);
		writel (tmp | NIOS_SPI_SSO, &epcs->control);
	} else {
		/* Let all bits shift out */
		start = get_timer (0);
		while ((readl (&epcs->status) & NIOS_SPI_TMT) == 0)
			if (get_timer (start) > EPCS_TIMEOUT)
				return (-1);
		tmp = readl (&epcs->control);
		writel (tmp & ~NIOS_SPI_SSO, &epcs->control);
	}
	return (0);
}

static int epcs_tx (unsigned char c)
{
	ulong start;

	start = get_timer (0);
	while ((readl (&epcs->status) & NIOS_SPI_TRDY) == 0)
		if (get_timer (start) > EPCS_TIMEOUT)
			return (-1);
	writel (c, &epcs->txdata);
	return (0);
}

static int epcs_rx (void)
{
	ulong start;

	start = get_timer (0);
	while ((readl (&epcs->status) & NIOS_SPI_RRDY) == 0)
		if (get_timer (start) > EPCS_TIMEOUT)
			return (-1);
	return (readl (&epcs->rxdata));
}

static unsigned char bitrev[] = {
	0x00, 0x08, 0x04, 0x0c, 0x02, 0x0a, 0x06, 0x0e,
	0x01, 0x09, 0x05, 0x0d, 0x03, 0x0b, 0x07, 0x0f
};

static unsigned char epcs_bitrev (unsigned char c)
{
	unsigned char val;

	val  = bitrev[c>>4];
	val |= bitrev[c & 0x0f]<<4;
	return (val);
}

static void epcs_rcv (unsigned char *dst, int len)
{
	while (len--) {
		epcs_tx (0);
		*dst++ = epcs_rx ();
	}
}

static void epcs_rrcv (unsigned char *dst, int len)
{
	while (len--) {
		epcs_tx (0);
		*dst++ = epcs_bitrev (epcs_rx ());
	}
}

static void epcs_snd (unsigned char *src, int len)
{
	while (len--) {
		epcs_tx (*src++);
		epcs_rx ();
	}
}

static void epcs_rsnd (unsigned char *src, int len)
{
	while (len--) {
		epcs_tx (epcs_bitrev (*src++));
		epcs_rx ();
	}
}

static void epcs_wr_enable (void)
{
	epcs_cs (1);
	epcs_tx (EPCS_WRITE_ENA);
	epcs_rx ();
	epcs_cs (0);
}

static unsigned char epcs_status_rd (void)
{
	unsigned char status;

	epcs_cs (1);
	epcs_tx (EPCS_READ_STAT);
	epcs_rx ();
	epcs_tx (0);
	status = epcs_rx ();
	epcs_cs (0);
	return (status);
}

static void epcs_status_wr (unsigned char status)
{
	epcs_wr_enable ();
	epcs_cs (1);
	epcs_tx (EPCS_WRITE_STAT);
	epcs_rx ();
	epcs_tx (status);
	epcs_rx ();
	epcs_cs (0);
	return;
}

/***********************************************************************
 * Device information
 ***********************************************************************/

static struct epcs_devinfo_t devinfo[] = {
	{ "EPCS1 ", 0x10, 17, 4, 15, 8, 0x0c },
	{ "EPCS4 ", 0x12, 19, 8, 16, 8, 0x1c },
	{ "EPCS16", 0x14, 21, 32, 16, 8, 0x1c },
	{ "EPCS64", 0x16, 23,128, 16, 8, 0x1c },
	{ 0, 0, 0, 0, 0, 0 }
};

int epcs_reset (void)
{
	/* When booting from an epcs controller, the epcs bootrom
	 * code may leave the slave select in an asserted state.
	 * This causes two problems: (1) The initial epcs access
	 * will fail -- not a big deal, and (2) a software reset
	 * will cause the bootrom code to hang since it does not
	 * ensure the select is negated prior to first access -- a
	 * big deal. Here we just negate chip select and everything
	 * gets better :-)
	 */
	epcs_cs (0); /* Negate chip select */
	return (0);
}

epcs_devinfo_t *epcs_dev_find (void)
{
	unsigned char buf[4];
	unsigned char id;
	int i;
	struct epcs_devinfo_t *dev = NULL;

	/* Read silicon id requires 3 "dummy bytes" before it's put
	 * on the wire.
	 */
	buf[0] = EPCS_READ_ID;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 0;

	epcs_cs (1);
	epcs_snd (buf,4);
	epcs_rcv (buf,1);
	if (epcs_cs (0) == -1)
		return (NULL);
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
int epcs_cfgsz (void)
{
	int sz = 0;
	unsigned char buf[128];
	unsigned char *p;
	struct epcs_devinfo_t *dev = epcs_dev_find ();

	if (!dev)
		return (-1);

	/* Read in the first 128 bytes of the device */
	buf[0] = EPCS_READ_BYTES;
	buf[1] = 0;
	buf[2] = 0;
	buf[3] = 0;

	epcs_cs (1);
	epcs_snd (buf,4);
	epcs_rrcv (buf, sizeof(buf));
	epcs_cs (0);

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

int epcs_erase (unsigned start, unsigned end)
{
	unsigned off, sectsz;
	unsigned char buf[4];
	struct epcs_devinfo_t *dev = epcs_dev_find ();

	if (!dev || (start>end))
		return (-1);

	/* Erase the requested sectors. An address is required
	 * that lies within the requested sector -- we'll just
	 * use the first address in the sector.
	 */
	printf ("epcs erasing sector %d ", start);
	if (start != end)
		printf ("to %d ", end);
	sectsz = (1 << dev->sz_sect);
	while (start <= end) {
		off = start * sectsz;
		start++;

		buf[0] = EPCS_ERASE_SECT;
		buf[1] = off >> 16;
		buf[2] = off >> 8;
		buf[3] = off;

		epcs_wr_enable ();
		epcs_cs (1);
		epcs_snd (buf,4);
		epcs_cs (0);

		printf ("."); /* Some user feedback */

		/* Wait for erase to complete */
		while (epcs_status_rd() & EPCS_STATUS_WIP)
			;
	}
	printf (" done.\n");
	return (0);
}

int epcs_read (ulong addr, ulong off, ulong cnt)
{
	unsigned char buf[4];
	struct epcs_devinfo_t *dev = epcs_dev_find ();

	if (!dev)
		return (-1);

	buf[0] = EPCS_READ_BYTES;
	buf[1] = off >> 16;
	buf[2] = off >> 8;
	buf[3] = off;

	epcs_cs (1);
	epcs_snd (buf,4);
	epcs_rrcv ((unsigned char *)addr, cnt);
	epcs_cs (0);

	return (0);
}

int epcs_write (ulong addr, ulong off, ulong cnt)
{
	ulong wrcnt;
	unsigned pgsz;
	unsigned char buf[4];
	struct epcs_devinfo_t *dev = epcs_dev_find ();

	if (!dev)
		return (-1);

	pgsz = (1<<dev->sz_page);
	while (cnt) {
		if (off % pgsz)
			wrcnt = pgsz - (off % pgsz);
		else
			wrcnt = pgsz;
		wrcnt = (wrcnt > cnt) ? cnt : wrcnt;

		buf[0] = EPCS_WRITE_BYTES;
		buf[1] = off >> 16;
		buf[2] = off >> 8;
		buf[3] = off;

		epcs_wr_enable ();
		epcs_cs (1);
		epcs_snd (buf,4);
		epcs_rsnd ((unsigned char *)addr, wrcnt);
		epcs_cs (0);

		/* Wait for write to complete */
		while (epcs_status_rd() & EPCS_STATUS_WIP)
			;

		cnt -= wrcnt;
		off += wrcnt;
		addr += wrcnt;
	}

	return (0);
}

int epcs_verify (ulong addr, ulong off, ulong cnt, ulong *err)
{
	ulong rdcnt;
	unsigned char buf[256];
	unsigned char *start,*end;
	int i;

	start = end = (unsigned char *)addr;
	while (cnt) {
		rdcnt = (cnt>sizeof(buf)) ? sizeof(buf) : cnt;
		epcs_read ((ulong)buf, off, rdcnt);
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

static int epcs_sect_erased (int sect, unsigned *offset,
		struct epcs_devinfo_t *dev)
{
	unsigned char buf[128];
	unsigned off, end;
	unsigned sectsz;
	int i;

	sectsz = (1 << dev->sz_sect);
	off = sectsz * sect;
	end = off + sectsz;

	while (off < end) {
		epcs_read ((ulong)buf, off, sizeof(buf));
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
void do_epcs_info (struct epcs_devinfo_t *dev, int argc, char * const argv[])
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
	stat = epcs_status_rd();
	printf ("status: 0x%02x (WIP:%d, WEL:%d, PROT:%s)\n",
		stat,
		(stat & EPCS_STATUS_WIP) ? 1 : 0,
	        (stat & EPCS_STATUS_WEL) ? 1 : 0,
		(stat & dev->prot_mask) ? "on" : "off" );

	/* Configuration  */
	tmp = epcs_cfgsz ();
	if (tmp) {
		printf ("config: 0x%06x (%d) bytes\n", tmp, tmp );
	} else {
		printf ("config: unknown\n" );
	}

	/* Sector info */
	for (i=0; (i < dev->num_sects) && (argc > 1); i++) {
		erased = epcs_sect_erased (i, &tmp, dev);
		if ((i & 0x03) == 0) printf ("\n");
		printf ("%4d: %07x ",
			i, i*(1<<dev->sz_sect) );
		if (erased)
			printf ("E ");
		else
			printf ("  ");
	}
	printf ("\n");

	return;
}

static
void do_epcs_erase (struct epcs_devinfo_t *dev, int argc, char * const argv[])
{
	unsigned start,end;

	if ((argc < 3) || (argc > 4)) {
		printf ("USAGE: epcs erase sect [end]\n");
		return;
	}
	if ((epcs_status_rd() & dev->prot_mask) != 0) {
		printf ( "epcs: device protected.\n");
		return;
	}

	start = simple_strtoul (argv[2], NULL, 10);
	if (argc > 3)
		end = simple_strtoul (argv[3], NULL, 10);
	else
		end = start;
	if ((start >= dev->num_sects) || (start > end)) {
		printf ("epcs: invalid sector range: [%d:%d]\n",
			start, end );
		return;
	}

	epcs_erase (start, end);

	return;
}

static
void do_epcs_protect (struct epcs_devinfo_t *dev, int argc, char * const argv[])
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
		printf ("USAGE: epcs protect on | off\n");
		return;
	}
	if (!dev)
		return;

	/* Protection on/off is just a matter of setting/clearing
	 * all protection bits in the status register.
	 */
	stat = epcs_status_rd ();
	if (strcmp ("on", argv[2]) == 0) {
		stat |= dev->prot_mask;
	} else if (strcmp ("off", argv[2]) == 0 ) {
		stat &= ~dev->prot_mask;
	} else {
		printf ("epcs: unknown protection: %s\n", argv[2]);
		return;
	}
	epcs_status_wr (stat);
	return;
}

static
void do_epcs_read (struct epcs_devinfo_t *dev, int argc, char * const argv[])
{
	ulong addr,off,cnt;
	ulong sz;

	if (argc < 5) {
		printf ("USAGE: epcs read addr offset count\n");
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
	printf ("epcs: read %08lx <- %06lx (0x%lx bytes)\n",
			addr, off, cnt);
	epcs_read (addr, off, cnt);

	return;
}

static
void do_epcs_write (struct epcs_devinfo_t *dev, int argc, char * const argv[])
{
	ulong addr,off,cnt;
	ulong sz;
	ulong err;

	if (argc < 5) {
		printf ("USAGE: epcs write addr offset count\n");
		return;
	}
	if ((epcs_status_rd() & dev->prot_mask) != 0) {
		printf ( "epcs: device protected.\n");
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
	printf ("epcs: write %08lx -> %06lx (0x%lx bytes)\n",
			addr, off, cnt);
	epcs_write (addr, off, cnt);
	if (epcs_verify (addr, off, cnt, &err) != 0)
		printf ("epcs: write error at offset %06lx\n", err);

	return;
}

static
void do_epcs_verify (struct epcs_devinfo_t *dev, int argc, char * const argv[])
{
	ulong addr,off,cnt;
	ulong sz;
	ulong err;

	if (argc < 5) {
		printf ("USAGE: epcs verify addr offset count\n");
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
	printf ("epcs: verify %08lx -> %06lx (0x%lx bytes)\n",
			addr, off, cnt);
	if (epcs_verify (addr, off, cnt, &err) != 0)
		printf ("epcs: verify error at offset %06lx\n", err);

	return;
}

/*-----------------------------------------------------------------------*/
int do_epcs (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int len;
	struct epcs_devinfo_t *dev = epcs_dev_find ();

	if (!dev) {
		printf ("epcs: device not found.\n");
		return (-1);
	}

	if (argc < 2) {
		do_epcs_info (dev, argc, argv);
		return (0);
	}

	len = strlen (argv[1]);
	if (strncmp ("info", argv[1], len) == 0) {
		do_epcs_info (dev, argc, argv);
	} else if (strncmp ("erase", argv[1], len) == 0) {
		do_epcs_erase (dev, argc, argv);
	} else if (strncmp ("protect", argv[1], len) == 0) {
		do_epcs_protect (dev, argc, argv);
	} else if (strncmp ("read", argv[1], len) == 0) {
		do_epcs_read (dev, argc, argv);
	} else if (strncmp ("write", argv[1], len) == 0) {
		do_epcs_write (dev, argc, argv);
	} else if (strncmp ("verify", argv[1], len) == 0) {
		do_epcs_verify (dev, argc, argv);
	} else {
		printf ("epcs: unknown operation: %s\n", argv[1]);
	}

	return (0);
}

/*-----------------------------------------------------------------------*/


U_BOOT_CMD( epcs, 5, 0, do_epcs, SHORT_HELP, LONG_HELP );

#endif /* CONFIG_NIOS_EPCS */
