/*
 * (C) Copyright 2008 Semihalf
 *
 * Written by: Rafal Czubak <rcz@semihalf.com>
 *             Bartlomiej Sieka <tur@semihalf.com>
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
 *
 */

#include <common.h>

#if !(defined(CONFIG_FIT) && defined(CONFIG_OF_LIBFDT))
#error "CONFIG_FIT and CONFIG_OF_LIBFDT are required for auto-update feature"
#endif

#if defined(CONFIG_SYS_NO_FLASH)
#error "CONFIG_SYS_NO_FLASH defined, but FLASH is required for auto-update feature"
#endif

#include <command.h>
#include <flash.h>
#include <net.h>
#include <malloc.h>

/* env variable holding the location of the update file */
#define UPDATE_FILE_ENV		"updatefile"

/* set configuration defaults if needed */
#ifndef CONFIG_UPDATE_LOAD_ADDR
#define CONFIG_UPDATE_LOAD_ADDR	0x100000
#endif

#ifndef CONFIG_UPDATE_TFTP_MSEC_MAX
#define CONFIG_UPDATE_TFTP_MSEC_MAX	100
#endif

#ifndef CONFIG_UPDATE_TFTP_CNT_MAX
#define CONFIG_UPDATE_TFTP_CNT_MAX	0
#endif

extern ulong TftpRRQTimeoutMSecs;
extern int TftpRRQTimeoutCountMax;
extern flash_info_t flash_info[];
extern ulong load_addr;

static uchar *saved_prot_info;

static int update_load(char *filename, ulong msec_max, int cnt_max, ulong addr)
{
	int size, rv;
	ulong saved_timeout_msecs;
	int saved_timeout_count;
	char *saved_netretry, *saved_bootfile;

	rv = 0;
	/* save used globals and env variable */
	saved_timeout_msecs = TftpRRQTimeoutMSecs;
	saved_timeout_count = TftpRRQTimeoutCountMax;
	saved_netretry = strdup(getenv("netretry"));
	saved_bootfile = strdup(BootFile);

	/* set timeouts for auto-update */
	TftpRRQTimeoutMSecs = msec_max;
	TftpRRQTimeoutCountMax = cnt_max;

	/* we don't want to retry the connection if errors occur */
	setenv("netretry", "no");

	/* download the update file */
	load_addr = addr;
	copy_filename(BootFile, filename, sizeof(BootFile));
	size = NetLoop(TFTP);

	if (size < 0)
		rv = 1;
	else if (size > 0)
		flush_cache(addr, size);

	/* restore changed globals and env variable */
	TftpRRQTimeoutMSecs = saved_timeout_msecs;
	TftpRRQTimeoutCountMax = saved_timeout_count;

	setenv("netretry", saved_netretry);
	if (saved_netretry != NULL)
		free(saved_netretry);

	if (saved_bootfile != NULL) {
		copy_filename(BootFile, saved_bootfile, sizeof(BootFile));
		free(saved_bootfile);
	}

	return rv;
}

static int update_flash_protect(int prot, ulong addr_first, ulong addr_last)
{
	uchar *sp_info_ptr;
	ulong s;
	int i, bank, cnt;
	flash_info_t *info;

	sp_info_ptr = NULL;

	if (prot == 0) {
		saved_prot_info =
			calloc(CONFIG_SYS_MAX_FLASH_BANKS * CONFIG_SYS_MAX_FLASH_SECT, 1);
		if (!saved_prot_info)
			return 1;
	}

	for (bank = 0; bank < CONFIG_SYS_MAX_FLASH_BANKS; ++bank) {
		cnt = 0;
		info = &flash_info[bank];

		/* Nothing to do if the bank doesn't exist */
		if (info->sector_count == 0)
			return 0;

		/* Point to current bank protection information */
		sp_info_ptr = saved_prot_info + (bank * CONFIG_SYS_MAX_FLASH_SECT);

		/*
		 * Adjust addr_first or addr_last if we are on bank boundary.
		 * Address space between banks must be continuous for other
		 * flash functions (like flash_sect_erase or flash_write) to
		 * succeed. Banks must also be numbered in correct order,
		 * according to increasing addresses.
		 */
		if (addr_last > info->start[0] + info->size - 1)
			addr_last = info->start[0] + info->size - 1;
		if (addr_first < info->start[0])
			addr_first = info->start[0];

		for (i = 0; i < info->sector_count; i++) {
			/* Save current information about protected sectors */
			if (prot == 0) {
				s = info->start[i];
				if ((s >= addr_first) && (s <= addr_last))
					sp_info_ptr[i] = info->protect[i];

			}

			/* Protect/unprotect sectors */
			if (sp_info_ptr[i] == 1) {
#if defined(CONFIG_SYS_FLASH_PROTECTION)
				if (flash_real_protect(info, i, prot))
					return 1;
#else
				info->protect[i] = prot;
#endif
				cnt++;
			}
		}

		if (cnt) {
			printf("%sProtected %d sectors\n",
						prot ? "": "Un-", cnt);
		}
	}

	if((prot == 1) && saved_prot_info)
		free(saved_prot_info);

	return 0;
}

