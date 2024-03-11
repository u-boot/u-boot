// SPDX-License-Identifier: GPL-2.0-only

#include <clk.h>
#include <time.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <dm.h>
#include <dm/devres.h>
#include <regmap.h>
#include <dm/device_compat.h>
#include "mt6735_pwrap_hal.h"
#include <pwrap/pwrap.h>
#include <reset.h>

u32 pmic_readl(u32 reg)
{
	void __iomem *base;
	base = ioremap(PWRAP_BASE, 0x1000);
	return readl(base + reg);
}

void pmic_writel(u32 val, u32 reg)
{
	void __iomem *base;
	base = ioremap(PWRAP_BASE, 0x1000);
	writel(val, base + reg);
}

static u32 pwrap_get_fsm_state(struct udevice *dev)
{

	u32 val;

	val = pmic_readl(PWRAP_WACS2_RDATA);
    return PWRAP_GET_WACS_FSM(val);
}


static bool pwrap_is_fsm_vldclr(struct udevice *dev)
{

	return pwrap_get_fsm_state(dev) == PWRAP_WACS_FSM_WFVLDCLR;
}

static bool pwrap_is_fsm_idle(struct udevice *dev)
{
	u32 val = pmic_readl(PWRAP_WACS2_RDATA);

	return PWRAP_GET_WACS_FSM(val) == PWRAP_WACS_FSM_IDLE;
}

static inline void pwrap_leave_fsm_vldclr(struct udevice *dev)
{

	if (pwrap_is_fsm_vldclr(dev))
		pmic_writel(1, PWRAP_WACS2_VLDCLR);
}

static bool pwrap_is_sync_idle(struct udevice *dev)
{

	return pmic_readl(PWRAP_WACS2_RDATA) & PWRAP_STATE_SYNC_IDLE0;
}

static int pwrap_wait_for_state(struct udevice *dev, bool (*fp)(struct udevice *))
{
	unsigned long timeout;

	timeout = timer_get_us() + 10000;

	do {
		if (time_after(timer_get_us(), timeout))
			return fp(dev) ? 0 : -ETIMEDOUT;
		if (fp(dev))
			return 0;
	} while (1);
}


int pwrap_read(struct udevice *dev, u32 adr, u32 *rdata)
{
	int ret;
	ret = pwrap_wait_for_state(dev, pwrap_is_fsm_idle);
	if (ret) {
		pwrap_leave_fsm_vldclr(dev);
		return ret;
	}

	pmic_writel((adr >> 1) << 16, PWRAP_WACS2_CMD);

	ret = pwrap_wait_for_state(dev, pwrap_is_fsm_vldclr);
	if (ret)
		return ret;

	*rdata = PWRAP_GET_WACS_RDATA(pmic_readl(PWRAP_WACS2_RDATA));
	pmic_writel(1, PWRAP_WACS2_VLDCLR);
	return 0;
}

int pwrap_write(struct udevice *dev, u32 adr, u32 wdata)
{
	int ret;
	ret = pwrap_wait_for_state(dev, pwrap_is_fsm_idle);
	if (ret) {
		pwrap_leave_fsm_vldclr(dev);
		return ret;
	}
    pmic_writel(BIT(31) | ((adr >> 1) << 16) | wdata,
			     PWRAP_WACS2_CMD);

    return 0;
}

static int pwrap_reset_spislave(struct udevice *dev)
{
	int ret, i;

	pmic_writel(0, PWRAP_HIPRIO_ARB_EN);
	pmic_writel(0, PWRAP_WRAP_EN);
	pmic_writel(1, PWRAP_MUX_SEL);
	pmic_writel(1, PWRAP_MAN_EN);
	pmic_writel(0, PWRAP_DIO_EN);

	pmic_writel((1 << 13) | PWRAP_MAN_CMD_OP_CSL,
			PWRAP_MAN_CMD);
	pmic_writel((1 << 13) | PWRAP_MAN_CMD_OP_OUTS,
			PWRAP_MAN_CMD);
	pmic_writel((1 << 13) | PWRAP_MAN_CMD_OP_CSH,
			PWRAP_MAN_CMD);

	for (i = 0; i < 4; i++)
		pmic_writel((1 << 13) | PWRAP_MAN_CMD_OP_OUTS,
				PWRAP_MAN_CMD);

	ret = pwrap_wait_for_state(dev, pwrap_is_sync_idle);
	if (ret) {
		pr_err("%s fail, ret=%d\n", __func__, ret);
        return ret;
	}

	pmic_writel(0, PWRAP_MAN_EN);
	pmic_writel(0, PWRAP_MUX_SEL);

	return 0;
}

