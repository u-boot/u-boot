/*
 * (C) Copyright 2008 Semihalf
 *
 * (C) Copyright 2000-2009
 * DENX Software Engineering
 * Wolfgang Denk, wd@denx.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "mkimage.h"
#include <image.h>
#include <version.h>

static void copy_file(int, const char *, int);
static void usage(void);

/* parameters initialized by core will be used by the image type code */
struct image_tool_params params = {
	.os = IH_OS_LINUX,
	.arch = IH_ARCH_PPC,
	.type = IH_TYPE_KERNEL,
	.comp = IH_COMP_GZIP,
	.dtc = MKIMAGE_DEFAULT_DTC_OPTIONS,
	.imagename = "",
	.imagename2 = "",
};

static int h_compare_image_name(const void *vtype1, const void *vtype2)
{
	const int *type1 = vtype1;
	const int *type2 = vtype2;
	const char *name1 = genimg_get_type_short_name(*type1);
	const char *name2 = genimg_get_type_short_name(*type2);

	return strcmp(name1, name2);
}

/* Show all image types supported by mkimage */
static void show_image_types(void)
{
	struct image_type_params *tparams;
	int order[IH_TYPE_COUNT];
	int count;
	int type;
	int i;

	/* Sort the names in order of short name for easier reading */
	memset(order, '\0', sizeof(order));
	for (count = 0, type = 0; type < IH_TYPE_COUNT; type++) {
		tparams = imagetool_get_type(type);
		if (tparams)
			order[count++] = type;
	}
	qsort(order, count, sizeof(int), h_compare_image_name);

	fprintf(stderr, "\nInvalid image type. Supported image types:\n");
	for (i = 0; i < count; i++) {
		type = order[i];
		tparams = imagetool_get_type(type);
		if (tparams) {
			fprintf(stderr, "\t%-15s  %s\n",
				genimg_get_type_short_name(type),
				genimg_get_type_name(type));
		}
	}
	fprintf(stderr, "\n");
}

