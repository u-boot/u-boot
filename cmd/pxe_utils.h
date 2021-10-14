/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __PXE_UTILS_H
#define __PXE_UTILS_H

#include <linux/list.h>

/*
 * A note on the pxe file parser.
 *
 * We're parsing files that use syslinux grammar, which has a few quirks.
 * String literals must be recognized based on context - there is no
 * quoting or escaping support. There's also nothing to explicitly indicate
 * when a label section completes. We deal with that by ending a label
 * section whenever we see a line that doesn't include.
 *
 * As with the syslinux family, this same file format could be reused in the
 * future for non pxe purposes. The only action it takes during parsing that
 * would throw this off is handling of include files. It assumes we're using
 * pxe, and does a tftp download of a file listed as an include file in the
 * middle of the parsing operation. That could be handled by refactoring it to
 * take a 'include file getter' function.
 */

/*
 * Describes a single label given in a pxe file.
 *
 * Create these with the 'label_create' function given below.
 *
 * name - the name of the menu as given on the 'menu label' line.
 * kernel - the path to the kernel file to use for this label.
 * append - kernel command line to use when booting this label
 * initrd - path to the initrd to use for this label.
 * attempted - 0 if we haven't tried to boot this label, 1 if we have.
 * localboot - 1 if this label specified 'localboot', 0 otherwise.
 * list - lets these form a list, which a pxe_menu struct will hold.
 */
struct pxe_label {
	char num[4];
	char *name;
	char *menu;
	char *kernel;
	char *config;
	char *append;
	char *initrd;
	char *fdt;
	char *fdtdir;
	char *fdtoverlays;
	int ipappend;
	int attempted;
	int localboot;
	int localboot_val;
	struct list_head list;
};

/*
 * Describes a pxe menu as given via pxe files.
 *
 * title - the name of the menu as given by a 'menu title' line.
 * default_label - the name of the default label, if any.
 * bmp - the bmp file name which is displayed in background
 * timeout - time in tenths of a second to wait for a user key-press before
 *           booting the default label.
 * prompt - if 0, don't prompt for a choice unless the timeout period is
 *          interrupted.  If 1, always prompt for a choice regardless of
 *          timeout.
 * labels - a list of labels defined for the menu.
 */
struct pxe_menu {
	char *title;
	char *default_label;
	char *bmp;
	int timeout;
	int prompt;
	struct list_head labels;
};

extern bool is_pxe;

extern int (*do_getfile)(struct cmd_tbl *cmdtp, const char *file_path,
			 char *file_addr);
void destroy_pxe_menu(struct pxe_menu *cfg);

/**
 * get_pxe_file() - Read a file
 *
 * Retrieve the file at 'file_path' to the locate given by 'file_addr'. If
 * 'bootfile' was specified in the environment, the path to bootfile will be
 * prepended to 'file_path' and the resulting path will be used.
 *
 * @cmdtp: Pointer to command-table entry for the initiating command
 * @file_path: Path to file
 * @file_addr: Address to place file
 * Returns 1 on success, or < 0 for error
 */
int get_pxe_file(struct cmd_tbl *cmdtp, const char *file_path,
		 ulong file_addr);

/**
 * get_pxelinux_path() - Read a file from the same place as pxelinux.cfg
 *
 * Retrieves a file in the 'pxelinux.cfg' folder. Since this uses get_pxe_file()
 * to do the hard work, the location of the 'pxelinux.cfg' folder is generated
 * from the bootfile path, as described in get_pxe_file().
 *
 * @cmdtp: Pointer to command-table entry for the initiating command
 * @file: Relative path to file
 * @pxefile_addr_r: Address to load file
 * Returns 1 on success or < 0 on error.
 */
int get_pxelinux_path(struct cmd_tbl *cmdtp, const char *file,
		      ulong pxefile_addr_r);

/**
 * handle_pxe_menu() - Boot the system as prescribed by a pxe_menu.
 *
 * Use the menu system to either get the user's choice or the default, based
 * on config or user input.  If there is no default or user's choice,
 * attempted to boot labels in the order they were given in pxe files.
 * If the default or user's choice fails to boot, attempt to boot other
 * labels in the order they were given in pxe files.
 *
 * If this function returns, there weren't any labels that successfully
 * booted, or the user interrupted the menu selection via ctrl+c.
 *
 * @cmdtp: Pointer to command-table entry for the initiating command
 * @cfg: PXE menu
 */
void handle_pxe_menu(struct cmd_tbl *cmdtp, struct pxe_menu *cfg);

/**
 * parse_pxefile() - Parsing a pxe file
 *
 * This is only used for the top-level file.
 *
 * @cmdtp: Pointer to command-table entry for the initiating command
 * @menucfg: Address of PXE file
 *
 * Returns NULL if there is an error, otherwise, returns a pointer to a
 * pxe_menu struct populated with the results of parsing the pxe file (and any
 * files it includes). The resulting pxe_menu struct can be free()'d by using
 * the destroy_pxe_menu() function.
 */
struct pxe_menu *parse_pxefile(struct cmd_tbl *cmdtp, ulong menucfg);

/**
 * format_mac_pxe() - Convert a MAC address to PXE format
 *
 * Convert an ethaddr from the environment to the format used by pxelinux
 * filenames based on mac addresses. Convert's ':' to '-', and adds "01-" to
 * the beginning of the ethernet address to indicate a hardware type of
 * Ethernet. Also converts uppercase hex characters into lowercase, to match
 * pxelinux's behavior.
 *
 * @outbuf: Buffer to hold the output (must hold 22 bytes)
 * @outbuf_len: Length of buffer
 * Returns 1 for success, -ENOENT if 'ethaddr' is undefined in the
 * environment, or some other value < 0 on error.
 */
int format_mac_pxe(char *outbuf, size_t outbuf_len);

#endif /* __PXE_UTILS_H */
