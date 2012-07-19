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
 * Copyright 2007 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */

#ifndef	_SYS_FS_ZFS_ACL_H
#define	_SYS_FS_ZFS_ACL_H

typedef struct zfs_oldace {
	uint32_t	z_fuid;		/* "who" */
	uint32_t	z_access_mask;	/* access mask */
	uint16_t	z_flags;	/* flags, i.e inheritance */
	uint16_t	z_type;		/* type of entry allow/deny */
} zfs_oldace_t;

#define	ACE_SLOT_CNT	6

typedef struct zfs_znode_acl_v0 {
	uint64_t	z_acl_extern_obj;	  /* ext acl pieces */
	uint32_t	z_acl_count;		  /* Number of ACEs */
	uint16_t	z_acl_version;		  /* acl version */
	uint16_t	z_acl_pad;		  /* pad */
	zfs_oldace_t	z_ace_data[ACE_SLOT_CNT]; /* 6 standard ACEs */
} zfs_znode_acl_v0_t;

#define	ZFS_ACE_SPACE	(sizeof(zfs_oldace_t) * ACE_SLOT_CNT)

typedef struct zfs_znode_acl {
	uint64_t	z_acl_extern_obj;	  /* ext acl pieces */
	uint32_t	z_acl_size;		  /* Number of bytes in ACL */
	uint16_t	z_acl_version;		  /* acl version */
	uint16_t	z_acl_count;		  /* ace count */
	uint8_t	z_ace_data[ZFS_ACE_SPACE]; /* space for embedded ACEs */
} zfs_znode_acl_t;


#endif	/* _SYS_FS_ZFS_ACL_H */
