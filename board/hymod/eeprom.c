/*
 * (C) Copyright 2001
 * Murray Jensen, CSIRO-MIT, <Murray.Jensen@csiro.au>
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
#include <mpc8260.h>

/* imports from fetch.c */
extern int fetch_and_parse (char *, ulong, int (*)(uchar *, uchar *));

/* imports from input.c */
extern int hymod_get_serno (const char *);

/* this is relative to the root of the server's tftp directory */
static char *def_bddb_cfgdir = "/hymod/bddb";

static int
hymod_eeprom_load (int which, hymod_eeprom_t *ep)
{
	unsigned dev_addr = CFG_I2C_EEPROM_ADDR | \
		(which ? HYMOD_EEOFF_MEZZ : HYMOD_EEOFF_MAIN);
	unsigned offset = 0;
	uchar data[HYMOD_EEPROM_MAXLEN], *dp, *edp;
	hymod_eehdr_t hdr;
	ulong len, crc;

	memset (ep, 0, sizeof *ep);

	eeprom_read (dev_addr, offset, (uchar *)&hdr, sizeof (hdr));
	offset += sizeof (hdr);

	if (hdr.id != HYMOD_EEPROM_ID || hdr.ver > HYMOD_EEPROM_VER ||
	  (len = hdr.len) > HYMOD_EEPROM_MAXLEN)
	    return (0);

	eeprom_read (dev_addr, offset, data, len);
	offset += len;

	eeprom_read (dev_addr, offset, (uchar *)&crc, sizeof (ulong));
	offset += sizeof (ulong);

	if (crc32 (crc32 (0, (uchar *)&hdr, sizeof hdr), data, len) != crc)
		return (0);

	ep->ver = hdr.ver;
	dp = data; edp = dp + len;

	for (;;) {
		ulong rtyp;
		uchar rlen, *rdat;

		rtyp = *dp++;
		if ((rtyp & 0x80) == 0)
			rlen = *dp++;
		else {
			uchar islarge = rtyp & 0x40;

			rtyp = ((rtyp & 0x3f) << 8) | *dp++;
			if (islarge) {
				rtyp = (rtyp << 8) | *dp++;
				rtyp = (rtyp << 8) | *dp++;
			}

			rlen = *dp++;
			rlen = (rlen << 8) | *dp++;
			if (islarge) {
				rlen = (rlen << 8) | *dp++;
				rlen = (rlen << 8) | *dp++;
			}
		}

		if (rtyp == 0)
			break;

		rdat = dp;
		dp += rlen;

		if (dp > edp)	/* error? */
			break;

		switch (rtyp) {

		case HYMOD_EEREC_SERNO:		/* serial number */
			if (rlen == sizeof (ulong))
				ep->serno = \
					((ulong)rdat[0] << 24) | \
					((ulong)rdat[1] << 16) | \
					((ulong)rdat[2] << 8) | \
					(ulong)rdat[3];
			break;

		case HYMOD_EEREC_DATE:		/* date */
			if (rlen == sizeof (hymod_date_t)) {
				ep->date.year = ((ushort)rdat[0] << 8) | \
					(ushort)rdat[1];
				ep->date.month = rdat[2];
				ep->date.day = rdat[3];
			}
			break;

		case HYMOD_EEREC_BATCH:		/* batch */
			if (rlen <= HYMOD_MAX_BATCH)
				memcpy (ep->batch, rdat, ep->batchlen = rlen);
			break;

		case HYMOD_EEREC_TYPE:		/* board type */
			if (rlen == 1)
				ep->bdtype = *rdat;
			break;

		case HYMOD_EEREC_REV:		/* board revision */
			if (rlen == 1)
				ep->bdrev = *rdat;
			break;

		case HYMOD_EEREC_SDRAM:		/* sdram size(s) */
			if (rlen > 0 && rlen <= HYMOD_MAX_SDRAM) {
				int i;

				for (i = 0; i < rlen; i++)
					ep->sdramsz[i] = rdat[i];
				ep->nsdram = rlen;
			}
			break;

		case HYMOD_EEREC_FLASH:		/* flash size(s) */
			if (rlen > 0 && rlen <= HYMOD_MAX_FLASH) {
				int i;

				for (i = 0; i < rlen; i++)
					ep->flashsz[i] = rdat[i];
				ep->nflash = rlen;
			}
			break;

		case HYMOD_EEREC_ZBT:		/* zbt ram size(s) */
			if (rlen > 0 && rlen <= HYMOD_MAX_ZBT) {
				int i;

				for (i = 0; i < rlen; i++)
					ep->zbtsz[i] = rdat[i];
				ep->nzbt = rlen;
			}
			break;

		case HYMOD_EEREC_XLXTYP:	/* xilinx fpga type(s) */
			if (rlen > 0 && rlen <= HYMOD_MAX_XLX) {
				int i;

				for (i = 0; i < rlen; i++)
					ep->xlx[i].type = rdat[i];
				ep->nxlx = rlen;
			}
			break;

		case HYMOD_EEREC_XLXSPD:	/* xilinx fpga speed(s) */
			if (rlen > 0 && rlen <= HYMOD_MAX_XLX) {
				int i;

				for (i = 0; i < rlen; i++)
					ep->xlx[i].speed = rdat[i];
			}
			break;

		case HYMOD_EEREC_XLXTMP:	/* xilinx fpga temperature(s) */
			if (rlen > 0 && rlen <= HYMOD_MAX_XLX) {
				int i;

				for (i = 0; i < rlen; i++)
					ep->xlx[i].temp = rdat[i];
			}
			break;

		case HYMOD_EEREC_XLXGRD:	/* xilinx fpga grade(s) */
			if (rlen > 0 && rlen <= HYMOD_MAX_XLX) {
				int i;

				for (i = 0; i < rlen; i++)
					ep->xlx[i].grade = rdat[i];
			}
			break;

		case HYMOD_EEREC_CPUTYP:	/* CPU type */
			if (rlen == 1)
				ep->mpc.type = *rdat;
			break;

		case HYMOD_EEREC_CPUSPD:	/* CPU speed */
			if (rlen == 1)
				ep->mpc.cpuspd = *rdat;
			break;

		case HYMOD_EEREC_CPMSPD:	/* CPM speed */
			if (rlen == 1)
				ep->mpc.cpmspd = *rdat;
			break;

		case HYMOD_EEREC_BUSSPD:	/* bus speed */
			if (rlen == 1)
				ep->mpc.busspd = *rdat;
			break;

		case HYMOD_EEREC_HSTYPE:	/* hs-serial chip type */
			if (rlen == 1)
				ep->hss.type = *rdat;
			break;

		case HYMOD_EEREC_HSCHIN:	/* num hs-serial input chans */
			if (rlen == 1)
				ep->hss.nchin = *rdat;
			break;

		case HYMOD_EEREC_HSCHOUT:	/* num hs-serial output chans */
			if (rlen == 1)
				ep->hss.nchout = *rdat;
			break;

		default:	/* ignore */
			break;
		}
	}

	return (1);
}

