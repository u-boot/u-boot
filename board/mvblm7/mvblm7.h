#ifndef __MVBC_H__
#define __MVBC_H__

#define MV_GPIO

#define FPGA_CONFIG     0x80000000
#define FPGA_CCLK       0x40000000
#define FPGA_DIN        0x20000000
#define FPGA_STATUS     0x10000000
#define FPGA_CONF_DONE  0x08000000
#define MMC_CS		0x04000000

#define WD_WDI          0x00400000
#define WD_TS           0x00200000
#define MAN_RST         0x00100000

#define MV_GPIO_DAT	(WD_TS)
#define MV_GPIO_OUT	(FPGA_CONFIG|FPGA_DIN|FPGA_CCLK|WD_TS|WD_WDI|MMC_CS)
#define MV_GPIO_ODE	(FPGA_CONFIG|MAN_RST)

#endif
