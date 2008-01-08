/*
 * (C) Copyright 2000-2004
 * DENX Software Engineering
 * Wolfgang Denk, wd@denx.de
 * All rights reserved.
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

#include "mkimage.h"
#include <image.h>

extern int errno;

#ifndef MAP_FAILED
#define MAP_FAILED (-1)
#endif

char *cmdname;

extern unsigned long crc32 (unsigned long crc, const char *buf, unsigned int len);

typedef struct table_entry {
	int	val;		/* as defined in image.h	*/
	char	*sname;		/* short (input) name		*/
	char	*lname;		/* long (output) name		*/
} table_entry_t;

table_entry_t arch_name[] = {
    {	IH_ARCH_INVALID,	NULL,		"Invalid CPU",	},
    {	IH_ARCH_ALPHA,		"alpha",	"Alpha",	},
    {	IH_ARCH_ARM,		"arm",		"ARM",		},
    {	IH_ARCH_I386,		"x86",		"Intel x86",	},
    {	IH_ARCH_IA64,		"ia64",		"IA64",		},
    {	IH_ARCH_M68K,		"m68k",		"MC68000",	},
    {	IH_ARCH_MICROBLAZE,	"microblaze",	"MicroBlaze",	},
    {	IH_ARCH_MIPS,		"mips",		"MIPS",		},
    {	IH_ARCH_MIPS64,		"mips64",	"MIPS 64 Bit",	},
    {	IH_ARCH_NIOS,		"nios",		"NIOS",		},
    {	IH_ARCH_NIOS2,		"nios2",	"NIOS II",	},
    {	IH_ARCH_PPC,		"ppc",		"PowerPC",	},
    {	IH_ARCH_S390,		"s390",		"IBM S390",	},
    {	IH_ARCH_SH,		"sh",		"SuperH",	},
    {	IH_ARCH_SPARC,		"sparc",	"SPARC",	},
    {	IH_ARCH_SPARC64,	"sparc64",	"SPARC 64 Bit",	},
    {	IH_ARCH_BLACKFIN,	"blackfin",	"Blackfin",	},
    {	IH_ARCH_AVR32,		"avr32",	"AVR32",	},
    {	-1,			"",		"",		},
};

table_entry_t os_name[] = {
    {	IH_OS_INVALID,	NULL,		"Invalid OS",		},
    {	IH_OS_4_4BSD,	"4_4bsd",	"4_4BSD",		},
    {	IH_OS_ARTOS,	"artos",	"ARTOS",		},
    {	IH_OS_DELL,	"dell",		"Dell",			},
    {	IH_OS_ESIX,	"esix",		"Esix",			},
    {	IH_OS_FREEBSD,	"freebsd",	"FreeBSD",		},
    {	IH_OS_IRIX,	"irix",		"Irix",			},
    {	IH_OS_LINUX,	"linux",	"Linux",		},
    {	IH_OS_LYNXOS,	"lynxos",	"LynxOS",		},
    {	IH_OS_NCR,	"ncr",		"NCR",			},
    {	IH_OS_NETBSD,	"netbsd",	"NetBSD",		},
    {	IH_OS_OPENBSD,	"openbsd",	"OpenBSD",		},
    {	IH_OS_PSOS,	"psos",		"pSOS",			},
    {	IH_OS_QNX,	"qnx",		"QNX",			},
    {	IH_OS_RTEMS,	"rtems",	"RTEMS",		},
    {	IH_OS_SCO,	"sco",		"SCO",			},
    {	IH_OS_SOLARIS,	"solaris",	"Solaris",		},
    {	IH_OS_SVR4,	"svr4",		"SVR4",			},
    {	IH_OS_U_BOOT,	"u-boot",	"U-Boot",		},
    {	IH_OS_VXWORKS,	"vxworks",	"VxWorks",		},
    {	-1,		"",		"",			},
};

table_entry_t type_name[] = {
    {	IH_TYPE_INVALID,    NULL,	  "Invalid Image",	},
    {	IH_TYPE_FILESYSTEM, "filesystem", "Filesystem Image",	},
    {	IH_TYPE_FIRMWARE,   "firmware",	  "Firmware",		},
    {	IH_TYPE_KERNEL,	    "kernel",	  "Kernel Image",	},
    {	IH_TYPE_MULTI,	    "multi",	  "Multi-File Image",	},
    {	IH_TYPE_RAMDISK,    "ramdisk",	  "RAMDisk Image",	},
    {	IH_TYPE_SCRIPT,     "script",	  "Script",		},
    {	IH_TYPE_STANDALONE, "standalone", "Standalone Program", },
    {	IH_TYPE_FLATDT,     "flat_dt",    "Flat Device Tree",	},
    {	-1,		    "",		  "",			},
};

