/*
 * (C) Copyright 2008 Semihalf
 *
 * (C) Copyright 2000-2005
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 *
 ********************************************************************
 * NOTE: This header file defines an interface to U-Boot. Including
 * this (unmodified) header file in another file is considered normal
 * use of U-Boot, and does *not* fall under the heading of "derived
 * work".
 ********************************************************************
 */

#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "compiler.h"
#include <asm/byteorder.h>

/* Define this to avoid #ifdefs later on */
struct lmb;

#ifdef USE_HOSTCC

/* new uImage format support enabled on host */
#define CONFIG_FIT		1
#define CONFIG_OF_LIBFDT	1
#define CONFIG_FIT_VERBOSE	1 /* enable fit_format_{error,warning}() */

/* Support FIT image signing on host */
#define CONFIG_FIT_SIGNATURE

#define IMAGE_ENABLE_IGNORE	0
#define IMAGE_INDENT_STRING	""

#else

#include <lmb.h>
#include <asm/u-boot.h>
#include <command.h>

/* Take notice of the 'ignore' property for hashes */
#define IMAGE_ENABLE_IGNORE	1
#define IMAGE_INDENT_STRING	"   "

#endif /* USE_HOSTCC */

#if defined(CONFIG_FIT)
#include <libfdt.h>
#include <fdt_support.h>
# ifdef CONFIG_SPL_BUILD
#  ifdef CONFIG_SPL_CRC32_SUPPORT
#   define IMAGE_ENABLE_CRC32	1
#  endif
#  ifdef CONFIG_SPL_MD5_SUPPORT
#   define IMAGE_ENABLE_MD5	1
#  endif
#  ifdef CONFIG_SPL_SHA1_SUPPORT
#   define IMAGE_ENABLE_SHA1	1
#  endif
# else
#  define CONFIG_CRC32		/* FIT images need CRC32 support */
#  define CONFIG_MD5		/* and MD5 */
#  define CONFIG_SHA1		/* and SHA1 */
#  define IMAGE_ENABLE_CRC32	1
#  define IMAGE_ENABLE_MD5	1
#  define IMAGE_ENABLE_SHA1	1
# endif

#ifndef IMAGE_ENABLE_CRC32
#define IMAGE_ENABLE_CRC32	0
#endif

#ifndef IMAGE_ENABLE_MD5
#define IMAGE_ENABLE_MD5	0
#endif

#ifndef IMAGE_ENABLE_SHA1
#define IMAGE_ENABLE_SHA1	0
#endif

#endif /* CONFIG_FIT */

#ifdef CONFIG_SYS_BOOT_RAMDISK_HIGH
# define IMAGE_ENABLE_RAMDISK_HIGH	1
#else
# define IMAGE_ENABLE_RAMDISK_HIGH	0
#endif

#ifdef CONFIG_OF_LIBFDT
# define IMAGE_ENABLE_OF_LIBFDT	1
#else
# define IMAGE_ENABLE_OF_LIBFDT	0
#endif

#ifdef CONFIG_SYS_BOOT_GET_CMDLINE
# define IMAGE_BOOT_GET_CMDLINE		1
#else
# define IMAGE_BOOT_GET_CMDLINE		0
#endif

#ifdef CONFIG_OF_BOARD_SETUP
# define IMAAGE_OF_BOARD_SETUP		1
#else
# define IMAAGE_OF_BOARD_SETUP		0
#endif

/*
 * Operating System Codes
 */
#define IH_OS_INVALID		0	/* Invalid OS	*/
#define IH_OS_OPENBSD		1	/* OpenBSD	*/
#define IH_OS_NETBSD		2	/* NetBSD	*/
#define IH_OS_FREEBSD		3	/* FreeBSD	*/
#define IH_OS_4_4BSD		4	/* 4.4BSD	*/
#define IH_OS_LINUX		5	/* Linux	*/
#define IH_OS_SVR4		6	/* SVR4		*/
#define IH_OS_ESIX		7	/* Esix		*/
#define IH_OS_SOLARIS		8	/* Solaris	*/
#define IH_OS_IRIX		9	/* Irix		*/
#define IH_OS_SCO		10	/* SCO		*/
#define IH_OS_DELL		11	/* Dell		*/
#define IH_OS_NCR		12	/* NCR		*/
#define IH_OS_LYNXOS		13	/* LynxOS	*/
#define IH_OS_VXWORKS		14	/* VxWorks	*/
#define IH_OS_PSOS		15	/* pSOS		*/
#define IH_OS_QNX		16	/* QNX		*/
#define IH_OS_U_BOOT		17	/* Firmware	*/
#define IH_OS_RTEMS		18	/* RTEMS	*/
#define IH_OS_ARTOS		19	/* ARTOS	*/
#define IH_OS_UNITY		20	/* Unity OS	*/
#define IH_OS_INTEGRITY		21	/* INTEGRITY	*/
#define IH_OS_OSE		22	/* OSE		*/
#define IH_OS_PLAN9		23	/* Plan 9	*/

