/*
 * (C) Copyright 2008
 * Martha J Marx, Silicon Turnkey Express, mmarx@silicontkx.com
 * mpc512x I/O pin/pad initialization for the ADS5121 board
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

#define IOCTL_MEM		0x000
#define IOCTL_GP		0x004
#define IOCTL_LPC_CLK		0x008
#define IOCTL_LPC_OE		0x00C
#define IOCTL_LPC_RWB		0x010
#define IOCTL_LPC_ACK		0x014
#define IOCTL_LPC_CS0		0x018
#define IOCTL_NFC_CE0		0x01C
#define IOCTL_LPC_CS1		0x020
#define IOCTL_LPC_CS2		0x024
#define IOCTL_LPC_AX03		0x028
#define IOCTL_EMB_AX02		0x02C
#define IOCTL_EMB_AX01		0x030
#define IOCTL_EMB_AX00		0x034
#define IOCTL_EMB_AD31		0x038
#define IOCTL_EMB_AD30		0x03C
#define IOCTL_EMB_AD29		0x040
#define IOCTL_EMB_AD28		0x044
#define IOCTL_EMB_AD27		0x048
#define IOCTL_EMB_AD26		0x04C
#define IOCTL_EMB_AD25		0x050
#define IOCTL_EMB_AD24		0x054
#define IOCTL_EMB_AD23		0x058
#define IOCTL_EMB_AD22		0x05C
#define IOCTL_EMB_AD21		0x060
#define IOCTL_EMB_AD20		0x064
#define IOCTL_EMB_AD19		0x068
#define IOCTL_EMB_AD18		0x06C
#define IOCTL_EMB_AD17		0x070
#define IOCTL_EMB_AD16		0x074
#define IOCTL_EMB_AD15		0x078
#define IOCTL_EMB_AD14		0x07C
#define IOCTL_EMB_AD13		0x080
#define IOCTL_EMB_AD12		0x084
#define IOCTL_EMB_AD11		0x088
#define IOCTL_EMB_AD10		0x08C
#define IOCTL_EMB_AD09		0x090
#define IOCTL_EMB_AD08		0x094
#define IOCTL_EMB_AD07		0x098
#define IOCTL_EMB_AD06		0x09C
#define IOCTL_EMB_AD05		0x0A0
#define IOCTL_EMB_AD04		0x0A4
#define IOCTL_EMB_AD03		0x0A8
#define IOCTL_EMB_AD02		0x0AC
#define IOCTL_EMB_AD01		0x0B0
#define IOCTL_EMB_AD00		0x0B4
#define IOCTL_PATA_CE1		0x0B8
#define IOCTL_PATA_CE2		0x0BC
#define IOCTL_PATA_ISOLATE	0x0C0
#define IOCTL_PATA_IOR		0x0C4
#define IOCTL_PATA_IOW		0x0C8
#define IOCTL_PATA_IOCHRDY	0x0CC
#define IOCTL_PATA_INTRQ	0x0D0
#define IOCTL_PATA_DRQ		0x0D4
#define IOCTL_PATA_DACK		0x0D8
#define IOCTL_NFC_WP		0x0DC
#define IOCTL_NFC_RB		0x0E0
#define IOCTL_NFC_ALE		0x0E4
#define IOCTL_NFC_CLE		0x0E8
#define IOCTL_NFC_WE		0x0EC
#define IOCTL_NFC_RE		0x0F0
#define IOCTL_PCI_AD31		0x0F4
#define IOCTL_PCI_AD30		0x0F8
#define IOCTL_PCI_AD29		0x0FC
#define IOCTL_PCI_AD28		0x100
#define IOCTL_PCI_AD27		0x104
#define IOCTL_PCI_AD26		0x108
#define IOCTL_PCI_AD25		0x10C
#define IOCTL_PCI_AD24		0x110
#define IOCTL_PCI_AD23		0x114
#define IOCTL_PCI_AD22		0x118
#define IOCTL_PCI_AD21		0x11C
#define IOCTL_PCI_AD20		0x120
#define IOCTL_PCI_AD19		0x124
#define IOCTL_PCI_AD18		0x128
#define IOCTL_PCI_AD17		0x12C
#define IOCTL_PCI_AD16		0x130
#define IOCTL_PCI_AD15		0x134
#define IOCTL_PCI_AD14		0x138
#define IOCTL_PCI_AD13		0x13C
#define IOCTL_PCI_AD12		0x140
#define IOCTL_PCI_AD11		0x144
#define IOCTL_PCI_AD10		0x148
#define IOCTL_PCI_AD09		0x14C
#define IOCTL_PCI_AD08		0x150
#define IOCTL_PCI_AD07		0x154
#define IOCTL_PCI_AD06		0x158
#define IOCTL_PCI_AD05		0x15C
#define IOCTL_PCI_AD04		0x160
#define IOCTL_PCI_AD03		0x164
#define IOCTL_PCI_AD02		0x168
#define IOCTL_PCI_AD01		0x16C
#define IOCTL_PCI_AD00		0x170
#define IOCTL_PCI_CBE0		0x174
#define IOCTL_PCI_CBE1		0x178
#define IOCTL_PCI_CBE2		0x17C
#define IOCTL_PCI_CBE3		0x180
#define IOCTL_PCI_GNT2		0x184
#define IOCTL_PCI_REQ2		0x188
#define IOCTL_PCI_GNT1		0x18C
#define IOCTL_PCI_REQ1		0x190
#define IOCTL_PCI_GNT0		0x194
#define IOCTL_PCI_REQ0		0x198
#define IOCTL_PCI_INTA		0x19C
#define IOCTL_PCI_CLK		0x1A0
#define IOCTL_PCI_RST_OUT	0x1A4
#define IOCTL_PCI_FRAME		0x1A8
#define IOCTL_PCI_IDSEL		0x1AC
#define IOCTL_PCI_DEVSEL	0x1B0
#define IOCTL_PCI_IRDY		0x1B4
#define IOCTL_PCI_TRDY		0x1B8
#define IOCTL_PCI_STOP		0x1BC
#define IOCTL_PCI_PAR		0x1C0
#define IOCTL_PCI_PERR		0x1C4
#define IOCTL_PCI_SERR		0x1C8
#define IOCTL_SPDIF_TXCLK	0x1CC
#define IOCTL_SPDIF_TX		0x1D0
#define IOCTL_SPDIF_RX		0x1D4
#define IOCTL_I2C0_SCL		0x1D8
#define IOCTL_I2C0_SDA		0x1DC
#define IOCTL_I2C1_SCL		0x1E0
#define IOCTL_I2C1_SDA		0x1E4
#define IOCTL_I2C2_SCL		0x1E8
#define IOCTL_I2C2_SDA		0x1EC
#define IOCTL_IRQ0		0x1F0
#define IOCTL_IRQ1		0x1F4
#define IOCTL_CAN1_TX		0x1F8
#define IOCTL_CAN2_TX		0x1FC
#define IOCTL_J1850_TX		0x200
#define IOCTL_J1850_RX		0x204
#define IOCTL_PSC_MCLK_IN	0x208
#define IOCTL_PSC0_0		0x20C
#define IOCTL_PSC0_1		0x210
#define IOCTL_PSC0_2		0x214
#define IOCTL_PSC0_3		0x218
#define IOCTL_PSC0_4		0x21C
#define IOCTL_PSC1_0		0x220
#define IOCTL_PSC1_1		0x224
#define IOCTL_PSC1_2		0x228
#define IOCTL_PSC1_3		0x22C
#define IOCTL_PSC1_4		0x230
#define IOCTL_PSC2_0		0x234
#define IOCTL_PSC2_1		0x238
#define IOCTL_PSC2_2		0x23C
#define IOCTL_PSC2_3		0x240
#define IOCTL_PSC2_4		0x244
#define IOCTL_PSC3_0		0x248
#define IOCTL_PSC3_1		0x24C
#define IOCTL_PSC3_2		0x250
#define IOCTL_PSC3_3		0x254
#define IOCTL_PSC3_4		0x258
#define IOCTL_PSC4_0		0x25C
#define IOCTL_PSC4_1		0x260
#define IOCTL_PSC4_2		0x264
#define IOCTL_PSC4_3		0x268
#define IOCTL_PSC4_4		0x26C
#define IOCTL_PSC5_0		0x270
#define IOCTL_PSC5_1		0x274
#define IOCTL_PSC5_2		0x278
#define IOCTL_PSC5_3		0x27C
#define IOCTL_PSC5_4		0x280
#define IOCTL_PSC6_0		0x284
#define IOCTL_PSC6_1		0x288
#define IOCTL_PSC6_2		0x28C
#define IOCTL_PSC6_3		0x290
#define IOCTL_PSC6_4		0x294
#define IOCTL_PSC7_0		0x298
#define IOCTL_PSC7_1		0x29C
#define IOCTL_PSC7_2		0x2A0
#define IOCTL_PSC7_3		0x2A4
#define IOCTL_PSC7_4		0x2A8
#define IOCTL_PSC8_0		0x2AC
#define IOCTL_PSC8_1		0x2B0
#define IOCTL_PSC8_2		0x2B4
#define IOCTL_PSC8_3		0x2B8
#define IOCTL_PSC8_4		0x2BC
#define IOCTL_PSC9_0		0x2C0
#define IOCTL_PSC9_1		0x2C4
#define IOCTL_PSC9_2		0x2C8
#define IOCTL_PSC9_3		0x2CC
#define IOCTL_PSC9_4		0x2D0
#define IOCTL_PSC10_0		0x2D4
#define IOCTL_PSC10_1		0x2D8
#define IOCTL_PSC10_2		0x2DC
#define IOCTL_PSC10_3		0x2E0
#define IOCTL_PSC10_4		0x2E4
#define IOCTL_PSC11_0		0x2E8
#define IOCTL_PSC11_1		0x2EC
#define IOCTL_PSC11_2		0x2F0
#define IOCTL_PSC11_3		0x2F4
#define IOCTL_PSC11_4		0x2F8
#define IOCTL_HRESET		0x2FC
#define IOCTL_SRESET		0x300
#define IOCTL_CKSTP_OUT		0x304
#define IOCTL_USB2_VBUS_PWR_FAULT	0x308
#define IOCTL_USB2_VBUS_PWR_SELECT	0x30C
#define IOCTL_USB2_PHY_DRVV_BUS		0x310

extern void iopin_initialize(void);
