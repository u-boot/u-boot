// SPDX-License-Identifier: GPL-2.0-only

#include <common.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include "pll.h"

#define aor(v, a, o) (((v) & (a)) | (o))
#define DRV_Reg32(x)           (*(volatile unsigned int *)(x))
#define DRV_WriteReg32(x, y)    (*(volatile unsigned int *)(x) = (unsigned int)(y))

/* Manual PLL and clock initialization code for mt6735. Temporary, until mainline code is ported */

void mt_pll_init(void)
{
    unsigned int temp;
    
    DRV_WriteReg32(ACLKEN_DIV, 0x12);
    DRV_WriteReg32(CLKSQ_STB_CON0, 0x98940501);
    DRV_WriteReg32(PLL_ISO_CON0, 0x00080008);
    DRV_WriteReg32(AP_PLL_CON6, 0x00000000);
    
    temp = DRV_Reg32(ARMPLL_PWR_CON0);
    DRV_WriteReg32(ARMPLL_PWR_CON0, temp | 0x1);
    
    temp = DRV_Reg32(MAINPLL_PWR_CON0);
    DRV_WriteReg32(MAINPLL_PWR_CON0, temp | 0x1);
    
    temp = DRV_Reg32(UNIVPLL_PWR_CON0);
    DRV_WriteReg32(UNIVPLL_PWR_CON0, temp | 0x1);
    
    temp = DRV_Reg32(MMPLL_PWR_CON0);
    DRV_WriteReg32(MMPLL_PWR_CON0, temp | 0x1);
    
    temp = DRV_Reg32(MSDCPLL_PWR_CON0);
    DRV_WriteReg32(MSDCPLL_PWR_CON0, temp | 0x1);
    
    temp = DRV_Reg32(VENCPLL_PWR_CON0);
    DRV_WriteReg32(VENCPLL_PWR_CON0, temp | 0x1);
    
    temp = DRV_Reg32(TVDPLL_PWR_CON0);
    DRV_WriteReg32(TVDPLL_PWR_CON0, temp | 0x1);
    
    temp = DRV_Reg32(APLL1_PWR_CON0);
    DRV_WriteReg32(APLL1_PWR_CON0, temp | 0x1);
    
    temp = DRV_Reg32(APLL2_PWR_CON0);
    DRV_WriteReg32(APLL2_PWR_CON0, temp | 0x1);
    
    udelay(5);
    
    temp = DRV_Reg32(ARMPLL_PWR_CON0);
    DRV_WriteReg32(ARMPLL_PWR_CON0, temp & 0xFFFFFFFD);
    
    temp = DRV_Reg32(MAINPLL_PWR_CON0);
    DRV_WriteReg32(MAINPLL_PWR_CON0, temp & 0xFFFFFFFD);
    
    temp = DRV_Reg32(UNIVPLL_PWR_CON0);
    DRV_WriteReg32(UNIVPLL_PWR_CON0, temp & 0xFFFFFFFD);
    
    temp = DRV_Reg32(MMPLL_PWR_CON0);
    DRV_WriteReg32(MMPLL_PWR_CON0, temp & 0xFFFFFFFD);
    
    temp = DRV_Reg32(MSDCPLL_PWR_CON0);
    DRV_WriteReg32(MSDCPLL_PWR_CON0, temp & 0xFFFFFFFD);
    
    temp = DRV_Reg32(VENCPLL_PWR_CON0);
    DRV_WriteReg32(VENCPLL_PWR_CON0, temp & 0xFFFFFFFD);
    
    temp = DRV_Reg32(TVDPLL_PWR_CON0);
    DRV_WriteReg32(TVDPLL_PWR_CON0, temp & 0xFFFFFFFD);
    
    temp = DRV_Reg32(APLL1_PWR_CON0);
    DRV_WriteReg32(APLL1_PWR_CON0, temp & 0xFFFFFFFD);
    
    temp = DRV_Reg32(APLL2_PWR_CON0);
    DRV_WriteReg32(APLL2_PWR_CON0, temp & 0xFFFFFFFD);
    
    DRV_WriteReg32(ARMPLL_CON1, 0x810FC000);
    DRV_WriteReg32(MAINPLL_CON1, 0x800A8000);
    DRV_WriteReg32(UNIVPLL_CON1, 0x81180000);
    DRV_WriteReg32(MMPLL_CON1, 0x82114EC4);
    DRV_WriteReg32(MSDCPLL_CON1, 0x810F6276);
    DRV_WriteReg32(VENCPLL_CON1, 0x831713B1);
    DRV_WriteReg32(TVDPLL_CON1, 0x8316D89D);
    DRV_WriteReg32(APLL2_CON1, 0xB7945EA6);
    
    temp = DRV_Reg32(ARMPLL_CON0);
    DRV_WriteReg32(ARMPLL_CON0, temp | 0x1);
    
    temp = DRV_Reg32(MAINPLL_CON0);
    DRV_WriteReg32(MAINPLL_CON0, temp | 0x1);
    
    temp = DRV_Reg32(UNIVPLL_CON0);
    DRV_WriteReg32(UNIVPLL_CON0, temp | 0x1);
    
    temp = DRV_Reg32(MMPLL_CON0);
    DRV_WriteReg32(MMPLL_CON0, temp | 0x1);
    
    temp = DRV_Reg32(MSDCPLL_CON0);
    DRV_WriteReg32(MSDCPLL_CON0, temp | 0x1);
    
    temp = DRV_Reg32(VENCPLL_CON0);
    DRV_WriteReg32(VENCPLL_CON0, temp | 0x1);
    
    temp = DRV_Reg32(TVDPLL_CON0);
    DRV_WriteReg32(TVDPLL_CON0, temp | 0x1);
    
    temp = DRV_Reg32(APLL1_CON0);
    DRV_WriteReg32(APLL1_CON0, temp | 0x1);
    
    temp = DRV_Reg32(APLL2_CON0);
    DRV_WriteReg32(APLL2_CON0, temp | 0x1);
    
    udelay(40);
    
    temp = DRV_Reg32(MAINPLL_CON0);
    DRV_WriteReg32(MAINPLL_CON0, temp | 0x01000000);
    
    temp = DRV_Reg32(UNIVPLL_CON0);
    DRV_WriteReg32(UNIVPLL_CON0, temp | 0x01000000);
    
    temp = DRV_Reg32(TOP_DCMCTL);
    DRV_WriteReg32(TOP_DCMCTL, temp | 0x1); // Enable INFRA Bus Divider
    
    DRV_WriteReg32(INFRA_GLOBALCON_DCMDBC,
                   aor(DRV_Reg32(INFRA_GLOBALCON_DCMDBC),
                       ~INFRA_GLOBALCON_DCMDBC_MASK, INFRA_GLOBALCON_DCMDBC_ON));
    DRV_WriteReg32(INFRA_GLOBALCON_DCMFSEL,
                   aor(DRV_Reg32(INFRA_GLOBALCON_DCMFSEL),
                       ~INFRA_GLOBALCON_DCMFSEL_MASK, INFRA_GLOBALCON_DCMFSEL_ON));
    DRV_WriteReg32(INFRA_GLOBALCON_DCMCTL,
                   aor(DRV_Reg32(INFRA_GLOBALCON_DCMCTL),
                       ~INFRA_GLOBALCON_DCMCTL_MASK, INFRA_GLOBALCON_DCMCTL_ON));
    
    temp = DRV_Reg32(TOP_CKDIV1);
    DRV_WriteReg32(TOP_CKDIV1, temp & 0xFFFFFFE0);
    
    temp = DRV_Reg32(TOP_CKMUXSEL);
    DRV_WriteReg32(TOP_CKMUXSEL, temp | 0x4);
    
    DRV_WriteReg32(CLK_CFG_0, 0x01000002);
    DRV_WriteReg32(CLK_CFG_1, 0x01010180);
    DRV_WriteReg32(CLK_CFG_2, 0x01010100);
    DRV_WriteReg32(CLK_CFG_3, 0x07020202);
    DRV_WriteReg32(CLK_CFG_4, 0x01000100);
    DRV_WriteReg32(CLK_CFG_5, 0x01010101);
    DRV_WriteReg32(CLK_CFG_6, 0x01010001);
    DRV_WriteReg32(CLK_CFG_7, 0x00000000);
    DRV_WriteReg32(CLK_SCP_CFG_0, 0x3FF);
    DRV_WriteReg32(CLK_SCP_CFG_1, 0x11);
    DRV_WriteReg32(INFRA_PDN_CLR0, 0xFFFFFFFF);
    DRV_WriteReg32(PERI_PDN_CLR0, 0xFFFFFFFF);
    DRV_WriteReg32(CLK_CFG_0, 0x01000102);
    
    temp = DRV_Reg32(AP_PLL_CON3);
    DRV_WriteReg32(AP_PLL_CON3, temp & 0xFFF44440); // Only UNIVPLL SW Control

    //step 49
    temp = DRV_Reg32(AP_PLL_CON4);
    DRV_WriteReg32(AP_PLL_CON4, temp & 0xFFFFFFF4); // Only UNIVPLL SW Control
    
}
