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

#if (CONFIG_COMMANDS & CFG_CMD_FDOS)
#include <linux/ctype.h>

#include "dos.h"
#include "fdos.h"

static int dir_read (Fs_t *fs,
		     Slot_t *dir,
		     Directory_t *dirent,
		     int num,
		     struct vfat_state *v);

static int unicode_read (char *in, char *out, int num);
static int match (const char *s, const char *p);
static unsigned char sum_shortname (char *name);
static int check_vfat (struct vfat_state *v, Directory_t *dir);
static char *conv_name (char *name, char *ext, char Case, char *ans);


/*-----------------------------------------------------------------------------
 * clear_vfat --
 *-----------------------------------------------------------------------------
 */
static void clear_vfat (struct vfat_state *v)
{
    v -> subentries = 0;
    v -> status = 0;
}

/*-----------------------------------------------------------------------------
 * vfat_lookup --
 *-----------------------------------------------------------------------------
 */
int vfat_lookup (Slot_t *dir,
		 Fs_t *fs,
		 Directory_t *dirent,
		 int *entry,
		 int *vfat_start,
		 char *filename,
		 int flags,
		 char *outname,
		 Slot_t *file)
{
    int found;
    struct vfat_state vfat;
    char newfile [VSE_NAMELEN];
    int vfat_present = 0;

    if (*entry == -1) {
	return -1;
    }

    found = 0;
    clear_vfat (&vfat);
    while (1) {
	if (dir_read (fs, dir, dirent, *entry, &vfat) < 0) {
	    if (vfat_start) {
		*vfat_start = *entry;
	    }
	    break;
	}
	(*entry)++;

	/* Empty slot                                                        */
	if (dirent -> name[0] == '\0'){
	    if (vfat_start == 0) {
		break;
	    }
	    continue;
	}

	if (dirent -> attr == ATTR_VSE) {
	    /* VSE entry, continue                                           */
	    continue;
	}
	if ( (dirent -> name [0] == DELMARK) ||
	     ((dirent -> attr & ATTR_DIRECTORY) != 0 &&
	      (flags & ACCEPT_DIR) == 0) ||
	     ((dirent -> attr & ATTR_VOLUME) != 0 &&
	      (flags & ACCEPT_LABEL) == 0) ||
	     (((dirent -> attr & (ATTR_DIRECTORY | ATTR_VOLUME)) == 0) &&
	      (flags & ACCEPT_PLAIN) == 0)) {
	    clear_vfat (&vfat);
	    continue;
	}

	vfat_present = check_vfat (&vfat, dirent);
	if (vfat_start) {
	    *vfat_start = *entry - 1;
	    if (vfat_present) {
		*vfat_start -= vfat.subentries;
	    }
	}

	if (dirent -> attr & ATTR_VOLUME) {
	    strncpy (newfile, dirent -> name, 8);
	    newfile [8] = '\0';
	    strncat (newfile, dirent -> ext, 3);
	    newfile [11] = '\0';
	}
	else {
	    conv_name (dirent -> name, dirent -> ext, dirent -> Case, newfile);
	}

	if (flags & MATCH_ANY) {
	    found = 1;
	    break;
	}

	if ((vfat_present && match (vfat.name, filename)) ||
	    (match (newfile, filename))) {
	    found = 1;
	    break;
	}
	clear_vfat (&vfat);
    }

    if (found) {
	if ((flags & DO_OPEN) && file) {
	    if (open_file (file, dirent) < 0) {
		return (-1);
	    }
	}
	if (outname) {
	    if (vfat_present) {
		strcpy (outname, vfat.name);
	    }
	    else {
		strcpy (outname, newfile);
	    }
	}
	return (0);                    /* File found                         */
    } else {
	*entry = -1;
	return -1;                      /* File not found                    */
    }
}

/*-----------------------------------------------------------------------------
 * dir_read -- Read one directory entry
 *-----------------------------------------------------------------------------
 */
static int dir_read (Fs_t *fs,
	      Slot_t *dir,
	      Directory_t *dirent,
	      int num,
	      struct vfat_state *v)
{

