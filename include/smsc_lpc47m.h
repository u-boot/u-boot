/*
 * Copyright (C) 2014, Bin Meng <bmeng.cn@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef _SMSC_LPC47M_H_
#define _SMSC_LPC47M_H_

/**
 * Configure the base I/O port of the specified serial device and enable the
 * serial device.
 *
 * @dev: High 8 bits = Super I/O port, low 8 bits = logical device number.
 * @iobase: Processor I/O port address to assign to this serial device.
 */
void lpc47m_enable_serial(u16 dev, u16 iobase);

#endif /* _SMSC_LPC47M_H_ */
