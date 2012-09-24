/*
 *  GRUB  --  GRand Unified Bootloader
 *  Copyright (C) 1999,2000,2001,2002,2003,2004  Free Software Foundation, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
/*
 * Copyright 2010 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */
#ifndef	_SYS_SA_IMPL_H
#define	_SYS_SA_IMPL_H

typedef struct sa_hdr_phys {
	uint32_t sa_magic;
	uint16_t sa_layout_info;
	uint16_t sa_lengths[1];
} sa_hdr_phys_t;

#define	SA_HDR_SIZE(hdr)	BF32_GET_SB(hdr->sa_layout_info, 10, 16, 3, 0)
#define	SA_SIZE_OFFSET	0x8

#endif	/* _SYS_SA_IMPL_H */
