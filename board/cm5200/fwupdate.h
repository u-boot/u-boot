/*
 * (C) Copyright 2007 Schindler Lift Inc.
 *
 * Author: Michel Marti <mma@objectxp.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+ 
 */

#ifndef __FW_UPDATE_H
#define __FW_UPDATE_H

/* Default prefix for output messages */
#define LOG_PREFIX	"CM5200:"

/* Extra debug macro */
#ifdef CONFIG_FWUPDATE_DEBUG
#define FW_DEBUG(fmt...) printf(LOG_PREFIX fmt)
#else
#define FW_DEBUG(fmt...)
#endif

/* Name of the directory holding firmware images */
#define FW_DIR		"nx-fw"
#define RESCUE_IMAGE	"nxrs.img"
#define LOAD_ADDR	0x400000
#define RS_BOOTARGS	"ramdisk_size=8192K"

/* Main function for fwupdate */
void cm5200_fwupdate(void);

#endif /* __FW_UPDATE_H */
