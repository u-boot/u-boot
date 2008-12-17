/*
 * (C) Copyright 2000-2008
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2008
 * Guennadi Liakhovetski, DENX Software Engineering, lg@denx.de.
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

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef MTD_OLD
# include <stdint.h>
# include <linux/mtd/mtd.h>
#else
# define  __user	/* nothing */
# include <mtd/mtd-user.h>
#endif

#include "fw_env.h"

#define	CMD_GETENV	"fw_printenv"
#define	CMD_SETENV	"fw_setenv"

#define min(x, y) ({				\
	typeof(x) _min1 = (x);			\
	typeof(y) _min2 = (y);			\
	(void) (&_min1 == &_min2);		\
	_min1 < _min2 ? _min1 : _min2; })

struct envdev_s {
	char devname[16];		/* Device name */
	ulong devoff;			/* Device offset */
	ulong env_size;			/* environment size */
	ulong erase_size;		/* device erase size */
	ulong env_sectors;		/* number of environment sectors */
	uint8_t mtd_type;		/* type of the MTD device */
};

static struct envdev_s envdevices[2] =
{
	{
		.mtd_type = MTD_ABSENT,
	}, {
		.mtd_type = MTD_ABSENT,
	},
};
static int dev_current;

#define DEVNAME(i)    envdevices[(i)].devname
#define DEVOFFSET(i)  envdevices[(i)].devoff
#define ENVSIZE(i)    envdevices[(i)].env_size
#define DEVESIZE(i)   envdevices[(i)].erase_size
#define ENVSECTORS(i) envdevices[(i)].env_sectors
#define DEVTYPE(i)    envdevices[(i)].mtd_type

#define CONFIG_ENV_SIZE ENVSIZE(dev_current)

#define ENV_SIZE      getenvsize()

struct env_image_single {
	uint32_t	crc;	/* CRC32 over data bytes    */
	char		data[];
};

struct env_image_redundant {
	uint32_t	crc;	/* CRC32 over data bytes    */
	unsigned char	flags;	/* active or obsolete */
	char		data[];
};

enum flag_scheme {
	FLAG_NONE,
	FLAG_BOOLEAN,
	FLAG_INCREMENTAL,
};

struct environment {
	void			*image;
	uint32_t		*crc;
	unsigned char		*flags;
	char			*data;
	enum flag_scheme	flag_scheme;
};

static struct environment environment = {
	.flag_scheme = FLAG_NONE,
};

static int HaveRedundEnv = 0;

static unsigned char active_flag = 1;
/* obsolete_flag must be 0 to efficiently set it on NOR flash without erasing */
static unsigned char obsolete_flag = 0;


#define XMK_STR(x)	#x
#define MK_STR(x)	XMK_STR(x)