/*
 * CPU Architecture Codes (supported by Linux)
 */
#define IH_ARCH_INVALID		0	/* Invalid CPU	*/
#define IH_ARCH_ALPHA		1	/* Alpha	*/
#define IH_ARCH_ARM		2	/* ARM		*/
#define IH_ARCH_I386		3	/* Intel x86	*/
#define IH_ARCH_IA64		4	/* IA64		*/
#define IH_ARCH_MIPS		5	/* MIPS		*/
#define IH_ARCH_MIPS64		6	/* MIPS	 64 Bit */
#define IH_ARCH_PPC		7	/* PowerPC	*/
#define IH_ARCH_S390		8	/* IBM S390	*/
#define IH_ARCH_SH		9	/* SuperH	*/
#define IH_ARCH_SPARC		10	/* Sparc	*/
#define IH_ARCH_SPARC64		11	/* Sparc 64 Bit */
#define IH_ARCH_M68K		12	/* M68K		*/
#define IH_ARCH_MICROBLAZE	14	/* MicroBlaze   */
#define IH_ARCH_NIOS2		15	/* Nios-II	*/
#define IH_ARCH_BLACKFIN	16	/* Blackfin	*/
#define IH_ARCH_AVR32		17	/* AVR32	*/
#define IH_ARCH_ST200	        18	/* STMicroelectronics ST200  */
#define IH_ARCH_SANDBOX		19	/* Sandbox architecture (test only) */
#define IH_ARCH_NDS32	        20	/* ANDES Technology - NDS32  */
#define IH_ARCH_OPENRISC        21	/* OpenRISC 1000  */

/*
 * Image Types
 *
 * "Standalone Programs" are directly runnable in the environment
 *	provided by U-Boot; it is expected that (if they behave
 *	well) you can continue to work in U-Boot after return from
 *	the Standalone Program.
 * "OS Kernel Images" are usually images of some Embedded OS which
 *	will take over control completely. Usually these programs
 *	will install their own set of exception handlers, device
 *	drivers, set up the MMU, etc. - this means, that you cannot
 *	expect to re-enter U-Boot except by resetting the CPU.
 * "RAMDisk Images" are more or less just data blocks, and their
 *	parameters (address, size) are passed to an OS kernel that is
 *	being started.
 * "Multi-File Images" contain several images, typically an OS
 *	(Linux) kernel image and one or more data images like
 *	RAMDisks. This construct is useful for instance when you want
 *	to boot over the network using BOOTP etc., where the boot
 *	server provides just a single image file, but you want to get
 *	for instance an OS kernel and a RAMDisk image.
 *
 *	"Multi-File Images" start with a list of image sizes, each
 *	image size (in bytes) specified by an "uint32_t" in network
 *	byte order. This list is terminated by an "(uint32_t)0".
 *	Immediately after the terminating 0 follow the images, one by
 *	one, all aligned on "uint32_t" boundaries (size rounded up to
 *	a multiple of 4 bytes - except for the last file).
 *
 * "Firmware Images" are binary images containing firmware (like
 *	U-Boot or FPGA images) which usually will be programmed to
 *	flash memory.
 *
 * "Script files" are command sequences that will be executed by
 *	U-Boot's command interpreter; this feature is especially
 *	useful when you configure U-Boot to use a real shell (hush)
 *	as command interpreter (=> Shell Scripts).
 */