table_entry_t comp_name[] = {
    {	IH_COMP_NONE,	"none",		"uncompressed",		},
    {	IH_COMP_BZIP2,	"bzip2",	"bzip2 compressed",	},
    {	IH_COMP_GZIP,	"gzip",		"gzip compressed",	},
    {	-1,		"",		"",			},
};

static	void	copy_file (int, const char *, int);
static	void	usage	(void);
static	void	print_header (image_header_t *);
static	void	print_type (image_header_t *);
static	char	*put_table_entry (table_entry_t *, char *, int);
static	char	*put_arch (int);
static	char	*put_type (int);
static	char	*put_os   (int);
static	char	*put_comp (int);
static	int	get_table_entry (table_entry_t *, char *, char *);
static	int	get_arch(char *);
static	int	get_comp(char *);
static	int	get_os  (char *);
static	int	get_type(char *);


char	*datafile;
char	*imagefile;

int dflag    = 0;
int eflag    = 0;
int lflag    = 0;
int vflag    = 0;
int xflag    = 0;
int opt_os   = IH_OS_LINUX;
int opt_arch = IH_ARCH_PPC;
int opt_type = IH_TYPE_KERNEL;
int opt_comp = IH_COMP_GZIP;

image_header_t header;
image_header_t *hdr = &header;

