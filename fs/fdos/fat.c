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
#include <malloc.h>

#if (CONFIG_COMMANDS & CFG_CMD_FDOS) || defined(CONFIG_CMD_FDOS)

#include "dos.h"
#include "fdos.h"


/*-----------------------------------------------------------------------------
 * fat_decode --
 *-----------------------------------------------------------------------------
 */
unsigned int fat_decode (Fs_t *fs, unsigned int num)
{
    unsigned int start = num * 3 / 2;
    unsigned char *address = fs -> fat_buf + start;

    if (num < 2 || start + 1 > (fs -> fat_len * SZ_STD_SECTOR))
	return 1;

    if (num & 1)
	return ((address [1] & 0xff) << 4) | ((address [0] & 0xf0 ) >> 4);
    else
	return ((address [1] & 0xf) << 8) | (address [0] & 0xff );
}
/*-----------------------------------------------------------------------------
 * check_fat --
 *-----------------------------------------------------------------------------
 */
static int check_fat (Fs_t *fs)
{
    int i, f;

    /* Cluster verification                                                  */
    for (i = 3 ; i < fs -> num_clus; i++){
	f = fat_decode (fs, i);
	if (f < FAT12_LAST && f > fs -> num_clus){
	    /* Wrong cluster number detected                                 */
	    return (-1);
	}
    }
    return (0);
}
/*-----------------------------------------------------------------------------
 * read_one_fat --
 *-----------------------------------------------------------------------------
 */
static int read_one_fat (BootSector_t *boot, Fs_t *fs, int nfat)
{
    if (dev_read (fs -> fat_buf,
		  (fs -> fat_start + nfat * fs -> fat_len),
		  fs -> fat_len) < 0) {
	return (-1);
    }

    if (fs -> fat_buf [0] || fs -> fat_buf [1] || fs -> fat_buf [2]) {
	if ((fs -> fat_buf [0] != boot -> descr &&
	     (fs -> fat_buf [0] != 0xf9 || boot -> descr != MEDIA_STD)) ||
	    fs -> fat_buf [0] < MEDIA_STD){
	    /* Unknown Media                                                 */
	    return (-1);
	}
	if (fs -> fat_buf [1] != 0xff || fs -> fat_buf [2] != 0xff){
	    /* FAT doesn't start with good values                            */
	    return (-1);
	}
    }

    if (fs -> num_clus >= FAT12_MAX_NB) {
	/* Too much clusters                                                 */
	return (-1);
    }

    return check_fat (fs);
}
/*-----------------------------------------------------------------------------
 * read_fat --
 *-----------------------------------------------------------------------------
 */
int read_fat (BootSector_t *boot, Fs_t *fs)
{
    unsigned int buflen;
    int i;

    /* Allocate Fat Buffer                                                   */
    buflen = fs -> fat_len * SZ_STD_SECTOR;
    if (fs -> fat_buf) {
	free (fs -> fat_buf);
    }

    if ((fs -> fat_buf = malloc (buflen)) == NULL) {
	return (-1);
    }

    /* Try to read each Fat                                                  */
    for (i = 0; i< fs -> nb_fat; i++){
	if (read_one_fat (boot, fs, i) == 0) {
	    /* Fat is OK                                                     */
	    fs -> num_fat = i;
	    break;
	}
    }

    if (i == fs -> nb_fat){
	return (-1);
    }

    if (fs -> fat_len > (((fs -> num_clus + 2) *
			  (FAT_BITS / 4) -1 ) / 2 /
			 SZ_STD_SECTOR + 1)) {
	return (-1);
    }
    return (0);
}

#endif