#define IH_TYPE_INVALID		0	/* Invalid Image		*/
#define IH_TYPE_STANDALONE	1	/* Standalone Program		*/
#define IH_TYPE_KERNEL		2	/* OS Kernel Image		*/
#define IH_TYPE_RAMDISK		3	/* RAMDisk Image		*/
#define IH_TYPE_MULTI		4	/* Multi-File Image		*/
#define IH_TYPE_FIRMWARE	5	/* Firmware Image		*/
#define IH_TYPE_SCRIPT		6	/* Script file			*/
#define IH_TYPE_FILESYSTEM	7	/* Filesystem Image (any type)	*/
#define IH_TYPE_FLATDT		8	/* Binary Flat Device Tree Blob	*/
#define IH_TYPE_KWBIMAGE	9	/* Kirkwood Boot Image		*/
#define IH_TYPE_IMXIMAGE	10	/* Freescale IMXBoot Image	*/
#define IH_TYPE_UBLIMAGE	11	/* Davinci UBL Image		*/
#define IH_TYPE_OMAPIMAGE	12	/* TI OMAP Config Header Image	*/
#define IH_TYPE_AISIMAGE	13	/* TI Davinci AIS Image		*/
#define IH_TYPE_KERNEL_NOLOAD	14	/* OS Kernel Image, can run from any load address */
#define IH_TYPE_PBLIMAGE	15	/* Freescale PBL Boot Image	*/

/*
 * Compression Types
 */
#define IH_COMP_NONE		0	/*  No	 Compression Used	*/
#define IH_COMP_GZIP		1	/* gzip	 Compression Used	*/
#define IH_COMP_BZIP2		2	/* bzip2 Compression Used	*/
#define IH_COMP_LZMA		3	/* lzma  Compression Used	*/
#define IH_COMP_LZO		4	/* lzo   Compression Used	*/

#define IH_MAGIC	0x27051956	/* Image Magic Number		*/
#define IH_NMLEN		32	/* Image Name Length		*/

/* Reused from common.h */
#define ROUND(a, b)		(((a) + (b) - 1) & ~((b) - 1))

/*
 * Legacy format image header,
 * all data in network byte order (aka natural aka bigendian).
 */
typedef struct image_header {
	__be32		ih_magic;	/* Image Header Magic Number	*/
	__be32		ih_hcrc;	/* Image Header CRC Checksum	*/
	__be32		ih_time;	/* Image Creation Timestamp	*/
	__be32		ih_size;	/* Image Data Size		*/
	__be32		ih_load;	/* Data	 Load  Address		*/
	__be32		ih_ep;		/* Entry Point Address		*/
	__be32		ih_dcrc;	/* Image Data CRC Checksum	*/
	uint8_t		ih_os;		/* Operating System		*/
	uint8_t		ih_arch;	/* CPU architecture		*/
	uint8_t		ih_type;	/* Image Type			*/
	uint8_t		ih_comp;	/* Compression Type		*/
	uint8_t		ih_name[IH_NMLEN];	/* Image Name		*/
} image_header_t;

typedef struct image_info {
	ulong		start, end;		/* start/end of blob */
	ulong		image_start, image_len; /* start of image within blob, len of image */
	ulong		load;			/* load addr for the image */
	uint8_t		comp, type, os;		/* compression, type of image, os type */
} image_info_t;

/*
 * Legacy and FIT format headers used by do_bootm() and do_bootm_<os>()
 * routines.
 */
typedef struct bootm_headers {
	/*
	 * Legacy os image header, if it is a multi component image
	 * then boot_get_ramdisk() and get_fdt() will attempt to get
	 * data from second and third component accordingly.
	 */
	image_header_t	*legacy_hdr_os;		/* image header pointer */
	image_header_t	legacy_hdr_os_copy;	/* header copy */
	ulong		legacy_hdr_valid;

#if defined(CONFIG_FIT)
	const char	*fit_uname_cfg;	/* configuration node unit name */

	void		*fit_hdr_os;	/* os FIT image header */
	const char	*fit_uname_os;	/* os subimage node unit name */
	int		fit_noffset_os;	/* os subimage node offset */

	void		*fit_hdr_rd;	/* init ramdisk FIT image header */
	const char	*fit_uname_rd;	/* init ramdisk subimage node unit name */
	int		fit_noffset_rd;	/* init ramdisk subimage node offset */

	void		*fit_hdr_fdt;	/* FDT blob FIT image header */
	const char	*fit_uname_fdt;	/* FDT blob subimage node unit name */
	int		fit_noffset_fdt;/* FDT blob subimage node offset */
#endif

#ifndef USE_HOSTCC
	image_info_t	os;		/* os image info */
	ulong		ep;		/* entry point of OS */

	ulong		rd_start, rd_end;/* ramdisk start/end */

	char		*ft_addr;	/* flat dev tree address */
	ulong		ft_len;		/* length of flat device tree */

	ulong		initrd_start;
	ulong		initrd_end;
	ulong		cmdline_start;
	ulong		cmdline_end;
	bd_t		*kbd;
#endif

	int		verify;		/* getenv("verify")[0] != 'n' */

#define	BOOTM_STATE_START	(0x00000001)
#define	BOOTM_STATE_FINDOS	(0x00000002)
#define	BOOTM_STATE_FINDOTHER	(0x00000004)
#define	BOOTM_STATE_LOADOS	(0x00000008)
#define	BOOTM_STATE_RAMDISK	(0x00000010)
#define	BOOTM_STATE_FDT		(0x00000020)
#define	BOOTM_STATE_OS_CMDLINE	(0x00000040)
#define	BOOTM_STATE_OS_BD_T	(0x00000080)
#define	BOOTM_STATE_OS_PREP	(0x00000100)
#define	BOOTM_STATE_OS_FAKE_GO	(0x00000200)	/* 'Almost' run the OS */
#define	BOOTM_STATE_OS_GO	(0x00000400)
	int		state;

#ifdef CONFIG_LMB
	struct lmb	lmb;		/* for memory mgmt */
#endif
} bootm_headers_t;

