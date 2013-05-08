/*
 * Copyright (c) 2013, Google Inc.
 *
 * Copyright (C) 2011
 * Corscience GmbH & Co. KG - Simon Schwarz <schwarz@corscience.de>
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
#ifndef ARM_BOOTM_H
#define ARM_BOOTM_H

extern void udc_disconnect(void);

#if defined(CONFIG_SETUP_MEMORY_TAGS) || \
		defined(CONFIG_CMDLINE_TAG) || \
		defined(CONFIG_INITRD_TAG) || \
		defined(CONFIG_SERIAL_TAG) || \
		defined(CONFIG_REVISION_TAG)
# define BOOTM_ENABLE_TAGS		1
#else
# define BOOTM_ENABLE_TAGS		0
#endif

#ifdef CONFIG_SETUP_MEMORY_TAGS
# define BOOTM_ENABLE_MEMORY_TAGS	1
#else
# define BOOTM_ENABLE_MEMORY_TAGS	0
#endif

#ifdef CONFIG_CMDLINE_TAG
 #define BOOTM_ENABLE_CMDLINE_TAG	1
#else
 #define BOOTM_ENABLE_CMDLINE_TAG	0
#endif

#ifdef CONFIG_INITRD_TAG
 #define BOOTM_ENABLE_INITRD_TAG	1
#else
 #define BOOTM_ENABLE_INITRD_TAG	0
#endif

#ifdef CONFIG_SERIAL_TAG
 #define BOOTM_ENABLE_SERIAL_TAG	1
void get_board_serial(struct tag_serialnr *serialnr);
#else
 #define BOOTM_ENABLE_SERIAL_TAG	0
static inline void get_board_serial(struct tag_serialnr *serialnr)
{
}
#endif

#ifdef CONFIG_REVISION_TAG
 #define BOOTM_ENABLE_REVISION_TAG	1
u32 get_board_rev(void);
#else
 #define BOOTM_ENABLE_REVISION_TAG	0
static inline u32 get_board_rev(void)
{
	return 0;
}
#endif

#endif
