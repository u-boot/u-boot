#ifndef _CRAMFS_FS_SB
#define _CRAMFS_FS_SB

/*
 * cramfs super-block data in memory
 */
struct cramfs_sb_info {
			unsigned long magic;
			unsigned long size;
			unsigned long blocks;
			unsigned long files;
			unsigned long flags;
#ifdef CONFIG_CRAMFS_LINEAR
			unsigned long linear_phys_addr;
			char *        linear_virt_addr;
#endif
};

#endif
