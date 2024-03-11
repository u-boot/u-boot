// SPDX-License-Identifier: GPL-2.0

#ifndef _DT_BINDINGS_CLK_MT6735_H
#define _DT_BINDINGS_CLK_MT6735_H

/* TOPCKGEN */
#define CLK_TOP_UNIVPLL_D2      0
#define CLK_TOP_UNIVPLL_D3      1
#define CLK_TOP_UNIVPLL1_D2	    2
#define CLK_TOP_AD_APLL1_CK		3
#define CLK_TOP_AD_SYS_26M_CK   4
#define CLK_TOP_AD_SYS_26M_D2   5
#define CLK_TOP_DMPLL_CK		6
#define CLK_TOP_DMPLL_D2		7
#define CLK_TOP_DMPLL_D4		8
#define CLK_TOP_DMPLL_D8		9
#define CLK_TOP_MMPLL_CK		10
#define CLK_TOP_MSDCPLL_CK		11
#define CLK_TOP_MSDCPLL_D16		12
#define CLK_TOP_MSDCPLL_D2		13
#define CLK_TOP_MSDCPLL_D4		14
#define CLK_TOP_MSDCPLL_D8		15
#define CLK_TOP_SYSPLL_D2		16
#define CLK_TOP_SYSPLL_D3		17
#define CLK_TOP_SYSPLL_D5		18
#define CLK_TOP_SYSPLL1_D16		19
#define CLK_TOP_SYSPLL1_D2		20
#define CLK_TOP_SYSPLL1_D4		21
#define CLK_TOP_SYSPLL1_D8		22
#define CLK_TOP_SYSPLL2_D2		23
#define CLK_TOP_SYSPLL2_D4		24
#define CLK_TOP_SYSPLL3_D2		25
#define CLK_TOP_SYSPLL3_D4		26
#define CLK_TOP_SYSPLL4_D2		27
#define CLK_TOP_SYSPLL4_D2_D8	28
#define CLK_TOP_SYSPLL4_D4		29
#define CLK_TOP_TVDPLL_CK		30
#define CLK_TOP_TVDPLL_D2		31
#define CLK_TOP_TVDPLL_D4		32
#define CLK_TOP_UNIVPLL_D26		33
#define CLK_TOP_UNIVPLL_D5		34
#define CLK_TOP_UNIVPLL1_D4		35
#define CLK_TOP_UNIVPLL1_D8		36
#define CLK_TOP_UNIVPLL2_D2		37
#define CLK_TOP_UNIVPLL2_D4		38
#define CLK_TOP_UNIVPLL2_D8		39
#define CLK_TOP_UNIVPLL3_D2		40
#define CLK_TOP_UNIVPLL3_D4		41
#define CLK_TOP_USB_PHY48M      42
#define CLK_TOP_VENCPLL_CK		 43
#define CLK_TOP_VENCPLL_D3		 44
#define CLK_TOP_WHPLL_AUDIO_CK	 45
#define CLK_TOP_CLKPH_MCK_O		 46
#define CLK_TOP_DPI_CK			 47
#define CLK_TOP_MUX_MM			 48
#define CLK_TOP_MUX_DDRPHY		 49
#define CLK_TOP_MUX_MEM			 50
#define CLK_TOP_MUX_AXI			 51
#define CLK_TOP_MUX_CAMTG		 52
#define CLK_TOP_MUX_MFG			 53
#define CLK_TOP_MUX_VDEC		 54
#define CLK_TOP_MUX_PWM			 55
#define CLK_TOP_MUX_MSDC50_0	 56
#define CLK_TOP_MUX_USB20		 57
#define CLK_TOP_MUX_SPI			 58
#define CLK_TOP_MUX_UART		 59
#define CLK_TOP_MUX_MSDC30_3	 60
#define CLK_TOP_MUX_MSDC30_2	 61
#define CLK_TOP_MUX_MSDC30_1	 62
#define CLK_TOP_MUX_MSDC30_0	 63
#define CLK_TOP_MUX_SCP			 64
#define CLK_TOP_MUX_PMICSPI		 65
#define CLK_TOP_MUX_AUDINTBUS	 66
#define CLK_TOP_MUX_AUDIO		 67
#define CLK_TOP_MUX_MFG13M		 68
#define CLK_TOP_MUX_SCAM		 69
#define CLK_TOP_MUX_DPI0		 70
#define CLK_TOP_MUX_ATB			 71
#define CLK_TOP_MUX_IRTX		 72
#define CLK_TOP_MUX_IRDA		 73
#define CLK_TOP_MUX_AUD2		 74
#define CLK_TOP_MUX_AUD1		 75
#define CLK_TOP_MUX_DISPPWM		 76
#define CLK_TOP_NR_CLK			 77


/* CLK_APMIXED_SYS */
#define CLK_APMIXED_ARMPLL	0
#define CLK_APMIXED_MAINPLL	1
#define CLK_APMIXED_MSDCPLL	2
#define CLK_APMIXED_UNIVPLL	3
#define CLK_APMIXED_MMPLL	4
#define CLK_APMIXED_VENCPLL	5
#define CLK_APMIXED_TVDPLL	6
#define CLK_APMIXED_APLL1	7
#define CLK_APMIXED_APLL2	8
#define CLK_APMIXED_NR_CLK	9

