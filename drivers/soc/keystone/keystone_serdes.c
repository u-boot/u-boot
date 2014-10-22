/*
 * TI serdes driver for keystone2.
 *
 * (C) Copyright 2014
 *     Texas Instruments Incorporated, <www.ti.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>

void ks2_serdes_sgmii_156p25mhz_setup(void)
{
	unsigned int cnt;

	/*
	 * configure Serializer/Deserializer (SerDes) hardware. SerDes IP
	 * hardware vendor published only register addresses and their values
	 * to be used for configuring SerDes. So had to use hardcoded values
	 * below.
	 */
	clrsetbits_le32(0x0232a000, 0xffff0000, 0x00800000);
	clrsetbits_le32(0x0232a014, 0x0000ffff, 0x00008282);
	clrsetbits_le32(0x0232a060, 0x00ffffff, 0x00142438);
	clrsetbits_le32(0x0232a064, 0x00ffff00, 0x00c3c700);
	clrsetbits_le32(0x0232a078, 0x0000ff00, 0x0000c000);

	clrsetbits_le32(0x0232a204, 0xff0000ff, 0x38000080);
	clrsetbits_le32(0x0232a208, 0x000000ff, 0x00000000);
	clrsetbits_le32(0x0232a20c, 0xff000000, 0x02000000);
	clrsetbits_le32(0x0232a210, 0xff000000, 0x1b000000);
	clrsetbits_le32(0x0232a214, 0x0000ffff, 0x00006fb8);
	clrsetbits_le32(0x0232a218, 0xffff00ff, 0x758000e4);
	clrsetbits_le32(0x0232a2ac, 0x0000ff00, 0x00004400);
	clrsetbits_le32(0x0232a22c, 0x00ffff00, 0x00200800);
	clrsetbits_le32(0x0232a280, 0x00ff00ff, 0x00820082);
	clrsetbits_le32(0x0232a284, 0xffffffff, 0x1d0f0385);

	clrsetbits_le32(0x0232a404, 0xff0000ff, 0x38000080);
	clrsetbits_le32(0x0232a408, 0x000000ff, 0x00000000);
	clrsetbits_le32(0x0232a40c, 0xff000000, 0x02000000);
	clrsetbits_le32(0x0232a410, 0xff000000, 0x1b000000);
	clrsetbits_le32(0x0232a414, 0x0000ffff, 0x00006fb8);
	clrsetbits_le32(0x0232a418, 0xffff00ff, 0x758000e4);
	clrsetbits_le32(0x0232a4ac, 0x0000ff00, 0x00004400);
	clrsetbits_le32(0x0232a42c, 0x00ffff00, 0x00200800);
	clrsetbits_le32(0x0232a480, 0x00ff00ff, 0x00820082);
	clrsetbits_le32(0x0232a484, 0xffffffff, 0x1d0f0385);

	clrsetbits_le32(0x0232a604, 0xff0000ff, 0x38000080);
	clrsetbits_le32(0x0232a608, 0x000000ff, 0x00000000);
	clrsetbits_le32(0x0232a60c, 0xff000000, 0x02000000);
	clrsetbits_le32(0x0232a610, 0xff000000, 0x1b000000);
	clrsetbits_le32(0x0232a614, 0x0000ffff, 0x00006fb8);
	clrsetbits_le32(0x0232a618, 0xffff00ff, 0x758000e4);
	clrsetbits_le32(0x0232a6ac, 0x0000ff00, 0x00004400);
	clrsetbits_le32(0x0232a62c, 0x00ffff00, 0x00200800);
	clrsetbits_le32(0x0232a680, 0x00ff00ff, 0x00820082);
	clrsetbits_le32(0x0232a684, 0xffffffff, 0x1d0f0385);

	clrsetbits_le32(0x0232a804, 0xff0000ff, 0x38000080);
	clrsetbits_le32(0x0232a808, 0x000000ff, 0x00000000);
	clrsetbits_le32(0x0232a80c, 0xff000000, 0x02000000);
	clrsetbits_le32(0x0232a810, 0xff000000, 0x1b000000);
	clrsetbits_le32(0x0232a814, 0x0000ffff, 0x00006fb8);
	clrsetbits_le32(0x0232a818, 0xffff00ff, 0x758000e4);
	clrsetbits_le32(0x0232a8ac, 0x0000ff00, 0x00004400);
	clrsetbits_le32(0x0232a82c, 0x00ffff00, 0x00200800);
	clrsetbits_le32(0x0232a880, 0x00ff00ff, 0x00820082);
	clrsetbits_le32(0x0232a884, 0xffffffff, 0x1d0f0385);

	clrsetbits_le32(0x0232aa00, 0x0000ff00, 0x00000800);
	clrsetbits_le32(0x0232aa08, 0xffff0000, 0x38a20000);
	clrsetbits_le32(0x0232aa30, 0x00ffff00, 0x008a8a00);
	clrsetbits_le32(0x0232aa84, 0x0000ff00, 0x00000600);
	clrsetbits_le32(0x0232aa94, 0xff000000, 0x10000000);
	clrsetbits_le32(0x0232aaa0, 0xff000000, 0x81000000);
	clrsetbits_le32(0x0232aabc, 0xff000000, 0xff000000);
	clrsetbits_le32(0x0232aac0, 0x000000ff, 0x0000008b);
	clrsetbits_le32(0x0232ab08, 0xffff0000, 0x583f0000);
	clrsetbits_le32(0x0232ab0c, 0x000000ff, 0x0000004e);
	clrsetbits_le32(0x0232a000, 0x000000ff, 0x00000003);
	clrsetbits_le32(0x0232aa00, 0x000000ff, 0x0000005f);

	clrsetbits_le32(0x0232aa48, 0x00ffff00, 0x00fd8c00);
	clrsetbits_le32(0x0232aa54, 0x00ffffff, 0x002fec72);
	clrsetbits_le32(0x0232aa58, 0xffffff00, 0x00f92100);
	clrsetbits_le32(0x0232aa5c, 0xffffffff, 0x00040060);
	clrsetbits_le32(0x0232aa60, 0xffffffff, 0x00008000);
	clrsetbits_le32(0x0232aa64, 0xffffffff, 0x0c581220);
	clrsetbits_le32(0x0232aa68, 0xffffffff, 0xe13b0602);
	clrsetbits_le32(0x0232aa6c, 0xffffffff, 0xb8074cc1);
	clrsetbits_le32(0x0232aa70, 0xffffffff, 0x3f02e989);
	clrsetbits_le32(0x0232aa74, 0x000000ff, 0x00000001);
	clrsetbits_le32(0x0232ab20, 0x00ff0000, 0x00370000);
	clrsetbits_le32(0x0232ab1c, 0xff000000, 0x37000000);
	clrsetbits_le32(0x0232ab20, 0x000000ff, 0x0000005d);

	/*Bring SerDes out of Reset if SerDes is Shutdown & is in Reset Mode*/
	clrbits_le32(0x0232a010, 1 << 28);

	/* Enable TX and RX via the LANExCTL_STS 0x0000 + x*4 */
	clrbits_le32(0x0232a228, 1 << 29);
	writel(0xF800F8C0, 0x0232bfe0);
	clrbits_le32(0x0232a428, 1 << 29);
	writel(0xF800F8C0, 0x0232bfe4);
	clrbits_le32(0x0232a628, 1 << 29);
	writel(0xF800F8C0, 0x0232bfe8);
	clrbits_le32(0x0232a828, 1 << 29);
	writel(0xF800F8C0, 0x0232bfec);

	/*Enable pll via the pll_ctrl 0x0014*/
	writel(0xe0000000, 0x0232bff4)
		;

	/*Waiting for SGMII Serdes PLL lock.*/
	for (cnt = 10000; cnt > 0 && ((readl(0x02090114) & 0x10) == 0); cnt--)
		;
	for (cnt = 10000; cnt > 0 && ((readl(0x02090214) & 0x10) == 0); cnt--)
		;
	for (cnt = 10000; cnt > 0 && ((readl(0x02090414) & 0x10) == 0); cnt--)
		;
	for (cnt = 10000; cnt > 0 && ((readl(0x02090514) & 0x10) == 0); cnt--)
		;

	udelay(45000);
}
