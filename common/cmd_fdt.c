/*
 * (C) Copyright 2007
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com
 * Based on code written by:
 *   Pantelis Antoniou <pantelis.antoniou@gmail.com> and
 *   Matthew McClintock <msm@freescale.com>
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

#include <common.h>
#include <command.h>
#include <linux/ctype.h>
#include <linux/types.h>
#include <asm/global_data.h>
#include <fdt.h>
#include <libfdt.h>
#include <fdt_support.h>

#define MAX_LEVEL	32		/* how deeply nested we will go */
#define SCRATCHPAD	1024		/* bytes of scratchpad memory */

/*
 * Global data (for the gd->bd)
 */
DECLARE_GLOBAL_DATA_PTR;

static int fdt_valid(void);
static int fdt_parse_prop(char *pathp, char *prop, char *newval,
	char *data, int *len);
static int fdt_print(const char *pathp, char *prop, int depth);

/*
 * Flattened Device Tree command, see the help for parameter definitions.
 */
int do_fdt (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	if (argc < 2) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	/********************************************************************
	 * Set the address of the fdt
	 ********************************************************************/
	if (argv[1][0] == 'a') {
		/*
		 * Set the address [and length] of the fdt.
		 */
		fdt = (struct fdt_header *)simple_strtoul(argv[2], NULL, 16);

		if (!fdt_valid()) {
			return 1;
		}

		if (argc >= 4) {
			int  len;
			int  err;
			/*
			 * Optional new length
			 */
			len = simple_strtoul(argv[3], NULL, 16);
			if (len < fdt_totalsize(fdt)) {
				printf ("New length %d < existing length %d, "
					"ignoring.\n",
					len, fdt_totalsize(fdt));
			} else {
				/*
				 * Open in place with a new length.
				 */
				err = fdt_open_into(fdt, fdt, len);
				if (err != 0) {
					printf ("libfdt fdt_open_into(): %s\n",
						fdt_strerror(err));
				}
			}
		}

	/********************************************************************
	 * Move the fdt
	 ********************************************************************/
	} else if ((argv[1][0] == 'm') && (argv[1][1] == 'o')) {
		struct fdt_header *newaddr;
		int  len;
		int  err;

		if (argc < 4) {
			printf ("Usage:\n%s\n", cmdtp->usage);
			return 1;
		}

		/*
		 * Set the address and length of the fdt.
		 */
		fdt = (struct fdt_header *)simple_strtoul(argv[2], NULL, 16);
		if (!fdt_valid()) {
			return 1;
		}

		newaddr = (struct fdt_header *)simple_strtoul(argv[3],NULL,16);

		/*
		 * If the user specifies a length, use that.  Otherwise use the
		 * current length.
		 */
		if (argc <= 4) {
			len = fdt_totalsize(fdt);
		} else {
			len = simple_strtoul(argv[4], NULL, 16);
			if (len < fdt_totalsize(fdt)) {
				printf ("New length 0x%X < existing length "
					"0x%X, aborting.\n",
					len, fdt_totalsize(fdt));
				return 1;
			}
		}

		/*
		 * Copy to the new location.
		 */
		err = fdt_open_into(fdt, newaddr, len);
		if (err != 0) {
			printf ("libfdt fdt_open_into(): %s\n",
				fdt_strerror(err));
			return 1;
		}
		fdt = newaddr;

	/********************************************************************
	 * Make a new node
	 ********************************************************************/
	} else if ((argv[1][0] == 'm') && (argv[1][1] == 'k')) {
		char *pathp;		/* path */
		char *nodep;		/* new node to add */
		int  nodeoffset;	/* node offset from libfdt */
		int  err;

		/*
		 * Parameters: Node path, new node to be appended to the path.
		 */
		if (argc < 4) {
			printf ("Usage:\n%s\n", cmdtp->usage);
			return 1;
		}

		pathp = argv[2];
		nodep = argv[3];

		nodeoffset = fdt_path_offset (fdt, pathp);
		if (nodeoffset < 0) {
			/*
			 * Not found or something else bad happened.
			 */
			printf ("libfdt fdt_path_offset() returned %s\n",
				fdt_strerror(nodeoffset));
			return 1;
		}
		err = fdt_add_subnode(fdt, nodeoffset, nodep);
		if (err < 0) {
			printf ("libfdt fdt_add_subnode(): %s\n",
				fdt_strerror(err));
			return 1;
		}

	/********************************************************************
	 * Set the value of a property in the fdt.
	 ********************************************************************/
	} else if (argv[1][0] == 's') {
		char *pathp;		/* path */
		char *prop;		/* property */
		char *newval;		/* value from the user (as a string) */
		int  nodeoffset;	/* node offset from libfdt */
		static char data[SCRATCHPAD];	/* storage for the property */
		int  len;		/* new length of the property */
		int  ret;		/* return value */

		/*
		 * Parameters: Node path, property, value.
		 */
		if (argc < 5) {
			printf ("Usage:\n%s\n", cmdtp->usage);
			return 1;
		}

		pathp  = argv[2];
		prop   = argv[3];
		newval = argv[4];

		nodeoffset = fdt_path_offset (fdt, pathp);
		if (nodeoffset < 0) {
			/*
			 * Not found or something else bad happened.
			 */
			printf ("libfdt fdt_path_offset() returned %s\n",
				fdt_strerror(nodeoffset));
			return 1;
		}
		ret = fdt_parse_prop(pathp, prop, newval, data, &len);
		if (ret != 0)
			return ret;

		ret = fdt_setprop(fdt, nodeoffset, prop, data, len);
		if (ret < 0) {
			printf ("libfdt fdt_setprop(): %s\n", fdt_strerror(ret));
			return 1;
		}

	/********************************************************************
	 * Print (recursive) / List (single level)
	 ********************************************************************/
	} else if ((argv[1][0] == 'p') || (argv[1][0] == 'l')) {
		int depth = MAX_LEVEL;	/* how deep to print */
		char *pathp;		/* path */
		char *prop;		/* property */
		int  ret;		/* return value */
		static char root[2] = "/";

		/*
		 * list is an alias for print, but limited to 1 level
		 */
		if (argv[1][0] == 'l') {
			depth = 1;
		}

		/*
		 * Get the starting path.  The root node is an oddball,
		 * the offset is zero and has no name.
		 */
		if (argc == 2)
			pathp = root;
		else
			pathp = argv[2];
		if (argc > 3)
			prop = argv[3];
		else
			prop = NULL;

		ret = fdt_print(pathp, prop, depth);
		if (ret != 0)
			return ret;

	/********************************************************************
	 * Remove a property/node
	 ********************************************************************/
	} else if (argv[1][0] == 'r') {
		int  nodeoffset;	/* node offset from libfdt */
		int  err;

		/*
		 * Get the path.  The root node is an oddball, the offset
		 * is zero and has no name.
		 */
		nodeoffset = fdt_path_offset (fdt, argv[2]);
		if (nodeoffset < 0) {
			/*
			 * Not found or something else bad happened.
			 */
			printf ("libfdt fdt_path_offset() returned %s\n",
				fdt_strerror(nodeoffset));
			return 1;
		}
		/*
		 * Do the delete.  A fourth parameter means delete a property,
		 * otherwise delete the node.
		 */
		if (argc > 3) {
			err = fdt_delprop(fdt, nodeoffset, argv[3]);
			if (err < 0) {
				printf("libfdt fdt_delprop():  %s\n",
					fdt_strerror(err));
				return err;
			}
		} else {
			err = fdt_del_node(fdt, nodeoffset);
			if (err < 0) {
				printf("libfdt fdt_del_node():  %s\n",
					fdt_strerror(err));
				return err;
			}
		}
	}
#ifdef CONFIG_OF_BOARD_SETUP
	/* Call the board-specific fixup routine */
	else if (argv[1][0] == 'b')
		ft_board_setup(fdt, gd->bd);
#endif
	/* Create a chosen node */
	else if (argv[1][0] == 'c')
		fdt_chosen(fdt, 0, 0, 1);

#ifdef CONFIG_OF_HAS_UBOOT_ENV
	/* Create a u-boot-env node */
	else if (argv[1][0] == 'e')
		fdt_env(fdt);
#endif
#ifdef CONFIG_OF_HAS_BD_T
	/* Create a bd_t node */
	else if (argv[1][0] == 'b')
		fdt_bd_t(fdt);
#endif
	else {
		/* Unrecognized command */
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	return 0;
}

/****************************************************************************/

static int fdt_valid(void)
{
	int  err;

	if (fdt == NULL) {
		printf ("The address of the fdt is invalid (NULL).\n");
		return 0;
	}

	err = fdt_check_header(fdt);
	if (err == 0)
		return 1;	/* valid */

	if (err < 0) {
		printf("libfdt fdt_check_header(): %s", fdt_strerror(err));
		/*
		 * Be more informative on bad version.
		 */
		if (err == -FDT_ERR_BADVERSION) {
			if (fdt_version(fdt) < FDT_FIRST_SUPPORTED_VERSION) {
				printf (" - too old, fdt $d < %d",
					fdt_version(fdt),
					FDT_FIRST_SUPPORTED_VERSION);
				fdt = NULL;
			}
			if (fdt_last_comp_version(fdt) > FDT_LAST_SUPPORTED_VERSION) {
				printf (" - too new, fdt $d > %d",
					fdt_version(fdt),
					FDT_LAST_SUPPORTED_VERSION);
				fdt = NULL;
			}
			return 0;
		}
		printf("\n");
		return 0;
	}
	return 1;
}

/****************************************************************************/

/*
 * Parse the user's input, partially heuristic.  Valid formats:
 * <00>		- hex byte
 * <0011>	- hex half word (16 bits)
 * <00112233>	- hex word (32 bits)
 *		- hex double words (64 bits) are not supported, must use
 *			a byte stream instead.
 * [00 11 22 .. nn] - byte stream
 * "string"	- If the the value doesn't start with "<" or "[", it is
 *			treated as a string.  Note that the quotes are
 *			stripped by the parser before we get the string.
 */
static int fdt_parse_prop(char *pathp, char *prop, char *newval,
	char *data, int *len)
{
	char *cp;		/* temporary char pointer */
	unsigned long tmp;	/* holds converted values */

	if (*newval == '<') {
		/*
		 * Bigger values than bytes.
		 */
		*len = 0;
		newval++;
		while ((*newval != '>') && (*newval != '\0')) {
			cp = newval;
			tmp = simple_strtoul(cp, &newval, 16);
			if ((newval - cp) <= 2) {
				*data = tmp & 0xFF;
				data  += 1;
				*len += 1;
			} else if ((newval - cp) <= 4) {
				*(uint16_t *)data = __cpu_to_be16(tmp);
				data  += 2;
				*len += 2;
			} else if ((newval - cp) <= 8) {
				*(uint32_t *)data = __cpu_to_be32(tmp);
				data  += 4;
				*len += 4;
			} else {
				printf("Sorry, I could not convert \"%s\"\n",
					cp);
				return 1;
			}
			while (*newval == ' ')
				newval++;
		}
		if (*newval != '>') {
			printf("Unexpected character '%c'\n", *newval);
			return 1;
		}
	} else if (*newval == '[') {
		/*
		 * Byte stream.  Convert the values.
		 */
		*len = 0;
		newval++;
		while ((*newval != ']') && (*newval != '\0')) {
			tmp = simple_strtoul(newval, &newval, 16);
			*data++ = tmp & 0xFF;
			*len    = *len + 1;
			while (*newval == ' ')
				newval++;
		}
		if (*newval != ']') {
			printf("Unexpected character '%c'\n", *newval);
			return 1;
		}
	} else {
		/*
		 * Assume it is a string.  Copy it into our data area for
		 * convenience (including the terminating '\0').
		 */
		*len = strlen(newval) + 1;
		strcpy(data, newval);
	}
	return 0;
}

/****************************************************************************/

/*
 * Heuristic to guess if this is a string or concatenated strings.
 */

static int is_printable_string(const void *data, int len)
{
	const char *s = data;

	/* zero length is not */
	if (len == 0)
		return 0;

	/* must terminate with zero */
	if (s[len - 1] != '\0')
		return 0;

	/* printable or a null byte (concatenated strings) */
	while (((*s == '\0') || isprint(*s)) && (len > 0)) {
		/*
		 * If we see a null, there are three possibilities:
		 * 1) If len == 1, it is the end of the string, printable
		 * 2) Next character also a null, not printable.
		 * 3) Next character not a null, continue to check.
		 */
		if (s[0] == '\0') {
			if (len == 1)
				return 1;
			if (s[1] == '\0')
				return 0;
		}
		s++;
		len--;
	}

	/* Not the null termination, or not done yet: not printable */
	if (*s != '\0' || (len != 0))
		return 0;

	return 1;
}


/*
 * Print the property in the best format, a heuristic guess.  Print as
 * a string, concatenated strings, a byte, word, double word, or (if all
 * else fails) it is printed as a stream of bytes.
 */
static void print_data(const void *data, int len)
{
	int j;
	const u8 *s;

	/* no data, don't print */
	if (len == 0)
		return;

	/*
	 * It is a string, but it may have multiple strings (embedded '\0's).
	 */
	if (is_printable_string(data, len)) {
		puts("\"");
		j = 0;
		while (j < len) {
			if (j > 0)
				puts("\", \"");
			puts(data);
			j    += strlen(data) + 1;
			data += strlen(data) + 1;
		}
		puts("\"");
		return;
	}

	switch (len) {
	case 1:	 /* byte */
		printf("<0x%02x>", (*(u8 *) data) & 0xff);
		break;
	case 2:	 /* half-word */
		printf("<0x%04x>", be16_to_cpu(*(u16 *) data) & 0xffff);
		break;
	case 4:	 /* word */
		printf("<0x%08x>", be32_to_cpu(*(u32 *) data) & 0xffffffffU);
		break;
	case 8:	 /* double-word */
#if __WORDSIZE == 64
		printf("<0x%016llx>", be64_to_cpu(*(uint64_t *) data));
#else
		printf("<0x%08x ", be32_to_cpu(*(u32 *) data) & 0xffffffffU);
		data += 4;
		printf("0x%08x>", be32_to_cpu(*(u32 *) data) & 0xffffffffU);
#endif
		break;
	default:		/* anything else... hexdump */
		printf("[");
		for (j = 0, s = data; j < len; j++)
			printf("%02x%s", s[j], j < len - 1 ? " " : "");
		printf("]");

		break;
	}
}

/****************************************************************************/

/*
 * Recursively print (a portion of) the fdt.  The depth parameter
 * determines how deeply nested the fdt is printed.
 */
static int fdt_print(const char *pathp, char *prop, int depth)
{
	static char tabs[MAX_LEVEL+1] =
		"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t"
		"\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t";
	const void *nodep;	/* property node pointer */
	int  nodeoffset;	/* node offset from libfdt */
	int  nextoffset;	/* next node offset from libfdt */
	uint32_t tag;		/* tag */
	int  len;		/* length of the property */
	int  level = 0;		/* keep track of nesting level */
	const struct fdt_property *fdt_prop;

	nodeoffset = fdt_path_offset (fdt, pathp);
	if (nodeoffset < 0) {
		/*
		 * Not found or something else bad happened.
		 */
		printf ("libfdt fdt_path_offset() returned %s\n",
			fdt_strerror(nodeoffset));
		return 1;
	}
	/*
	 * The user passed in a property as well as node path.
	 * Print only the given property and then return.
	 */
	if (prop) {
		nodep = fdt_getprop (fdt, nodeoffset, prop, &len);
		if (len == 0) {
			/* no property value */
			printf("%s %s\n", pathp, prop);
			return 0;
		} else if (len > 0) {
			printf("%s=", prop);
			print_data (nodep, len);
			printf("\n");
			return 0;
		} else {
			printf ("libfdt fdt_getprop(): %s\n",
				fdt_strerror(len));
			return 1;
		}
	}

	/*
	 * The user passed in a node path and no property,
	 * print the node and all subnodes.
	 */
	while(level >= 0) {
		tag = fdt_next_tag(fdt, nodeoffset, &nextoffset);
		switch(tag) {
		case FDT_BEGIN_NODE:
			pathp = fdt_get_name(fdt, nodeoffset, NULL);
			if (level <= depth) {
				if (pathp == NULL)
					pathp = "/* NULL pointer error */";
				if (*pathp == '\0')
					pathp = "/";	/* root is nameless */
				printf("%s%s {\n",
					&tabs[MAX_LEVEL - level], pathp);
			}
			level++;
			if (level >= MAX_LEVEL) {
				printf("Nested too deep, aborting.\n");
				return 1;
			}
			break;
		case FDT_END_NODE:
			level--;
			if (level <= depth)
				printf("%s};\n", &tabs[MAX_LEVEL - level]);
			if (level == 0) {
				level = -1;		/* exit the loop */
			}
			break;
		case FDT_PROP:
			fdt_prop = fdt_offset_ptr(fdt, nodeoffset,
					sizeof(*fdt_prop));
			pathp    = fdt_string(fdt,
					fdt32_to_cpu(fdt_prop->nameoff));
			len      = fdt32_to_cpu(fdt_prop->len);
			nodep    = fdt_prop->data;
			if (len < 0) {
				printf ("libfdt fdt_getprop(): %s\n",
					fdt_strerror(len));
				return 1;
			} else if (len == 0) {
				/* the property has no value */
				if (level <= depth)
					printf("%s%s;\n",
						&tabs[MAX_LEVEL - level],
						pathp);
			} else {
				if (level <= depth) {
					printf("%s%s=",
						&tabs[MAX_LEVEL - level],
						pathp);
					print_data (nodep, len);
					printf(";\n");
				}
			}
			break;
		case FDT_NOP:
			printf("/* NOP */\n", &tabs[MAX_LEVEL - level]);
			break;
		case FDT_END:
			return 1;
		default:
			if (level <= depth)
				printf("Unknown tag 0x%08X\n", tag);
			return 1;
		}
		nodeoffset = nextoffset;
	}
	return 0;
}

/********************************************************************/

U_BOOT_CMD(
	fdt,	5,	0,	do_fdt,
	"fdt     - flattened device tree utility commands\n",
	    "addr   <addr> [<length>]        - Set the fdt location to <addr>\n"
#ifdef CONFIG_OF_BOARD_SETUP
	"fdt boardsetup                      - Do board-specific set up\n"
#endif
	"fdt move   <fdt> <newaddr> <length> - Copy the fdt to <addr>\n"
	"fdt print  <path> [<prop>]          - Recursive print starting at <path>\n"
	"fdt list   <path> [<prop>]          - Print one level starting at <path>\n"
	"fdt set    <path> <prop> [<val>]    - Set <property> [to <val>]\n"
	"fdt mknode <path> <node>            - Create a new node after <path>\n"
	"fdt rm     <path> [<prop>]          - Delete the node or <property>\n"
	"fdt chosen - Add/update the /chosen branch in the tree\n"
#ifdef CONFIG_OF_HAS_UBOOT_ENV
	"fdt env    - Add/replace the /u-boot-env branch in the tree\n"
#endif
#ifdef CONFIG_OF_HAS_BD_T
	"fdt bd_t   - Add/replace the /bd_t branch in the tree\n"
#endif
	"Hints:\n"
	" If the property you are setting/printing has a '#' character or spaces,\n"
	"     you MUST escape it with a \\ character or quote it with \".\n"
	"Examples: fdt print /               # print the whole tree\n"
	"          fdt print /cpus \"#address-cells\"\n"
	"          fdt set   /cpus \"#address-cells\" \"[00 00 00 01]\"\n"
);
