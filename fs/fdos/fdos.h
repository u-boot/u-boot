/*
 * (C) Copyright 2002
 * Stäubli Faverges - <www.staubli.com>
 * Pierre AUBERT  p.aubert@staubli.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _FDOS_H_
#define _FDOS_H_


#undef	FDOS_DEBUG

#ifdef	FDOS_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

/* Data structure describing media                                           */
typedef struct fs
{
    unsigned long       tot_sectors;

    int                 cluster_size;
    int                 num_clus;

    int                 fat_start;
    int                 fat_len;
    int                 nb_fat;
    int                 num_fat;

    int                 dir_start;
    int                 dir_len;

    unsigned char       *fat_buf;

} Fs_t;

/* Data structure describing one file system slot                            */
typedef struct slot {
    int (*map) (struct fs *fs,
		struct slot *file,
		int where,
		int *len);
    unsigned long FileSize;

    unsigned short int FirstAbsCluNr;
    unsigned short int PreviousAbsCluNr;
    unsigned short int PreviousRelCluNr;

    Directory_t dir;
} Slot_t;

typedef struct file {
    char                *name;
    int                 Case;
    Fs_t                *fs;
    Slot_t              subdir;
    Slot_t              file;
} File_t;


/* dev.c                                                                     */
int dev_read (void *buffer, int where, int len);
int dev_open (void);
int check_dev (BootSector_t *boot, Fs_t *fs);

/* fat.c                                                                     */
unsigned int fat_decode (Fs_t *fs, unsigned int num);
int read_fat (BootSector_t *boot, Fs_t *fs);

/* vfat.c                                                                    */
int vfat_lookup (Slot_t *dir,
		 Fs_t *fs,
		 Directory_t *dirent,
		 int *entry,
		 int *vfat_start,
		 char *filename,
		 int flags,
		 char *outname,
		 Slot_t *file);

/* subdir.c                                                                  */
char *basename (char *name);
int open_subdir (File_t *desc);
int open_file (Slot_t *file, Directory_t *dir);
int read_file (Fs_t *fs,
	       Slot_t *file,
	       char *buf,
	       int where,
	       int len);
void init_subdir (void);

/* fs.c                                                                      */
int fs_init (Fs_t *fs);


#endif
