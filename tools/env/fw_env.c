/*
 * (C) Copyright 2000-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2008
 * Guennadi Liakhovetski, DENX Software Engineering, lg@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#define _GNU_SOURCE

#include <compiler.h>
#include <errno.h>
#include <env_flags.h>
#include <fcntl.h>
#include <linux/fs.h>
#include <linux/stringify.h>
#include <ctype.h>
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

struct env_opts default_opts = {
#ifdef CONFIG_FILE
	.config_file = CONFIG_FILE
#endif
};

#define DIV_ROUND_UP(n, d)	(((n) + (d) - 1) / (d))

#define min(x, y) ({				\
	typeof(x) _min1 = (x);			\
	typeof(y) _min2 = (y);			\
	(void) (&_min1 == &_min2);		\
	_min1 < _min2 ? _min1 : _min2; })

struct envdev_s {
	const char *devname;		/* Device name */
	long long devoff;		/* Device offset */
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

#define CUR_ENVSIZE ENVSIZE(dev_current)

static unsigned long usable_envsize;
#define ENV_SIZE      usable_envsize

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

static int env_aes_cbc_crypt(char *data, const int enc, uint8_t *key);

static int HaveRedundEnv = 0;

static unsigned char active_flag = 1;
/* obsolete_flag must be 0 to efficiently set it on NOR flash without erasing */
static unsigned char obsolete_flag = 0;

#define DEFAULT_ENV_INSTANCE_STATIC
#include <env_default.h>

static int flash_io (int mode);
static int parse_config(struct env_opts *opts);

#if defined(CONFIG_FILE)
static int get_config (char *);
#endif

static char *skip_chars(char *s)
{
	for (; *s != '\0'; s++) {
		if (isblank(*s))
			return s;
	}
	return NULL;
}

static char *skip_blanks(char *s)
{
	for (; *s != '\0'; s++) {
		if (!isblank(*s))
			return s;
	}
	return NULL;
}

/*
 * s1 is either a simple 'name', or a 'name=value' pair.
 * s2 is a 'name=value' pair.
 * If the names match, return the value of s2, else NULL.
 */
static char *envmatch(char *s1, char *s2)
{
	if (s1 == NULL || s2 == NULL)
		return NULL;

	while (*s1 == *s2++)
		if (*s1++ == '=')
			return s2;
	if (*s1 == '\0' && *(s2 - 1) == '=')
		return s2;
	return NULL;
}

/**
 * Search the environment for a variable.
 * Return the value, if found, or NULL, if not found.
 */
