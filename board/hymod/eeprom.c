/*
 * (C) Copyright 2001
 * Murray Jensen, CSIRO Manufacturing Science and Technology,
 * <Murray.Jensen@cmst.csiro.au>
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
extern int fetch_and_parse(bd_t *, char *, ulong, int (*)(uchar *, uchar *));

int
eeprom_load(unsigned offset, hymod_eeprom_t *ep)
{
    uchar data[HYMOD_EEPROM_SIZE], *dp, *edp;
    hymod_eehdr_t *hp;
    ulong len, crc;

    memset(ep, 0, sizeof *ep);
    memset(data, 0, HYMOD_EEPROM_SIZE);
    crc = 0;

    hp = (hymod_eehdr_t *)data;
    eeprom_read(CFG_DEF_EEPROM_ADDR, offset, (uchar *)hp, sizeof (*hp));
    offset += sizeof (*hp);

    if (hp->id != HYMOD_EEPROM_ID || hp->ver > HYMOD_EEPROM_VER ||
      (len = hp->len) > HYMOD_EEPROM_MAXLEN)
	return (0);

    dp = (uchar *)(hp + 1); edp = dp + len;
    eeprom_read(CFG_DEF_EEPROM_ADDR, offset, dp, len);
    offset += len;

    eeprom_read(CFG_DEF_EEPROM_ADDR, offset, (uchar *)&crc, sizeof (ulong));

    if (crc32(0, data, edp - data) != crc)
	return (0);

    ep->ver = hp->ver;

    for (;;) {
	hymod_eerec_t *rp = (hymod_eerec_t *)dp;
	ulong rtyp;
	uchar rlen, *rdat;
	uint rsiz;

	if (rp->small.topbit == 0) {
	    rtyp = rp->small.type;
	    rlen = rp->small.len;
	    rdat = rp->small.data;
	    rsiz = offsetof(hymod_eerec_t, small.data) + rlen;
	}
	else if (rp->medium.nxtbit == 0) {
	    rtyp = rp->medium.type;
	    rlen = rp->medium.len;
	    rdat = rp->medium.data;
	    rsiz = offsetof(hymod_eerec_t, medium.data) + rlen;
	}
	else {
	    rtyp = rp->large.type;
	    rlen = rp->large.len;
	    rdat = rp->large.data;
	    rsiz = offsetof(hymod_eerec_t, large.data) + rlen;
	}

	if (rtyp == 0)
	    break;

	dp += rsiz;
	if (dp > edp)	/* error? */
	    break;

	switch (rtyp) {

	case HYMOD_EEREC_SERNO:		/* serial number */
	    if (rlen == sizeof (ulong))
		memcpy(&ep->serno, rdat, sizeof (ulong));
	    break;

	case HYMOD_EEREC_DATE:		/* date */
	    if (rlen == sizeof (hymod_date_t))
		memcpy(&ep->date, rdat, sizeof (hymod_date_t));
	    break;

	case HYMOD_EEREC_BATCH:		/* batch */
	    if (rlen <= HYMOD_MAX_BATCH)
		memcpy(ep->batch, rdat, ep->batchlen = rlen);
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

	case HYMOD_EEREC_HSTYPE:	/* high-speed serial chip type */
	    if (rlen == 1)
		ep->hss.type = *rdat;
	    break;

	case HYMOD_EEREC_HSCHIN:	/* high-speed serial input channels */
	    if (rlen == 1)
		ep->hss.nchin = *rdat;
	    break;

	case HYMOD_EEREC_HSCHOUT:	/* high-speed serial output channels */
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
	uchar *(*handler)(struct _eerec_map *, uchar *, uchar *, uchar *);
	uint length;
	uint maxlen;
    }
eerec_map_t;