static char default_environment[] = {
#if defined(CONFIG_BOOTARGS)
	"bootargs=" CONFIG_BOOTARGS "\0"
#endif
#if defined(CONFIG_BOOTCOMMAND)
	"bootcmd=" CONFIG_BOOTCOMMAND "\0"
#endif
#if defined(CONFIG_RAMBOOTCOMMAND)
	"ramboot=" CONFIG_RAMBOOTCOMMAND "\0"
#endif
#if defined(CONFIG_NFSBOOTCOMMAND)
	"nfsboot=" CONFIG_NFSBOOTCOMMAND "\0"
#endif
#if defined(CONFIG_BOOTDELAY) && (CONFIG_BOOTDELAY >= 0)
	"bootdelay=" MK_STR (CONFIG_BOOTDELAY) "\0"
#endif
#if defined(CONFIG_BAUDRATE) && (CONFIG_BAUDRATE >= 0)
	"baudrate=" MK_STR (CONFIG_BAUDRATE) "\0"
#endif
#ifdef	CONFIG_LOADS_ECHO
	"loads_echo=" MK_STR (CONFIG_LOADS_ECHO) "\0"
#endif
#ifdef	CONFIG_ETHADDR
	"ethaddr=" MK_STR (CONFIG_ETHADDR) "\0"
#endif
#ifdef	CONFIG_ETH1ADDR
	"eth1addr=" MK_STR (CONFIG_ETH1ADDR) "\0"
#endif
#ifdef	CONFIG_ETH2ADDR
	"eth2addr=" MK_STR (CONFIG_ETH2ADDR) "\0"
#endif
#ifdef	CONFIG_ETH3ADDR
	"eth3addr=" MK_STR (CONFIG_ETH3ADDR) "\0"
#endif
#ifdef	CONFIG_ETH4ADDR
	"eth4addr=" MK_STR (CONFIG_ETH4ADDR) "\0"
#endif
#ifdef	CONFIG_ETH5ADDR
	"eth5addr=" MK_STR (CONFIG_ETH5ADDR) "\0"
#endif
#ifdef	CONFIG_ETHPRIME
	"ethprime=" CONFIG_ETHPRIME "\0"
#endif
#ifdef	CONFIG_IPADDR
	"ipaddr=" MK_STR (CONFIG_IPADDR) "\0"
#endif
#ifdef	CONFIG_SERVERIP
	"serverip=" MK_STR (CONFIG_SERVERIP) "\0"
#endif
#ifdef	CONFIG_SYS_AUTOLOAD
	"autoload=" CONFIG_SYS_AUTOLOAD "\0"
#endif
#ifdef	CONFIG_ROOTPATH
	"rootpath=" MK_STR (CONFIG_ROOTPATH) "\0"
#endif
#ifdef	CONFIG_GATEWAYIP
	"gatewayip=" MK_STR (CONFIG_GATEWAYIP) "\0"
#endif
#ifdef	CONFIG_NETMASK
	"netmask=" MK_STR (CONFIG_NETMASK) "\0"
#endif
#ifdef	CONFIG_HOSTNAME
	"hostname=" MK_STR (CONFIG_HOSTNAME) "\0"
#endif
#ifdef	CONFIG_BOOTFILE
	"bootfile=" MK_STR (CONFIG_BOOTFILE) "\0"
#endif
#ifdef	CONFIG_LOADADDR
	"loadaddr=" MK_STR (CONFIG_LOADADDR) "\0"
#endif
#ifdef	CONFIG_PREBOOT
	"preboot=" CONFIG_PREBOOT "\0"
#endif
#ifdef	CONFIG_CLOCKS_IN_MHZ
	"clocks_in_mhz=" "1" "\0"
#endif
#if defined(CONFIG_PCI_BOOTDELAY) && (CONFIG_PCI_BOOTDELAY > 0)
	"pcidelay=" MK_STR (CONFIG_PCI_BOOTDELAY) "\0"
#endif
#ifdef  CONFIG_EXTRA_ENV_SETTINGS
	CONFIG_EXTRA_ENV_SETTINGS
#endif
	"\0"		/* Termimate struct environment data with 2 NULs */
};

static int flash_io (int mode);
static char *envmatch (char * s1, char * s2);
static int env_init (void);
static int parse_config (void);

#if defined(CONFIG_FILE)
static int get_config (char *);
#endif
static inline ulong getenvsize (void)
{
	ulong rc = CONFIG_ENV_SIZE - sizeof (long);

	if (HaveRedundEnv)
		rc -= sizeof (char);
	return rc;
}

/*
 * Search the environment for a variable.
 * Return the value, if found, or NULL, if not found.
 */
char *fw_getenv (char *name)
{
	char *env, *nxt;

	if (env_init ())
		return NULL;

	for (env = environment.data; *env; env = nxt + 1) {
		char *val;

		for (nxt = env; *nxt; ++nxt) {
			if (nxt >= &environment.data[ENV_SIZE]) {
				fprintf (stderr, "## Error: "
					"environment not terminated\n");
				return NULL;
			}
		}
		val = envmatch (name, env);
		if (!val)
			continue;
		return val;
	}
	return NULL;
}

/*
 * Print the current definition of one, or more, or all
 * environment variables
 */