extern bootm_headers_t images;

/*
 * Some systems (for example LWMON) have very short watchdog periods;
 * we must make sure to split long operations like memmove() or
 * checksum calculations into reasonable chunks.
 */
#ifndef CHUNKSZ
#define CHUNKSZ (64 * 1024)
#endif

#ifndef CHUNKSZ_CRC32
#define CHUNKSZ_CRC32 (64 * 1024)
#endif

#ifndef CHUNKSZ_MD5
#define CHUNKSZ_MD5 (64 * 1024)
#endif

#ifndef CHUNKSZ_SHA1
#define CHUNKSZ_SHA1 (64 * 1024)
#endif

#define uimage_to_cpu(x)		be32_to_cpu(x)
#define cpu_to_uimage(x)		cpu_to_be32(x)

/*
 * Translation table for entries of a specific type; used by
 * get_table_entry_id() and get_table_entry_name().
 */
typedef struct table_entry {
	int	id;
	char	*sname;		/* short (input) name to find table entry */
	char	*lname;		/* long (output) name to print for messages */
} table_entry_t;

/*
 * get_table_entry_id() scans the translation table trying to find an
 * entry that matches the given short name. If a matching entry is
 * found, it's id is returned to the caller.
 */
int get_table_entry_id(const table_entry_t *table,
		const char *table_name, const char *name);
/*
 * get_table_entry_name() scans the translation table trying to find
 * an entry that matches the given id. If a matching entry is found,
 * its long name is returned to the caller.
 */
char *get_table_entry_name(const table_entry_t *table, char *msg, int id);

const char *genimg_get_os_name(uint8_t os);
const char *genimg_get_arch_name(uint8_t arch);
const char *genimg_get_type_name(uint8_t type);
const char *genimg_get_comp_name(uint8_t comp);
int genimg_get_os_id(const char *name);
int genimg_get_arch_id(const char *name);
int genimg_get_type_id(const char *name);
int genimg_get_comp_id(const char *name);
void genimg_print_size(uint32_t size);

#if defined(CONFIG_TIMESTAMP) || defined(CONFIG_CMD_DATE) || \
	defined(USE_HOSTCC)
#define IMAGE_ENABLE_TIMESTAMP 1
#else
#define IMAGE_ENABLE_TIMESTAMP 0
#endif
void genimg_print_time(time_t timestamp);

/* What to do with a image load address ('load = <> 'in the FIT) */
enum fit_load_op {
	FIT_LOAD_IGNORED,	/* Ignore load address */
	FIT_LOAD_OPTIONAL,	/* Can be provided, but optional */
	FIT_LOAD_REQUIRED,	/* Must be provided */
};

#ifndef USE_HOSTCC
/* Image format types, returned by _get_format() routine */
#define IMAGE_FORMAT_INVALID	0x00
#define IMAGE_FORMAT_LEGACY	0x01	/* legacy image_header based format */
#define IMAGE_FORMAT_FIT	0x02	/* new, libfdt based format */

int genimg_get_format(const void *img_addr);
int genimg_has_config(bootm_headers_t *images);
ulong genimg_get_image(ulong img_addr);

int boot_get_ramdisk(int argc, char * const argv[], bootm_headers_t *images,
		uint8_t arch, ulong *rd_start, ulong *rd_end);

