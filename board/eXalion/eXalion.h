/*
 * (C) Copyright 2002
 * Torsten Demke, FORCE Computers GmbH. torsten.demke@fci.com
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * (C) Copyright 2002
 * James Dougherty (jfd@broadcom.com)
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

#ifndef __EXALION_H
#define __EXALION_H

/* IRQ settings */
#define  PCI_INT_NA (0xff)   /* PCI Intr. not used */
#define  PCI_INT_A  (0x09)   /* PCI Intr. A Interrupt Request Line Nr. */
#define  PCI_INT_B  (0x0a)   /* PCI Intr. B Interrupt Request Line Nr. */
#define  PCI_INT_C  (0x0b)   /* PCI Intr. C Interrupt Request Line Nr. */
#define  PCI_INT_D  (0x0c)   /* PCI Intr. D Interrupt Request Line Nr. */
#if defined (CPU_MPC8245)
#define  LN_1_INT     PCI_INT_B  /* ethernet interrupt level */
#define  LN_2_INT     PCI_INT_C  /* ethernet interrupt level */
#define  BCM_1_INT    PCI_INT_A  /* BCM5690 interrupt level */
#define  BCM_2_INT    PCI_INT_B  /* BCM5690 interrupt level */
#elif defined (CPU_MPC8240)
#define  BCM_INT      PCI_INT_B  /* BCM5600 interrupt level */
#define  LN_INT       PCI_INT_C  /* ethernet interrupt level */
#endif

#ifndef __ASSEMBLY__
#endif /* !__ASSEMBLY__ */

#endif /* __EXALION_H */