static int update_flash(ulong addr_source, ulong addr_first, ulong size)
{
	ulong addr_last = addr_first + size - 1;

	/* round last address to the sector boundary */
	if (flash_sect_roundb(&addr_last) > 0)
		return 1;

	if (addr_first >= addr_last) {
		printf("Error: end address exceeds addressing space\n");
		return 1;
	}

	/* remove protection on processed sectors */
	if (update_flash_protect(0, addr_first, addr_last) > 0) {
		printf("Error: could not unprotect flash sectors\n");
		return 1;
	}

	printf("Erasing 0x%08lx - 0x%08lx", addr_first, addr_last);
	if (flash_sect_erase(addr_first, addr_last) > 0) {
		printf("Error: could not erase flash\n");
		return 1;
	}

	printf("Copying to flash...");
	if (flash_write((char *)addr_source, addr_first, size) > 0) {
		printf("Error: could not copy to flash\n");
		return 1;
	}
	printf("done\n");

	/* enable protection on processed sectors */
	if (update_flash_protect(1, addr_first, addr_last) > 0) {
		printf("Error: could not protect flash sectors\n");
		return 1;
	}

	return 0;
}

static int update_fit_getparams(const void *fit, int noffset, ulong *addr,
						ulong *fladdr, ulong *size)
{
	const void *data;

	if (fit_image_get_data(fit, noffset, &data, (size_t *)size))
		return 1;

	if (fit_image_get_load(fit, noffset, (ulong *)fladdr))
		return 1;

	*addr = (ulong)data;

	return 0;
}

int update_tftp(ulong addr)
{
	char *filename, *env_addr;
	int images_noffset, ndepth, noffset;
	ulong update_addr, update_fladdr, update_size;
	void *fit;
	int ret = 0;

	/* use already present image */
	if (addr)
		goto got_update_file;

	printf("Auto-update from TFTP: ");

	/* get the file name of the update file */
	filename = getenv(UPDATE_FILE_ENV);
	if (filename == NULL) {
		printf("failed, env. variable '%s' not found\n",
							UPDATE_FILE_ENV);
		return 1;
	}

	printf("trying update file '%s'\n", filename);

	/* get load address of downloaded update file */
	if ((env_addr = getenv("loadaddr")) != NULL)
		addr = simple_strtoul(env_addr, NULL, 16);
	else
		addr = CONFIG_UPDATE_LOAD_ADDR;


	if (update_load(filename, CONFIG_UPDATE_TFTP_MSEC_MAX,
					CONFIG_UPDATE_TFTP_CNT_MAX, addr)) {
		printf("Can't load update file, aborting auto-update\n");
		return 1;
	}

got_update_file:
	fit = (void *)addr;

	if (!fit_check_format((void *)fit)) {
		printf("Bad FIT format of the update file, aborting "
							"auto-update\n");
		return 1;
	}

	/* process updates */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);

	ndepth = 0;
	noffset = fdt_next_node(fit, images_noffset, &ndepth);
	while (noffset >= 0 && ndepth > 0) {
		if (ndepth != 1)
			goto next_node;

		printf("Processing update '%s' :",
			fit_get_name(fit, noffset, NULL));

		if (!fit_image_check_hashes(fit, noffset)) {
			printf("Error: invalid update hash, aborting\n");
			ret = 1;
			goto next_node;
		}

		printf("\n");
		if (update_fit_getparams(fit, noffset, &update_addr,
					&update_fladdr, &update_size)) {
			printf("Error: can't get update parameteres, "
								"aborting\n");
			ret = 1;
			goto next_node;
		}
		if (update_flash(update_addr, update_fladdr, update_size)) {
			printf("Error: can't flash update, aborting\n");
			ret = 1;
			goto next_node;
		}
next_node:
		noffset = fdt_next_node(fit, noffset, &ndepth);
	}

	return ret;
}