/* maps an ascii "name=value" into a binary eeprom data record */
typedef
	struct _eerec_map {
		char *name;
		uint type;
		uchar *(*handler) \
			(struct _eerec_map *, uchar *, uchar *, uchar *);
		uint length;
		uint maxlen;
	}
eerec_map_t;

static uchar *
uint_handler (eerec_map_t *rp, uchar *val, uchar *dp, uchar *edp)
{
	char *eval;
	ulong lval;

	lval = simple_strtol ((char *)val, &eval, 10);

	if ((uchar *)eval == val || *eval != '\0') {
		printf ("%s rec (%s) is not a valid uint\n", rp->name, val);
		return (NULL);
	}

	if (dp + 2 + rp->length > edp) {
		printf ("can't fit %s rec into eeprom\n", rp->name);
		return (NULL);
	}

	*dp++ = rp->type;
	*dp++ = rp->length;

	switch (rp->length) {

	case 1:
		if (lval >= 256) {
			printf ("%s rec value (%lu) out of range (0-255)\n",
				rp->name, lval);
			return (NULL);
		}
		*dp++ = lval;
		break;

	case 2:
		if (lval >= 65536) {
			printf ("%s rec value (%lu) out of range (0-65535)\n",
				rp->name, lval);
			return (NULL);
		}
		*dp++ = lval >> 8;
		*dp++ = lval;
		break;

	case 4:
		*dp++ = lval >> 24;
		*dp++ = lval >> 16;
		*dp++ = lval >> 8;
		*dp++ = lval;
		break;

	default:
		printf ("huh? rp->length not 1, 2 or 4! (%d)\n", rp->length);
		return (NULL);
	}

	return (dp);
}

