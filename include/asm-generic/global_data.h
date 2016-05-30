/*
 * Copyright (c) 2012 The Chromium OS Authors.
 * (C) Copyright 2002-2010
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_GENERIC_GBL_DATA_H
#define __ASM_GENERIC_GBL_DATA_H
/*
 * The following data structure is placed in some memory which is
 * available very early after boot (like DPRAM on MPC8xx/MPC82xx, or
 * some locked parts of the data cache) to allow for a minimum set of
 * global variables during system initialization (until we have set
 * up the memory controller so that we can use RAM).
 *
 * Keep it *SMALL* and remember to set GENERATED_GBL_DATA_SIZE > sizeof(gd_t)
 *
 * Each architecture has its own private fields. For now all are private
 */

#ifndef __ASSEMBLY__
#include <membuff.h>
#include <linux/list.h>

typedef struct global_data {
	bd_t *bd;
	unsigned long flags;
	unsigned int baudrate;
	unsigned long cpu_clk;	/* CPU clock in Hz!		*/
	unsigned long bus_clk;
	/* We cannot bracket this with CONFIG_PCI due to mpc5xxx */
	unsigned long pci_clk;
	unsigned long mem_clk;
#if defined(CONFIG_LCD) || defined(CONFIG_VIDEO)
	unsigned long fb_base;	/* Base address of framebuffer mem */
#endif
#if defined(CONFIG_POST) || defined(CONFIG_LOGBUFFER)
	unsigned long post_log_word;  /* Record POST activities */
	unsigned long post_log_res; /* success of POST test */
	unsigned long post_init_f_time;  /* When post_init_f started */
#endif
#ifdef CONFIG_BOARD_TYPES
	unsigned long board_type;
#endif
	unsigned long have_console;	/* serial_init() was called */
#ifdef CONFIG_PRE_CONSOLE_BUFFER
	unsigned long precon_buf_idx;	/* Pre-Console buffer index */
#endif
	unsigned long env_addr;	/* Address  of Environment struct */
	unsigned long env_valid;	/* Checksum of Environment valid? */

	unsigned long ram_top;	/* Top address of RAM used by U-Boot */

	unsigned long relocaddr;	/* Start address of U-Boot in RAM */
	phys_size_t ram_size;	/* RAM size */
#ifdef CONFIG_SYS_MEM_RESERVE_SECURE
#define MEM_RESERVE_SECURE_SECURED	0x1
#define MEM_RESERVE_SECURE_MAINTAINED	0x2
#define MEM_RESERVE_SECURE_ADDR_MASK	(~0x3)
	/*
	 * Secure memory addr
	 * This variable needs maintenance if the RAM base is not zero,
	 * or if RAM splits into non-consecutive banks. It also has a
	 * flag indicating the secure memory is marked as secure by MMU.
	 * Flags used: 0x1 secured
	 *             0x2 maintained
	 */
	phys_addr_t secure_ram;
#endif
	unsigned long mon_len;	/* monitor len */
	unsigned long irq_sp;		/* irq stack pointer */
	unsigned long start_addr_sp;	/* start_addr_stackpointer */
	unsigned long reloc_off;
	struct global_data *new_gd;	/* relocated global data */

#ifdef CONFIG_DM
	struct udevice	*dm_root;	/* Root instance for Driver Model */
	struct udevice	*dm_root_f;	/* Pre-relocation root instance */
	struct list_head uclass_root;	/* Head of core tree */
#endif
#ifdef CONFIG_TIMER
	struct udevice	*timer;	/* Timer instance for Driver Model */
#endif

	const void *fdt_blob;	/* Our device tree, NULL if none */
	void *new_fdt;		/* Relocated FDT */
	unsigned long fdt_size;	/* Space reserved for relocated FDT */
	struct jt_funcs *jt;		/* jump table */
	char env_buf[32];	/* buffer for getenv() before reloc. */
#ifdef CONFIG_TRACE
	void		*trace_buff;	/* The trace buffer */
#endif
#if defined(CONFIG_SYS_I2C)
	int		cur_i2c_bus;	/* current used i2c bus */
#endif
#ifdef CONFIG_SYS_I2C_MXC
	void *srdata[10];
#endif
	unsigned long timebase_h;
	unsigned long timebase_l;
#ifdef CONFIG_SYS_MALLOC_F_LEN
	unsigned long malloc_base;	/* base address of early malloc() */
	unsigned long malloc_limit;	/* limit address */
	unsigned long malloc_ptr;	/* current address */
#endif
#ifdef CONFIG_PCI
	struct pci_controller *hose;	/* PCI hose for early use */
	phys_addr_t pci_ram_top;	/* top of region accessible to PCI */
#endif
#ifdef CONFIG_PCI_BOOTDELAY
	int pcidelay_done;
#endif
	struct udevice *cur_serial_dev;	/* current serial device */
	struct arch_global_data arch;	/* architecture-specific data */
#ifdef CONFIG_CONSOLE_RECORD
	struct membuff console_out;	/* console output */
	struct membuff console_in;	/* console input */
#endif
#ifdef CONFIG_DM_VIDEO
	ulong video_top;		/* Top of video frame buffer area */
	ulong video_bottom;		/* Bottom of video frame buffer area */
#endif
} gd_t;
#endif

/*
 * Global Data Flags - the top 16 bits are reserved for arch-specific flags
 */
#define GD_FLG_RELOC		0x00001	/* Code was relocated to RAM	   */
#define GD_FLG_DEVINIT		0x00002	/* Devices have been initialized   */
#define GD_FLG_SILENT		0x00004	/* Silent mode			   */
#define GD_FLG_POSTFAIL		0x00008	/* Critical POST test failed	   */
#define GD_FLG_POSTSTOP		0x00010	/* POST seqeunce aborted	   */
#define GD_FLG_LOGINIT		0x00020	/* Log Buffer has been initialized */
#define GD_FLG_DISABLE_CONSOLE	0x00040	/* Disable console (in & out)	   */
#define GD_FLG_ENV_READY	0x00080	/* Env. imported into hash table   */
#define GD_FLG_SERIAL_READY	0x00100	/* Pre-reloc serial console ready  */
#define GD_FLG_FULL_MALLOC_INIT	0x00200	/* Full malloc() is ready	   */
#define GD_FLG_SPL_INIT		0x00400	/* spl_init() has been called	   */
#define GD_FLG_SKIP_RELOC	0x00800	/* Don't relocate */
#define GD_FLG_RECORD		0x01000	/* Record console */
#define GD_FLG_ENV_DEFAULT	0x02000 /* Default variable flag */

#endif /* __ASM_GENERIC_GBL_DATA_H */
