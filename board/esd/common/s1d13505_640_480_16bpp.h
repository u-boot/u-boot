/*
 * (C) Copyright 2008
 * Matthias Fuchs, esd gmbh germany, matthias.fuchs@esd-electronics.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Panel:  640x480 50Hz TFT Single 18-bit (PCLK=20.000 MHz)
 * Memory:  DRAM (MCLK=40.000 MHz)
 */
static S1D_REGS regs_13505_640_480_16bpp[] =
{
	{0x1B,0x00},   /* Miscellaneous Register */
	{0x23,0x20},   /* Performance Enhancement Register 1 */
	{0x01,0x30},   /* Memory Configuration Register */
	{0x22,0x24},   /* Performance Enhancement Register 0 */
	{0x02,0x25},   /* Panel Type Register */
	{0x03,0x00},   /* MOD Rate Register */
	{0x04,0x4F},   /* Horizontal Display Width Register */
	{0x05,0x0c},   /* Horizontal Non-Display Period Register */
	{0x06,0x00},   /* HRTC/FPLINE Start Position Register */
	{0x07,0x01},   /* HRTC/FPLINE Pulse Width Register */
	{0x08,0xDF},   /* Vertical Display Height Register 0 */
	{0x09,0x01},   /* Vertical Display Height Register 1 */
	{0x0A,0x3E},   /* Vertical Non-Display Period Register */
	{0x0B,0x00},   /* VRTC/FPFRAME Start Position Register */
	{0x0C,0x01},   /* VRTC/FPFRAME Pulse Width Register */
	{0x0E,0xFF},   /* Screen 1 Line Compare Register 0 */
	{0x0F,0x03},   /* Screen 1 Line Compare Register 1 */
	{0x10,0x00},   /* Screen 1 Display Start Address Register 0 */
	{0x11,0x00},   /* Screen 1 Display Start Address Register 1 */
	{0x12,0x00},   /* Screen 1 Display Start Address Register 2 */
	{0x13,0x00},   /* Screen 2 Display Start Address Register 0 */
	{0x14,0x00},   /* Screen 2 Display Start Address Register 1 */
	{0x15,0x00},   /* Screen 2 Display Start Address Register 2 */
	{0x16,0x80},   /* Memory Address Offset Register 0 */
	{0x17,0x02},   /* Memory Address Offset Register 1 */
	{0x18,0x00},   /* Pixel Panning Register */
	{0x19,0x01},   /* Clock Configuration Register */
	{0x1A,0x00},   /* Power Save Configuration Register */
	{0x1C,0x00},   /* MD Configuration Readback Register 0 */
	{0x1E,0x06},   /* General IO Pins Configuration Register 0 */
	{0x1F,0x00},   /* General IO Pins Configuration Register 1 */
	{0x20,0x00},   /* General IO Pins Control Register 0 */
	{0x21,0x00},   /* General IO Pins Control Register 1 */
	{0x23,0x20},   /* Performance Enhancement Register 1 */
	{0x0D,0x15},   /* Display Mode Register */
};