/**
 * fit_image_load() - load an image from a FIT
 *
 * This deals with all aspects of loading an image from a FIT, including
 * selecting the right image based on configuration, verifying it, printing
 * out progress messages, checking the type/arch/os and optionally copying it
 * to the right load address.
 *
 * @param images	Boot images structure
 * @param prop_name	Property name to look up (FIT_..._PROP)
 * @param addr		Address of FIT in memory
 * @param fit_unamep	On entry this is the requested image name
 *			(e.g. "kernel@1") or NULL to use the default. On exit
 *			points to the selected image name
 * @param fit_uname_config	Requested configuration name, or NULL for the
 *			default
 * @param arch		Expected architecture (IH_ARCH_...)
 * @param image_type	Required image type (IH_TYPE_...). If this is
 *			IH_TYPE_KERNEL then we allow IH_TYPE_KERNEL_NOLOAD
 *			also.
 * @param bootstage_id	ID of starting bootstage to use for progress updates.
 *			This will be added to the BOOTSTAGE_SUB values when
 *			calling bootstage_mark()
 * @param load_op	Decribes what to do with the load address
 * @param datap		Returns address of loaded image
 * @param lenp		Returns length of loaded image
 */
int fit_image_load(bootm_headers_t *images, const char *prop_name, ulong addr,
		   const char **fit_unamep, const char *fit_uname_config,
		   int arch, int image_type, int bootstage_id,
		   enum fit_load_op load_op, ulong *datap, ulong *lenp);

/**
 * fit_get_node_from_config() - Look up an image a FIT by type
 *
 * This looks in the selected conf@ node (images->fit_uname_cfg) for a
 * particular image type (e.g. "kernel") and then finds the image that is
 * referred to.
 *
 * For example, for something like:
 *
 * images {
 *	kernel@1 {
 *		...
 *	};
 * };
 * configurations {
 *	conf@1 {
 *		kernel = "kernel@1";
 *	};
 * };
 *
 * the function will return the node offset of the kernel@1 node, assuming
 * that conf@1 is the chosen configuration.
 *
 * @param images	Boot images structure
 * @param prop_name	Property name to look up (FIT_..._PROP)
 * @param addr		Address of FIT in memory
 */
int fit_get_node_from_config(bootm_headers_t *images, const char *prop_name,
			ulong addr);

int boot_get_fdt(int flag, int argc, char * const argv[], uint8_t arch,
		 bootm_headers_t *images,
		 char **of_flat_tree, ulong *of_size);
void boot_fdt_add_mem_rsv_regions(struct lmb *lmb, void *fdt_blob);
int boot_relocate_fdt(struct lmb *lmb, char **of_flat_tree, ulong *of_size);

int boot_ramdisk_high(struct lmb *lmb, ulong rd_data, ulong rd_len,
		  ulong *initrd_start, ulong *initrd_end);
int boot_get_cmdline(struct lmb *lmb, ulong *cmd_start, ulong *cmd_end);
#ifdef CONFIG_SYS_BOOT_GET_KBD
int boot_get_kbd(struct lmb *lmb, bd_t **kbd);
#endif /* CONFIG_SYS_BOOT_GET_KBD */
#endif /* !USE_HOSTCC */

/*******************************************************************/
/* Legacy format specific code (prefixed with image_) */
/*******************************************************************/
static inline uint32_t image_get_header_size(void)
{
	return (sizeof(image_header_t));
}