static uchar *
date_handler (eerec_map_t *rp, uchar *val, uchar *dp, uchar *edp)
{
	hymod_date_t date;
	char *p = (char *)val;
	char *ep;
	ulong lval;

	lval = simple_strtol (p, &ep, 10);
	if (ep == p || *ep++ != '-') {
bad_date:
		printf ("%s rec (%s) is not a valid date\n", rp->name, val);
		return (NULL);
	}
	if (lval >= 65536)
		goto bad_date;
	date.year = lval;

	lval = simple_strtol (p = ep, &ep, 10);
	if (ep == p || *ep++ != '-' || lval == 0 || lval > 12)
		goto bad_date;
	date.month = lval;

	lval = simple_strtol (p = ep, &ep, 10);
	if (ep == p || *ep != '\0' || lval == 0 || lval > 31)
		goto bad_date;
	date.day = lval;

	if (dp + 2 + rp->length > edp) {
		printf ("can't fit %s rec into eeprom\n", rp->name);
		return (NULL);
	}

	*dp++ = rp->type;
	*dp++ = rp->length;
	*dp++ = date.year >> 8;
	*dp++ = date.year;
	*dp++ = date.month;
	*dp++ = date.day;

	return (dp);
}

static uchar *
string_handler (eerec_map_t *rp, uchar *val, uchar *dp, uchar *edp)
{
	uint len;

	if ((len = strlen ((char *)val)) > rp->maxlen) {
		printf ("%s rec (%s) string is too long (%d>%d)\n",
			rp->name, val, len, rp->maxlen);
		return (NULL);
	}

	if (dp + 2 + len > edp) {
		printf ("can't fit %s rec into eeprom\n", rp->name);
		return (NULL);
	}

	*dp++ = rp->type;
	*dp++ = len;
	memcpy (dp, val, len);
	dp += len;

	return (dp);
}

static uchar *
bytes_handler (eerec_map_t *rp, uchar *val, uchar *dp, uchar *edp)
{
	uchar bytes[HYMOD_MAX_BYTES], nbytes, *p;
	char *ep;

	for (nbytes = 0, p = val; *p != '\0'; p = (uchar *)ep) {
		ulong lval;

		lval = simple_strtol ((char *)p, &ep, 10);
		if ((uchar *)ep == p || (*ep != '\0' && *ep != ',') || \
		    lval >= 256) {
			printf ("%s rec (%s) byte array has invalid uint\n",
				rp->name, val);
			return (NULL);
		}
		if (nbytes >= HYMOD_MAX_BYTES) {
			printf ("%s rec (%s) byte array too long\n",
				rp->name, val);
			return (NULL);
		}
		bytes[nbytes++] = lval;

		if (*ep != '\0')
			ep++;
	}

	if (dp + 2 + nbytes > edp) {
		printf ("can't fit %s rec into eeprom\n", rp->name);
		return (NULL);
	}

	*dp++ = rp->type;
	*dp++ = nbytes;
	memcpy (dp, bytes, nbytes);
	dp += nbytes;

	return (dp);
}

static eerec_map_t eerec_map[] = {
	/* name      type                 handler         len max             */
	{ "serno",   HYMOD_EEREC_SERNO,   uint_handler,   4,  0               },
	{ "date",    HYMOD_EEREC_DATE,    date_handler,   4,  0               },
	{ "batch",   HYMOD_EEREC_BATCH,   string_handler, 0,  HYMOD_MAX_BATCH },
	{ "type",    HYMOD_EEREC_TYPE,    uint_handler,   1,  0               },
	{ "rev",     HYMOD_EEREC_REV,     uint_handler,   1,  0               },
	{ "sdram",   HYMOD_EEREC_SDRAM,   bytes_handler,  0,  HYMOD_MAX_SDRAM },
	{ "flash",   HYMOD_EEREC_FLASH,   bytes_handler,  0,  HYMOD_MAX_FLASH },
	{ "zbt",     HYMOD_EEREC_ZBT,     bytes_handler,  0,  HYMOD_MAX_ZBT   },
	{ "xlxtyp",  HYMOD_EEREC_XLXTYP,  bytes_handler,  0,  HYMOD_MAX_XLX   },
	{ "xlxspd",  HYMOD_EEREC_XLXSPD,  bytes_handler,  0,  HYMOD_MAX_XLX   },
	{ "xlxtmp",  HYMOD_EEREC_XLXTMP,  bytes_handler,  0,  HYMOD_MAX_XLX   },
	{ "xlxgrd",  HYMOD_EEREC_XLXGRD,  bytes_handler,  0,  HYMOD_MAX_XLX   },
	{ "cputyp",  HYMOD_EEREC_CPUTYP,  uint_handler,   1,  0               },
	{ "cpuspd",  HYMOD_EEREC_CPUSPD,  uint_handler,   1,  0               },
	{ "cpmspd",  HYMOD_EEREC_CPMSPD,  uint_handler,   1,  0               },
	{ "busspd",  HYMOD_EEREC_BUSSPD,  uint_handler,   1,  0               },
	{ "hstype",  HYMOD_EEREC_HSTYPE,  uint_handler,   1,  0               },
	{ "hschin",  HYMOD_EEREC_HSCHIN,  uint_handler,   1,  0               },
	{ "hschout", HYMOD_EEREC_HSCHOUT, uint_handler,   1,  0               },
};

