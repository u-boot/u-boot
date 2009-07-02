/*
 * (C) Copyright 2008 Semihalf
 *
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

extern	unsigned long	crc32 (unsigned long crc, const char *buf, unsigned int len);
static	void		copy_file (int, const char *, int);
static	void		usage (void);
static	void		image_verify_header (char *, int);
static	void		fit_handle_file (void);

char	*datafile;
char	*imagefile;
char	*cmdname;

int dflag    = 0;
int eflag    = 0;
int fflag    = 0;
int lflag    = 0;
int vflag    = 0;
int xflag    = 0;
int opt_os   = IH_OS_LINUX;
int opt_arch = IH_ARCH_PPC;
int opt_type = IH_TYPE_KERNEL;
int opt_comp = IH_COMP_GZIP;
char *opt_dtc = MKIMAGE_DEFAULT_DTC_OPTIONS;

image_header_t header;
image_header_t *hdr = &header;

int
main (int argc, char **argv)
{
	int ifd = -1;
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
				    (opt_arch = genimg_get_arch_id (*++argv)) < 0)
					usage ();
				goto NXTARG;
			case 'C':
				if ((--argc <= 0) ||
				    (opt_comp = genimg_get_comp_id (*++argv)) < 0)
					usage ();
				goto NXTARG;
			case 'D':
				if (--argc <= 0)
					usage ();
				opt_dtc = *++argv;
				goto NXTARG;

			case 'O':
				if ((--argc <= 0) ||
				    (opt_os = genimg_get_os_id (*++argv)) < 0)
					usage ();
				goto NXTARG;
			case 'T':
				if ((--argc <= 0) ||
				    (opt_type = genimg_get_type_id (*++argv)) < 0)
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
			case 'f':
				if (--argc <= 0)
					usage ();
				datafile = *++argv;
				fflag = 1;
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

	if ((argc != 1) ||
		(dflag && (fflag || lflag)) ||
		(fflag && (dflag || lflag)) ||
		(lflag && (dflag || fflag)))
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

	if (!fflag){
		if (lflag) {
			ifd = open (imagefile, O_RDONLY|O_BINARY);
		} else {
			ifd = open (imagefile,
				O_RDWR|O_CREAT|O_TRUNC|O_BINARY, 0666);
		}

		if (ifd < 0) {
			fprintf (stderr, "%s: Can't open %s: %s\n",
				cmdname, imagefile, strerror(errno));
			exit (EXIT_FAILURE);
		}
	}

	if (lflag) {
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

		ptr = mmap(0, sbuf.st_size, PROT_READ, MAP_SHARED, ifd, 0);
		if (ptr == MAP_FAILED) {
			fprintf (stderr, "%s: Can't read %s: %s\n",
				cmdname, imagefile, strerror(errno));
			exit (EXIT_FAILURE);
		}

		if (fdt_check_header (ptr)) {
			/* old-style image */
			image_verify_header ((char *)ptr, sbuf.st_size);
			image_print_contents ((image_header_t *)ptr);
		} else {
			/* FIT image */
			fit_print_contents (ptr);
		}

		(void) munmap((void *)ptr, sbuf.st_size);
		(void) close (ifd);

		exit (EXIT_SUCCESS);
	} else if (fflag) {
		/* Flattened Image Tree (FIT) format  handling */
		debug ("FIT format handling\n");
		fit_handle_file ();
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
				size = cpu_to_uimage (sbuf.st_size);
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

	ptr = mmap(0, sbuf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, ifd, 0);
	if (ptr == MAP_FAILED) {
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

	image_print_contents (hdr);

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

	ptr = mmap(0, sbuf.st_size, PROT_READ, MAP_SHARED, dfd, 0);
	if (ptr == MAP_FAILED) {
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
			 "          -l ==> list image header information\n",
		cmdname);
	fprintf (stderr, "       %s [-x] -A arch -O os -T type -C comp "
			 "-a addr -e ep -n name -d data_file[:data_file...] image\n"
			 "          -A ==> set architecture to 'arch'\n"
			 "          -O ==> set operating system to 'os'\n"
			 "          -T ==> set image type to 'type'\n"
			 "          -C ==> set compression type 'comp'\n"
			 "          -a ==> set load address to 'addr' (hex)\n"
			 "          -e ==> set entry point to 'ep' (hex)\n"
			 "          -n ==> set image name to 'name'\n"
			 "          -d ==> use image data from 'datafile'\n"
			 "          -x ==> set XIP (execute in place)\n",
		cmdname);
	fprintf (stderr, "       %s [-D dtc_options] -f fit-image.its fit-image\n",
		cmdname);

	exit (EXIT_FAILURE);
}

static void
image_verify_header (char *ptr, int image_size)
{
	int len;
	char *data;
	uint32_t checksum;
	image_header_t header;
	image_header_t *hdr = &header;

	/*
	 * create copy of header so that we can blank out the
	 * checksum field for checking - this can't be done
	 * on the PROT_READ mapped data.
	 */
	memcpy (hdr, ptr, sizeof(image_header_t));

	if (be32_to_cpu(hdr->ih_magic) != IH_MAGIC) {
		fprintf (stderr,
			"%s: Bad Magic Number: \"%s\" is no valid image\n",
			cmdname, imagefile);
		exit (EXIT_FAILURE);
	}

	data = (char *)hdr;
	len  = sizeof(image_header_t);

	checksum = be32_to_cpu(hdr->ih_hcrc);
	hdr->ih_hcrc = cpu_to_be32(0);	/* clear for re-calculation */

	if (crc32 (0, data, len) != checksum) {
		fprintf (stderr,
			"%s: ERROR: \"%s\" has bad header checksum!\n",
			cmdname, imagefile);
		exit (EXIT_FAILURE);
	}

	data = ptr + sizeof(image_header_t);
	len  = image_size - sizeof(image_header_t) ;

	if (crc32 (0, data, len) != be32_to_cpu(hdr->ih_dcrc)) {
		fprintf (stderr,
			"%s: ERROR: \"%s\" has corrupted data!\n",
			cmdname, imagefile);
		exit (EXIT_FAILURE);
	}
}

/**
 * fit_handle_file - main FIT file processing function
 *
 * fit_handle_file() runs dtc to convert .its to .itb, includes
 * binary data, updates timestamp property and calculates hashes.
 *
 * datafile  - .its file
 * imagefile - .itb file
 *
 * returns:
 *     only on success, otherwise calls exit (EXIT_FAILURE);
 */
static void fit_handle_file (void)
{
	char tmpfile[MKIMAGE_MAX_TMPFILE_LEN];
	char cmd[MKIMAGE_MAX_DTC_CMDLINE_LEN];
	int tfd;
	struct stat sbuf;
	unsigned char *ptr;

	/* call dtc to include binary properties into the tmp file */
	if (strlen (imagefile) + strlen (MKIMAGE_TMPFILE_SUFFIX) + 1 >
		sizeof (tmpfile)) {
		fprintf (stderr, "%s: Image file name (%s) too long, "
				"can't create tmpfile",
				imagefile, cmdname);
		exit (EXIT_FAILURE);
	}
	sprintf (tmpfile, "%s%s", imagefile, MKIMAGE_TMPFILE_SUFFIX);

	/* dtc -I dts -O -p 200 datafile > tmpfile */
	sprintf (cmd, "%s %s %s > %s",
			MKIMAGE_DTC, opt_dtc, datafile, tmpfile);
	debug ("Trying to execute \"%s\"\n", cmd);
	if (system (cmd) == -1) {
		fprintf (stderr, "%s: system(%s) failed: %s\n",
				cmdname, cmd, strerror(errno));
		unlink (tmpfile);
		exit (EXIT_FAILURE);
	}

	/* load FIT blob into memory */
	tfd = open (tmpfile, O_RDWR|O_BINARY);

	if (tfd < 0) {
		fprintf (stderr, "%s: Can't open %s: %s\n",
				cmdname, tmpfile, strerror(errno));
		unlink (tmpfile);
		exit (EXIT_FAILURE);
	}

	if (fstat (tfd, &sbuf) < 0) {
		fprintf (stderr, "%s: Can't stat %s: %s\n",
				cmdname, tmpfile, strerror(errno));
		unlink (tmpfile);
		exit (EXIT_FAILURE);
	}

	ptr = mmap (0, sbuf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, tfd, 0);
	if (ptr == MAP_FAILED) {
		fprintf (stderr, "%s: Can't read %s: %s\n",
				cmdname, tmpfile, strerror(errno));
		unlink (tmpfile);
		exit (EXIT_FAILURE);
	}

	/* check if ptr has a valid blob */
	if (fdt_check_header (ptr)) {
		fprintf (stderr, "%s: Invalid FIT blob\n", cmdname);
		unlink (tmpfile);
		exit (EXIT_FAILURE);
	}

	/* set hashes for images in the blob */
	if (fit_set_hashes (ptr)) {
		fprintf (stderr, "%s Can't add hashes to FIT blob", cmdname);
		unlink (tmpfile);
		exit (EXIT_FAILURE);
	}

	/* add a timestamp at offset 0 i.e., root  */
	if (fit_set_timestamp (ptr, 0, sbuf.st_mtime)) {
		fprintf (stderr, "%s: Can't add image timestamp\n", cmdname);
		unlink (tmpfile);
		exit (EXIT_FAILURE);
	}
	debug ("Added timestamp successfully\n");

	munmap ((void *)ptr, sbuf.st_size);
	close (tfd);

	if (rename (tmpfile, imagefile) == -1) {
		fprintf (stderr, "%s: Can't rename %s to %s: %s\n",
				cmdname, tmpfile, imagefile, strerror (errno));
		unlink (tmpfile);
		unlink (imagefile);
		exit (EXIT_FAILURE);
	}
}