int fw_printenv (int argc, char *argv[])
{
	char *env, *nxt;
	int i, n_flag;
	int rc = 0;

	if (env_init ())
		return -1;

	if (argc == 1) {		/* Print all env variables  */
		for (env = environment.data; *env; env = nxt + 1) {
			for (nxt = env; *nxt; ++nxt) {
				if (nxt >= &environment.data[ENV_SIZE]) {
					fprintf (stderr, "## Error: "
						"environment not terminated\n");
					return -1;
				}
			}

			printf ("%s\n", env);
		}
		return 0;
	}

	if (strcmp (argv[1], "-n") == 0) {
		n_flag = 1;
		++argv;
		--argc;
		if (argc != 2) {
			fprintf (stderr, "## Error: "
				"`-n' option requires exactly one argument\n");
			return -1;
		}
	} else {
		n_flag = 0;
	}

	for (i = 1; i < argc; ++i) {	/* print single env variables   */
		char *name = argv[i];
		char *val = NULL;

		for (env = environment.data; *env; env = nxt + 1) {

			for (nxt = env; *nxt; ++nxt) {
				if (nxt >= &environment.data[ENV_SIZE]) {
					fprintf (stderr, "## Error: "
						"environment not terminated\n");
					return -1;
				}
			}
			val = envmatch (name, env);
			if (val) {
				if (!n_flag) {
					fputs (name, stdout);
					putc ('=', stdout);
				}
				puts (val);
				break;
			}
		}
		if (!val) {
			fprintf (stderr, "## Error: \"%s\" not defined\n", name);
			rc = -1;
		}
	}

	return rc;
}

/*
 * Deletes or sets environment variables. Returns -1 and sets errno error codes:
 * 0	  - OK
 * EINVAL - need at least 1 argument
 * EROFS  - certain variables ("ethaddr", "serial#") cannot be
 *	    modified or deleted
 *
 */
int fw_setenv (int argc, char *argv[])
{
	int i, len;
	char *env, *nxt;
	char *oldval = NULL;
	char *name;

	if (argc < 2) {
		errno = EINVAL;
		return -1;
	}

	if (env_init ())
		return -1;

	name = argv[1];

	/*
	 * search if variable with this name already exists
	 */
	for (nxt = env = environment.data; *env; env = nxt + 1) {
		for (nxt = env; *nxt; ++nxt) {
			if (nxt >= &environment.data[ENV_SIZE]) {
				fprintf (stderr, "## Error: "
					"environment not terminated\n");
				errno = EINVAL;
				return -1;
			}
		}
		if ((oldval = envmatch (name, env)) != NULL)
			break;
	}

	/*
	 * Delete any existing definition
	 */
	if (oldval) {
		/*
		 * Ethernet Address and serial# can be set only once
		 */
		if ((strcmp (name, "ethaddr") == 0) ||
			(strcmp (name, "serial#") == 0)) {
			fprintf (stderr, "Can't overwrite \"%s\"\n", name);
			errno = EROFS;
			return -1;
		}

		if (*++nxt == '\0') {
			*env = '\0';
		} else {
			for (;;) {
				*env = *nxt++;
				if ((*env == '\0') && (*nxt == '\0'))
					break;
				++env;
			}
		}
		*++env = '\0';
	}

	/* Delete only ? */
	if (argc < 3)
		goto WRITE_FLASH;

	/*
	 * Append new definition at the end
	 */
	for (env = environment.data; *env || *(env + 1); ++env);
	if (env > environment.data)
		++env;
	/*
	 * Overflow when:
	 * "name" + "=" + "val" +"\0\0"  > CONFIG_ENV_SIZE - (env-environment)
	 */
	len = strlen (name) + 2;
	/* add '=' for first arg, ' ' for all others */
	for (i = 2; i < argc; ++i) {
		len += strlen (argv[i]) + 1;
	}
	if (len > (&environment.data[ENV_SIZE] - env)) {
		fprintf (stderr,
			"Error: environment overflow, \"%s\" deleted\n",
			name);
		return -1;
	}
	while ((*env = *name++) != '\0')
		env++;
	for (i = 2; i < argc; ++i) {
		char *val = argv[i];

		*env = (i == 2) ? '=' : ' ';
		while ((*++env = *val++) != '\0');
	}

	/* end is marked with double '\0' */
	*++env = '\0';

  WRITE_FLASH:

	/*
	 * Update CRC
	 */
	*environment.crc = crc32 (0, (uint8_t *) environment.data, ENV_SIZE);

	/* write environment back to flash */
	if (flash_io (O_RDWR)) {
		fprintf (stderr, "Error: can't write fw_env to flash\n");
		return -1;
	}

	return 0;
}

