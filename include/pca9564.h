/*
 * File:         include/pca9564.h
 * Author:
 *
 * Created:      2009-06-23
 * Description:  PCA9564 i2c bridge driver
 *
 * Modified:
 *               Copyright 2009 CJSC "NII STT", http://www.niistt.ru/
 *
 * Bugs:
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see the file COPYING, or write
 * to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef _PCA9564_H
#define _PCA9564_H

/* Clock speeds for the bus */
#define PCA_CON_330kHz      0x00
#define PCA_CON_288kHz      0x01
#define PCA_CON_217kHz      0x02
#define PCA_CON_146kHz      0x03
#define PCA_CON_88kHz       0x04
#define PCA_CON_59kHz       0x05
#define PCA_CON_44kHz       0x06
#define PCA_CON_36kHz       0x07

#define PCA_CON_AA          0x80 /* Assert Acknowledge */
#define PCA_CON_ENSIO       0x40 /* Enable */
#define PCA_CON_STA         0x20 /* Start */
#define PCA_CON_STO         0x10 /* Stop */
#define PCA_CON_SI          0x08 /* Serial Interrupt */
#define PCA_CON_CR          0x07 /* Clock Rate (MASK) */

#endif