static uchar *
uint_handler(eerec_map_t *rp, uchar *value, uchar *dp, uchar *edp)
{
    uchar *eval;
    union {
	uchar cval[4];
	ushort sval[2];
	ulong lval;
    } rdata;

    rdata.lval = simple_strtol(value, (char **)&eval, 10);

    if (eval == value || *eval != '\0') {
	printf("%s record (%s) is not a valid uint\n", rp->name, value);
	return (NULL);
    }

    if (dp + 2 + rp->length > edp) {
	printf("can't fit %s record into eeprom\n", rp->name);
	return (NULL);
    }

    *dp++ = rp->type;
    *dp++ = rp->length;

    switch (rp->length) {

    case 1:
	if (rdata.lval >= 256) {
	    printf("%s record value (%lu) out of range (0-255)\n",
		rp->name, rdata.lval);
	    return (NULL);
	}
	*dp++ = rdata.cval[3];
	break;

    case 2:
	if (rdata.lval >= 65536) {
	    printf("%s record value (%lu) out of range (0-65535)\n",
		rp->name, rdata.lval);
	    return (NULL);
	}
	memcpy(dp, &rdata.sval[1], 2);
	dp += 2;
	break;

    case 4:
	memcpy(dp, &rdata.lval, 4);
	dp += 4;
	break;

    default:
	printf("huh? rp->length not 1, 2 or 4! (%d)\n", rp->length);
	return (NULL);
    }

    return (dp);
}

static uchar *
date_handler(eerec_map_t *rp, uchar *value, uchar *dp, uchar *edp)
{
    hymod_date_t date;
    uchar *p = value, *ep;

    date.year = simple_strtol(p, (char **)&ep, 10);
    if (ep == p || *ep++ != '-') {
bad_date:
	printf("%s record (%s) is not a valid date\n", rp->name, value);
	return (NULL);
    }

    date.month = simple_strtol(p = ep, (char **)&ep, 10);
    if (ep == p || *ep++ != '-' || date.month == 0 || date.month > 12)
	goto bad_date;

    date.day = simple_strtol(p = ep, (char **)&ep, 10);
    if (ep == p || *ep != '\0' || date.day == 0 || date.day > 31)
	goto bad_date;

    if (dp + 2 + sizeof (hymod_date_t) > edp) {
	printf("can't fit %s record into eeprom\n", rp->name);
	return (NULL);
    }

    *dp++ = rp->type;
    *dp++ = sizeof (hymod_date_t);
    memcpy(dp, &date, sizeof (hymod_date_t));
    dp += sizeof (hymod_date_t);

    return (dp);
}

static uchar *
string_handler(eerec_map_t *rp, uchar *value, uchar *dp, uchar *edp)
{
    uint len;

    if ((len = strlen(value)) > rp->maxlen) {
	printf("%s record (%s) string is too long (%d>%d)\n",
	    rp->name, value, len, rp->maxlen);
	return (NULL);
    }

    if (dp + 2 + len > edp) {
	printf("can't fit %s record into eeprom\n", rp->name);
	return (NULL);
    }

    *dp++ = rp->type;
    *dp++ = len;
    memcpy(dp, value, len);
    dp += len;

    return (dp);
}

static uchar *
bytes_handler(eerec_map_t *rp, uchar *value, uchar *dp, uchar *edp)
{
    uchar bytes[HYMOD_MAX_BYTES], nbytes = 0;
    uchar *p = value, *ep;

    for (;;) {

	if (nbytes >= HYMOD_MAX_BYTES) {
	    printf("%s record (%s) byte array too long\n", rp->name, value);
	    return (NULL);
	}

	bytes[nbytes++] = simple_strtol(p, (char **)&ep, 10);

	if (ep == p || (*ep != '\0' && *ep != ',')) {
	    printf("%s record (%s) byte array has invalid uint\n",
		rp->name, value);
	    return (NULL);
	}

	if (*ep++ == '\0')
	    break;

	p = ep;
    }

    if (dp + 2 + nbytes > edp) {
	printf("can't fit %s record into eeprom\n", rp->name);
	return (NULL);
    }

    *dp++ = rp->type;
    *dp++ = nbytes;
    memcpy(dp, bytes, nbytes);
    dp += nbytes;

    return (dp);
}

