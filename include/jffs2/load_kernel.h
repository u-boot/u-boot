#ifndef load_kernel_h
#define load_kernel_h
/*-------------------------------------------------------------------------
 * Filename:      load_kernel.h
 * Version:       $Id: load_kernel.h,v 1.3 2002/01/25 01:34:11 nyet Exp $
 * Copyright:     Copyright (C) 2001, Russ Dill
 * Author:        Russ Dill <Russ.Dill@asu.edu>
 * Description:   header for load kernel modules
 *-----------------------------------------------------------------------*/
/*
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

/* this struct is very similar to mtd_info */
struct part_info {
	u32 size;	 /* Total size of the Partition */

	/* "Major" erase size for the device. Naïve users may take this
	 * to be the only erase size available, or may use the more detailed
	 * information below if they desire
	 */
	u32 erasesize;

	/* Where in memory does this partition start? */
	char *offset;

	/* used by jffs2 set to NULL */
	void *jffs2_priv;

	/* private filed used by user */
	void *usr_priv;
};

struct part_info*
jffs2_part_info(int part_num);

struct kernel_loader {

	/* Return true if there is a kernel contained at src */
	int (* check_magic)(struct part_info *part);

	/* load the kernel from the partition part to dst, return the number
	 * of bytes copied if successful, zero if not */
	u32 (* load_kernel)(u32 *dst, struct part_info *part, const char *kernel_filename);

	/* A brief description of the module (ie, "cramfs") */
	char *name;
};

#define ldr_strlen	strlen
#define ldr_strncmp	strncmp
#define ldr_memcpy	memcpy
#define putstr(x)	printf("%s", x)
#define mmalloc		malloc
#define UDEBUG		printf

#define putnstr(str, size)	printf("%*.*s", size, size, str)
#define ldr_output_string(x)	puts(x)
#define putLabeledWord(x, y)	printf("%s %08x\n", x, (unsigned int)y)
#define led_blink(x, y, z, a)

#endif /* load_kernel_h */