    /* read the directory entry                                              */
    if (read_file (fs,
		   dir,
		   (char *)dirent,
		   num * MDIR_SIZE,
		   MDIR_SIZE) != MDIR_SIZE) {
	return (-1);
    }

    if (v && (dirent -> attr == ATTR_VSE)) {
	struct vfat_subentry *vse;
	unsigned char id, last_flag;
	char *c;

	vse = (struct vfat_subentry *) dirent;
	id = vse -> id & VSE_MASK;
	last_flag = (vse -> id & VSE_LAST);
	if (id > MAX_VFAT_SUBENTRIES) {
	    /* Invalid VSE entry                                             */
	    return (-1);
	}


	/* Decode VSE                                                        */
	if(v -> sum != vse -> sum) {
	    clear_vfat (v);
	    v -> sum = vse -> sum;
	}


	v -> status |= 1 << (id - 1);
	if (last_flag) {
	    v -> subentries = id;
	}

	c = &(v -> name [VSE_NAMELEN * (id - 1)]);
	c += unicode_read (vse->text1, c, VSE1SIZE);
	c += unicode_read (vse->text2, c, VSE2SIZE);
	c += unicode_read (vse->text3, c, VSE3SIZE);

	if (last_flag) {
	    *c = '\0';	        /* Null terminate long name                  */
	}

    }
    return (0);
}

/*-----------------------------------------------------------------------------
 * unicode_read --
 *-----------------------------------------------------------------------------
 */
static int unicode_read (char *in, char *out, int num)
{
    int j;

    for (j = 0; j < num; ++j) {
	if (in [1])
	    *out = '_';
	else
	    *out = in [0];
	out ++;
	in += 2;
    }
    return num;
}

/*-----------------------------------------------------------------------------
 * match --
 *-----------------------------------------------------------------------------
 */
static int match (const char *s, const char *p)
{

    for (; *p != '\0'; ) {
	if (toupper (*s) != toupper (*p)) {
	    return (0);
	}
	p++;
	s++;
    }

    if (*s != '\0') {
	return (0);
    }
    else {
	return (1);
    }
}
/*-----------------------------------------------------------------------------
 * sum_shortname --
 *-----------------------------------------------------------------------------
 */
static unsigned char sum_shortname (char *name)
{
    unsigned char sum;
    int j;

    for (j = sum = 0; j < 11; ++j) {
	sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) +
	    (name [j] ? name [j] : ' ');
    }
    return (sum);
}
/*-----------------------------------------------------------------------------
 * check_vfat --
 * Return 1 if long name is valid, 0 else
 *-----------------------------------------------------------------------------
 */
static int check_vfat (struct vfat_state *v, Directory_t *dir)
{
    char name[12];

    if (v -> subentries == 0) {
	return 0;
    }

    strncpy (name, dir -> name, 8);
    strncpy (name + 8, dir -> ext, 3);
    name [11] = '\0';

    if (v -> sum != sum_shortname (name)) {
	return 0;
    }

    if( (v -> status & ((1 << v -> subentries) - 1)) !=
	(1 << v -> subentries) - 1) {
	return 0;
    }
    v->name [VSE_NAMELEN * v -> subentries] = 0;

    return 1;
}
/*-----------------------------------------------------------------------------
 * conv_name --
 *-----------------------------------------------------------------------------
 */
static char *conv_name (char *name, char *ext, char Case, char *ans)
{
    char tname [9], text [4];
    int i;

    i = 0;
    while (i < 8 && name [i] != ' ' && name [i] != '\0') {
	tname [i] = name [i];
	i++;
    }
    tname [i] = '\0';

    if (Case & BASECASE) {
	for (i = 0; i < 8 && tname [i]; i++) {
	    tname [i] = tolower (tname [i]);
	}
    }

    i = 0;
    while (i < 3 && ext [i] != ' ' && ext [i] != '\0') {
	text [i] = ext [i];
	i++;
    }
    text [i] = '\0';

    if (Case & EXTCASE){
	for (i = 0; i < 3 && text [i]; i++) {
	    text [i] = tolower (text [i]);
	}
    }

    if (*text) {
	strcpy (ans, tname);
	strcat (ans, ".");
	strcat (ans, text);
    }
    else {
	strcpy(ans, tname);
    }
    return (ans);
}


#endif