char *fw_getenv (char *name)
{
	char *env, *nxt;

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
 * Search the default environment for a variable.
 * Return the value, if found, or NULL, if not found.
 */
char *fw_getdefenv(char *name)
{
	char *env, *nxt;

	for (env = default_environment; *env; env = nxt + 1) {
		char *val;

		for (nxt = env; *nxt; ++nxt) {
			if (nxt >= &default_environment[ENV_SIZE]) {
				fprintf(stderr, "## Error: "
					"default environment not terminated\n");
				return NULL;
			}
		}
		val = envmatch(name, env);
		if (!val)
			continue;
		return val;
	}
	return NULL;
}

int parse_aes_key(char *key, uint8_t *bin_key)
{
	char tmp[5] = { '0', 'x', 0, 0, 0 };
	unsigned long ul;
	int i;

	if (strnlen(key, 64) != 32) {
		fprintf(stderr,
			"## Error: '-a' option requires 16-byte AES key\n");
		return -1;
	}

	for (i = 0; i < 16; i++) {
		tmp[2] = key[0];
		tmp[3] = key[1];
		errno = 0;
		ul = strtoul(tmp, NULL, 16);
		if (errno) {
			fprintf(stderr,
				"## Error: '-a' option requires valid AES key\n");
			return -1;
		}
		bin_key[i] = ul & 0xff;
		key += 2;
	}
	return 0;
}

/*
 * Print the current definition of one, or more, or all
 * environment variables
 */
int fw_printenv(int argc, char *argv[], int value_only, struct env_opts *opts)
{
	int i, rc = 0;

	if (value_only && argc != 1) {
		fprintf(stderr,
			"## Error: `-n' option requires exactly one argument\n");
		return -1;
	}

	if (!opts)
		opts = &default_opts;

	if (fw_env_open(opts))
		return -1;

	if (argc == 0) {		/* Print all env variables  */
		char *env, *nxt;
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

	for (i = 0; i < argc; ++i) {	/* print a subset of env variables */
		char *name = argv[i];
		char *val = NULL;

		val = fw_getenv(name);
		if (!val) {
			fprintf (stderr, "## Error: \"%s\" not defined\n", name);
			rc = -1;
			continue;
		}

		if (value_only) {
			puts(val);
			break;
		}

		printf("%s=%s\n", name, val);
	}

	return rc;
}

int fw_env_close(struct env_opts *opts)
{
	int ret;

	if (!opts)
		opts = &default_opts;

	if (opts->aes_flag) {
		ret = env_aes_cbc_crypt(environment.data, 1,
					opts->aes_key);
		if (ret) {
			fprintf(stderr,
				"Error: can't encrypt env for flash\n");
			return ret;
		}
	}

	/*
	 * Update CRC
	 */
	*environment.crc = crc32(0, (uint8_t *) environment.data, ENV_SIZE);

	/* write environment back to flash */
	if (flash_io(O_RDWR)) {
		fprintf(stderr,
			"Error: can't write fw_env to flash\n");
			return -1;
	}

	return 0;
}


/*
 * Set/Clear a single variable in the environment.
 * This is called in sequence to update the environment
 * in RAM without updating the copy in flash after each set
 */
int fw_env_write(char *name, char *value)
{
	int len;
	char *env, *nxt;
	char *oldval = NULL;
	int deleting, creating, overwriting;

	/*
	 * search if variable with this name already exists
	 */
	for (nxt = env = environment.data; *env; env = nxt + 1) {
		for (nxt = env; *nxt; ++nxt) {
			if (nxt >= &environment.data[ENV_SIZE]) {
				fprintf(stderr, "## Error: "
					"environment not terminated\n");
				errno = EINVAL;
				return -1;
			}
		}
		if ((oldval = envmatch (name, env)) != NULL)
			break;
	}

	deleting = (oldval && !(value && strlen(value)));
	creating = (!oldval && (value && strlen(value)));
	overwriting = (oldval && (value && strlen(value)));

	/* check for permission */
	if (deleting) {
		if (env_flags_validate_varaccess(name,
		    ENV_FLAGS_VARACCESS_PREVENT_DELETE)) {
			printf("Can't delete \"%s\"\n", name);
			errno = EROFS;
			return -1;
		}
	} else if (overwriting) {
		if (env_flags_validate_varaccess(name,
		    ENV_FLAGS_VARACCESS_PREVENT_OVERWR)) {
			printf("Can't overwrite \"%s\"\n", name);
			errno = EROFS;
			return -1;
		} else if (env_flags_validate_varaccess(name,
		    ENV_FLAGS_VARACCESS_PREVENT_NONDEF_OVERWR)) {
			const char *defval = fw_getdefenv(name);

			if (defval == NULL)
				defval = "";
			if (strcmp(oldval, defval)
			    != 0) {
				printf("Can't overwrite \"%s\"\n", name);
				errno = EROFS;
				return -1;
			}
		}
	} else if (creating) {
		if (env_flags_validate_varaccess(name,
		    ENV_FLAGS_VARACCESS_PREVENT_CREATE)) {
			printf("Can't create \"%s\"\n", name);
			errno = EROFS;
			return -1;
		}
	} else
		/* Nothing to do */
		return 0;

	if (deleting || overwriting) {
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
	if (!value || !strlen(value))
		return 0;

	/*
	 * Append new definition at the end
	 */
	for (env = environment.data; *env || *(env + 1); ++env);
	if (env > environment.data)
		++env;
	/*
	 * Overflow when:
	 * "name" + "=" + "val" +"\0\0"  > CUR_ENVSIZE - (env-environment)
	 */
	len = strlen (name) + 2;
	/* add '=' for first arg, ' ' for all others */
	len += strlen(value) + 1;

	if (len > (&environment.data[ENV_SIZE] - env)) {
		fprintf (stderr,
			"Error: environment overflow, \"%s\" deleted\n",
			name);
		return -1;
	}

	while ((*env = *name++) != '\0')
		env++;
	*env = '=';
	while ((*++env = *value++) != '\0')
		;

	/* end is marked with double '\0' */
	*++env = '\0';

	return 0;
}

/*
 * Deletes or sets environment variables. Returns -1 and sets errno error codes:
 * 0	  - OK
 * EINVAL - need at least 1 argument
 * EROFS  - certain variables ("ethaddr", "serial#") cannot be
 *	    modified or deleted
 *
 */
int fw_setenv(int argc, char *argv[], struct env_opts *opts)
{
	int i;
	size_t len;
	char *name, **valv;
	char *value = NULL;
	int valc;

	if (!opts)
		opts = &default_opts;

	if (argc < 1) {
		fprintf(stderr, "## Error: variable name missing\n");
		errno = EINVAL;
		return -1;
	}

	if (fw_env_open(opts)) {
		fprintf(stderr, "Error: environment not initialized\n");
		return -1;
	}

	name = argv[0];
	valv = argv + 1;
	valc = argc - 1;

	if (env_flags_validate_env_set_params(name, valv, valc) < 0)
		return -1;

	len = 0;
	for (i = 0; i < valc; ++i) {
		char *val = valv[i];
		size_t val_len = strlen(val);

		if (value)
			value[len - 1] = ' ';
		value = realloc(value, len + val_len + 1);
		if (!value) {
			fprintf(stderr,
				"Cannot malloc %zu bytes: %s\n",
				len, strerror(errno));
			return -1;
		}

		memcpy(value + len, val, val_len);
		len += val_len;
		value[len++] = '\0';
	}

	fw_env_write(name, value);

	free(value);

	return fw_env_close(opts);
}

/*
 * Parse  a file  and configure the u-boot variables.
 * The script file has a very simple format, as follows:
 *
 * Each line has a couple with name, value:
 * <white spaces>variable_name<white spaces>variable_value
 *
 * Both variable_name and variable_value are interpreted as strings.
 * Any character after <white spaces> and before ending \r\n is interpreted
 * as variable's value (no comment allowed on these lines !)
 *
 * Comments are allowed if the first character in the line is #
 *
 * Returns -1 and sets errno error codes:
 * 0	  - OK
 * -1     - Error
 */
int fw_parse_script(char *fname, struct env_opts *opts)
{
	FILE *fp;
	char dump[1024];	/* Maximum line length in the file */
	char *name;
	char *val;
	int lineno = 0;
	int len;
	int ret = 0;

	if (!opts)
		opts = &default_opts;

	if (fw_env_open(opts)) {
		fprintf(stderr, "Error: environment not initialized\n");
		return -1;
	}

	if (strcmp(fname, "-") == 0)
		fp = stdin;
	else {
		fp = fopen(fname, "r");
		if (fp == NULL) {
			fprintf(stderr, "I cannot open %s for reading\n",
				 fname);
			return -1;
		}
	}

	while (fgets(dump, sizeof(dump), fp)) {
		lineno++;
		len = strlen(dump);

		/*
		 * Read a whole line from the file. If the line is too long
		 * or is not terminated, reports an error and exit.
		 */
		if (dump[len - 1] != '\n') {
			fprintf(stderr,
			"Line %d not corrected terminated or too long\n",
				lineno);
			ret = -1;
			break;
		}

		/* Drop ending line feed / carriage return */
		dump[--len] = '\0';
		if (len && dump[len - 1] == '\r')
			dump[--len] = '\0';

		/* Skip comment or empty lines */
		if (len == 0 || dump[0] == '#')
			continue;

		/*
		 * Search for variable's name,
		 * remove leading whitespaces
		 */
		name = skip_blanks(dump);
		if (!name)
			continue;

		/* The first white space is the end of variable name */
		val = skip_chars(name);
		len = strlen(name);
		if (val) {
			*val++ = '\0';
			if ((val - name) < len)
				val = skip_blanks(val);
			else
				val = NULL;
		}

#ifdef DEBUG
		fprintf(stderr, "Setting %s : %s\n",
			name, val ? val : " removed");
#endif

		if (env_flags_validate_type(name, val) < 0) {
			ret = -1;
			break;
		}

		/*
		 * If there is an error setting a variable,
		 * try to save the environment and returns an error
		 */
		if (fw_env_write(name, val)) {
			fprintf(stderr,
			"fw_env_write returns with error : %s\n",
				strerror(errno));
			ret = -1;
			break;
		}

	}

	/* Close file if not stdin */
	if (strcmp(fname, "-") != 0)
		fclose(fp);

	ret |= fw_env_close(opts);

	return ret;
}

/**
 * environment_end() - compute offset of first byte right after environemnt
 * @dev - index of enviroment buffer
 * Return:
 *  device offset of first byte right after environemnt
 */
off_t environment_end(int dev)
{
	/* environment is block aligned */
	return DEVOFFSET(dev) + ENVSECTORS(dev) * DEVESIZE(dev);
}

/*
 * Test for bad block on NAND, just returns 0 on NOR, on NAND:
 * 0	- block is good
 * > 0	- block is bad
 * < 0	- failed to test
 */
static int flash_bad_block(int fd, uint8_t mtd_type, loff_t blockstart)
{
	if (mtd_type == MTD_NANDFLASH) {
		int badblock = ioctl(fd, MEMGETBADBLOCK, &blockstart);

		if (badblock < 0) {
			perror ("Cannot read bad block mark");
			return badblock;
		}

		if (badblock) {
#ifdef DEBUG
			fprintf (stderr, "Bad block at 0x%llx, skipping\n",
				(unsigned long long)blockstart);
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
			   off_t offset)
{
	size_t blocklen;	/* erase / write length - one block on NAND,
				   0 on NOR */
	size_t processed = 0;	/* progress counter */
	size_t readlen = count;	/* current read length */
	off_t block_seek;	/* offset inside the current block to the start
				   of the data */
	loff_t blockstart;	/* running start of the current block -
				   MEMGETBADBLOCK needs 64 bits */
	int rc;

	blockstart = (offset / DEVESIZE (dev)) * DEVESIZE (dev);

	/* Offset inside a block */
	block_seek = offset - blockstart;

	if (DEVTYPE(dev) == MTD_NANDFLASH) {
		/*
		 * NAND: calculate which blocks we are reading. We have
		 * to read one block at a time to skip bad blocks.
		 */
		blocklen = DEVESIZE (dev);

		/* Limit to one block for the first read */
		if (readlen > blocklen - block_seek)
			readlen = blocklen - block_seek;
	} else {
		blocklen = 0;
	}

	/* This only runs once on NOR flash */
	while (processed < count) {
		rc = flash_bad_block(fd, DEVTYPE(dev), blockstart);
		if (rc < 0)		/* block test failed */
			return -1;

		if (blockstart + block_seek + readlen > environment_end(dev)) {
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
		fprintf(stderr, "Read 0x%x bytes at 0x%llx on %s\n",
			rc, (unsigned long long) blockstart + block_seek,
			DEVNAME(dev));
#endif
		processed += readlen;
		readlen = min (blocklen, count - processed);
		block_seek = 0;
		blockstart += blocklen;
	}

	return processed;
}

/*
 * Write count bytes from begin of environment, but stay within
 * ENVSECTORS(dev) sectors of
 * DEVOFFSET (dev). Similar to the read case above, on NOR and dataflash we
 * erase and write the whole data at once.
 */
static int flash_write_buf(int dev, int fd, void *buf, size_t count)
{
	void *data;
	struct erase_info_user erase;
	size_t blocklen;	/* length of NAND block / NOR erase sector */
	size_t erase_len;	/* whole area that can be erased - may include
				   bad blocks */
	size_t erasesize;	/* erase / write length - one block on NAND,
				   whole area on NOR */
	size_t processed = 0;	/* progress counter */
	size_t write_total;	/* total size to actually write - excluding
				   bad blocks */
	off_t erase_offset;	/* offset to the first erase block (aligned)
				   below offset */
	off_t block_seek;	/* offset inside the erase block to the start
				   of the data */
	loff_t blockstart;	/* running start of the current block -
				   MEMGETBADBLOCK needs 64 bits */
	int rc;

	/*
	 * For mtd devices only offset and size of the environment do matter
	 */
	if (DEVTYPE(dev) == MTD_ABSENT) {
		blocklen = count;
		erase_len = blocklen;
		blockstart = DEVOFFSET(dev);
		block_seek = 0;
		write_total = blocklen;
	} else {
		blocklen = DEVESIZE(dev);

		erase_offset = DEVOFFSET(dev);

		/* Maximum area we may use */
		erase_len = environment_end(dev) - erase_offset;

		blockstart = erase_offset;

		/* Offset inside a block */
		block_seek = DEVOFFSET(dev) - erase_offset;

		/*
		 * Data size we actually write: from the start of the block
		 * to the start of the data, then count bytes of data, and
		 * to the end of the block
		 */
		write_total = ((block_seek + count + blocklen - 1) /
							blocklen) * blocklen;
	}

	/*
	 * Support data anywhere within erase sectors: read out the complete
	 * area to be erased, replace the environment image, write the whole
	 * block back again.
	 */
	if (write_total > count) {
		data = malloc (erase_len);
		if (!data) {
			fprintf (stderr,
				 "Cannot malloc %zu bytes: %s\n",
				 erase_len, strerror (errno));
			return -1;
		}

		rc = flash_read_buf(dev, fd, data, write_total, erase_offset);
		if (write_total != rc)
			return -1;

#ifdef DEBUG
		fprintf(stderr, "Preserving data ");
		if (block_seek != 0)
			fprintf(stderr, "0x%x - 0x%lx", 0, block_seek - 1);
		if (block_seek + count != write_total) {
			if (block_seek != 0)
				fprintf(stderr, " and ");
			fprintf(stderr, "0x%lx - 0x%lx",
				(unsigned long) block_seek + count,
				(unsigned long) write_total - 1);
		}
		fprintf(stderr, "\n");
#endif
		/* Overwrite the old environment */
		memcpy (data + block_seek, buf, count);
	} else {
		/*
		 * We get here, iff offset is block-aligned and count is a
		 * multiple of blocklen - see write_total calculation above
		 */
		data = buf;
	}

	if (DEVTYPE(dev) == MTD_NANDFLASH) {
		/*
		 * NAND: calculate which blocks we are writing. We have
		 * to write one block at a time to skip bad blocks.
		 */
		erasesize = blocklen;
	} else {
		erasesize = erase_len;
	}

	erase.length = erasesize;

	/* This only runs once on NOR flash and SPI-dataflash */
	while (processed < write_total) {
		rc = flash_bad_block(fd, DEVTYPE(dev), blockstart);
		if (rc < 0)		/* block test failed */
			return rc;

		if (blockstart + erasesize > environment_end(dev)) {
			fprintf (stderr, "End of range reached, aborting\n");
			return -1;
		}

		if (rc) {		/* block is bad */
			blockstart += blocklen;
			continue;
		}

		if (DEVTYPE(dev) != MTD_ABSENT) {
			erase.start = blockstart;
			ioctl(fd, MEMUNLOCK, &erase);
			/* These do not need an explicit erase cycle */
			if (DEVTYPE(dev) != MTD_DATAFLASH)
				if (ioctl(fd, MEMERASE, &erase) != 0) {
					fprintf(stderr,
						"MTD erase error on %s: %s\n",
						DEVNAME(dev), strerror(errno));
					return -1;
				}
		}

		if (lseek (fd, blockstart, SEEK_SET) == -1) {
			fprintf (stderr,
				 "Seek error on %s: %s\n",
				 DEVNAME (dev), strerror (errno));
			return -1;
		}

#ifdef DEBUG
		fprintf(stderr, "Write 0x%llx bytes at 0x%llx\n",
			(unsigned long long) erasesize,
			(unsigned long long) blockstart);
#endif
		if (write (fd, data + processed, erasesize) != erasesize) {
			fprintf (stderr, "Write error on %s: %s\n",
				 DEVNAME (dev), strerror (errno));
			return -1;
		}

		if (DEVTYPE(dev) != MTD_ABSENT)
			ioctl(fd, MEMLOCK, &erase);

		processed  += erasesize;
		block_seek = 0;
		blockstart += erasesize;
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
	struct erase_info_user erase;

	erase.start  = DEVOFFSET (dev);
	erase.length = DEVESIZE (dev);
	/* This relies on the fact, that obsolete_flag == 0 */
	rc = lseek (fd, offset, SEEK_SET);
	if (rc < 0) {
		fprintf (stderr, "Cannot seek to set the flag on %s \n",
			 DEVNAME (dev));
		return rc;
	}
	ioctl (fd, MEMUNLOCK, &erase);
	rc = write (fd, &obsolete_flag, sizeof (obsolete_flag));
	ioctl (fd, MEMLOCK, &erase);
	if (rc < 0)
		perror ("Could not set obsolete flag");

	return rc;
}

/* Encrypt or decrypt the environment before writing or reading it. */
static int env_aes_cbc_crypt(char *payload, const int enc, uint8_t *key)
{
	uint8_t *data = (uint8_t *)payload;
	const int len = usable_envsize;
	uint8_t key_exp[AES_EXPAND_KEY_LENGTH];
	uint32_t aes_blocks;

	/* First we expand the key. */
	aes_expand_key(key, key_exp);

	/* Calculate the number of AES blocks to encrypt. */
	aes_blocks = DIV_ROUND_UP(len, AES_KEY_LENGTH);

	if (enc)
		aes_cbc_encrypt_blocks(key_exp, data, data, aes_blocks);
	else
		aes_cbc_decrypt_blocks(key_exp, data, data, aes_blocks);

	return 0;
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
	fprintf(stderr, "Writing new environment at 0x%llx on %s\n",
		DEVOFFSET (dev_target), DEVNAME (dev_target));
#endif

	rc = flash_write_buf(dev_target, fd_target, environment.image,
			     CUR_ENVSIZE);
	if (rc < 0)
		return rc;

	if (environment.flag_scheme == FLAG_BOOLEAN) {
		/* Have to set obsolete flag */
		off_t offset = DEVOFFSET (dev_current) +
			offsetof (struct env_image_redundant, flags);
#ifdef DEBUG
		fprintf(stderr,
			"Setting obsolete flag in environment at 0x%llx on %s\n",
			DEVOFFSET (dev_current), DEVNAME (dev_current));
#endif
		flash_flag_obsolete (dev_current, fd_current, offset);
	}

	return 0;
}

static int flash_read (int fd)
{
	int rc;

	rc = flash_read_buf(dev_current, fd, environment.image, CUR_ENVSIZE,
			    DEVOFFSET(dev_current));
	if (rc != CUR_ENVSIZE)
		return -1;

	return 0;
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
 * Prevent confusion if running from erased flash memory
 */
int fw_env_open(struct env_opts *opts)
{
	int crc0, crc0_ok;
	unsigned char flag0;
	void *addr0;

	int crc1, crc1_ok;
	unsigned char flag1;
	void *addr1;

	int ret;

	struct env_image_single *single;
	struct env_image_redundant *redundant;

	if (!opts)
		opts = &default_opts;

	if (parse_config(opts))		/* should fill envdevices */
		return -1;

	addr0 = calloc(1, CUR_ENVSIZE);
	if (addr0 == NULL) {
		fprintf(stderr,
			"Not enough memory for environment (%ld bytes)\n",
			CUR_ENVSIZE);
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

	if (opts->aes_flag) {
		ret = env_aes_cbc_crypt(environment.data, 0,
					opts->aes_key);
		if (ret)
			return ret;
	}

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
		addr1 = calloc(1, CUR_ENVSIZE);
		if (addr1 == NULL) {
			fprintf(stderr,
				"Not enough memory for environment (%ld bytes)\n",
				CUR_ENVSIZE);
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
		} else if (DEVTYPE(dev_current) == MTD_DATAFLASH &&
			   DEVTYPE(!dev_current) == MTD_DATAFLASH) {
			environment.flag_scheme = FLAG_BOOLEAN;
		} else if (DEVTYPE(dev_current) == MTD_UBIVOLUME &&
			   DEVTYPE(!dev_current) == MTD_UBIVOLUME) {
			environment.flag_scheme = FLAG_INCREMENTAL;
		} else if (DEVTYPE(dev_current) == MTD_ABSENT &&
			   DEVTYPE(!dev_current) == MTD_ABSENT) {
			environment.flag_scheme = FLAG_INCREMENTAL;
		} else {
			fprintf (stderr, "Incompatible flash types!\n");
			return -1;
		}

		crc1 = crc32 (0, (uint8_t *) redundant->data, ENV_SIZE);

		if (opts->aes_flag) {
			ret = env_aes_cbc_crypt(redundant->data, 0,
						opts->aes_key);
			if (ret)
				return ret;
		}

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
				if (flag0 == 255 && flag1 == 0)
					dev_current = 1;
				else if ((flag1 == 255 && flag0 == 0) ||
					 flag0 >= flag1)
					dev_current = 0;
				else /* flag1 > flag0 */
					dev_current = 1;
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
#ifdef DEBUG
		fprintf(stderr, "Selected env in %s\n", DEVNAME(dev_current));
#endif
	}
	return 0;
}

static int check_device_config(int dev)
{
	struct stat st;
	int fd, rc = 0;

	fd = open(DEVNAME(dev), O_RDONLY);
	if (fd < 0) {
		fprintf(stderr,
			"Cannot open %s: %s\n",
			DEVNAME(dev), strerror(errno));
		return -1;
	}

	rc = fstat(fd, &st);
	if (rc < 0) {
		fprintf(stderr, "Cannot stat the file %s\n",
			DEVNAME(dev));
		goto err;
	}

	if (S_ISCHR(st.st_mode)) {
		struct mtd_info_user mtdinfo;
		rc = ioctl(fd, MEMGETINFO, &mtdinfo);
		if (rc < 0) {
			fprintf(stderr, "Cannot get MTD information for %s\n",
				DEVNAME(dev));
			goto err;
		}
		if (mtdinfo.type != MTD_NORFLASH &&
		    mtdinfo.type != MTD_NANDFLASH &&
		    mtdinfo.type != MTD_DATAFLASH &&
		    mtdinfo.type != MTD_UBIVOLUME) {
			fprintf(stderr, "Unsupported flash type %u on %s\n",
				mtdinfo.type, DEVNAME(dev));
			goto err;
		}
		DEVTYPE(dev) = mtdinfo.type;
		if (DEVESIZE(dev) == 0)
			/* Assume the erase size is the same as the env-size */
			DEVESIZE(dev) = ENVSIZE(dev);
	} else {
		uint64_t size;
		DEVTYPE(dev) = MTD_ABSENT;
		if (DEVESIZE(dev) == 0)
			/* Assume the erase size to be 512 bytes */
			DEVESIZE(dev) = 0x200;

		/*
		 * Check for negative offsets, treat it as backwards offset
		 * from the end of the block device
		 */
		if (DEVOFFSET(dev) < 0) {
			rc = ioctl(fd, BLKGETSIZE64, &size);
			if (rc < 0) {
				fprintf(stderr, "Could not get block device size on %s\n",
					DEVNAME(dev));
				goto err;
			}

			DEVOFFSET(dev) = DEVOFFSET(dev) + size;
#ifdef DEBUG
			fprintf(stderr, "Calculated device offset 0x%llx on %s\n",
				DEVOFFSET(dev), DEVNAME(dev));
#endif
		}
	}

	if (ENVSECTORS(dev) == 0)
		/* Assume enough sectors to cover the environment */
		ENVSECTORS(dev) = DIV_ROUND_UP(ENVSIZE(dev), DEVESIZE(dev));

	if (DEVOFFSET(dev) % DEVESIZE(dev) != 0) {
		fprintf(stderr, "Environment does not start on (erase) block boundary\n");
		errno = EINVAL;
		return -1;
	}

	if (ENVSIZE(dev) > ENVSECTORS(dev) * DEVESIZE(dev)) {
		fprintf(stderr, "Environment does not fit into available sectors\n");
		errno = EINVAL;
		return -1;
	}

err:
	close(fd);
	return rc;
}

static int parse_config(struct env_opts *opts)
{
	int rc;

	if (!opts)
		opts = &default_opts;

#if defined(CONFIG_FILE)
	/* Fills in DEVNAME(), ENVSIZE(), DEVESIZE(). Or don't. */
	if (get_config(opts->config_file)) {
		fprintf(stderr, "Cannot parse config file '%s': %m\n",
			opts->config_file);
		return -1;
	}
#else
	DEVNAME (0) = DEVICE1_NAME;
	DEVOFFSET (0) = DEVICE1_OFFSET;
	ENVSIZE (0) = ENV1_SIZE;

	/* Set defaults for DEVESIZE, ENVSECTORS later once we
	 * know DEVTYPE
	 */
#ifdef DEVICE1_ESIZE
	DEVESIZE (0) = DEVICE1_ESIZE;
#endif
#ifdef DEVICE1_ENVSECTORS
	ENVSECTORS (0) = DEVICE1_ENVSECTORS;
#endif

#ifdef HAVE_REDUND
	DEVNAME (1) = DEVICE2_NAME;
	DEVOFFSET (1) = DEVICE2_OFFSET;
	ENVSIZE (1) = ENV2_SIZE;

	/* Set defaults for DEVESIZE, ENVSECTORS later once we
	 * know DEVTYPE
	 */
#ifdef DEVICE2_ESIZE
	DEVESIZE (1) = DEVICE2_ESIZE;
#endif
#ifdef DEVICE2_ENVSECTORS
	ENVSECTORS (1) = DEVICE2_ENVSECTORS;
#endif
	HaveRedundEnv = 1;
#endif
#endif
	rc = check_device_config(0);
	if (rc < 0)
		return rc;

	if (HaveRedundEnv) {
		rc = check_device_config(1);
		if (rc < 0)
			return rc;

		if (ENVSIZE(0) != ENVSIZE(1)) {
			fprintf(stderr,
				"Redundant environments have unequal size");
			return -1;
		}
	}

	usable_envsize = CUR_ENVSIZE - sizeof(uint32_t);
	if (HaveRedundEnv)
		usable_envsize -= sizeof(char);

	if (opts->aes_flag)
		usable_envsize &= ~(AES_KEY_LENGTH - 1);

	return 0;
}

#if defined(CONFIG_FILE)
static int get_config (char *fname)
{
	FILE *fp;
	int i = 0;
	int rc;
	char dump[128];
	char *devname;

	fp = fopen (fname, "r");
	if (fp == NULL)
		return -1;

	while (i < 2 && fgets (dump, sizeof (dump), fp)) {
		/* Skip incomplete conversions and comment strings */
		if (dump[0] == '#')
			continue;

		rc = sscanf(dump, "%ms %lli %lx %lx %lx",
			    &devname,
			    &DEVOFFSET(i),
			    &ENVSIZE(i),
			    &DEVESIZE(i),
			    &ENVSECTORS(i));

		if (rc < 3)
			continue;

		DEVNAME(i) = devname;

		/* Set defaults for DEVESIZE, ENVSECTORS later once we
		 * know DEVTYPE
		 */

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