/*
 * Test for bad block on NAND, just returns 0 on NOR, on NAND:
 * 0	- block is good
 * > 0	- block is bad
 * < 0	- failed to test
 */
static int flash_bad_block (int fd, uint8_t mtd_type, loff_t *blockstart)
{
	if (mtd_type == MTD_NANDFLASH) {
		int badblock = ioctl (fd, MEMGETBADBLOCK, blockstart);

		if (badblock < 0) {
			perror ("Cannot read bad block mark");
			return badblock;
		}

		if (badblock) {
#ifdef DEBUG
			fprintf (stderr, "Bad block at 0x%llx, "
				 "skipping\n", *blockstart);
#endif
			return badblock;
		}
	}

	return 0;
}

/*
 * Read data from flash at an offset into a provided buffer. On NAND it skips
 * bad blocks but makes sure it stays within ENVSECTORS (dev) starting from
 * the DEVOFFSET (dev) block. On NOR the loop is only run once.
 */
static int flash_read_buf (int dev, int fd, void *buf, size_t count,
			   off_t offset, uint8_t mtd_type)
{
	size_t blocklen;	/* erase / write length - one block on NAND,
				   0 on NOR */
	size_t processed = 0;	/* progress counter */
	size_t readlen = count;	/* current read length */
	off_t top_of_range;	/* end of the last block we may use */
	off_t block_seek;	/* offset inside the current block to the start
				   of the data */
	loff_t blockstart;	/* running start of the current block -
				   MEMGETBADBLOCK needs 64 bits */
	int rc;

	/*
	 * Start of the first block to be read, relies on the fact, that
	 * erase sector size is always a power of 2
	 */
	blockstart = offset & ~(DEVESIZE (dev) - 1);

	/* Offset inside a block */
	block_seek = offset - blockstart;

	if (mtd_type == MTD_NANDFLASH) {
		/*
		 * NAND: calculate which blocks we are reading. We have
		 * to read one block at a time to skip bad blocks.
		 */
		blocklen = DEVESIZE (dev);

		/*
		 * To calculate the top of the range, we have to use the
		 * global DEVOFFSET (dev), which can be different from offset
		 */
		top_of_range = (DEVOFFSET (dev) & ~(blocklen - 1)) +
			ENVSECTORS (dev) * blocklen;

		/* Limit to one block for the first read */
		if (readlen > blocklen - block_seek)
			readlen = blocklen - block_seek;
	} else {
		blocklen = 0;
		top_of_range = offset + count;
	}

	/* This only runs once on NOR flash */
	while (processed < count) {
		rc = flash_bad_block (fd, mtd_type, &blockstart);
		if (rc < 0)		/* block test failed */
			return -1;

		if (blockstart + block_seek + readlen > top_of_range) {
			/* End of range is reached */
			fprintf (stderr,
				 "Too few good blocks within range\n");
			return -1;
		}

		if (rc) {		/* block is bad */
			blockstart += blocklen;
			continue;
		}

		/*
		 * If a block is bad, we retry in the next block at the same
		 * offset - see common/env_nand.c::writeenv()
		 */
		lseek (fd, blockstart + block_seek, SEEK_SET);

		rc = read (fd, buf + processed, readlen);
		if (rc != readlen) {
			fprintf (stderr, "Read error on %s: %s\n",
				 DEVNAME (dev), strerror (errno));
			return -1;
		}
#ifdef DEBUG
		fprintf (stderr, "Read 0x%x bytes at 0x%llx\n",
			 rc, blockstart + block_seek);
#endif
		processed += readlen;
		readlen = min (blocklen, count - processed);
		block_seek = 0;
		blockstart += blocklen;
	}

	return processed;
}

