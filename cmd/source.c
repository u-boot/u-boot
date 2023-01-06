// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2001
 * Kyle Harris, kharris@nexus-tech.net
 */

/*
 * The "source" command allows to define "script images", i. e. files
 * that contain command sequences that can be executed by the command
 * interpreter. It returns the exit status of the last command
 * executed from the script. This is very similar to running a shell
 * script in a UNIX shell, hence the name for the command.
 */

/* #define DEBUG */

#include <common.h>
#include <command.h>
#include <env.h>
#include <image.h>
#include <log.h>
#include <malloc.h>
#include <mapmem.h>
#include <asm/byteorder.h>
#include <asm/io.h>

/**
 * get_default_image() - Return default property from /images
 *
 * Return: Pointer to value of default property (or NULL)
 */
static const char *get_default_image(const void *fit)
{
	int images_noffset;

	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (images_noffset < 0)
		return NULL;

	return fdt_getprop(fit, images_noffset, FIT_DEFAULT_PROP, NULL);
}

int image_locate_script(void *buf, int size, const char *fit_uname,
			const char *confname, char **datap, uint *lenp)
{
	ulong		len;
	const struct legacy_img_hdr *hdr;
	u32		*data;
	int		verify;
	const void*	fit_hdr;
	int		noffset;
	const void	*fit_data;
	size_t		fit_len;

	verify = env_get_yesno("verify");

	switch (genimg_get_format(buf)) {
	case IMAGE_FORMAT_LEGACY:
		if (IS_ENABLED(CONFIG_LEGACY_IMAGE_FORMAT)) {
			hdr = buf;

			if (!image_check_magic(hdr)) {
				puts("Bad magic number\n");
				return 1;
			}

			if (!image_check_hcrc(hdr)) {
				puts("Bad header crc\n");
				return 1;
			}

			if (verify) {
				if (!image_check_dcrc(hdr)) {
					puts("Bad data crc\n");
					return 1;
				}
			}

			if (!image_check_type(hdr, IH_TYPE_SCRIPT)) {
				puts("Bad image type\n");
				return 1;
			}

			/* get length of script */
			data = (u32 *)image_get_data(hdr);

			len = uimage_to_cpu(*data);
			if (!len) {
				puts("Empty Script\n");
				return 1;
			}

			/*
			 * scripts are just multi-image files with one
			 * component, so seek past the zero-terminated sequence
			 * of image lengths to get to the actual image data
			 */
			while (*data++);
		}
		break;
	case IMAGE_FORMAT_FIT:
		if (IS_ENABLED(CONFIG_FIT)) {
			fit_hdr = buf;
			if (fit_check_format(fit_hdr, IMAGE_SIZE_INVAL)) {
				puts("Bad FIT image format\n");
				return 1;
			}

			if (!fit_uname) {
				/* If confname is empty, use the default */
				if (confname && *confname)
					noffset = fit_conf_get_node(fit_hdr, confname);
				else
					noffset = fit_conf_get_node(fit_hdr, NULL);
				if (noffset < 0) {
					if (!confname)
						goto fallback;
					printf("Could not find config %s\n", confname);
					return 1;
				}

				if (verify && fit_config_verify(fit_hdr, noffset))
					return 1;

				noffset = fit_conf_get_prop_node(fit_hdr,
								 noffset,
								 FIT_SCRIPT_PROP,
								 IH_PHASE_NONE);
				if (noffset < 0) {
					if (!confname)
						goto fallback;
					printf("Could not find script in %s\n", confname);
					return 1;
				}
			} else {
fallback:
				if (!fit_uname || !*fit_uname)
					fit_uname = get_default_image(fit_hdr);
				if (!fit_uname) {
					puts("No FIT subimage unit name\n");
					return 1;
				}

				/* get script component image node offset */
				noffset = fit_image_get_node(fit_hdr, fit_uname);
				if (noffset < 0) {
					printf("Can't find '%s' FIT subimage\n",
					      fit_uname);
					return 1;
				}
			}

			if (!fit_image_check_type(fit_hdr, noffset,
						  IH_TYPE_SCRIPT)) {
				puts("Not a image image\n");
				return 1;
			}

			/* verify integrity */
			if (verify && !fit_image_verify(fit_hdr, noffset)) {
				puts("Bad Data Hash\n");
				return 1;
			}

			/* get script subimage data address and length */
			if (fit_image_get_data(fit_hdr, noffset, &fit_data, &fit_len)) {
				puts("Could not find script subimage data\n");
				return 1;
			}

			data = (u32 *)fit_data;
			len = (ulong)fit_len;
		}
		break;
	default:
		puts("Wrong image format for \"source\" command\n");
		return -EPERM;
	}

	*datap = (char *)data;
	*lenp = len;

	return 0;
}

int image_source_script(ulong addr, const char *fit_uname, const char *confname)
{
	char *data;
	void *buf;
	uint len;
	int ret;

	buf = map_sysmem(addr, 0);
	ret = image_locate_script(buf, 0, fit_uname, confname, &data, &len);
	unmap_sysmem(buf);
	if (ret)
		return CMD_RET_FAILURE;

	debug("** Script length: %d\n", len);
	return run_command_list(data, len, 0);
}

/**************************************************/
#if defined(CONFIG_CMD_SOURCE)
static int do_source(struct cmd_tbl *cmdtp, int flag, int argc,
		     char *const argv[])
{
	ulong addr;
	int rcode;
	const char *fit_uname = NULL, *confname = NULL;

	/* Find script image */
	if (argc < 2) {
		addr = CONFIG_SYS_LOAD_ADDR;
		debug("*  source: default load address = 0x%08lx\n", addr);
#if defined(CONFIG_FIT)
	} else if (fit_parse_subimage(argv[1], image_load_addr, &addr,
				      &fit_uname)) {
		debug("*  source: subimage '%s' from FIT image at 0x%08lx\n",
		      fit_uname, addr);
	} else if (fit_parse_conf(argv[1], image_load_addr, &addr, &confname)) {
		debug("*  source: config '%s' from FIT image at 0x%08lx\n",
		      confname, addr);
#endif
	} else {
		addr = hextoul(argv[1], NULL);
		debug("*  source: cmdline image address = 0x%08lx\n", addr);
	}

	printf ("## Executing script at %08lx\n", addr);
	rcode = image_source_script(addr, fit_uname, confname);
	return rcode;
}

#ifdef CONFIG_SYS_LONGHELP
static char source_help_text[] =
#if defined(CONFIG_FIT)
	"[<addr>][:[<image>]|#[<config>]]\n"
	"\t- Run script starting at addr\n"
	"\t- A FIT config name or subimage name may be specified with : or #\n"
	"\t  (like bootm). If the image or config name is omitted, the\n"
	"\t  default is used.";
#else
	"[<addr>]\n"
	"\t- Run script starting at addr";
#endif
#endif

U_BOOT_CMD(
	source, 2, 0,	do_source,
	"run script from memory", source_help_text
);
#endif
