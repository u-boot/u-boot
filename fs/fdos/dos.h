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

#ifndef _DOS_H_
#define _DOS_H_

/* Definitions for Dos diskettes                                             */

/* General definitions                                                       */
#define SZ_STD_SECTOR   512             /* Standard sector size              */
#define MDIR_SIZE	32		/* Direntry size                     */
#define FAT_BITS        12              /* Diskette use 12 bits fat          */

#define MAX_PATH	128		/* Max size of the MSDOS PATH        */
#define MAX_DIR_SECS	64		/* Taille max d'un repertoire (en    */
					/* secteurs)                         */
/* Misc. definitions                                                         */
#define DELMARK         '\xe5'
#define EXTENDED_BOOT   (0x29)
#define MEDIA_STD       (0xf0)
#define JUMP_0_1        (0xe9)
#define JUMP_0_2        (0xeb)

/* Boot size is 256 bytes, but we need to read almost a sector, then
   assume bootsize is 512                                                    */
#define BOOTSIZE        512

/* Fat definitions for 12 bits fat                                           */
#define FAT12_MAX_NB    4086
#define FAT12_LAST      0x0ff6
#define FAT12_END       0x0fff

/* file attributes                                                           */
#define ATTR_READONLY           0x01
#define ATTR_HIDDEN             0x02
#define ATTR_SYSTEM             0x04
#define ATTR_VOLUME             0x08
#define ATTR_DIRECTORY          0x10
#define ATTR_ARCHIVE            0x20
#define ATTR_VSE                0x0f

/* Name format                                                               */
#define EXTCASE                 0x10
#define BASECASE                0x8

/* Definition of the boot sector                                             */
#define BANNER_LG               8
#define LABEL_LG                11

typedef struct bootsector
{
    unsigned char jump [3];	/* 0  Jump to boot code                      */
    char banner [BANNER_LG];	/* 3  OEM name & version                     */
    unsigned short secsiz;	/* 11 Bytes per sector hopefully 512         */
    unsigned char clsiz;	/* 13 Cluster size in sectors                */
    unsigned short nrsvsect;	/* 14 Number of reserved (boot) sectors      */
    unsigned char nfat;		/* 16 Number of FAT tables hopefully 2       */
    unsigned short dirents;	/* 17 Number of directory slots              */
    unsigned short psect;	/* 19 Total sectors on disk                  */
    unsigned char descr;	/* 21 Media descriptor=first byte of FAT     */
    unsigned short fatlen;	/* 22 Sectors in FAT                         */
    unsigned short nsect;	/* 24 Sectors/track                          */
    unsigned short nheads;	/* 26 Heads                                  */
    unsigned int nhs;	        /* 28 number of hidden sectors               */
    unsigned int bigsect;	/* 32 big total sectors                      */
    unsigned char physdrive;	/* 36 physical drive ?                       */
    unsigned char reserved;	/* 37 reserved                               */
    unsigned char dos4;		/* 38 dos > 4.0 diskette                     */
    unsigned int serial;        /* 39 serial number                          */
    char label [LABEL_LG];	/* 43 disk label                             */
    char fat_type [8];		/* 54 FAT type                               */
    unsigned char res_2m;	/* 62 reserved by 2M                         */
    unsigned char CheckSum;	/* 63 2M checksum (not used)                 */
    unsigned char fmt_2mf;	/* 64 2MF format version                     */
    unsigned char wt;		/* 65 1 if write track after format          */
    unsigned char rate_0;	/* 66 data transfer rate on track 0          */
    unsigned char rate_any;	/* 67 data transfer rate on track<>0         */
    unsigned short BootP;	/* 68 offset to boot program                 */
    unsigned short Infp0;	/* 70 T1: information for track 0            */
    unsigned short InfpX;	/* 72 T2: information for track<>0           */
    unsigned short InfTm;	/* 74 T3: track sectors size table           */
    unsigned short DateF;	/* 76 Format date                            */
    unsigned short TimeF;	/* 78 Format time                            */
    unsigned char junk [BOOTSIZE - 80];	/* 80 remaining data                 */
} __attribute__ ((packed)) BootSector_t;

/* Structure d'une entree de repertoire                                      */
typedef struct directory {
    char name [8];		/* file name                                 */
    char ext [3];		/* file extension                            */
    unsigned char attr;		/* attribute byte                            */
    unsigned char Case;		/* case of short filename                    */
    unsigned char reserved [9];	/* ??                                        */
    unsigned char time [2];	/* time stamp                                */
    unsigned char date [2];	/* date stamp                                */
    unsigned short start;	/* starting cluster number                   */
    unsigned int size;	        /* size of the file                          */
} __attribute__ ((packed))  Directory_t;


#define MAX_VFAT_SUBENTRIES 20
#define VSE_NAMELEN 13

#define VSE1SIZE 5
#define VSE2SIZE 6
#define VSE3SIZE 2

#define VBUFSIZE ((MAX_VFAT_SUBENTRIES * VSE_NAMELEN) + 1)

#define MAX_VNAMELEN (255)

#define VSE_PRESENT 0x01
#define VSE_LAST 0x40
#define VSE_MASK 0x1f

/* Flag used by vfat_lookup                                                  */
#define DO_OPEN         1
#define ACCEPT_PLAIN    0x20
#define ACCEPT_DIR      0x10
#define ACCEPT_LABEL    0x08
#define SINGLE          2
#define MATCH_ANY       0x40

struct vfat_subentry {
    unsigned char id;		        /* VSE_LAST pour la fin, VSE_MASK    */
					/* pour un VSE                       */
    char text1 [VSE1SIZE * 2];          /* Caracteres encodes sur 16 bits    */
    unsigned char attribute;	        /* 0x0f pour les VFAT                */
    unsigned char hash1;		/* toujours 0                        */
    unsigned char sum;		        /* Checksum du nom court             */
    char text2 [VSE2SIZE * 2];          /* Caracteres encodes sur 16 bits    */
    unsigned char sector_l;             /* 0 pour les VFAT                   */
    unsigned char sector_u;		/* 0 pour les VFAT                   */
    char text3 [VSE3SIZE * 2];          /* Caracteres encodes sur 16 bits    */
} __attribute__ ((packed)) ;

struct vfat_state {
    char name [VBUFSIZE];
    int status;             /* is now a bit map of 32 bits                   */
    int subentries;
    unsigned char sum;      /* no need to remember the sum for each          */
			    /*   entry, it is the same anyways               */
} __attribute__ ((packed)) ;

/* Conversion macros                                                         */
#define	DOS_YEAR(dir) (((dir)->date[1] >> 1) + 1980)
#define	DOS_MONTH(dir) (((((dir)->date[1]&0x1) << 3) + ((dir)->date[0] >> 5)))
#define	DOS_DAY(dir) ((dir)->date[0] & 0x1f)
#define	DOS_HOUR(dir) ((dir)->time[1] >> 3)
#define	DOS_MINUTE(dir) (((((dir)->time[1]&0x7) << 3) + ((dir)->time[0] >> 5)))
#define	DOS_SEC(dir) (((dir)->time[0] & 0x1f) * 2)


#endif
