/*
 * Copyright (C) 2015, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _QUARK_MSG_PORT_H_
#define _QUARK_MSG_PORT_H_

/*
 * In the Quark SoC, some chipset commands are accomplished by utilizing
 * the internal message network within the host bridge (D0:F0). Accesses
 * to this network are accomplished by populating the message control
 * register (MCR), Message Control Register eXtension (MCRX) and the
 * message data register (MDR).
 */
#define MSG_CTRL_REG		0xd0	/* Message Control Register */
#define MSG_DATA_REG		0xd4	/* Message Data Register */
#define MSG_CTRL_EXT_REG	0xd8	/* Message Control Register EXT */

/* Normal Read/Write OpCodes */
#define MSG_OP_READ		0x10
#define MSG_OP_WRITE		0x11

/* Alternative Read/Write OpCodes */
#define MSG_OP_ALT_READ		0x06
#define MSG_OP_ALT_WRITE	0x07

/* IO Read/Write OpCodes */
#define MSG_OP_IO_READ		0x02
#define MSG_OP_IO_WRITE		0x03

/* All byte enables */
#define MSG_BYTE_ENABLE		0xf0

#ifndef __ASSEMBLY__

/**
 * msg_port_setup - set up the message port control register
 *
 * @op:     message bus access opcode
 * @port:   port number on the message bus
 * @reg:    register number within a port
 */
void msg_port_setup(int op, int port, int reg);

/**
 * msg_port_read - read a message port register using normal opcode
 *
 * @port:   port number on the message bus
 * @reg:    register number within a port
 *
 * @return: message port register value
 */
u32 msg_port_read(u8 port, u32 reg);

/**
 * msg_port_write - write a message port register using normal opcode
 *
 * @port:   port number on the message bus
 * @reg:    register number within a port
 * @value:  register value to write
 */
void msg_port_write(u8 port, u32 reg, u32 value);

/**
 * msg_port_alt_read - read a message port register using alternative opcode
 *
 * @port:   port number on the message bus
 * @reg:    register number within a port
 *
 * @return: message port register value
 */
u32 msg_port_alt_read(u8 port, u32 reg);

/**
 * msg_port_alt_write - write a message port register using alternative opcode
 *
 * @port:   port number on the message bus
 * @reg:    register number within a port
 * @value:  register value to write
 */
void msg_port_alt_write(u8 port, u32 reg, u32 value);

/**
 * msg_port_io_read - read a message port register using I/O opcode
 *
 * @port:   port number on the message bus
 * @reg:    register number within a port
 *
 * @return: message port register value
 */
u32 msg_port_io_read(u8 port, u32 reg);

/**
 * msg_port_io_write - write a message port register using I/O opcode
 *
 * @port:   port number on the message bus
 * @reg:    register number within a port
 * @value:  register value to write
 */
void msg_port_io_write(u8 port, u32 reg, u32 value);

#endif /* __ASSEMBLY__ */

#endif /* _QUARK_MSG_PORT_H_ */