/* CLK_INFRA_SYS, infrasys */
#define CLK_INFRA_DBGCLK        0
#define CLK_INFRA_GCE           1
#define CLK_INFRA_TRBG          2
#define CLK_INFRA_CPUM          3
#define CLK_INFRA_DEVAPC        4
#define CLK_INFRA_AUDIO         5
#define CLK_INFRA_GCPU          6
#define CLK_INFRA_L2C_SRAM      7
#define CLK_INFRA_M4U           8
#define CLK_INFRA_CLDMA         9
#define CLK_INFRA_CONNMCU_BUS   10
#define CLK_INFRA_KP            11
#define CLK_INFRA_APXGPT        12
#define CLK_INFRA_SEJ           13
#define CLK_INFRA_CCIF0_AP      14
#define CLK_INFRA_CCIF1_AP      15
#define CLK_INFRA_PMIC_SPI      16
#define CLK_INFRA_PMIC_WRAP     17
#define CLK_INFRA_NR_CLK        18

/* CLK_PERI_SYS, perisys */
#define CLK_PERI_DISP_PWM       0
#define CLK_PERI_THERM          1
#define CLK_PERI_PWM1           2
#define CLK_PERI_PWM2           3
#define CLK_PERI_PWM3           4
#define CLK_PERI_PWM4           5
#define CLK_PERI_PWM5           6
#define CLK_PERI_PWM6           7
#define CLK_PERI_PWM7           8
#define CLK_PERI_PWM            9
#define CLK_PERI_USB0           10
#define CLK_PERI_IRDA           11
#define CLK_PERI_APDMA          12
#define CLK_PERI_MSDC30_0       13
#define CLK_PERI_MSDC30_1       14
#define CLK_PERI_MSDC30_2       15
#define CLK_PERI_MSDC30_3       16
#define CLK_PERI_UART0          17
#define CLK_PERI_UART1          18
#define CLK_PERI_UART2          19
#define CLK_PERI_UART3          20
#define CLK_PERI_UART4          21
#define CLK_PERI_BTIF           22
#define CLK_PERI_I2C0           23
#define CLK_PERI_I2C1           24
#define CLK_PERI_I2C2           25
#define CLK_PERI_I2C3           26
#define CLK_PERI_AUXADC         27
#define CLK_PERI_SPI0           28
#define CLK_PERI_IRTX           29
#define CLK_PERI_NR_CLK         30

/* MFG_SYS, mfgsys */
#define CLK_MFG_BG3D	0
#define CLK_MFG_NR_CLK	1

/* CLK_IMG_SYS, imgsys */
#define CLK_IMG_IMAGE_LARB2_SMI 0
#define CLK_IMG_IMAGE_CAM_SMI	1
#define CLK_IMG_IMAGE_CAM_CAM	2
#define CLK_IMG_IMAGE_SEN_TG	3
#define CLK_IMG_IMAGE_SEN_CAM	4
#define CLK_IMG_IMAGE_CAM_SV	5
#define CLK_IMG_IMAGE_SUFOD     6
#define CLK_IMG_IMAGE_FD        7
#define CLK_IMG_NR_CLK          8

/* CLK_MM_SYS, mmsys */
#define CLK_MM_DISP0_SMI_COMMON     0
#define CLK_MM_DISP0_SMI_LARB0      1
#define CLK_MM_DISP0_CAM_MDP        2
#define CLK_MM_DISP0_MDP_RDMA       3
#define CLK_MM_DISP0_MDP_RSZ0       4
#define CLK_MM_DISP0_MDP_RSZ1       5
#define CLK_MM_DISP0_MDP_TDSHP      6
#define CLK_MM_DISP0_MDP_WDMA       7
#define CLK_MM_DISP0_MDP_WROT       8
#define CLK_MM_DISP0_FAKE_ENG       9
#define CLK_MM_DISP0_DISP_OVL0      10
#define CLK_MM_DISP0_DISP_RDMA0     11
#define CLK_MM_DISP0_DISP_RDMA1     12
#define CLK_MM_DISP0_DISP_WDMA0     13
#define CLK_MM_DISP0_DISP_COLOR     14
#define CLK_MM_DISP0_DISP_CCORR     15
#define CLK_MM_DISP0_DISP_AAL       16
#define CLK_MM_DISP0_DISP_GAMMA     17
#define CLK_MM_DISP0_DISP_DITHER    18
#define CLK_MM_DISP1_DSI_ENGINE     19
#define CLK_MM_DISP1_DSI_DIGITAL    20
#define CLK_MM_DISP1_DPI_ENGINE     21
#define CLK_MM_DISP1_DPI_PIXEL      22
#define CLK_MM_NR_CLK               23

/* VDEC_SYS, vdecsys */
#define CLK_VDEC0_VDEC		0
#define CLK_VDEC1_LARB		1
#define CLK_VDEC_NR_CLK		2

/* VENC_SYS, vencsys */
#define CLK_VENC_LARB	0
#define CLK_VENC_VENC	1
#define CLK_VENC_JPGENC	2
#define CLK_VENC_JPGDEC	3
#define CLK_VENC_NR_CLK	4

/* AUDIO_SYS, audiosys */
#define CLK_AUDIO_AFE           0
#define CLK_AUDIO_I2S           1
#define CLK_AUDIO_22M           2
#define CLK_AUDIO_24M           3
#define CLK_AUDIO_APLL2_TUNER	4
#define CLK_AUDIO_APLL_TUNER	5
#define CLK_AUDIO_ADC           6
#define CLK_AUDIO_DAC           7
#define CLK_AUDIO_DAC_PREDIS	8
#define CLK_AUDIO_TML           9
#define CLK_AUDIO_NR_CLK        10
#endif /* _DT_BINDINGS_CLK_MT6735_H */
