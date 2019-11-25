/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef __PXE_UTILS_H
#define __PXE_UTILS_H

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

extern int (*do_getfile)(cmd_tbl_t *cmdtp, const char *file_path,
			 char *file_addr);
void destroy_pxe_menu(struct pxe_menu *cfg);
int get_pxe_file(cmd_tbl_t *cmdtp, const char *file_path,
		 unsigned long file_addr);
int get_pxelinux_path(cmd_tbl_t *cmdtp, const char *file,
		      unsigned long pxefile_addr_r);
void handle_pxe_menu(cmd_tbl_t *cmdtp, struct pxe_menu *cfg);
struct pxe_menu *parse_pxefile(cmd_tbl_t *cmdtp, unsigned long menucfg);
int format_mac_pxe(char *outbuf, size_t outbuf_len);

#endif /* __PXE_UTILS_H */