int pwrap_init_sidly(struct udevice *dev)
{

	u32 rdata;
	u32 i;
	u32 pass = 0;
	signed char dly[16] = {
		-1, 0, 1, 0, 2, -1, 1, 1, 3, -1, -1, -1, 3, -1, 2, 1
	};

	for (i = 0; i < 4; i++) {
		pmic_writel(i, PWRAP_SIDLY);
		pwrap_read(dev, 0x02d6, &rdata);
		if (rdata == 0x5aa5) {
			pr_debug("[Read Test] pass, SIDLY=%x\n", i);
            pass |= 1 << i;
		}
	}

	if (dly[pass] < 0) {
		pr_err("sidly pass range 0x%x not continuous\n",
				pass);
		return -EIO;
	}

	pmic_writel(dly[pass], PWRAP_SIDLY);

	return 0;
}

u32 pmic_read_interface(struct udevice *dev, u32 RegNum, u32 *val, u32 MASK, u32 SHIFT)
{
    u32 return_value = 0;
    u32 pmic_reg = 0;
    u32 rdata;

    return_value = pwrap_read(dev, RegNum, &rdata);
    pmic_reg=rdata;
    if(return_value != 0)
    {
        printf("[pmic_read_interface] Reg[%x]= pmic_wrap read data fail\n", RegNum);
        return return_value;
    }
    
    pmic_reg &= (MASK << SHIFT);
    *val = (pmic_reg >> SHIFT);

    return return_value;
}

u32 pmic_config_interface(struct udevice *dev, u32 RegNum, u32 val, u32 MASK, u32 SHIFT)
{
    u32 return_value = 0;
    u32 pmic_reg = 0;
    u32 rdata;

    return_value = pwrap_read(dev, RegNum, &rdata);
    pmic_reg = rdata;
    if(return_value != 0)
    {
        return return_value;
    }

    pmic_reg &= ~(MASK << SHIFT);
    pmic_reg |= (val << SHIFT);

    return_value= pwrap_write(dev, RegNum, pmic_reg);
    if(return_value != 0)
    {
        return return_value;
    }

    return return_value;
}

unsigned int pmic_read_efuse(struct udevice *dev, unsigned int addr)
{
	printf("pmic_6328_efuse_read started\n");
	unsigned int ret,reg_val;

	pmic_config_interface(dev, 0x0c00, addr*2, 0x3f, 0);
	printf("pmic_config_interface(dev, 0x0c00, 0x%x, 0x3f, 0);\n",  addr*2);
	ret = pmic_read_interface(dev, 0x0c10, &reg_val, 0x1, 0);

	if(reg_val == 0)
	{
		pmic_config_interface(dev, 0x0c10, 1, 0x1, 0);
	}
	else
	{
		pmic_config_interface(dev, 0x0c10, 0, 0x1, 0);
	}
    
	do
	{
	ret = pmic_read_interface(dev, 0x0c1a, &reg_val, 0x1, 0);
	printf("Read register 0x0c1a = 0x%x\n", reg_val);
	}while(reg_val == 1);

	ret = pmic_read_interface(dev, 0x0c18, &reg_val, 0xffff, 0);

	return reg_val;

}

u32 efuse_data[0x20]={0};