static int neerecs = sizeof eerec_map / sizeof eerec_map[0];

static uchar data[HYMOD_EEPROM_SIZE], *sdp, *dp, *edp;

static int
eerec_callback (uchar *name, uchar *val)
{
	eerec_map_t *rp;

	for (rp = eerec_map; rp < &eerec_map[neerecs]; rp++)
		if (strcmp ((char *)name, rp->name) == 0)
			break;

	if (rp >= &eerec_map[neerecs])
		return (0);

	if ((dp = (*rp->handler) (rp, val, dp, edp)) == NULL)
		return (0);

	return (1);
}

static int
hymod_eeprom_fetch(int which, char *filename, ulong addr)
{
	unsigned dev_addr = CFG_I2C_EEPROM_ADDR | \
		(which ? HYMOD_EEOFF_MEZZ : HYMOD_EEOFF_MAIN);
	hymod_eehdr_t *hp = (hymod_eehdr_t *)&data[0];
	ulong crc;

	memset (hp, 0, sizeof *hp);
	hp->id = HYMOD_EEPROM_ID;
	hp->ver = HYMOD_EEPROM_VER;

	dp = sdp = (uchar *)(hp + 1);
	edp = dp + HYMOD_EEPROM_MAXLEN;

	if (fetch_and_parse (filename, addr, eerec_callback) == 0)
		return (0);

	hp->len = dp - sdp;

	crc = crc32 (0, data, dp - data);
	memcpy (dp, &crc, sizeof (ulong));
	dp += sizeof (ulong);

	eeprom_write (dev_addr, 0, data, dp - data);

	return (1);
}

static char *type_vals[] = {
	"NONE", "IO", "CLP", "DSP", "INPUT", "ALT-INPUT", "DISPLAY"
};

static char *xlxtyp_vals[] = {
	"NONE", "XCV300E", "XCV400E", "XCV600E"
};

static char *xlxspd_vals[] = {
	"NONE", "6", "7", "8"
};

static char *xlxtmp_vals[] = {
	"NONE", "COM", "IND"
};

static char *xlxgrd_vals[] = {
	"NONE", "NORMAL", "ENGSAMP"
};

static char *cputyp_vals[] = {
	"NONE", "MPC8260"
};

static char *clk_vals[] = {
	"NONE", "33", "66", "100", "133", "166", "200"
};

static char *hstype_vals[] = {
	"NONE", "AMCC-S2064A"
};

static void
print_mem (char *l, char *s, uchar n, uchar a[])
{
	if (n > 0) {
		if (n == 1)
			printf ("%s%dMB %s", s, 1 << (a[0] - 20), l);
		else {
			ulong t = 0;
			int i;

			for (i = 0; i < n; i++)
				t += 1 << (a[i] - 20);

			printf ("%s%luMB %s (%d banks:", s, t, l, n);

			for (i = 0; i < n; i++)
				printf ("%dMB%s",
					1 << (a[i] - 20),
					(i == n - 1) ? ")" : ",");
		}
	}
	else
		printf ("%sNO %s", s, l);
}

