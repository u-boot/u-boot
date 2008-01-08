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

#include <asm/byteorder.h>
#ifndef USE_HOSTCC
#include <linux/string.h>
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
#define IH_ARCH_NIOS		13	/* Nios-32	*/
#define IH_ARCH_MICROBLAZE	14	/* MicroBlaze   */
#define IH_ARCH_NIOS2		15	/* Nios-II	*/
#define IH_ARCH_BLACKFIN	16	/* Blackfin	*/
#define IH_ARCH_AVR32		17	/* AVR32	*/
#define IH_ARCH_ST200	        18	/* STMicroelectronics ST200  */

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

/*
 * Compression Types
 */
#define IH_COMP_NONE		0	/*  No	 Compression Used	*/
#define IH_COMP_GZIP		1	/* gzip	 Compression Used	*/
#define IH_COMP_BZIP2		2	/* bzip2 Compression Used	*/

#define IH_MAGIC	0x27051956	/* Image Magic Number		*/
#define IH_NMLEN		32	/* Image Name Length		*/

/*
 * all data in network byte order (aka natural aka bigendian)
 */

typedef struct image_header {
	uint32_t	ih_magic;	/* Image Header Magic Number	*/
	uint32_t	ih_hcrc;	/* Image Header CRC Checksum	*/
	uint32_t	ih_time;	/* Image Creation Timestamp	*/
	uint32_t	ih_size;	/* Image Data Size		*/
	uint32_t	ih_load;	/* Data	 Load  Address		*/
	uint32_t	ih_ep;		/* Entry Point Address		*/
	uint32_t	ih_dcrc;	/* Image Data CRC Checksum	*/
	uint8_t		ih_os;		/* Operating System		*/
	uint8_t		ih_arch;	/* CPU architecture		*/
	uint8_t		ih_type;	/* Image Type			*/
	uint8_t		ih_comp;	/* Compression Type		*/
	uint8_t		ih_name[IH_NMLEN];	/* Image Name		*/
} image_header_t;

/*
 * Some systems (for example LWMON) have very short watchdog periods;
 * we must make sure to split long operations like memmove() or
 * crc32() into reasonable chunks.
 */
#define CHUNKSZ (64 * 1024)

#define image_to_cpu(x)		ntohl(x)
#define cpu_to_image(x)		htonl(x)

static inline uint32_t image_get_header_size (void)
{
	return (sizeof (image_header_t));
}

#define image_get_hdr_l(f) \
	static inline uint32_t image_get_##f(image_header_t *hdr) \
	{ \
		return image_to_cpu (hdr->ih_##f); \
	}
image_get_hdr_l (magic);
image_get_hdr_l (hcrc);
image_get_hdr_l (time);
image_get_hdr_l (size);
image_get_hdr_l (load);
image_get_hdr_l (ep);
image_get_hdr_l (dcrc);

#define image_get_hdr_b(f) \
	static inline uint8_t image_get_##f(image_header_t *hdr) \
	{ \
		return hdr->ih_##f; \
	}
image_get_hdr_b (os);
image_get_hdr_b (arch);
image_get_hdr_b (type);
image_get_hdr_b (comp);

static inline char *image_get_name (image_header_t *hdr)
{
	return (char *)hdr->ih_name;
}

static inline uint32_t image_get_data_size (image_header_t *hdr)
{
	return image_get_size (hdr);
}
static inline uint32_t image_get_image_size (image_header_t *hdr)
{
	return (image_get_size (hdr) + image_get_header_size ());
}
static inline ulong image_get_data (image_header_t *hdr)
{
	return ((ulong)hdr + image_get_header_size ());
}

#define image_set_hdr_l(f) \
	static inline void image_set_##f(image_header_t *hdr, uint32_t val) \
	{ \
		hdr->ih_##f = cpu_to_image (val); \
	}
image_set_hdr_l (magic);
image_set_hdr_l (hcrc);
image_set_hdr_l (time);
image_set_hdr_l (size);
image_set_hdr_l (load);
image_set_hdr_l (ep);
image_set_hdr_l (dcrc);

#define image_set_hdr_b(f) \
	static inline void image_set_##f(image_header_t *hdr, uint8_t val) \
	{ \
		hdr->ih_##f = val; \
	}
image_set_hdr_b (os);
image_set_hdr_b (arch);
image_set_hdr_b (type);
image_set_hdr_b (comp);

static inline void image_set_name (image_header_t *hdr, const char *name)
{
	strncpy (image_get_name (hdr), name, IH_NMLEN);
}

int image_check_hcrc (image_header_t *hdr);
int image_check_dcrc (image_header_t *hdr);
#ifndef USE_HOSTCC
int image_check_dcrc_wd (image_header_t *hdr, ulong chunksize);
int getenv_verify (void);
void memmove_wd (void *to, void *from, size_t len, ulong chunksz);
#endif

static inline int image_check_magic (image_header_t *hdr)
{
	return (image_get_magic (hdr) == IH_MAGIC);
}
static inline int image_check_type (image_header_t *hdr, uint8_t type)
{
	return (image_get_type (hdr) == type);
}
static inline int image_check_arch (image_header_t *hdr, uint8_t arch)
{
	return (image_get_arch (hdr) == arch);
}
static inline int image_check_os (image_header_t *hdr, uint8_t os)
{
	return (image_get_os (hdr) == os);
}

#ifndef USE_HOSTCC
static inline int image_check_target_arch (image_header_t *hdr)
{
#if defined(__ARM__)
	if (!image_check_arch (hdr, IH_ARCH_ARM))
#elif defined(__avr32__)
	if (!image_check_arch (hdr, IH_ARCH_AVR32))
#elif defined(__bfin__)
	if (!image_check_arch (hdr, IH_ARCH_BLACKFIN))
#elif defined(__I386__)
	if (!image_check_arch (hdr, IH_ARCH_I386))
#elif defined(__M68K__)
	if (!image_check_arch (hdr, IH_ARCH_M68K))
#elif defined(__microblaze__)
	if (!image_check_arch (hdr, IH_ARCH_MICROBLAZE))
#elif defined(__mips__)
	if (!image_check_arch (hdr, IH_ARCH_MIPS))
#elif defined(__nios__)
	if (!image_check_arch (hdr, IH_ARCH_NIOS))
#elif defined(__nios2__)
	if (!image_check_arch (hdr, IH_ARCH_NIOS2))
#elif defined(__PPC__)
	if (!image_check_arch (hdr, IH_ARCH_PPC))
#elif defined(__sh__)
	if (!image_check_arch (hdr, IH_ARCH_SH))
#else
# error Unknown CPU type
#endif
		return 0;

	return 1;
}
#endif

#endif /* __IMAGE_H__ */
