/*
 * (C) Copyright 2003
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <common.h>
#include <command.h>
#include <image.h>
#include <zlib.h>
#include <asm/byteorder.h>
#include <asm/addrspace.h>

DECLARE_GLOBAL_DATA_PTR;

#define	LINUX_MAX_ENVS		256
#define	LINUX_MAX_ARGS		256

static int	linux_argc;
static char **	linux_argv;

static char **	linux_env;
static char *	linux_env_p;
static int	linux_env_idx;

static void linux_params_init (ulong start, char * commandline);
static void linux_env_set (char * env_name, char * env_val);

extern int do_reset (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

void do_bootm_linux (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[],
		     bootm_headers_t *images)
{
	ulong	initrd_start, initrd_end;
	ulong	ep = 0;
	void	(*theKernel) (int, char **, char **, int *);
	char	*commandline = getenv ("bootargs");
	char	env_buf[12];
	int	ret;
	char	*cp;

	/* find kernel entry point */
	if (images->legacy_hdr_valid) {
		ep = image_get_ep (&images->legacy_hdr_os_copy);
#if defined(CONFIG_FIT)
	} else if (images->fit_uname_os) {
		ret = fit_image_get_entry (images->fit_hdr_os,
				images->fit_noffset_os, &ep);
		if (ret) {
			puts ("Can't get entry point property!\n");
			goto error;
		}
#endif
	} else {
		puts ("Could not find kernel entry point!\n");
		goto error;
	}
	theKernel = (void (*)(int, char **, char **, int *))ep;

	ret = boot_get_ramdisk (argc, argv, images, IH_ARCH_MIPS,
			&initrd_start, &initrd_end);
	if (ret)
		goto error;

	show_boot_progress (15);

#ifdef DEBUG
	printf ("## Transferring control to Linux (at address %08lx) ...\n",
		(ulong) theKernel);
#endif

	linux_params_init (UNCACHED_SDRAM (gd->bd->bi_boot_params), commandline);

#ifdef CONFIG_MEMSIZE_IN_BYTES
	sprintf (env_buf, "%lu", (ulong)gd->ram_size);
	debug ("## Giving linux memsize in bytes, %lu\n", (ulong)gd->ram_size);
#else
	sprintf (env_buf, "%lu", (ulong)(gd->ram_size >> 20));
	debug ("## Giving linux memsize in MB, %lu\n", (ulong)(gd->ram_size >> 20));
#endif /* CONFIG_MEMSIZE_IN_BYTES */

	linux_env_set ("memsize", env_buf);

	sprintf (env_buf, "0x%08X", (uint) UNCACHED_SDRAM (initrd_start));
	linux_env_set ("initrd_start", env_buf);

	sprintf (env_buf, "0x%X", (uint) (initrd_end - initrd_start));
	linux_env_set ("initrd_size", env_buf);

	sprintf (env_buf, "0x%08X", (uint) (gd->bd->bi_flashstart));
	linux_env_set ("flash_start", env_buf);

	sprintf (env_buf, "0x%X", (uint) (gd->bd->bi_flashsize));
	linux_env_set ("flash_size", env_buf);

	cp = getenv("ethaddr");
	if (cp != NULL) {
		linux_env_set("ethaddr", cp);
	}

	cp = getenv("eth1addr");
	if (cp != NULL) {
		linux_env_set("eth1addr", cp);
	}

	/* we assume that the kernel is in place */
	printf ("\nStarting kernel ...\n\n");

	theKernel (linux_argc, linux_argv, linux_env, 0);
	/* does not return */
	return;

error:
	do_reset (cmdtp, flag, argc, argv);
	return;
}

static void linux_params_init (ulong start, char *line)
{
	char *next, *quote, *argp;

	linux_argc = 1;
	linux_argv = (char **) start;
	linux_argv[0] = 0;
	argp = (char *) (linux_argv + LINUX_MAX_ARGS);

	next = line;

	while (line && *line && linux_argc < LINUX_MAX_ARGS) {
		quote = strchr (line, '"');
		next = strchr (line, ' ');

		while (next != NULL && quote != NULL && quote < next) {
			/* we found a left quote before the next blank
			 * now we have to find the matching right quote
			 */
			next = strchr (quote + 1, '"');
			if (next != NULL) {
				quote = strchr (next + 1, '"');
				next = strchr (next + 1, ' ');
			}
		}

		if (next == NULL) {
			next = line + strlen (line);
		}

		linux_argv[linux_argc] = argp;
		memcpy (argp, line, next - line);
		argp[next - line] = 0;

		argp += next - line + 1;
		linux_argc++;

		if (*next)
			next++;

		line = next;
	}

	linux_env = (char **) (((ulong) argp + 15) & ~15);
	linux_env[0] = 0;
	linux_env_p = (char *) (linux_env + LINUX_MAX_ENVS);
	linux_env_idx = 0;
}

static void linux_env_set (char *env_name, char *env_val)
{
	if (linux_env_idx < LINUX_MAX_ENVS - 1) {
		linux_env[linux_env_idx] = linux_env_p;

		strcpy (linux_env_p, env_name);
		linux_env_p += strlen (env_name);

		strcpy (linux_env_p, "=");
		linux_env_p += 1;

		strcpy (linux_env_p, env_val);
		linux_env_p += strlen (env_val);

		linux_env_p++;
		linux_env[++linux_env_idx] = 0;
	}
}
