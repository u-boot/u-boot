/*
 * transitional header until we merge global_data.h
 *
 * (C) Copyright 2000-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Licensed under the GPL-2 or later.
 */

#ifndef __ASM_GENERIC_GLOBAL_DATA_FLAGS_H
#define __ASM_GENERIC_GLOBAL_DATA_FLAGS_H

/*
 * Global Data Flags
 *
 * Note: The low 16 bits are expected for common code.  If your arch
 *       really needs to add your own, use the high 16bits.
 */
#define GD_FLG_RELOC		0x0001	/* Code was relocated to RAM */
#define GD_FLG_DEVINIT		0x0002	/* Devices have been initialized */
#define GD_FLG_SILENT		0x0004	/* Silent mode */
#define GD_FLG_POSTFAIL		0x0008	/* Critical POST test failed */
#define GD_FLG_POSTSTOP		0x0010	/* POST seqeunce aborted */
#define GD_FLG_LOGINIT		0x0020	/* Log Buffer has been initialized */
#define GD_FLG_DISABLE_CONSOLE	0x0040	/* Disable console (in & out) */
#define GD_FLG_ENV_READY	0x0080	/* Environment imported into hash table */

#endif
