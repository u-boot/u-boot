/*
 * Elf Load/Boot Functions
 */
#ifndef	_CMD_ELF_H
#define _CMD_ELF_H

#if (CONFIG_COMMANDS & CFG_CMD_ELF)

#define CMD_TBL_BOOTELF MK_CMD_TBL_ENTRY(                               \
        "bootelf",      7,      2,      0,      do_bootelf,             \
        "bootelf - Boot from an ELF image in memory\n",                 \
        " [address] - load address of ELF image.\n"                     \
        ),

#define CMD_TBL_BOOTVX  MK_CMD_TBL_ENTRY(                               \
        "bootvx",       6,      2,      0,      do_bootvx,              \
        "bootvx  - Boot vxWorks from an ELF image\n",                   \
        " [address] - load address of vxWorks ELF image.\n"             \
        ),

extern int do_bootelf (cmd_tbl_t *, int, int, char *[]);
extern int do_bootvx (cmd_tbl_t *, int, int, char *[]);

/* Supporting routines */
extern int           valid_elf_image (unsigned long);
extern unsigned long load_elf_image (unsigned long);

#else	/* ! CFG_CMD_ELF */

#define CMD_TBL_BOOTELF
#define CMD_TBL_BOOTVX

#endif	/* CFG_CMD_ELF */
#endif	/* _CMD_ELF_H */