int
main (int argc, char **argv)
{
	int ifd;
	uint32_t checksum;
	uint32_t addr;
	uint32_t ep;
	struct stat sbuf;
	unsigned char *ptr;
	char *name = "";

	cmdname = *argv;

	addr = ep = 0;

	while (--argc > 0 && **++argv == '-') {
		while (*++*argv) {
			switch (**argv) {
			case 'l':
				lflag = 1;
				break;
			case 'A':
				if ((--argc <= 0) ||
				    (opt_arch = get_arch(*++argv)) < 0)
					usage ();
				goto NXTARG;
			case 'C':
				if ((--argc <= 0) ||
				    (opt_comp = get_comp(*++argv)) < 0)
					usage ();
				goto NXTARG;
			case 'O':
				if ((--argc <= 0) ||
				    (opt_os = get_os(*++argv)) < 0)
					usage ();
				goto NXTARG;
			case 'T':
				if ((--argc <= 0) ||
				    (opt_type = get_type(*++argv)) < 0)
					usage ();
				goto NXTARG;

			case 'a':
				if (--argc <= 0)
					usage ();
				addr = strtoul (*++argv, (char **)&ptr, 16);
				if (*ptr) {
					fprintf (stderr,
						"%s: invalid load address %s\n",
						cmdname, *argv);
					exit (EXIT_FAILURE);
				}
				goto NXTARG;
			case 'd':
				if (--argc <= 0)
					usage ();
				datafile = *++argv;
				dflag = 1;
				goto NXTARG;
			case 'e':
				if (--argc <= 0)
					usage ();
				ep = strtoul (*++argv, (char **)&ptr, 16);
				if (*ptr) {
					fprintf (stderr,
						"%s: invalid entry point %s\n",
						cmdname, *argv);
					exit (EXIT_FAILURE);
				}
				eflag = 1;
				goto NXTARG;
			case 'n':
				if (--argc <= 0)
					usage ();
				name = *++argv;
				goto NXTARG;
			case 'v':
				vflag++;
				break;
			case 'x':
				xflag++;
				break;
			default:
				usage ();
			}
		}
NXTARG:		;
	}

	if ((argc != 1) || ((lflag ^ dflag) == 0))
		usage();

	if (!eflag) {
		ep = addr;
		/* If XIP, entry point must be after the U-Boot header */
		if (xflag)
			ep += image_get_header_size ();
	}

	/*
	 * If XIP, ensure the entry point is equal to the load address plus
	 * the size of the U-Boot header.
	 */
	if (xflag) {
		if (ep != addr + image_get_header_size ()) {
			fprintf (stderr,
				"%s: For XIP, the entry point must be the load addr + %lu\n",
				cmdname,
				(unsigned long)image_get_header_size ());
			exit (EXIT_FAILURE);
		}
	}

	imagefile = *argv;

	if (lflag) {
		ifd = open(imagefile, O_RDONLY|O_BINARY);
	} else {
		ifd = open(imagefile, O_RDWR|O_CREAT|O_TRUNC|O_BINARY, 0666);
	}

	if (ifd < 0) {
		fprintf (stderr, "%s: Can't open %s: %s\n",
			cmdname, imagefile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	if (lflag) {
		int len;
		char *data;
		/*
		 * list header information of existing image
		 */
		if (fstat(ifd, &sbuf) < 0) {
			fprintf (stderr, "%s: Can't stat %s: %s\n",
				cmdname, imagefile, strerror(errno));
			exit (EXIT_FAILURE);
		}

		if ((unsigned)sbuf.st_size < image_get_header_size ()) {
			fprintf (stderr,
				"%s: Bad size: \"%s\" is no valid image\n",
				cmdname, imagefile);
			exit (EXIT_FAILURE);
		}

		ptr = (unsigned char *)mmap(0, sbuf.st_size,
					    PROT_READ, MAP_SHARED, ifd, 0);
		if ((caddr_t)ptr == (caddr_t)-1) {
			fprintf (stderr, "%s: Can't read %s: %s\n",
				cmdname, imagefile, strerror(errno));
			exit (EXIT_FAILURE);
		}

		/*
		 * image_check_hcrc() creates copy of header so that
		 * we can blank out the checksum field for checking -
		 * this can't be done on the PROT_READ mapped data.
		 */
		hdr = (image_header_t *)ptr;

		if (!image_check_magic (hdr)) {
			fprintf (stderr,
				"%s: Bad Magic Number: \"%s\" is no valid image\n",
				cmdname, imagefile);
			exit (EXIT_FAILURE);
		}

		if (!image_check_hcrc (hdr)) {
			fprintf (stderr,
				"%s: ERROR: \"%s\" has bad header checksum!\n",
				cmdname, imagefile);
			exit (EXIT_FAILURE);
		}

		data = (char *)image_get_data (hdr);
		len  = sbuf.st_size - image_get_header_size ();

		if (crc32(0, data, len) != image_get_dcrc (hdr)) {
			fprintf (stderr,
				"%s: ERROR: \"%s\" has corrupted data!\n",
				cmdname, imagefile);
			exit (EXIT_FAILURE);
		}

		/* for multi-file images we need the data part, too */
		print_header ((image_header_t *)ptr);

		(void) munmap((void *)ptr, sbuf.st_size);
		(void) close (ifd);

		exit (EXIT_SUCCESS);
	}

	/*
	 * Must be -w then:
	 *
	 * write dummy header, to be fixed later
	 */
	memset (hdr, 0, image_get_header_size ());

	if (write(ifd, hdr, image_get_header_size ()) != image_get_header_size ()) {
		fprintf (stderr, "%s: Write error on %s: %s\n",
			cmdname, imagefile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	if (opt_type == IH_TYPE_MULTI || opt_type == IH_TYPE_SCRIPT) {
		char *file = datafile;
		uint32_t size;

		for (;;) {
			char *sep = NULL;

			if (file) {
				if ((sep = strchr(file, ':')) != NULL) {
					*sep = '\0';
				}

				if (stat (file, &sbuf) < 0) {
					fprintf (stderr, "%s: Can't stat %s: %s\n",
						cmdname, file, strerror(errno));
					exit (EXIT_FAILURE);
				}
				size = cpu_to_image (sbuf.st_size);
			} else {
				size = 0;
			}

			if (write(ifd, (char *)&size, sizeof(size)) != sizeof(size)) {
				fprintf (stderr, "%s: Write error on %s: %s\n",
					cmdname, imagefile, strerror(errno));
				exit (EXIT_FAILURE);
			}

			if (!file) {
				break;
			}

			if (sep) {
				*sep = ':';
				file = sep + 1;
			} else {
				file = NULL;
			}
		}

		file = datafile;

		for (;;) {
			char *sep = strchr(file, ':');
			if (sep) {
				*sep = '\0';
				copy_file (ifd, file, 1);
				*sep++ = ':';
				file = sep;
			} else {
				copy_file (ifd, file, 0);
				break;
			}
		}
	} else {
		copy_file (ifd, datafile, 0);
	}

	/* We're a bit of paranoid */
#if defined(_POSIX_SYNCHRONIZED_IO) && !defined(__sun__) && !defined(__FreeBSD__) && !defined(__APPLE__)
	(void) fdatasync (ifd);
#else
	(void) fsync (ifd);
#endif

	if (fstat(ifd, &sbuf) < 0) {
		fprintf (stderr, "%s: Can't stat %s: %s\n",
			cmdname, imagefile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	ptr = (unsigned char *)mmap(0, sbuf.st_size,
				    PROT_READ|PROT_WRITE, MAP_SHARED, ifd, 0);
	if (ptr == (unsigned char *)MAP_FAILED) {
		fprintf (stderr, "%s: Can't map %s: %s\n",
			cmdname, imagefile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	hdr = (image_header_t *)ptr;

	checksum = crc32 (0,
			  (const char *)(ptr + image_get_header_size ()),
			  sbuf.st_size - image_get_header_size ()
			 );

	/* Build new header */
	image_set_magic (hdr, IH_MAGIC);
	image_set_time (hdr, sbuf.st_mtime);
	image_set_size (hdr, sbuf.st_size - image_get_header_size ());
	image_set_load (hdr, addr);
	image_set_ep (hdr, ep);
	image_set_dcrc (hdr, checksum);
	image_set_os (hdr, opt_os);
	image_set_arch (hdr, opt_arch);
	image_set_type (hdr, opt_type);
	image_set_comp (hdr, opt_comp);

	image_set_name (hdr, name);

	checksum = crc32 (0, (const char *)hdr, image_get_header_size ());

	image_set_hcrc (hdr, checksum);

	print_header (hdr);

	(void) munmap((void *)ptr, sbuf.st_size);

	/* We're a bit of paranoid */
#if defined(_POSIX_SYNCHRONIZED_IO) && !defined(__sun__) && !defined(__FreeBSD__) && !defined(__APPLE__)
	(void) fdatasync (ifd);
#else
	(void) fsync (ifd);
#endif

	if (close(ifd)) {
		fprintf (stderr, "%s: Write error on %s: %s\n",
			cmdname, imagefile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	exit (EXIT_SUCCESS);
}

static void
copy_file (int ifd, const char *datafile, int pad)
{
	int dfd;
	struct stat sbuf;
	unsigned char *ptr;
	int tail;
	int zero = 0;
	int offset = 0;
	int size;

	if (vflag) {
		fprintf (stderr, "Adding Image %s\n", datafile);
	}

	if ((dfd = open(datafile, O_RDONLY|O_BINARY)) < 0) {
		fprintf (stderr, "%s: Can't open %s: %s\n",
			cmdname, datafile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	if (fstat(dfd, &sbuf) < 0) {
		fprintf (stderr, "%s: Can't stat %s: %s\n",
			cmdname, datafile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	ptr = (unsigned char *)mmap(0, sbuf.st_size,
				    PROT_READ, MAP_SHARED, dfd, 0);
	if (ptr == (unsigned char *)MAP_FAILED) {
		fprintf (stderr, "%s: Can't read %s: %s\n",
			cmdname, datafile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	if (xflag) {
		unsigned char *p = NULL;
		/*
		 * XIP: do not append the image_header_t at the
		 * beginning of the file, but consume the space
		 * reserved for it.
		 */

		if ((unsigned)sbuf.st_size < image_get_header_size ()) {
			fprintf (stderr,
				"%s: Bad size: \"%s\" is too small for XIP\n",
				cmdname, datafile);
			exit (EXIT_FAILURE);
		}

		for (p = ptr; p < ptr + image_get_header_size (); p++) {
			if ( *p != 0xff ) {
				fprintf (stderr,
					"%s: Bad file: \"%s\" has invalid buffer for XIP\n",
					cmdname, datafile);
				exit (EXIT_FAILURE);
			}
		}

		offset = image_get_header_size ();
	}

	size = sbuf.st_size - offset;
	if (write(ifd, ptr + offset, size) != size) {
		fprintf (stderr, "%s: Write error on %s: %s\n",
			cmdname, imagefile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	if (pad && ((tail = size % 4) != 0)) {

		if (write(ifd, (char *)&zero, 4-tail) != 4-tail) {
			fprintf (stderr, "%s: Write error on %s: %s\n",
				cmdname, imagefile, strerror(errno));
			exit (EXIT_FAILURE);
		}
	}

	(void) munmap((void *)ptr, sbuf.st_size);
	(void) close (dfd);
}

void
usage ()
{
	fprintf (stderr, "Usage: %s -l image\n"
			 "          -l ==> list image header information\n"
			 "       %s [-x] -A arch -O os -T type -C comp "
			 "-a addr -e ep -n name -d data_file[:data_file...] image\n",
		cmdname, cmdname);
	fprintf (stderr, "          -A ==> set architecture to 'arch'\n"
			 "          -O ==> set operating system to 'os'\n"
			 "          -T ==> set image type to 'type'\n"
			 "          -C ==> set compression type 'comp'\n"
			 "          -a ==> set load address to 'addr' (hex)\n"
			 "          -e ==> set entry point to 'ep' (hex)\n"
			 "          -n ==> set image name to 'name'\n"
			 "          -d ==> use image data from 'datafile'\n"
			 "          -x ==> set XIP (execute in place)\n"
		);
	exit (EXIT_FAILURE);
}

static void
print_header (image_header_t *hdr)
{
	time_t timestamp;
	uint32_t size;

	timestamp = (time_t)image_get_time (hdr);
	size = image_get_data_size (hdr);

	printf ("Image Name:   %.*s\n", IH_NMLEN, image_get_name (hdr));
	printf ("Created:      %s", ctime(&timestamp));
	printf ("Image Type:   "); print_type(hdr);
	printf ("Data Size:    %d Bytes = %.2f kB = %.2f MB\n",
		size, (double)size / 1.024e3, (double)size / 1.048576e6 );
	printf ("Load Address: 0x%08X\n", image_get_load (hdr));
	printf ("Entry Point:  0x%08X\n", image_get_ep (hdr));

	if (image_check_type (hdr, IH_TYPE_MULTI) ||
			image_check_type (hdr, IH_TYPE_SCRIPT)) {
		int i, ptrs;
		uint32_t pos;
		uint32_t *len_ptr = (uint32_t *) (
					(unsigned long)hdr + image_get_header_size ()
				);

		/* determine number of images first (to calculate image offsets) */
		for (i=0; len_ptr[i]; ++i)	/* null pointer terminates list */
			;
		ptrs = i;		/* null pointer terminates list */

		pos = image_get_header_size () + ptrs * sizeof(long);
		printf ("Contents:\n");
		for (i=0; len_ptr[i]; ++i) {
			size = image_to_cpu (len_ptr[i]);

			printf ("   Image %d: %8d Bytes = %4d kB = %d MB\n",
				i, size, size>>10, size>>20);
			if (image_check_type (hdr, IH_TYPE_SCRIPT) && i > 0) {
				/*
				 * the user may need to know offsets
				 * if planning to do something with
				 * multiple files
				 */
				printf ("    Offset = %08X\n", pos);
			}
			/* copy_file() will pad the first files to even word align */
			size += 3;
			size &= ~3;
			pos += size;
		}
	}
}


static void
print_type (image_header_t *hdr)
{
	printf ("%s %s %s (%s)\n",
		put_arch (image_get_arch (hdr)),
		put_os   (image_get_os (hdr)),
		put_type (image_get_type (hdr)),
		put_comp (image_get_comp (hdr))
	);
}

static char *put_arch (int arch)
{
	return (put_table_entry(arch_name, "Unknown Architecture", arch));
}

static char *put_os (int os)
{
	return (put_table_entry(os_name, "Unknown OS", os));
}

static char *put_type (int type)
{
	return (put_table_entry(type_name, "Unknown Image", type));
}

static char *put_comp (int comp)
{
	return (put_table_entry(comp_name, "Unknown Compression", comp));
}

static char *put_table_entry (table_entry_t *table, char *msg, int type)
{
	for (; table->val>=0; ++table) {
		if (table->val == type)
			return (table->lname);
	}
	return (msg);
}

static int get_arch(char *name)
{
	return (get_table_entry(arch_name, "CPU", name));
}


static int get_comp(char *name)
{
	return (get_table_entry(comp_name, "Compression", name));
}


static int get_os (char *name)
{
	return (get_table_entry(os_name, "OS", name));
}


static int get_type(char *name)
{
	return (get_table_entry(type_name, "Image", name));
}

static int get_table_entry (table_entry_t *table, char *msg, char *name)
{
	table_entry_t *t;
	int first = 1;

	for (t=table; t->val>=0; ++t) {
		if (t->sname && strcasecmp(t->sname, name)==0)
			return (t->val);
	}
	fprintf (stderr, "\nInvalid %s Type - valid names are", msg);
	for (t=table; t->val>=0; ++t) {
		if (t->sname == NULL)
			continue;
		fprintf (stderr, "%c %s", (first) ? ':' : ',', t->sname);
		first = 0;
	}
	fprintf (stderr, "\n");
	return (-1);
}
