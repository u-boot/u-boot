/*
 * Copyright (c) 2012
 *
 * Gabriel Huau <contact@huau-gabriel.fr>
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

#ifndef _S3C24X0_IOMUX_H_
#define _S3C24X0_IOMUX_H_

enum s3c2440_iomux_func {
	/* PORT A */
	IOMUXA_ADDR0	= 1,
	IOMUXA_ADDR16	= (1 << 1),
	IOMUXA_ADDR17	= (1 << 2),
	IOMUXA_ADDR18	= (1 << 3),
	IOMUXA_ADDR19	= (1 << 4),
	IOMUXA_ADDR20	= (1 << 5),
	IOMUXA_ADDR21	= (1 << 6),
	IOMUXA_ADDR22	= (1 << 7),
	IOMUXA_ADDR23	= (1 << 8),
	IOMUXA_ADDR24	= (1 << 9),
	IOMUXA_ADDR25	= (1 << 10),
	IOMUXA_ADDR26	= (1 << 11),
	IOMUXA_nGCS1	= (1 << 12),
	IOMUXA_nGCS2	= (1 << 13),
	IOMUXA_nGCS3	= (1 << 14),
	IOMUXA_nGCS4	= (1 << 15),
	IOMUXA_nGCS5	= (1 << 16),
	IOMUXA_CLE	= (1 << 17),
	IOMUXA_ALE	= (1 << 18),
	IOMUXA_nFWE	= (1 << 19),
	IOMUXA_nFRE	= (1 << 20),
	IOMUXA_nRSTOUT	= (1 << 21),
	IOMUXA_nFCE		= (1 << 22),

	/* PORT B */
	IOMUXB_nXDREQ0	= (2 << 20),
	IOMUXB_nXDACK0	= (2 << 18),
	IOMUXB_nXDREQ1	= (2 << 16),
	IOMUXB_nXDACK1	= (2 << 14),
	IOMUXB_nXBREQ	= (2 << 12),
	IOMUXB_nXBACK	= (2 << 10),
	IOMUXB_TCLK0	= (2 << 8),
	IOMUXB_TOUT3	= (2 << 6),
	IOMUXB_TOUT2	= (2 << 4),
	IOMUXB_TOUT1	= (2 << 2),
	IOMUXB_TOUT0	= 2,

	/* PORT C */
	IOMUXC_VS7	= (2 << 30),
	IOMUXC_VS6	= (2 << 28),
	IOMUXC_VS5	= (2 << 26),
	IOMUXC_VS4	= (2 << 24),
	IOMUXC_VS3	= (2 << 22),
	IOMUXC_VS2	= (2 << 20),
	IOMUXC_VS1	= (2 << 18),
	IOMUXC_VS0	= (2 << 16),
	IOMUXC_LCD_LPCREVB	= (2 << 14),
	IOMUXC_LCD_LPCREV	= (2 << 12),
	IOMUXC_LCD_LPCOE	= (2 << 10),
	IOMUXC_VM		= (2 << 8),
	IOMUXC_VFRAME	= (2 << 6),
	IOMUXC_VLINE	= (2 << 4),
	IOMUXC_VCLK		= (2 << 2),
	IOMUXC_LEND		= 2,
	IOMUXC_I2SSDI	= (3 << 8),

	/* PORT D */
	IOMUXD_VS23	= (2 << 30),
	IOMUXD_VS22	= (2 << 28),
	IOMUXD_VS21	= (2 << 26),
	IOMUXD_VS20	= (2 << 24),
	IOMUXD_VS19	= (2 << 22),
	IOMUXD_VS18	= (2 << 20),
	IOMUXD_VS17	= (2 << 18),
	IOMUXD_VS16	= (2 << 16),
	IOMUXD_VS15	= (2 << 14),
	IOMUXD_VS14	= (2 << 12),
	IOMUXD_VS13	= (2 << 10),
	IOMUXD_VS12	= (2 << 8),
	IOMUXD_VS11	= (2 << 6),
	IOMUXD_VS10	= (2 << 4),
	IOMUXD_VS9	= (2 << 2),
	IOMUXD_VS8	= 2,
	IOMUXD_nSS0	= (3 << 30),
	IOMUXD_nSS1	= (3 << 28),
	IOMUXD_SPICLK1	= (3 << 20),
	IOMUXD_SPIMOSI1	= (3 << 18),
	IOMUXD_SPIMISO1	= (3 << 16),

	/* PORT E */
	IOMUXE_IICSDA	= (2 << 30),
	IOMUXE_IICSCL	= (2 << 28),
	IOMUXE_SPICLK0	= (2 << 26),
	IOMUXE_SPIMOSI0	= (2 << 24),
	IOMUXE_SPIMISO0	= (2 << 22),
	IOMUXE_SDDAT3	= (2 << 20),
	IOMUXE_SDDAT2	= (2 << 18),
	IOMUXE_SDDAT1	= (2 << 16),
	IOMUXE_SDDAT0	= (2 << 14),
	IOMUXE_SDCMD	= (2 << 12),
	IOMUXE_SDCLK	= (2 << 10),
	IOMUXE_I2SDO	= (2 << 8),
	IOMUXE_I2SDI	= (2 << 6),
	IOMUXE_CDCLK	= (2 << 4),
	IOMUXE_I2SSCLK	= (2 << 2),
	IOMUXE_I2SLRCK	= 2,
	IOMUXE_AC_SDATA_OUT	= (3 << 8),
	IOMUXE_AC_SDATA_IN	= (3 << 6),
	IOMUXE_AC_nRESET	= (3 << 4),
	IOMUXE_AC_BIT_CLK	= (3 << 2),
	IOMUXE_AC_SYNC		= 3,

	/* PORT F */
	IOMUXF_EINT7	= (2 << 14),
	IOMUXF_EINT6	= (2 << 12),
	IOMUXF_EINT5	= (2 << 10),
	IOMUXF_EINT4	= (2 << 8),
	IOMUXF_EINT3	= (2 << 6),
	IOMUXF_EINT2	= (2 << 4),
	IOMUXF_EINT1	= (2 << 2),
	IOMUXF_EINT0	= 2,

	/* PORT G */
	IOMUXG_EINT23	= (2 << 30),
	IOMUXG_EINT22	= (2 << 28),
	IOMUXG_EINT21	= (2 << 26),
	IOMUXG_EINT20	= (2 << 24),
	IOMUXG_EINT19	= (2 << 22),
	IOMUXG_EINT18	= (2 << 20),
	IOMUXG_EINT17	= (2 << 18),
	IOMUXG_EINT16	= (2 << 16),
	IOMUXG_EINT15	= (2 << 14),
	IOMUXG_EINT14	= (2 << 12),
	IOMUXG_EINT13	= (2 << 10),
	IOMUXG_EINT12	= (2 << 8),
	IOMUXG_EINT11	= (2 << 6),
	IOMUXG_EINT10	= (2 << 4),
	IOMUXG_EINT9	= (2 << 2),
	IOMUXG_EINT8	= 2,
	IOMUXG_TCLK1	= (3 << 22),
	IOMUXG_nCTS1	= (3 << 20),
	IOMUXG_nRTS1	= (3 << 18),
	IOMUXG_SPICLK1	= (3 << 14),
	IOMUXG_SPIMOSI1	= (3 << 12),
	IOMUXG_SPIMISO1	= (3 << 10),
	IOMUXG_LCD_PWRDN	= (3 << 8),
	IOMUXG_nSS1			= (3 << 6),
	IOMUXG_nSS0			= (3 << 4),

	/* PORT H */
	IOMUXH_CLKOUT1	= (2 << 20),
	IOMUXH_CLKOUT0	= (2 << 18),
	IOMUXH_UEXTCLK	= (2 << 16),
	IOMUXH_RXD2		= (2 << 14),
	IOMUXH_TXD2		= (2 << 12),
	IOMUXH_RXD1		= (2 << 10),
	IOMUXH_TXD1		= (2 << 8),
	IOMUXH_RXD0		= (2 << 6),
	IOMUXH_TXD0		= (2 << 4),
	IOMUXH_nRTS0	= (2 << 2),
	IOMUXH_nCTS0	= 2,
	IOMUXH_nCTS1	= (3 << 14),
	IOMUXH_nRTS1	= (3 << 12),

	/* PORT J */
	IOMUXJ_CAMRESET		= (2 << 24),
	IOMUXJ_CAMCLKOUT	= (2 << 22),
	IOMUXJ_CAMHREF		= (2 << 20),
	IOMUXJ_CAMVSYNC		= (2 << 18),
	IOMUXJ_CAMPCLK		= (2 << 16),
	IOMUXJ_CAMDATA7		= (2 << 14),
	IOMUXJ_CAMDATA6		= (2 << 12),
	IOMUXJ_CAMDATA5		= (2 << 10),
	IOMUXJ_CAMDATA4		= (2 << 8),
	IOMUXJ_CAMDATA3		= (2 << 6),
	IOMUXJ_CAMDATA2		= (2 << 4),
	IOMUXJ_CAMDATA1		= (2 << 2),
	IOMUXJ_CAMDATA0		= 2
};

#endif