void
hymod_eeprom_print (hymod_eeprom_t *ep)
{
	int i;

	printf ("         Hymod %s board, rev %03d\n",
		type_vals[ep->bdtype], ep->bdrev);

	printf ("         serial #: %010lu, date %04d-%02d-%02d",
		ep->serno, ep->date.year, ep->date.month, ep->date.day);
	if (ep->batchlen > 0)
		printf (", batch \"%.*s\"", ep->batchlen, ep->batch);
	puts ("\n");

	switch (ep->bdtype) {

	case HYMOD_BDTYPE_IO:
	case HYMOD_BDTYPE_CLP:
	case HYMOD_BDTYPE_DSP:
		printf ("         Motorola %s CPU, speeds: %s/%s/%s",
		    cputyp_vals[ep->mpc.type], clk_vals[ep->mpc.cpuspd],
		    clk_vals[ep->mpc.cpmspd], clk_vals[ep->mpc.busspd]);

		print_mem ("SDRAM", ", ", ep->nsdram, ep->sdramsz);

		print_mem ("FLASH", ", ", ep->nflash, ep->flashsz);

		puts ("\n");

		print_mem ("ZBT", "         ", ep->nzbt, ep->zbtsz);

		if (ep->nxlx > 0) {
			hymod_xlx_t *xp;

			if (ep->nxlx == 1) {
				xp = &ep->xlx[0];
				printf (", Xilinx %s FPGA (%s/%s/%s)",
					xlxtyp_vals[xp->type],
					xlxspd_vals[xp->speed],
					xlxtmp_vals[xp->temp],
					xlxgrd_vals[xp->grade]);
			}
			else {
				printf (", %d Xilinx FPGAs (", ep->nxlx);
				for (i = 0; i < ep->nxlx; i++) {
					xp = &ep->xlx[i];
					printf ("%s[%s/%s/%s]%s",
					    xlxtyp_vals[xp->type],
					    xlxspd_vals[xp->speed],
					    xlxtmp_vals[xp->temp],
					    xlxgrd_vals[xp->grade],
					    (i == ep->nxlx - 1) ? ")" : ", ");
				}
			}
		}
		else
			puts(", NO FPGAs");

		puts ("\n");

		if (ep->hss.type > 0)
			printf ("         High Speed Serial: "
				"%s, %d input%s, %d output%s\n",
				hstype_vals[ep->hss.type],
				ep->hss.nchin,
				(ep->hss.nchin == 1 ? "" : "s"),
				ep->hss.nchout,
				(ep->hss.nchout == 1 ? "" : "s"));
		break;

	case HYMOD_BDTYPE_INPUT:
	case HYMOD_BDTYPE_ALTINPUT:
	case HYMOD_BDTYPE_DISPLAY:
		break;

	default:
		/* crap! */
		printf ("         UNKNOWN BOARD TYPE: %d\n", ep->bdtype);
		break;
	}
}

int
hymod_eeprom_read (int which, hymod_eeprom_t *ep)
{
	char *label = which ? "mezzanine" : "main";
	unsigned dev_addr = CFG_I2C_EEPROM_ADDR | \
		(which ? HYMOD_EEOFF_MEZZ : HYMOD_EEOFF_MAIN);
	char filename[50], prompt[50], *dir;
	int serno, count = 0, rc;

	rc = eeprom_probe (dev_addr, 0);

	if (rc > 0) {
		printf ("*** probe for eeprom failed with code %d\n", rc);
		return (0);
	}

	if (rc < 0)
		return (rc);

	sprintf (prompt, "Enter %s board serial number: ", label);

	if ((dir = getenv ("bddb_cfgdir")) == NULL)
		dir = def_bddb_cfgdir;

	for (;;) {
		int rc;

		if (hymod_eeprom_load (which, ep))
			return (1);

		printf ("*** %s board EEPROM contents are %sinvalid\n",
			label, count == 0 ? "" : "STILL ");

		puts ("*** will fetch from server (Ctrl-C to abort)\n");

		serno = hymod_get_serno (prompt);

		if (serno < 0) {
			if (serno == -1)
				puts ("\n*** interrupted!");
			else
				puts ("\n*** timeout!");
			puts (" - ignoring eeprom contents\n");
			return (0);
		}

		sprintf (filename, "%s/%010d.cfg", dir, serno);

		printf ("*** fetching %s board EEPROM contents from server\n",
			label);

		rc = hymod_eeprom_fetch (which, filename, CFG_LOAD_ADDR);

		if (rc == 0) {
			puts ("*** fetch failed - ignoring eeprom contents\n");
			return (0);
		}

		count++;
	}
}
