#include "dm/device.h"
#include <bootstage.h>
#include <dm.h>
#include <init.h>
#include <log.h>
#include <asm/global_data.h>
#include <dm/ofnode.h>
#include <mapmem.h>
#include <dt-structs.h>
#include <timer.h>
#include <asm/io.h>

struct sunxi_timer {
    uint32_t irq_en;
    uint32_t irq_status;
    uint32_t res0[3];
    uint32_t timer0_ctrl;
    uint32_t timer0_intv_val;
    uint32_t timer0_curr_val;
    uint32_t res1[2];
    uint32_t timer1_ctrl;
    uint32_t timer1_intv_val;
    uint32_t timer1_curr_val;
};

enum pr_scale {
    SCALE_1,
    SCALE_2,
    SCALE_4,
    SCALE_8,
    SCALE_16,
    SCALE_32,
    SCALE_64,
    SCALE_128,
};

enum {
    SINGLE_MODE,
    CONTINUE_MODE,
};

enum {
    LOSC,
    OSC24M,
};

#define TIMER_MODE(_x)  (_x << 7)
#define CLK_SRC(_x)     (_x << 2)
#define CLK_PRES(_x)    (_x << 4)
#define RELOAD          (1 << 1)
#define TIMER_EN(_x)    (_x << 0)


struct sunxi_timer_priv {
    struct sunxi_timer *timer;
};

DECLARE_GLOBAL_DATA_PTR;

static int sunxi_timer_start(struct udevice *dev)
{
    struct sunxi_timer_priv *priv = dev_get_priv(dev);
    struct sunxi_timer *timer = priv->timer;
    uint32_t val = TIMER_MODE(CONTINUE_MODE) | CLK_SRC(OSC24M) | CLK_PRES(SCALE_1);

    writel(~0u, &timer->timer0_intv_val);
    writel(val, &timer->timer0_ctrl);

    val = readl(&timer->timer0_ctrl);
    val |= RELOAD;

    writel(val, &timer->timer0_ctrl);
    val = readl(&timer->timer0_ctrl);

    val |= TIMER_EN(1);

    writel(val, &timer->timer0_ctrl);

    return 0;
}

static u64 sunxi_timer_get_count(struct udevice *dev)
{
    struct sunxi_timer_priv *priv = dev_get_priv(dev);
    struct sunxi_timer *timer = priv->timer;
    uint32_t val = readl(&timer->timer0_curr_val);

    return ~0uLL - val;
}

static int sunxi_timer_probe(struct udevice *dev)
{
    return sunxi_timer_start(dev);
}

static int sunxi_clk_of_to_plat(struct udevice *dev)
{
    if(CONFIG_IS_ENABLED(OF_REAL)) {
        struct sunxi_timer_priv *priv = dev_get_priv(dev);
        priv->timer = dev_read_addr_ptr(dev);
        if (!priv->timer)
            return -ENOENT;
    }
    return 0;
}

static const struct timer_ops sunxi_timer_ops = {
    .get_count = sunxi_timer_get_count,
};

static const struct udevice_id allwinner_sunxi_timer_ids[] = {
    {.compatible = "allwinner,sun50i-a133-timer"},
    {}
};

U_BOOT_DRIVER(allwinner_sunxi_timer) = {
    .name = "allwinner_sunxi_timer",
    .id = UCLASS_TIMER,
    .of_match = allwinner_sunxi_timer_ids,
    .probe = sunxi_timer_probe,
    .ops = &sunxi_timer_ops,
    .priv_auto = sizeof(struct sunxi_timer_priv),
    .of_to_plat = sunxi_clk_of_to_plat,
};