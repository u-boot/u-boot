/*
 * (C) Copyright 2002
 * Stäubli Faverges - <www.staubli.com>
 * Pierre AUBERT  p.aubert@staubli.com
 *
 * (C) Copyright 2005
 * Martin Krause TQ-Systems GmbH martin.krause@tqs.de
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Basic video support for SMI SM501 "Voyager" graphic controller
 */

#ifndef _SM501_H_
#define _SM501_H_

#define PCI_VENDOR_SM		0x126f
#define PCI_DEVICE_SM501	0x0501

typedef struct {
	unsigned int Index;
	unsigned int Value;
} SMI_REGS;

/* Board specific functions                                                  */
unsigned int board_video_init (void);
void board_validate_screen (unsigned int base);
const SMI_REGS *board_get_regs (void);
int board_get_width (void);
int board_get_height (void);
unsigned int board_video_get_fb (void);

#endif /* _SM501_H_ */