int main(int argc, char **argv)
{
	int ifd = -1;
	struct stat sbuf;
	char *ptr;
	int retval = 0;
	struct image_type_params *tparams = NULL;
	int pad_len = 0;
	int dfd;

	params.cmdname = *argv;
	params.addr = params.ep = 0;

	while (--argc > 0 && **++argv == '-') {
		while (*++*argv) {
			switch (**argv) {
			case 'l':
				params.lflag = 1;
				break;
			case 'A':
				if ((--argc <= 0) ||
					(params.arch =
					genimg_get_arch_id (*++argv)) < 0)
					usage ();
				goto NXTARG;
			case 'c':
				if (--argc <= 0)
					usage();
				params.comment = *++argv;
				goto NXTARG;
			case 'C':
				if ((--argc <= 0) ||
					(params.comp =
					genimg_get_comp_id (*++argv)) < 0)
					usage ();
				goto NXTARG;
			case 'D':
				if (--argc <= 0)
					usage ();
				params.dtc = *++argv;
				goto NXTARG;

			case 'O':
				if ((--argc <= 0) ||
					(params.os =
					genimg_get_os_id (*++argv)) < 0)
					usage ();
				goto NXTARG;
			case 'T':
				params.type = -1;
				if (--argc >= 0 && argv[1]) {
					params.type =
						genimg_get_type_id(*++argv);
				}
				if (params.type < 0) {
					show_image_types();
					usage();
				}
				goto NXTARG;
			case 'a':
				if (--argc <= 0)
					usage ();
				params.addr = strtoul (*++argv, &ptr, 16);
				if (*ptr) {
					fprintf (stderr,
						"%s: invalid load address %s\n",
						params.cmdname, *argv);
					exit (EXIT_FAILURE);
				}
				goto NXTARG;
			case 'd':
				if (--argc <= 0)
					usage ();
				params.datafile = *++argv;
				params.dflag = 1;
				goto NXTARG;
			case 'e':
				if (--argc <= 0)
					usage ();
				params.ep = strtoul (*++argv, &ptr, 16);
				if (*ptr) {
					fprintf (stderr,
						"%s: invalid entry point %s\n",
						params.cmdname, *argv);
					exit (EXIT_FAILURE);
				}
				params.eflag = 1;
				goto NXTARG;
			case 'f':
				if (--argc <= 0)
					usage ();
				params.datafile = *++argv;
				/* no break */
			case 'F':
				/*
				 * The flattened image tree (FIT) format
				 * requires a flattened device tree image type
				 */
				params.type = IH_TYPE_FLATDT;
				params.fflag = 1;
				goto NXTARG;
			case 'k':
				if (--argc <= 0)
					usage();
				params.keydir = *++argv;
				goto NXTARG;
			case 'K':
				if (--argc <= 0)
					usage();
				params.keydest = *++argv;
				goto NXTARG;
			case 'n':
				if (--argc <= 0)
					usage ();
				params.imagename = *++argv;
				goto NXTARG;
			case 'r':
				params.require_keys = 1;
				break;
			case 'R':
				if (--argc <= 0)
					usage();
				/*
				 * This entry is for the second configuration
				 * file, if only one is not enough.
				 */
				params.imagename2 = *++argv;
				goto NXTARG;
			case 's':
				params.skipcpy = 1;
				break;
			case 'v':
				params.vflag++;
				break;
			case 'V':
				printf("mkimage version %s\n", PLAIN_VERSION);
				exit(EXIT_SUCCESS);
			case 'x':
				params.xflag++;
				break;
			default:
				usage ();
			}
		}
NXTARG:		;
	}

	if (argc != 1)
		usage ();

	/* set tparams as per input type_id */
	tparams = imagetool_get_type(params.type);
	if (tparams == NULL) {
		fprintf (stderr, "%s: unsupported type %s\n",
			params.cmdname, genimg_get_type_name(params.type));
		exit (EXIT_FAILURE);
	}

	/*
	 * check the passed arguments parameters meets the requirements
	 * as per image type to be generated/listed
	 */
	if (tparams->check_params)
		if (tparams->check_params (&params))
			usage ();

	if (!params.eflag) {
		params.ep = params.addr;
		/* If XIP, entry point must be after the U-Boot header */
		if (params.xflag)
			params.ep += tparams->header_size;
	}

	params.imagefile = *argv;

	if (params.fflag){
		if (tparams->fflag_handle)
			/*
			 * in some cases, some additional processing needs
			 * to be done if fflag is defined
			 *
			 * For ex. fit_handle_file for Fit file support
			 */
			retval = tparams->fflag_handle(&params);

		if (retval != EXIT_SUCCESS)
			exit (retval);
	}

	if (params.lflag || params.fflag) {
		ifd = open (params.imagefile, O_RDONLY|O_BINARY);
	} else {
		ifd = open (params.imagefile,
			O_RDWR|O_CREAT|O_TRUNC|O_BINARY, 0666);
	}

	if (ifd < 0) {
		fprintf (stderr, "%s: Can't open %s: %s\n",
			params.cmdname, params.imagefile,
			strerror(errno));
		exit (EXIT_FAILURE);
	}

	if (params.lflag || params.fflag) {
		/*
		 * list header information of existing image
		 */
		if (fstat(ifd, &sbuf) < 0) {
			fprintf (stderr, "%s: Can't stat %s: %s\n",
				params.cmdname, params.imagefile,
				strerror(errno));
			exit (EXIT_FAILURE);
		}

		if ((unsigned)sbuf.st_size < tparams->header_size) {
			fprintf (stderr,
				"%s: Bad size: \"%s\" is not valid image\n",
				params.cmdname, params.imagefile);
			exit (EXIT_FAILURE);
		}

		ptr = mmap(0, sbuf.st_size, PROT_READ, MAP_SHARED, ifd, 0);
		if (ptr == MAP_FAILED) {
			fprintf (stderr, "%s: Can't read %s: %s\n",
				params.cmdname, params.imagefile,
				strerror(errno));
			exit (EXIT_FAILURE);
		}

		/*
		 * scan through mkimage registry for all supported image types
		 * and verify the input image file header for match
		 * Print the image information for matched image type
		 * Returns the error code if not matched
		 */
		retval = imagetool_verify_print_header(ptr, &sbuf,
				tparams, &params);

		(void) munmap((void *)ptr, sbuf.st_size);
		(void) close (ifd);

		exit (retval);
	}

	dfd = open(params.datafile, O_RDONLY | O_BINARY);
	if (dfd < 0) {
		fprintf(stderr, "%s: Can't open %s: %s\n",
			params.cmdname, params.datafile, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (fstat(dfd, &sbuf) < 0) {
		fprintf(stderr, "%s: Can't stat %s: %s\n",
			params.cmdname, params.datafile, strerror(errno));
		exit(EXIT_FAILURE);
	}

	params.file_size = sbuf.st_size + tparams->header_size;
	close(dfd);

	/*
	 * In case there an header with a variable
	 * length will be added, the corresponding
	 * function is called. This is responsible to
	 * allocate memory for the header itself.
	 */
	if (tparams->vrec_header)
		pad_len = tparams->vrec_header(&params, tparams);
	else
		memset(tparams->hdr, 0, tparams->header_size);

	if (write(ifd, tparams->hdr, tparams->header_size)
					!= tparams->header_size) {
		fprintf (stderr, "%s: Write error on %s: %s\n",
			params.cmdname, params.imagefile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	if (!params.skipcpy) {
		if (params.type == IH_TYPE_MULTI ||
		    params.type == IH_TYPE_SCRIPT) {
			char *file = params.datafile;
			uint32_t size;

			for (;;) {
				char *sep = NULL;

				if (file) {
					if ((sep = strchr(file, ':')) != NULL) {
						*sep = '\0';
					}

					if (stat (file, &sbuf) < 0) {
						fprintf (stderr, "%s: Can't stat %s: %s\n",
							 params.cmdname, file, strerror(errno));
						exit (EXIT_FAILURE);
					}
					size = cpu_to_uimage (sbuf.st_size);
				} else {
					size = 0;
				}

				if (write(ifd, (char *)&size, sizeof(size)) != sizeof(size)) {
					fprintf (stderr, "%s: Write error on %s: %s\n",
						 params.cmdname, params.imagefile,
						 strerror(errno));
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

			file = params.datafile;

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
		} else if (params.type == IH_TYPE_PBLIMAGE) {
			/* PBL has special Image format, implements its' own */
			pbl_load_uboot(ifd, &params);
		} else {
			copy_file(ifd, params.datafile, pad_len);
		}
	}

	/* We're a bit of paranoid */
#if defined(_POSIX_SYNCHRONIZED_IO) && \
   !defined(__sun__) && \
   !defined(__FreeBSD__) && \
   !defined(__OpenBSD__) && \
   !defined(__APPLE__)
	(void) fdatasync (ifd);
#else
	(void) fsync (ifd);
#endif

	if (fstat(ifd, &sbuf) < 0) {
		fprintf (stderr, "%s: Can't stat %s: %s\n",
			params.cmdname, params.imagefile, strerror(errno));
		exit (EXIT_FAILURE);
	}
	params.file_size = sbuf.st_size;

	ptr = mmap(0, sbuf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, ifd, 0);
	if (ptr == MAP_FAILED) {
		fprintf (stderr, "%s: Can't map %s: %s\n",
			params.cmdname, params.imagefile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	/* Setup the image header as per input image type*/
	if (tparams->set_header)
		tparams->set_header (ptr, &sbuf, ifd, &params);
	else {
		fprintf (stderr, "%s: Can't set header for %s: %s\n",
			params.cmdname, tparams->name, strerror(errno));
		exit (EXIT_FAILURE);
	}

	/* Print the image information by processing image header */
	if (tparams->print_header)
		tparams->print_header (ptr);
	else {
		fprintf (stderr, "%s: Can't print header for %s: %s\n",
			params.cmdname, tparams->name, strerror(errno));
		exit (EXIT_FAILURE);
	}

	(void) munmap((void *)ptr, sbuf.st_size);

	/* We're a bit of paranoid */
#if defined(_POSIX_SYNCHRONIZED_IO) && \
   !defined(__sun__) && \
   !defined(__FreeBSD__) && \
   !defined(__OpenBSD__) && \
   !defined(__APPLE__)
	(void) fdatasync (ifd);
#else
	(void) fsync (ifd);
#endif

	if (close(ifd)) {
		fprintf (stderr, "%s: Write error on %s: %s\n",
			params.cmdname, params.imagefile, strerror(errno));
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
	uint8_t zeros[4096];
	int offset = 0;
	int size;
	struct image_type_params *tparams = imagetool_get_type(params.type);

	memset(zeros, 0, sizeof(zeros));

	if (params.vflag) {
		fprintf (stderr, "Adding Image %s\n", datafile);
	}

	if ((dfd = open(datafile, O_RDONLY|O_BINARY)) < 0) {
		fprintf (stderr, "%s: Can't open %s: %s\n",
			params.cmdname, datafile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	if (fstat(dfd, &sbuf) < 0) {
		fprintf (stderr, "%s: Can't stat %s: %s\n",
			params.cmdname, datafile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	ptr = mmap(0, sbuf.st_size, PROT_READ, MAP_SHARED, dfd, 0);
	if (ptr == MAP_FAILED) {
		fprintf (stderr, "%s: Can't read %s: %s\n",
			params.cmdname, datafile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	if (params.xflag) {
		unsigned char *p = NULL;
		/*
		 * XIP: do not append the image_header_t at the
		 * beginning of the file, but consume the space
		 * reserved for it.
		 */

		if ((unsigned)sbuf.st_size < tparams->header_size) {
			fprintf (stderr,
				"%s: Bad size: \"%s\" is too small for XIP\n",
				params.cmdname, datafile);
			exit (EXIT_FAILURE);
		}

		for (p = ptr; p < ptr + tparams->header_size; p++) {
			if ( *p != 0xff ) {
				fprintf (stderr,
					"%s: Bad file: \"%s\" has invalid buffer for XIP\n",
					params.cmdname, datafile);
				exit (EXIT_FAILURE);
			}
		}

		offset = tparams->header_size;
	}

	size = sbuf.st_size - offset;
	if (write(ifd, ptr + offset, size) != size) {
		fprintf (stderr, "%s: Write error on %s: %s\n",
			params.cmdname, params.imagefile, strerror(errno));
		exit (EXIT_FAILURE);
	}

	tail = size % 4;
	if ((pad == 1) && (tail != 0)) {

		if (write(ifd, (char *)&zero, 4-tail) != 4-tail) {
			fprintf (stderr, "%s: Write error on %s: %s\n",
				params.cmdname, params.imagefile,
				strerror(errno));
			exit (EXIT_FAILURE);
		}
	} else if (pad > 1) {
		while (pad > 0) {
			int todo = sizeof(zeros);

			if (todo > pad)
				todo = pad;
			if (write(ifd, (char *)&zeros, todo) != todo) {
				fprintf(stderr, "%s: Write error on %s: %s\n",
					params.cmdname, params.imagefile,
					strerror(errno));
				exit(EXIT_FAILURE);
			}
			pad -= todo;
		}
	}

	(void) munmap((void *)ptr, sbuf.st_size);
	(void) close (dfd);
}

static void usage(void)
{
	fprintf (stderr, "Usage: %s -l image\n"
			 "          -l ==> list image header information\n",
		params.cmdname);
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
		params.cmdname);
	fprintf(stderr, "       %s [-D dtc_options] [-f fit-image.its|-F] fit-image\n",
		params.cmdname);
	fprintf(stderr, "          -D => set all options for device tree compiler\n"
			"          -f => input filename for FIT source\n");
#ifdef CONFIG_FIT_SIGNATURE
	fprintf(stderr, "Signing / verified boot options: [-k keydir] [-K dtb] [ -c <comment>] [-r]\n"
			"          -k => set directory containing private keys\n"
			"          -K => write public keys to this .dtb file\n"
			"          -c => add comment in signature node\n"
			"          -F => re-sign existing FIT image\n"
			"          -r => mark keys used as 'required' in dtb\n");
#else
	fprintf(stderr, "Signing / verified boot not supported (CONFIG_FIT_SIGNATURE undefined)\n");
#endif
	fprintf (stderr, "       %s -V ==> print version information and exit\n",
		params.cmdname);
	fprintf(stderr, "Use -T to see a list of available image types\n");

	exit (EXIT_FAILURE);
}