void pmic_6328_efuse_management(struct udevice *dev)
{

    int is_efuse_trimed=0;
    u32 trim;
    
    pmic_config_interface(dev, 0x000a, 0x1, 0x1, 11);
    pmic_config_interface(dev, 0x000a, 0x0, 0x1, 10);
    
	pwrap_read(dev, 0xC5C, &trim);
    is_efuse_trimed = (((trim) >> 15) & 0x0001);

    if(is_efuse_trimed == 1)
    {
        
        pmic_config_interface(dev, 0x0278, 0x00, 0x1, 6);
        pmic_config_interface(dev, 0x024e, 0x00, 0x1, 2);
        pmic_config_interface(dev, 0x0c16, 0x01, 0x1, 0);
        
        pwrap_write(dev, 0x434, 0x3);
        pwrap_write(dev, 0x438, 0x13);
        pwrap_write(dev, 0x464, 0x8002);
        pwrap_write(dev, 0x46e, 0x404);
        pwrap_write(dev, 0x444, 0x108);
        pwrap_write(dev, 0x458, 0x410);
        pwrap_write(dev, 0x44e, 0x200);
        pwrap_write(dev, 0xa52, 0x8000);
        pwrap_write(dev, 0xa56, 0x8000);
        pwrap_write(dev, 0xa58, 0x8030);
        pwrap_write(dev, 0xa7c, 0x8000);
        pwrap_write(dev, 0xa7e, 0x8030);
        pwrap_write(dev, 0xa60, 0x8020);
        pwrap_write(dev, 0xa62, 0x8f20);
        pwrap_write(dev, 0xa66, 0x8100);
        pwrap_write(dev, 0xa64, 0x8110);
        pwrap_write(dev, 0xa72, 0x8050);
        pwrap_write(dev, 0xa84, 0x8030);
        pwrap_write(dev, 0xa7a, 0x8070);
        pwrap_write(dev, 0xa5c, 0x8400);
        pwrap_write(dev, 0xa6a, 0x8110);
        pwrap_write(dev, 0xa6c, 0x8150);
        pwrap_write(dev, 0x470, 0x430);
        pwrap_write(dev, 0x46c, 0x1022);
        pwrap_write(dev, 0x466, 0x2);
        pwrap_write(dev, 0x442, 0x1042);
        pwrap_write(dev, 0x45a, 0x430);
        pwrap_write(dev, 0x456, 0x1004);
        pwrap_write(dev, 0x450, 0x200);
        pwrap_write(dev, 0x44c, 0x41c);
    }
    
    pmic_config_interface(dev, 0x0278, 0x01, 0x1, 2);
    pmic_config_interface(dev, 0x024e, 0x01, 0x1, 6);
    pmic_config_interface(dev, 0x02a4, 0x2, 0xffff, 0);
    pmic_config_interface(dev, 0x02a2, 0x1, 0xffff, 0);
    pmic_config_interface(dev, 0x042e, 0x1, 0x1ff, 0);
    pmic_config_interface(dev, 0x0a44, 0x1, 0x1, 1);
    pmic_config_interface(dev, 0x04b4, 0x68, 0xffff, 0x0);
    pmic_config_interface(dev, 0x04b8, 0x68, 0xffff, 0x0);
    pmic_config_interface(dev, 0x0a88, 0x68, 0xffff, 0x0);
    pwrap_write(dev, 0x0612, 0x3021);
    pmic_config_interface(dev, 0x0616, 0x68, 0x7f, 0);
    pmic_config_interface(dev, 0x0618, 0x68, 0x7f, 0);
    pwrap_write(dev, 0x0a36, 0xc102);
    pwrap_write(dev, 0x0a46, 0xc102);
}

void mtk_pwrap_init(struct udevice *dev)
{
	writel(0x80, INFRA_GLOBALCON_RST0);
    writel(0x80, INFRA_GLOBALCON_RST1);
	writel(0x70000, 0x10000088);
	pmic_writel(3, PWRAP_DCM_EN);
	pmic_writel(0, PWRAP_DCM_DBC_PRD);
    pwrap_reset_spislave(dev);
	pmic_writel(1, PWRAP_WRAP_EN);
   	pmic_writel(0x3fff, PWRAP_HIPRIO_ARB_EN);
	pmic_writel(1, PWRAP_WACS2_EN);
    pmic_writel(0x88, PWRAP_RDDMY);
	pwrap_write(dev, 0x02ee, 0x8);
	pmic_writel(0x0, PWRAP_CSHEXT_READ);
    pmic_writel(0x33, PWRAP_CSHEXT_WRITE);
    pmic_writel(0x0, PWRAP_CSLEXT_START);
    pmic_writel(0x0, PWRAP_CSLEXT_END);
	pwrap_init_sidly(dev);
	pwrap_write(dev, 0x02d4, 1);
	pmic_writel(1, PWRAP_DIO_EN);
	pmic_writel(0x1, PWRAP_WACS0_EN);
	pmic_writel(0x1, PWRAP_WACS1_EN);
	pmic_writel(0x1, PWRAP_WACS2_EN);
	pmic_writel(0x5, PWRAP_STAUPD_PRD);
	pmic_writel(0xf, PWRAP_WDT_UNIT);
	pmic_writel(0xfffffbff, PWRAP_WDT_SRC_EN);
	pmic_writel(0x1, PWRAP_TIMER_EN);
	pmic_writel(0xfffffbfb, PWRAP_INT_EN);
	pmic_writel(1, PWRAP_INIT_DONE2);
	pmic_writel(1, PWRAP_INIT_DONE0);
	pmic_writel(1, PWRAP_INIT_DONE1);
	pmic_6328_efuse_management(dev);
}


