// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014 Broadcom Corporation
 */

/*
 * Minimal semihosting implementation for reading files into memory. If more
 * features like writing files or console output are required they can be
 * added later. This code has been tested on arm64/aarch64 fastmodel only.
 * An untested placeholder exists for armv7 architectures, but since they
 * are commonly available in silicon now, fastmodel usage makes less sense
 * for them.
 */
#include <common.h>
#include <command.h>
#include <env.h>
#include <log.h>
#include <semihosting.h>

#define SYSOPEN		0x01
#define SYSCLOSE	0x02
#define SYSREAD		0x06
#define SYSFLEN		0x0C
#define SYSERRNO	0x13

/*
 * Call the handler
 */
static noinline long smh_trap(unsigned int sysnum, void *addr)
{
	register long result asm("r0");
#if defined(CONFIG_ARM64)
	asm volatile ("hlt #0xf000" : "=r" (result) : "0"(sysnum), "r"(addr));
#elif defined(CONFIG_CPU_V7M)
	asm volatile ("bkpt #0xAB" : "=r" (result) : "0"(sysnum), "r"(addr));
#else
	/* Note - untested placeholder */
	asm volatile ("svc #0x123456" : "=r" (result) : "0"(sysnum), "r"(addr));
#endif
	return result;
}

/**
 * smh_errno() - Read the host's errno
 *
 * This gets the value of the host's errno and negates it. The host's errno may
 * or may not be set, so only call this function if a previous semihosting call
 * has failed.
 *
 * Return: a negative error value
 */
static int smh_errno(void)
{
	long ret = smh_trap(SYSERRNO, NULL);

	if (ret > 0 && ret < INT_MAX)
		return -ret;
	return -EIO;
}

long smh_open(const char *fname, enum smh_open_mode mode)
{
	long fd;
	struct smh_open_s {
		const char *fname;
		unsigned long mode;
		size_t len;
	} open;

	debug("%s: file \'%s\', mode \'%u\'\n", __func__, fname, mode);

	open.fname = fname;
	open.len = strlen(fname);
	open.mode = mode;

	/* Open the file on the host */
	fd = smh_trap(SYSOPEN, &open);
	if (fd == -1)
		return smh_errno();
	return fd;
}

long smh_read(long fd, void *memp, size_t len)
{
	long ret;
	struct smh_read_s {
		long fd;
		void *memp;
		size_t len;
	} read;

	debug("%s: fd %ld, memp %p, len %zu\n", __func__, fd, memp, len);

	read.fd = fd;
	read.memp = memp;
	read.len = len;

	ret = smh_trap(SYSREAD, &read);
	if (ret < 0)
		return smh_errno();
	return len - ret;
}

long smh_close(long fd)
{
	long ret;

	debug("%s: fd %ld\n", __func__, fd);

	ret = smh_trap(SYSCLOSE, &fd);
	if (ret == -1)
		return smh_errno();
	return 0;
}

long smh_flen(long fd)
{
	long ret;

	debug("%s: fd %ld\n", __func__, fd);

	ret = smh_trap(SYSFLEN, &fd);
	if (ret == -1)
		return smh_errno();
	return ret;
}

static int smh_load_file(const char * const name, ulong load_addr,
			 ulong *end_addr)
{
	long fd;
	long len;
	long ret;

	fd = smh_open(name, MODE_READ | MODE_BINARY);
	if (fd < 0)
		return fd;

	len = smh_flen(fd);
	if (len < 0) {
		smh_close(fd);
		return len;
	}

	ret = smh_read(fd, (void *)load_addr, len);
	smh_close(fd);

	if (ret == len) {
		*end_addr = load_addr + len - 1;
		printf("loaded file %s from %08lX to %08lX, %08lX bytes\n",
		       name,
		       load_addr,
		       *end_addr,
		       len);
	} else if (ret >= 0) {
		ret = -EAGAIN;
	}

	if (ret < 0) {
		printf("read failed: %ld\n", ret);
		return ret;
	}

	return 0;
}

static int do_smhload(struct cmd_tbl *cmdtp, int flag, int argc,
		      char *const argv[])
{
	if (argc == 3 || argc == 4) {
		ulong load_addr;
		ulong end_addr = 0;
		int ret;
		char end_str[64];

		load_addr = hextoul(argv[2], NULL);
		if (!load_addr)
			return -1;

		ret = smh_load_file(argv[1], load_addr, &end_addr);
		if (ret < 0)
			return CMD_RET_FAILURE;

		/* Optionally save returned end to the environment */
		if (argc == 4) {
			sprintf(end_str, "0x%08lx", end_addr);
			env_set(argv[3], end_str);
		}
	} else {
		return CMD_RET_USAGE;
	}
	return 0;
}

U_BOOT_CMD(smhload, 4, 0, do_smhload, "load a file using semihosting",
	   "<file> 0x<address> [end var]\n"
	   "    - load a semihosted file to the address specified\n"
	   "      if the optional [end var] is specified, the end\n"
	   "      address of the file will be stored in this environment\n"
	   "      variable.\n");