/*
 * Write count bytes at offset, but stay within ENVSETCORS (dev) sectors of
 * DEVOFFSET (dev). Similar to the read case above, on NOR we erase and write
 * the whole data at once.
 */
static int flash_write_buf (int dev, int fd, void *buf, size_t count,
			    off_t offset, uint8_t mtd_type)
{
	void *data;
	struct erase_info_user erase;
	size_t blocklen;	/* length of NAND block / NOR erase sector */
	size_t erase_len;	/* whole area that can be erased - may include
				   bad blocks */
	size_t erasesize;	/* erase / write length - one block on NAND,
				   whole area on NOR */
	size_t processed = 0;	/* progress counter */
	size_t write_total;	/* total size to actually write - excludinig
				   bad blocks */
	off_t erase_offset;	/* offset to the first erase block (aligned)
				   below offset */
	off_t block_seek;	/* offset inside the erase block to the start
				   of the data */
	off_t top_of_range;	/* end of the last block we may use */
	loff_t blockstart;	/* running start of the current block -
				   MEMGETBADBLOCK needs 64 bits */
	int rc;

	blocklen = DEVESIZE (dev);

	/* Erase sector size is always a power of 2 */
	top_of_range = (DEVOFFSET (dev) & ~(blocklen - 1)) +
		ENVSECTORS (dev) * blocklen;

	erase_offset = offset & ~(blocklen - 1);

	/* Maximum area we may use */
	erase_len = top_of_range - erase_offset;

	blockstart = erase_offset;
	/* Offset inside a block */
	block_seek = offset - erase_offset;

	/*
	 * Data size we actually have to write: from the start of the block
	 * to the start of the data, then count bytes of data, and to the
	 * end of the block
	 */
	write_total = (block_seek + count + blocklen - 1) & ~(blocklen - 1);

	/*
	 * Support data anywhere within erase sectors: read out the complete
	 * area to be erased, replace the environment image, write the whole
	 * block back again.
	 */
	if (write_total > count) {
		data = malloc (erase_len);
		if (!data) {
			fprintf (stderr,
				 "Cannot malloc %u bytes: %s\n",
				 erase_len, strerror (errno));
			return -1;
		}

		rc = flash_read_buf (dev, fd, data, write_total, erase_offset,
				     mtd_type);
		if (write_total != rc)
			return -1;

		/* Overwrite the old environment */
		memcpy (data + block_seek, buf, count);
	} else {
		/*
		 * We get here, iff offset is block-aligned and count is a
		 * multiple of blocklen - see write_total calculation above
		 */
		data = buf;
	}

	if (mtd_type == MTD_NANDFLASH) {
		/*
		 * NAND: calculate which blocks we are writing. We have
		 * to write one block at a time to skip bad blocks.
		 */
		erasesize = blocklen;
	} else {
		erasesize = erase_len;
	}

	erase.length = erasesize;

	/* This only runs once on NOR flash */
	while (processed < write_total) {
		rc = flash_bad_block (fd, mtd_type, &blockstart);
		if (rc < 0)		/* block test failed */
			return rc;

		if (blockstart + erasesize > top_of_range) {
			fprintf (stderr, "End of range reached, aborting\n");
			return -1;
		}

		if (rc) {		/* block is bad */
			blockstart += blocklen;
			continue;
		}

		erase.start = blockstart;
		ioctl (fd, MEMUNLOCK, &erase);

		if (ioctl (fd, MEMERASE, &erase) != 0) {
			fprintf (stderr, "MTD erase error on %s: %s\n",
				 DEVNAME (dev),
				 strerror (errno));
			return -1;
		}

		if (lseek (fd, blockstart, SEEK_SET) == -1) {
			fprintf (stderr,
				 "Seek error on %s: %s\n",
				 DEVNAME (dev), strerror (errno));
			return -1;
		}

#ifdef DEBUG
		printf ("Write 0x%x bytes at 0x%llx\n", erasesize, blockstart);
#endif
		if (write (fd, data + processed, erasesize) != erasesize) {
			fprintf (stderr, "Write error on %s: %s\n",
				 DEVNAME (dev), strerror (errno));
			return -1;
		}

		ioctl (fd, MEMLOCK, &erase);

		processed  += blocklen;
		block_seek = 0;
		blockstart += blocklen;
	}

	if (write_total > count)
		free (data);

	return processed;
}