static eerec_map_t eerec_map[] = {
   /* name       type                 handler         len max             */
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
eeprom_fetch_callback(uchar *name, uchar *value)
{
    eerec_map_t *rp;

    for (rp = eerec_map; rp < &eerec_map[neerecs]; rp++)
	if (strcmp(name, rp->name) == 0)
	    break;

    if (rp >= &eerec_map[neerecs])
	return (0);

    if ((dp = (*rp->handler)(rp, value, dp, edp)) == NULL)
	return (0);

    return (1);
}

int
eeprom_fetch(unsigned offset, bd_t *bd, char *filename, ulong addr)
{
    hymod_eehdr_t *hp = (hymod_eehdr_t *)&data[0];
    ulong crc;

    hp->id = HYMOD_EEPROM_ID;
    hp->ver = HYMOD_EEPROM_VER;

    dp = sdp = (uchar *)(hp + 1);
    edp = dp + HYMOD_EEPROM_MAXLEN;

    if (fetch_and_parse(bd, filename, addr, eeprom_fetch_callback) == 0)
	return (0);

    hp->len = dp - sdp;

    crc = crc32(0, data, dp - data);
    memcpy(dp, &crc, sizeof (ulong));
    dp += sizeof (ulong);

    eeprom_write(CFG_DEF_EEPROM_ADDR, offset, data, dp - data);

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
print_mem(char *l, char *s, uchar n, uchar a[])
{
    if (n > 0) {
	if (n == 1)
	    printf("%s%dMB %s", s, 1 << (a[0] - 20), l);
	else {
	    ulong t = 0;
	    int i;

	    for (i = 0; i < n; i++)
		t += 1 << (a[i] - 20);

	    printf("%s%luMB %s (%d banks:", s, t, l, n);

	    for (i = 0; i < n; i++)
		printf("%dMB%s", 1 << (a[i] - 20), (i == n - 1) ? ")" : ",");
	}
    }
    else
	printf("%sNO %s", s, l);
}

void
eeprom_print(hymod_eeprom_t *ep)
{
    int i;

    printf("         Hymod %s board, rev %03d\n",
	type_vals[ep->bdtype], ep->bdrev);

    printf("         serial #: %010lu, date %04d-%02d-%02d",
	ep->serno, ep->date.year, ep->date.month, ep->date.day);
    if (ep->batchlen > 0)
	printf(", batch \"%.*s\"", ep->batchlen, ep->batch);
    puts("\n");

    switch (ep->bdtype) {

    case HYMOD_BDTYPE_IO:
    case HYMOD_BDTYPE_CLP:
    case HYMOD_BDTYPE_DSP:
	printf("         Motorola %s CPU, speeds: %s/%s/%s",
	    cputyp_vals[ep->mpc.type], clk_vals[ep->mpc.cpuspd],
	    clk_vals[ep->mpc.cpmspd], clk_vals[ep->mpc.busspd]);

	print_mem("SDRAM", ", ", ep->nsdram, ep->sdramsz);

	print_mem("FLASH", ", ", ep->nflash, ep->flashsz);

	puts("\n");

	print_mem("ZBT", "         ", ep->nzbt, ep->zbtsz);

	if (ep->nxlx > 0) {
	    hymod_xlx_t *xp;

	    if (ep->nxlx == 1) {
		xp = &ep->xlx[0];
		printf(", Xilinx %s FPGA (%s/%s/%s)",
		    xlxtyp_vals[xp->type], xlxspd_vals[xp->speed],
		    xlxtmp_vals[xp->temp], xlxgrd_vals[xp->grade]);
	    }
	    else {
		printf(", %d Xilinx FPGAs (", ep->nxlx);
		for (i = 0; i < ep->nxlx; i++) {
		    xp = &ep->xlx[i];
		    printf("%s[%s/%s/%s]%s",
			xlxtyp_vals[xp->type], xlxspd_vals[xp->speed],
			xlxtmp_vals[xp->temp], xlxgrd_vals[xp->grade],
			(i == ep->nxlx - 1) ? ")" : ", ");
		}
	    }
	}
	else
	    puts(", NO FPGAs");

	puts("\n");

	if (ep->hss.type > 0)
	    printf("         High Speed Serial: %s, %d input%s, %d output%s\n",
		hstype_vals[ep->hss.type],
		ep->hss.nchin, (ep->hss.nchin == 1 ? "" : "s"),
		ep->hss.nchout, (ep->hss.nchout == 1 ? "" : "s"));
	break;

    case HYMOD_BDTYPE_INPUT:
    case HYMOD_BDTYPE_ALTINPUT:
    case HYMOD_BDTYPE_DISPLAY:
	break;

    default:
	/* crap! */
	printf("         UNKNOWN BOARD TYPE: %d\n", ep->bdtype);
	break;
    }
}
