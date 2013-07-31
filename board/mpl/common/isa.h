/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland, d.peter@mpl.ch
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _ISA_H_
#define _ISA_H_
/* Super IO */
#define SIO_CFG_PORT	0x3F0	/* Config Port Address */

#if defined(CONFIG_PIP405)
/* table fore SIO initialization */
typedef struct {
	const uchar index;
	const uchar val;
} SIO_LOGDEV_TABLE;

typedef struct {
	const uchar ldev;
	const SIO_LOGDEV_TABLE *ldev_table;
} SIO_TABLE;


unsigned char open_cfg_super_IO(int address);
unsigned char read_cfg_super_IO(int address, unsigned char function, unsigned char regaddr);
void write_cfg_super_IO(int address, unsigned char function, unsigned char regaddr, unsigned char data);
void close_cfg_super_IO(int address);
void isa_sio_setup(void);
#endif

void isa_irq_install_handler(int vec, interrupt_handler_t *handler, void *arg);
void isa_irq_free_handler(int vec);
int handle_isa_int(void);
void isa_init_irq_contr(void);
void isa_show_irq(void);
int isa_irq_get_count(int vec);


#endif