/*
 * Set obsolete flag at offset - NOR flash only
 */
static int flash_flag_obsolete (int dev, int fd, off_t offset)
{
	int rc;

	/* This relies on the fact, that obsolete_flag == 0 */
	rc = lseek (fd, offset, SEEK_SET);
	if (rc < 0) {
		fprintf (stderr, "Cannot seek to set the flag on %s \n",
			 DEVNAME (dev));
		return rc;
	}
	rc = write (fd, &obsolete_flag, sizeof (obsolete_flag));
	if (rc < 0)
		perror ("Could not set obsolete flag");

	return rc;
}

static int flash_write (int fd_current, int fd_target, int dev_target)
{
	int rc;

	switch (environment.flag_scheme) {
	case FLAG_NONE:
		break;
	case FLAG_INCREMENTAL:
		(*environment.flags)++;
		break;
	case FLAG_BOOLEAN:
		*environment.flags = active_flag;
		break;
	default:
		fprintf (stderr, "Unimplemented flash scheme %u \n",
			 environment.flag_scheme);
		return -1;
	}

#ifdef DEBUG
	printf ("Writing new environment at 0x%lx on %s\n",
		DEVOFFSET (dev_target), DEVNAME (dev_target));
#endif
	rc = flash_write_buf (dev_target, fd_target, environment.image,
			      CONFIG_ENV_SIZE, DEVOFFSET (dev_target),
			      DEVTYPE(dev_target));
	if (rc < 0)
		return rc;

	if (environment.flag_scheme == FLAG_BOOLEAN) {
		/* Have to set obsolete flag */
		off_t offset = DEVOFFSET (dev_current) +
			offsetof (struct env_image_redundant, flags);
#ifdef DEBUG
		printf ("Setting obsolete flag in environment at 0x%lx on %s\n",
			DEVOFFSET (dev_current), DEVNAME (dev_current));
#endif
		flash_flag_obsolete (dev_current, fd_current, offset);
	}

	return 0;
}

static int flash_read (int fd)
{
	struct mtd_info_user mtdinfo;
	int rc;

	rc = ioctl (fd, MEMGETINFO, &mtdinfo);
	if (rc < 0) {
		perror ("Cannot get MTD information");
		return -1;
	}

	if (mtdinfo.type != MTD_NORFLASH && mtdinfo.type != MTD_NANDFLASH) {
		fprintf (stderr, "Unsupported flash type %u\n", mtdinfo.type);
		return -1;
	}

	DEVTYPE(dev_current) = mtdinfo.type;

	rc = flash_read_buf (dev_current, fd, environment.image, CONFIG_ENV_SIZE,
			     DEVOFFSET (dev_current), mtdinfo.type);

	return (rc != CONFIG_ENV_SIZE) ? -1 : 0;
}

static int flash_io (int mode)
{
	int fd_current, fd_target, rc, dev_target;

	/* dev_current: fd_current, erase_current */
	fd_current = open (DEVNAME (dev_current), mode);
	if (fd_current < 0) {
		fprintf (stderr,
			 "Can't open %s: %s\n",
			 DEVNAME (dev_current), strerror (errno));
		return -1;
	}

	if (mode == O_RDWR) {
		if (HaveRedundEnv) {
			/* switch to next partition for writing */
			dev_target = !dev_current;
			/* dev_target: fd_target, erase_target */
			fd_target = open (DEVNAME (dev_target), mode);
			if (fd_target < 0) {
				fprintf (stderr,
					 "Can't open %s: %s\n",
					 DEVNAME (dev_target),
					 strerror (errno));
				rc = -1;
				goto exit;
			}
		} else {
			dev_target = dev_current;
			fd_target = fd_current;
		}

		rc = flash_write (fd_current, fd_target, dev_target);

		if (HaveRedundEnv) {
			if (close (fd_target)) {
				fprintf (stderr,
					"I/O error on %s: %s\n",
					DEVNAME (dev_target),
					strerror (errno));
				rc = -1;
			}
		}
	} else {
		rc = flash_read (fd_current);
	}

exit:
	if (close (fd_current)) {
		fprintf (stderr,
			 "I/O error on %s: %s\n",
			 DEVNAME (dev_current), strerror (errno));
		return -1;
	}

	return rc;
}

