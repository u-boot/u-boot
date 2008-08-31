/*
 * (C) Copyright 2002
 * Stäubli Faverges - <www.staubli.com>
 * Pierre AUBERT  p.aubert@staubli.com
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
#include <config.h>

#include "dos.h"
#include "fdos.h"

#define NB_HEADS        2
#define NB_TRACKS       80
#define NB_SECTORS      18


static int lastwhere;

/*-----------------------------------------------------------------------------
 * dev_open --
 *-----------------------------------------------------------------------------
 */
int dev_open (void)
{
    lastwhere = 0;
    return (0);
}

/*-----------------------------------------------------------------------------
 * dev_read -- len and where are sectors number
 *-----------------------------------------------------------------------------
 */
int dev_read (void *buffer, int where, int len)
{
    PRINTF ("dev_read (len = %d, where = %d)\n", len, where);

    /* Si on ne desire pas lire a la position courante, il faut un seek      */
    if (where != lastwhere) {
	if (!fdc_fdos_seek (where)) {
	    PRINTF ("seek error in dev_read");
	    lastwhere = -1;
	    return (-1);
	}
    }

    if (!fdc_fdos_read (buffer, len)) {
	PRINTF ("read error\n");
	lastwhere = -1;
	return (-1);
    }
    lastwhere = where + len;
    return (0);
}
/*-----------------------------------------------------------------------------
 * check_dev -- verify the diskette format
 *-----------------------------------------------------------------------------
 */
int check_dev (BootSector_t *boot, Fs_t *fs)
{
    unsigned int heads, sectors, tracks;
    int BootP, Infp0, InfpX, InfTm;
    int sect_per_track;

    /* Display Boot header                                                   */
    PRINTF ("Jump to boot code                  0x%02x 0x%02x 0x%02x\n",
	    boot -> jump [0], boot -> jump [1], boot -> jump[2]);
    PRINTF ("OEM name & version                 '%*.*s'\n",
	    BANNER_LG, BANNER_LG, boot -> banner );
    PRINTF ("Bytes per sector hopefully 512     %d\n",
	    __le16_to_cpu (boot -> secsiz));
    PRINTF ("Cluster size in sectors            %d\n",
	    boot -> clsiz);
    PRINTF ("Number of reserved (boot) sectors  %d\n",
	    __le16_to_cpu (boot -> nrsvsect));
    PRINTF ("Number of FAT tables hopefully 2   %d\n",
	    boot -> nfat);
    PRINTF ("Number of directory slots          %d\n",
	    __le16_to_cpu (boot -> dirents));
    PRINTF ("Total sectors on disk              %d\n",
	    __le16_to_cpu (boot -> psect));
    PRINTF ("Media descriptor=first byte of FAT %d\n",
	    boot -> descr);
    PRINTF ("Sectors in FAT                     %d\n",
	    __le16_to_cpu (boot -> fatlen));
    PRINTF ("Sectors/track                      %d\n",
	    __le16_to_cpu (boot -> nsect));
    PRINTF ("Heads                              %d\n",
	    __le16_to_cpu (boot -> nheads));
    PRINTF ("number of hidden sectors           %d\n",
	    __le32_to_cpu (boot -> nhs));
    PRINTF ("big total sectors                  %d\n",
	    __le32_to_cpu (boot -> bigsect));
    PRINTF ("physical drive ?                   %d\n",
	    boot -> physdrive);
    PRINTF ("reserved                           %d\n",
	    boot -> reserved);
    PRINTF ("dos > 4.0 diskette                 %d\n",
	    boot -> dos4);
    PRINTF ("serial number                      %d\n",
	    __le32_to_cpu (boot -> serial));
    PRINTF ("disk label                         %*.*s\n",
	    LABEL_LG, LABEL_LG, boot -> label);
    PRINTF ("FAT type                           %8.8s\n",
	    boot -> fat_type);
    PRINTF ("reserved by 2M                     %d\n",
	    boot -> res_2m);
    PRINTF ("2M checksum (not used)             %d\n",
	    boot -> CheckSum);
    PRINTF ("2MF format version                 %d\n",
	    boot -> fmt_2mf);
    PRINTF ("1 if write track after format      %d\n",
	    boot -> wt);
    PRINTF ("data transfer rate on track 0      %d\n",
	    boot -> rate_0);
    PRINTF ("data transfer rate on track<>0     %d\n",
	    boot -> rate_any);
    PRINTF ("offset to boot program             %d\n",
	    __le16_to_cpu (boot -> BootP));
    PRINTF ("T1: information for track 0        %d\n",
	    __le16_to_cpu (boot -> Infp0));
    PRINTF ("T2: information for track<>0       %d\n",
	    __le16_to_cpu (boot -> InfpX));
    PRINTF ("T3: track sectors size table       %d\n",
	    __le16_to_cpu (boot -> InfTm));
    PRINTF ("Format date                        0x%04x\n",
	    __le16_to_cpu (boot -> DateF));
    PRINTF ("Format time                        0x%04x\n",
	    __le16_to_cpu (boot -> TimeF));


    /* information is extracted from boot sector                           */
    heads = __le16_to_cpu (boot -> nheads);
    sectors = __le16_to_cpu (boot -> nsect);
    fs -> tot_sectors = __le32_to_cpu (boot -> bigsect);
    if (__le16_to_cpu (boot -> psect) != 0) {
	fs -> tot_sectors = __le16_to_cpu (boot -> psect);
    }

    sect_per_track = heads * sectors;
    tracks = (fs -> tot_sectors + sect_per_track - 1) / sect_per_track;

    BootP = __le16_to_cpu (boot -> BootP);
    Infp0 = __le16_to_cpu (boot -> Infp0);
    InfpX = __le16_to_cpu (boot -> InfpX);
    InfTm = __le16_to_cpu (boot -> InfTm);

    if (boot -> dos4 == EXTENDED_BOOT &&
	strncmp( boot->banner,"2M", 2 ) == 0 &&
	BootP < SZ_STD_SECTOR &&
	Infp0 < SZ_STD_SECTOR &&
	InfpX < SZ_STD_SECTOR &&
	InfTm < SZ_STD_SECTOR &&
	BootP >= InfTm + 2 &&
	InfTm >= InfpX &&
	InfpX >= Infp0 &&
	Infp0 >= 76 ) {

	return (-1);
    }

    if (heads != NB_HEADS ||
	tracks != NB_TRACKS ||
	sectors != NB_SECTORS ||
	__le16_to_cpu (boot -> secsiz) != SZ_STD_SECTOR ||
	fs -> tot_sectors == 0 ||
	(fs -> tot_sectors % sectors) != 0) {
	return (-1);
    }

    return (0);
}