#define image_get_hdr_l(f) \
	static inline uint32_t image_get_##f(const image_header_t *hdr) \
	{ \
		return uimage_to_cpu(hdr->ih_##f); \
	}
image_get_hdr_l(magic)		/* image_get_magic */
image_get_hdr_l(hcrc)		/* image_get_hcrc */
image_get_hdr_l(time)		/* image_get_time */
image_get_hdr_l(size)		/* image_get_size */
image_get_hdr_l(load)		/* image_get_load */
image_get_hdr_l(ep)		/* image_get_ep */
image_get_hdr_l(dcrc)		/* image_get_dcrc */

#define image_get_hdr_b(f) \
	static inline uint8_t image_get_##f(const image_header_t *hdr) \
	{ \
		return hdr->ih_##f; \
	}
image_get_hdr_b(os)		/* image_get_os */
image_get_hdr_b(arch)		/* image_get_arch */
image_get_hdr_b(type)		/* image_get_type */
image_get_hdr_b(comp)		/* image_get_comp */

static inline char *image_get_name(const image_header_t *hdr)
{
	return (char *)hdr->ih_name;
}

static inline uint32_t image_get_data_size(const image_header_t *hdr)
{
	return image_get_size(hdr);
}

/**
 * image_get_data - get image payload start address
 * @hdr: image header
 *
 * image_get_data() returns address of the image payload. For single
 * component images it is image data start. For multi component
 * images it points to the null terminated table of sub-images sizes.
 *
 * returns:
 *     image payload data start address
 */
static inline ulong image_get_data(const image_header_t *hdr)
{
	return ((ulong)hdr + image_get_header_size());
}

static inline uint32_t image_get_image_size(const image_header_t *hdr)
{
	return (image_get_size(hdr) + image_get_header_size());
}
static inline ulong image_get_image_end(const image_header_t *hdr)
{
	return ((ulong)hdr + image_get_image_size(hdr));
}

#define image_set_hdr_l(f) \
	static inline void image_set_##f(image_header_t *hdr, uint32_t val) \
	{ \
		hdr->ih_##f = cpu_to_uimage(val); \
	}
image_set_hdr_l(magic)		/* image_set_magic */
image_set_hdr_l(hcrc)		/* image_set_hcrc */
image_set_hdr_l(time)		/* image_set_time */
image_set_hdr_l(size)		/* image_set_size */
image_set_hdr_l(load)		/* image_set_load */
image_set_hdr_l(ep)		/* image_set_ep */
image_set_hdr_l(dcrc)		/* image_set_dcrc */

#define image_set_hdr_b(f) \
	static inline void image_set_##f(image_header_t *hdr, uint8_t val) \
	{ \
		hdr->ih_##f = val; \
	}
image_set_hdr_b(os)		/* image_set_os */
image_set_hdr_b(arch)		/* image_set_arch */
image_set_hdr_b(type)		/* image_set_type */
image_set_hdr_b(comp)		/* image_set_comp */

static inline void image_set_name(image_header_t *hdr, const char *name)
{
	strncpy(image_get_name(hdr), name, IH_NMLEN);
}

int image_check_hcrc(const image_header_t *hdr);
int image_check_dcrc(const image_header_t *hdr);
#ifndef USE_HOSTCC
ulong getenv_bootm_low(void);
phys_size_t getenv_bootm_size(void);
phys_size_t getenv_bootm_mapsize(void);
void memmove_wd(void *to, void *from, size_t len, ulong chunksz);
#endif

static inline int image_check_magic(const image_header_t *hdr)
{
	return (image_get_magic(hdr) == IH_MAGIC);
}
static inline int image_check_type(const image_header_t *hdr, uint8_t type)
{
	return (image_get_type(hdr) == type);
}
static inline int image_check_arch(const image_header_t *hdr, uint8_t arch)
{
	return (image_get_arch(hdr) == arch);
}
static inline int image_check_os(const image_header_t *hdr, uint8_t os)
{
	return (image_get_os(hdr) == os);
}

ulong image_multi_count(const image_header_t *hdr);
void image_multi_getimg(const image_header_t *hdr, ulong idx,
			ulong *data, ulong *len);

void image_print_contents(const void *hdr);

#ifndef USE_HOSTCC
static inline int image_check_target_arch(const image_header_t *hdr)
{
#ifndef IH_ARCH_DEFAULT
# error "please define IH_ARCH_DEFAULT in your arch asm/u-boot.h"
#endif
	return image_check_arch(hdr, IH_ARCH_DEFAULT);
}
#endif /* USE_HOSTCC */

/**
 * Set up properties in the FDT
 *
 * This sets up properties in the FDT that is to be passed to linux.
 *
 * @images:	Images information
 * @blob:	FDT to update
 * @of_size:	Size of the FDT
 * @lmb:	Points to logical memory block structure
 * @return 0 if ok, <0 on failure
 */
int image_setup_libfdt(bootm_headers_t *images, void *blob,
		       int of_size, struct lmb *lmb);

/**
 * Set up the FDT to use for booting a kernel
 *
 * This performs ramdisk setup, sets up the FDT if required, and adds
 * paramters to the FDT if libfdt is available.
 *
 * @param images	Images information
 * @return 0 if ok, <0 on failure
 */
int image_setup_linux(bootm_headers_t *images);

/*******************************************************************/
/* New uImage format specific code (prefixed with fit_) */
/*******************************************************************/
#if defined(CONFIG_FIT)

#define FIT_IMAGES_PATH		"/images"
#define FIT_CONFS_PATH		"/configurations"

/* hash/signature node */
#define FIT_HASH_NODENAME	"hash"
#define FIT_ALGO_PROP		"algo"
#define FIT_VALUE_PROP		"value"
#define FIT_IGNORE_PROP		"uboot-ignore"
#define FIT_SIG_NODENAME	"signature"

/* image node */
#define FIT_DATA_PROP		"data"
#define FIT_TIMESTAMP_PROP	"timestamp"
#define FIT_DESC_PROP		"description"
#define FIT_ARCH_PROP		"arch"
#define FIT_TYPE_PROP		"type"
#define FIT_OS_PROP		"os"
#define FIT_COMP_PROP		"compression"
#define FIT_ENTRY_PROP		"entry"
#define FIT_LOAD_PROP		"load"

/* configuration node */
#define FIT_KERNEL_PROP		"kernel"
#define FIT_RAMDISK_PROP	"ramdisk"
#define FIT_FDT_PROP		"fdt"
#define FIT_DEFAULT_PROP	"default"

#define FIT_MAX_HASH_LEN	20	/* max(crc32_len(4), sha1_len(20)) */

/* cmdline argument format parsing */
int fit_parse_conf(const char *spec, ulong addr_curr,
		ulong *addr, const char **conf_name);
int fit_parse_subimage(const char *spec, ulong addr_curr,
		ulong *addr, const char **image_name);

void fit_print_contents(const void *fit);
void fit_image_print(const void *fit, int noffset, const char *p);

/**
 * fit_get_end - get FIT image size
 * @fit: pointer to the FIT format image header
 *
 * returns:
 *     size of the FIT image (blob) in memory
 */
static inline ulong fit_get_size(const void *fit)
{
	return fdt_totalsize(fit);
}

/**
 * fit_get_end - get FIT image end
 * @fit: pointer to the FIT format image header
 *
 * returns:
 *     end address of the FIT image (blob) in memory
 */
static inline ulong fit_get_end(const void *fit)
{
	return (ulong)fit + fdt_totalsize(fit);
}

/**
 * fit_get_name - get FIT node name
 * @fit: pointer to the FIT format image header
 *
 * returns:
 *     NULL, on error
 *     pointer to node name, on success
 */
static inline const char *fit_get_name(const void *fit_hdr,
		int noffset, int *len)
{
	return fdt_get_name(fit_hdr, noffset, len);
}

int fit_get_desc(const void *fit, int noffset, char **desc);
int fit_get_timestamp(const void *fit, int noffset, time_t *timestamp);

int fit_image_get_node(const void *fit, const char *image_uname);
int fit_image_get_os(const void *fit, int noffset, uint8_t *os);
int fit_image_get_arch(const void *fit, int noffset, uint8_t *arch);
int fit_image_get_type(const void *fit, int noffset, uint8_t *type);
int fit_image_get_comp(const void *fit, int noffset, uint8_t *comp);
int fit_image_get_load(const void *fit, int noffset, ulong *load);
int fit_image_get_entry(const void *fit, int noffset, ulong *entry);
int fit_image_get_data(const void *fit, int noffset,
				const void **data, size_t *size);

int fit_image_hash_get_algo(const void *fit, int noffset, char **algo);
int fit_image_hash_get_value(const void *fit, int noffset, uint8_t **value,
				int *value_len);

int fit_set_timestamp(void *fit, int noffset, time_t timestamp);

/**
 * fit_add_verification_data() - Calculate and add hashes to FIT
 *
 * @fit:	Fit image to process
 * @return 0 if ok, <0 for error
 */
int fit_add_verification_data(void *fit);

int fit_image_verify(const void *fit, int noffset);
int fit_config_verify(const void *fit, int conf_noffset);
int fit_all_image_verify(const void *fit);
int fit_image_check_os(const void *fit, int noffset, uint8_t os);
int fit_image_check_arch(const void *fit, int noffset, uint8_t arch);
int fit_image_check_type(const void *fit, int noffset, uint8_t type);
int fit_image_check_comp(const void *fit, int noffset, uint8_t comp);
int fit_check_format(const void *fit);

int fit_conf_find_compat(const void *fit, const void *fdt);
int fit_conf_get_node(const void *fit, const char *conf_uname);

/**
 * fit_conf_get_prop_node() - Get node refered to by a configuration
 * @fit:	FIT to check
 * @noffset:	Offset of conf@xxx node to check
 * @prop_name:	Property to read from the conf node
 *
 * The conf@ nodes contain references to other nodes, using properties
 * like 'kernel = "kernel@1"'. Given such a property name (e.g. "kernel"),
 * return the offset of the node referred to (e.g. offset of node
 * "/images/kernel@1".
 */
int fit_conf_get_prop_node(const void *fit, int noffset,
		const char *prop_name);

void fit_conf_print(const void *fit, int noffset, const char *p);

int fit_check_ramdisk(const void *fit, int os_noffset,
		uint8_t arch, int verify);

int calculate_hash(const void *data, int data_len, const char *algo,
			uint8_t *value, int *value_len);

/*
 * At present we only support signing on the host, and verification on the
 * device
 */
#if defined(CONFIG_FIT_SIGNATURE)
# ifdef USE_HOSTCC
#  define IMAGE_ENABLE_SIGN	1
#  define IMAGE_ENABLE_VERIFY	0
#else
#  define IMAGE_ENABLE_SIGN	0
#  define IMAGE_ENABLE_VERIFY	1
# endif
#else
# define IMAGE_ENABLE_SIGN	0
# define IMAGE_ENABLE_VERIFY	0
#endif

#ifdef USE_HOSTCC
# define gd_fdt_blob()		NULL
#else
# define gd_fdt_blob()		(gd->fdt_blob)
#endif

#ifdef CONFIG_FIT_BEST_MATCH
#define IMAGE_ENABLE_BEST_MATCH	1
#else
#define IMAGE_ENABLE_BEST_MATCH	0
#endif

/* Information passed to the signing routines */
struct image_sign_info {
	const char *keydir;		/* Directory conaining keys */
	const char *keyname;		/* Name of key to use */
	void *fit;			/* Pointer to FIT blob */
	int node_offset;		/* Offset of signature node */
	struct image_sig_algo *algo;	/* Algorithm information */
	const void *fdt_blob;		/* FDT containing public keys */
	int required_keynode;		/* Node offset of key to use: -1=any */
	const char *require_keys;	/* Value for 'required' property */
};

/* A part of an image, used for hashing */
struct image_region {
	const void *data;
	int size;
};

struct image_sig_algo {
	const char *name;		/* Name of algorithm */

	/**
	 * sign() - calculate and return signature for given input data
	 *
	 * @info:	Specifies key and FIT information
	 * @data:	Pointer to the input data
	 * @data_len:	Data length
	 * @sigp:	Set to an allocated buffer holding the signature
	 * @sig_len:	Set to length of the calculated hash
	 *
	 * This computes input data signature according to selected algorithm.
	 * Resulting signature value is placed in an allocated buffer, the
	 * pointer is returned as *sigp. The length of the calculated
	 * signature is returned via the sig_len pointer argument. The caller
	 * should free *sigp.
	 *
	 * @return: 0, on success, -ve on error
	 */
	int (*sign)(struct image_sign_info *info,
		    const struct image_region region[],
		    int region_count, uint8_t **sigp, uint *sig_len);

	/**
	 * add_verify_data() - Add verification information to FDT
	 *
	 * Add public key information to the FDT node, suitable for
	 * verification at run-time. The information added depends on the
	 * algorithm being used.
	 *
	 * @info:	Specifies key and FIT information
	 * @keydest:	Destination FDT blob for public key data
	 * @return: 0, on success, -ve on error
	 */
	int (*add_verify_data)(struct image_sign_info *info, void *keydest);

	/**
	 * verify() - Verify a signature against some data
	 *
	 * @info:	Specifies key and FIT information
	 * @data:	Pointer to the input data
	 * @data_len:	Data length
	 * @sig:	Signature
	 * @sig_len:	Number of bytes in signature
	 * @return 0 if verified, -ve on error
	 */
	int (*verify)(struct image_sign_info *info,
		      const struct image_region region[], int region_count,
		      uint8_t *sig, uint sig_len);
};

/**
 * image_get_sig_algo() - Look up a signature algortihm
 *
 * @param name		Name of algorithm
 * @return pointer to algorithm information, or NULL if not found
 */
struct image_sig_algo *image_get_sig_algo(const char *name);

static inline int fit_image_check_target_arch(const void *fdt, int node)
{
	return fit_image_check_arch(fdt, node, IH_ARCH_DEFAULT);
}

#ifdef CONFIG_FIT_VERBOSE
#define fit_unsupported(msg)	printf("! %s:%d " \
				"FIT images not supported for '%s'\n", \
				__FILE__, __LINE__, (msg))

#define fit_unsupported_reset(msg)	printf("! %s:%d " \
				"FIT images not supported for '%s' " \
				"- must reset board to recover!\n", \
				__FILE__, __LINE__, (msg))
#else
#define fit_unsupported(msg)
#define fit_unsupported_reset(msg)
#endif /* CONFIG_FIT_VERBOSE */
#endif /* CONFIG_FIT */

#endif	/* __IMAGE_H__ */