/*
 * s1 is either a simple 'name', or a 'name=value' pair.
 * s2 is a 'name=value' pair.
 * If the names match, return the value of s2, else NULL.
 */

static char *envmatch (char * s1, char * s2)
{

	while (*s1 == *s2++)
		if (*s1++ == '=')
			return s2;
	if (*s1 == '\0' && *(s2 - 1) == '=')
		return s2;
	return NULL;
}

/*
 * Prevent confusion if running from erased flash memory
 */
static int env_init (void)
{
	int crc0, crc0_ok;
	char flag0;
	void *addr0;

	int crc1, crc1_ok;
	char flag1;
	void *addr1;

	struct env_image_single *single;
	struct env_image_redundant *redundant;

	if (parse_config ())		/* should fill envdevices */
		return -1;

	addr0 = calloc (1, CONFIG_ENV_SIZE);
	if (addr0 == NULL) {
		fprintf (stderr,
			"Not enough memory for environment (%ld bytes)\n",
			CONFIG_ENV_SIZE);
		return -1;
	}

	/* read environment from FLASH to local buffer */
	environment.image = addr0;

	if (HaveRedundEnv) {
		redundant = addr0;
		environment.crc		= &redundant->crc;
		environment.flags	= &redundant->flags;
		environment.data	= redundant->data;
	} else {
		single = addr0;
		environment.crc		= &single->crc;
		environment.flags	= NULL;
		environment.data	= single->data;
	}

	dev_current = 0;
	if (flash_io (O_RDONLY))
		return -1;

	crc0 = crc32 (0, (uint8_t *) environment.data, ENV_SIZE);
	crc0_ok = (crc0 == *environment.crc);
	if (!HaveRedundEnv) {
		if (!crc0_ok) {
			fprintf (stderr,
				"Warning: Bad CRC, using default environment\n");
			memcpy(environment.data, default_environment, sizeof default_environment);
		}
	} else {
		flag0 = *environment.flags;

		dev_current = 1;
		addr1 = calloc (1, CONFIG_ENV_SIZE);
		if (addr1 == NULL) {
			fprintf (stderr,
				"Not enough memory for environment (%ld bytes)\n",
				CONFIG_ENV_SIZE);
			return -1;
		}
		redundant = addr1;

		/*
		 * have to set environment.image for flash_read(), careful -
		 * other pointers in environment still point inside addr0
		 */
		environment.image = addr1;
		if (flash_io (O_RDONLY))
			return -1;

		/* Check flag scheme compatibility */
		if (DEVTYPE(dev_current) == MTD_NORFLASH &&
		    DEVTYPE(!dev_current) == MTD_NORFLASH) {
			environment.flag_scheme = FLAG_BOOLEAN;
		} else if (DEVTYPE(dev_current) == MTD_NANDFLASH &&
			   DEVTYPE(!dev_current) == MTD_NANDFLASH) {
			environment.flag_scheme = FLAG_INCREMENTAL;
		} else {
			fprintf (stderr, "Incompatible flash types!\n");
			return -1;
		}

		crc1 = crc32 (0, (uint8_t *) redundant->data, ENV_SIZE);
		crc1_ok = (crc1 == redundant->crc);
		flag1 = redundant->flags;

		if (crc0_ok && !crc1_ok) {
			dev_current = 0;
		} else if (!crc0_ok && crc1_ok) {
			dev_current = 1;
		} else if (!crc0_ok && !crc1_ok) {
			fprintf (stderr,
				"Warning: Bad CRC, using default environment\n");
			memcpy (environment.data, default_environment,
				sizeof default_environment);
			dev_current = 0;
		} else {
			switch (environment.flag_scheme) {
			case FLAG_BOOLEAN:
				if (flag0 == active_flag &&
				    flag1 == obsolete_flag) {
					dev_current = 0;
				} else if (flag0 == obsolete_flag &&
					   flag1 == active_flag) {
					dev_current = 1;
				} else if (flag0 == flag1) {
					dev_current = 0;
				} else if (flag0 == 0xFF) {
					dev_current = 0;
				} else if (flag1 == 0xFF) {
					dev_current = 1;
				} else {
					dev_current = 0;
				}
				break;
			case FLAG_INCREMENTAL:
				if ((flag0 == 255 && flag1 == 0) ||
				    flag1 > flag0)
					dev_current = 1;
				else if ((flag1 == 255 && flag0 == 0) ||
					 flag0 > flag1)
					dev_current = 0;
				else /* flags are equal - almost impossible */
					dev_current = 0;
				break;
			default:
				fprintf (stderr, "Unknown flag scheme %u \n",
					 environment.flag_scheme);
				return -1;
			}
		}

		/*
		 * If we are reading, we don't need the flag and the CRC any
		 * more, if we are writing, we will re-calculate CRC and update
		 * flags before writing out
		 */
		if (dev_current) {
			environment.image	= addr1;
			environment.crc		= &redundant->crc;
			environment.flags	= &redundant->flags;
			environment.data	= redundant->data;
			free (addr0);
		} else {
			environment.image	= addr0;
			/* Other pointers are already set */
			free (addr1);
		}
	}
	return 0;
}


static int parse_config ()
{
	struct stat st;

#if defined(CONFIG_FILE)
	/* Fills in DEVNAME(), ENVSIZE(), DEVESIZE(). Or don't. */
	if (get_config (CONFIG_FILE)) {
		fprintf (stderr,
			"Cannot parse config file: %s\n", strerror (errno));
		return -1;
	}
#else
	strcpy (DEVNAME (0), DEVICE1_NAME);
	DEVOFFSET (0) = DEVICE1_OFFSET;
	ENVSIZE (0) = ENV1_SIZE;
	DEVESIZE (0) = DEVICE1_ESIZE;
	ENVSECTORS (0) = DEVICE1_ENVSECTORS;
#ifdef HAVE_REDUND
	strcpy (DEVNAME (1), DEVICE2_NAME);
	DEVOFFSET (1) = DEVICE2_OFFSET;
	ENVSIZE (1) = ENV2_SIZE;
	DEVESIZE (1) = DEVICE2_ESIZE;
	ENVSECTORS (1) = DEVICE2_ENVSECTORS;
	HaveRedundEnv = 1;
#endif
#endif
	if (stat (DEVNAME (0), &st)) {
		fprintf (stderr,
			"Cannot access MTD device %s: %s\n",
			DEVNAME (0), strerror (errno));
		return -1;
	}

	if (HaveRedundEnv && stat (DEVNAME (1), &st)) {
		fprintf (stderr,
			"Cannot access MTD device %s: %s\n",
			DEVNAME (1), strerror (errno));
		return -1;
	}
	return 0;
}

#if defined(CONFIG_FILE)
static int get_config (char *fname)
{
	FILE *fp;
	int i = 0;
	int rc;
	char dump[128];

	fp = fopen (fname, "r");
	if (fp == NULL)
		return -1;

	while (i < 2 && fgets (dump, sizeof (dump), fp)) {
		/* Skip incomplete conversions and comment strings */
		if (dump[0] == '#')
			continue;

		rc = sscanf (dump, "%s %lx %lx %lx %lx",
			     DEVNAME (i),
			     &DEVOFFSET (i),
			     &ENVSIZE (i),
			     &DEVESIZE (i),
			     &ENVSECTORS (i));

		if (rc < 4)
			continue;

		if (rc < 5)
			/* Default - 1 sector */
			ENVSECTORS (i) = 1;

		i++;
	}
	fclose (fp);

	HaveRedundEnv = i - 1;
	if (!i) {			/* No valid entries found */
		errno = EINVAL;
		return -1;
	} else
		return 0;
}
#endif
