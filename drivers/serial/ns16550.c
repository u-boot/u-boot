/*
 * COM1 NS16550 support
 * originally from linux source (arch/powerpc/boot/ns16550.c)
 * modified to use CONFIG_SYS_ISA_MEM and new defines
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <ns16550.h>
#include <serial.h>
#include <watchdog.h>
#include <linux/types.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_LCRVAL UART_LCR_8N1		/* 8 data, 1 stop, no parity */
#define UART_MCRVAL (UART_MCR_DTR | \
		     UART_MCR_RTS)		/* RTS/DTR */
#define UART_FCRVAL (UART_FCR_FIFO_EN |	\
		     UART_FCR_RXSR |	\
		     UART_FCR_TXSR)		/* Clear & enable FIFOs */

#ifndef CONFIG_DM_SERIAL
#ifdef CONFIG_SYS_NS16550_PORT_MAPPED
#define serial_out(x, y)	outb(x, (ulong)y)
#define serial_in(y)		inb((ulong)y)
#elif defined(CONFIG_SYS_NS16550_MEM32) && (CONFIG_SYS_NS16550_REG_SIZE > 0)
#define serial_out(x, y)	out_be32(y, x)
#define serial_in(y)		in_be32(y)
#elif defined(CONFIG_SYS_NS16550_MEM32) && (CONFIG_SYS_NS16550_REG_SIZE < 0)
#define serial_out(x, y)	out_le32(y, x)
#define serial_in(y)		in_le32(y)
#else
#define serial_out(x, y)	writeb(x, y)
#define serial_in(y)		readb(y)
#endif
#endif /* !CONFIG_DM_SERIAL */

#if defined(CONFIG_SOC_KEYSTONE)
#define UART_REG_VAL_PWREMU_MGMT_UART_DISABLE   0
#define UART_REG_VAL_PWREMU_MGMT_UART_ENABLE ((1 << 14) | (1 << 13) | (1 << 0))
#undef UART_MCRVAL
#ifdef CONFIG_SERIAL_HW_FLOW_CONTROL
#define UART_MCRVAL             (UART_MCR_RTS | UART_MCR_AFE)
#else
#define UART_MCRVAL             (UART_MCR_RTS)
#endif
#endif

#ifndef CONFIG_SYS_NS16550_IER
#define CONFIG_SYS_NS16550_IER  0x00
#endif /* CONFIG_SYS_NS16550_IER */

#ifdef CONFIG_DM_SERIAL

#ifndef CONFIG_SYS_NS16550_CLK
#define CONFIG_SYS_NS16550_CLK  0
#endif

static inline void serial_out_shift(void *addr, int shift, int value)
{
#ifdef CONFIG_SYS_NS16550_PORT_MAPPED
	outb(value, (ulong)addr);
#elif defined(CONFIG_SYS_NS16550_MEM32) && !defined(CONFIG_SYS_BIG_ENDIAN)
	out_le32(addr, value);
#elif defined(CONFIG_SYS_NS16550_MEM32) && defined(CONFIG_SYS_BIG_ENDIAN)
	out_be32(addr, value);
#elif defined(CONFIG_SYS_NS16550_MEM32)
	writel(value, addr);
#elif defined(CONFIG_SYS_BIG_ENDIAN)
	writeb(value, addr + (1 << shift) - 1);
#else
	writeb(value, addr);
#endif
}

static inline int serial_in_shift(void *addr, int shift)
{
#ifdef CONFIG_SYS_NS16550_PORT_MAPPED
	return inb((ulong)addr);
#elif defined(CONFIG_SYS_NS16550_MEM32) && !defined(CONFIG_SYS_BIG_ENDIAN)
	return in_le32(addr);
#elif defined(CONFIG_SYS_NS16550_MEM32) && defined(CONFIG_SYS_BIG_ENDIAN)
	return in_be32(addr);
#elif defined(CONFIG_SYS_NS16550_MEM32)
	return readl(addr);
#elif defined(CONFIG_SYS_BIG_ENDIAN)
	return readb(addr + (1 << shift) - 1);
#else
	return readb(addr);
#endif
}

static void ns16550_writeb(NS16550_t port, int offset, int value)
{
	struct ns16550_platdata *plat = port->plat;
	unsigned char *addr;

	offset *= 1 << plat->reg_shift;
	addr = map_physmem(plat->base, 0, MAP_NOCACHE) + offset;
	/*
	 * As far as we know it doesn't make sense to support selection of
	 * these options at run-time, so use the existing CONFIG options.
	 */
	serial_out_shift(addr, plat->reg_shift, value);
}

static int ns16550_readb(NS16550_t port, int offset)
{
	struct ns16550_platdata *plat = port->plat;
	unsigned char *addr;

	offset *= 1 << plat->reg_shift;
	addr = map_physmem(plat->base, 0, MAP_NOCACHE) + offset;

	return serial_in_shift(addr, plat->reg_shift);
}

/* We can clean these up once everything is moved to driver model */
#define serial_out(value, addr)	\
	ns16550_writeb(com_port, \
		(unsigned char *)addr - (unsigned char *)com_port, value)
#define serial_in(addr) \
	ns16550_readb(com_port, \
		(unsigned char *)addr - (unsigned char *)com_port)
#endif

static inline int calc_divisor(NS16550_t port, int clock, int baudrate)
{
	const unsigned int mode_x_div = 16;

	return DIV_ROUND_CLOSEST(clock, mode_x_div * baudrate);
}

int ns16550_calc_divisor(NS16550_t port, int clock, int baudrate)
{
#ifdef CONFIG_OMAP1510
	/* If can't cleanly clock 115200 set div to 1 */
	if ((clock == 12000000) && (baudrate == 115200)) {
		port->osc_12m_sel = OSC_12M_SEL;  /* enable 6.5 * divisor */
		return 1;			/* return 1 for base divisor */
	}
	port->osc_12m_sel = 0;			/* clear if previsouly set */
#endif

	return calc_divisor(port, clock, baudrate);
}

static void NS16550_setbrg(NS16550_t com_port, int baud_divisor)
{
	serial_out(UART_LCR_BKSE | UART_LCRVAL, &com_port->lcr);
	serial_out(baud_divisor & 0xff, &com_port->dll);
	serial_out((baud_divisor >> 8) & 0xff, &com_port->dlm);
	serial_out(UART_LCRVAL, &com_port->lcr);
}

void NS16550_init(NS16550_t com_port, int baud_divisor)
{
#if (defined(CONFIG_SPL_BUILD) && \
		(defined(CONFIG_OMAP34XX) || defined(CONFIG_OMAP44XX)))
	/*
	 * On some OMAP3/OMAP4 devices when UART3 is configured for boot mode
	 * before SPL starts only THRE bit is set. We have to empty the
	 * transmitter before initialization starts.
	 */
	if ((serial_in(&com_port->lsr) & (UART_LSR_TEMT | UART_LSR_THRE))
	     == UART_LSR_THRE) {
		if (baud_divisor != -1)
			NS16550_setbrg(com_port, baud_divisor);
		serial_out(0, &com_port->mdr1);
	}
#endif

	while (!(serial_in(&com_port->lsr) & UART_LSR_TEMT))
		;

	serial_out(CONFIG_SYS_NS16550_IER, &com_port->ier);
#if defined(CONFIG_OMAP) || defined(CONFIG_AM33XX) || \
			defined(CONFIG_TI81XX) || defined(CONFIG_AM43XX)
	serial_out(0x7, &com_port->mdr1);	/* mode select reset TL16C750*/
#endif
	serial_out(UART_MCRVAL, &com_port->mcr);
	serial_out(UART_FCRVAL, &com_port->fcr);
	if (baud_divisor != -1)
		NS16550_setbrg(com_port, baud_divisor);
#if defined(CONFIG_OMAP) || \
	defined(CONFIG_AM33XX) || defined(CONFIG_SOC_DA8XX) || \
	defined(CONFIG_TI81XX) || defined(CONFIG_AM43XX)

	/* /16 is proper to hit 115200 with 48MHz */
	serial_out(0, &com_port->mdr1);
#endif /* CONFIG_OMAP */
#if defined(CONFIG_SOC_KEYSTONE)
	serial_out(UART_REG_VAL_PWREMU_MGMT_UART_ENABLE, &com_port->regC);
#endif
}

#ifndef CONFIG_NS16550_MIN_FUNCTIONS
void NS16550_reinit(NS16550_t com_port, int baud_divisor)
{
	serial_out(CONFIG_SYS_NS16550_IER, &com_port->ier);
	NS16550_setbrg(com_port, 0);
	serial_out(UART_MCRVAL, &com_port->mcr);
	serial_out(UART_FCRVAL, &com_port->fcr);
	NS16550_setbrg(com_port, baud_divisor);
}
#endif /* CONFIG_NS16550_MIN_FUNCTIONS */

void NS16550_putc(NS16550_t com_port, char c)
{
	while ((serial_in(&com_port->lsr) & UART_LSR_THRE) == 0)
		;
	serial_out(c, &com_port->thr);

	/*
	 * Call watchdog_reset() upon newline. This is done here in putc
	 * since the environment code uses a single puts() to print the complete
	 * environment upon "printenv". So we can't put this watchdog call
	 * in puts().
	 */
	if (c == '\n')
		WATCHDOG_RESET();
}

#ifndef CONFIG_NS16550_MIN_FUNCTIONS
char NS16550_getc(NS16550_t com_port)
{
	while ((serial_in(&com_port->lsr) & UART_LSR_DR) == 0) {
#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_USB_TTY)
		extern void usbtty_poll(void);
		usbtty_poll();
#endif
		WATCHDOG_RESET();
	}
	return serial_in(&com_port->rbr);
}

int NS16550_tstc(NS16550_t com_port)
{
	return (serial_in(&com_port->lsr) & UART_LSR_DR) != 0;
}

#endif /* CONFIG_NS16550_MIN_FUNCTIONS */

#ifdef CONFIG_DEBUG_UART_NS16550

#include <debug_uart.h>

#define serial_dout(reg, value)	\
	serial_out_shift((char *)com_port + \
		((char *)reg - (char *)com_port) * \
			(1 << CONFIG_DEBUG_UART_SHIFT), \
		CONFIG_DEBUG_UART_SHIFT, value)
#define serial_din(reg) \
	serial_in_shift((char *)com_port + \
		((char *)reg - (char *)com_port) * \
			(1 << CONFIG_DEBUG_UART_SHIFT), \
		CONFIG_DEBUG_UART_SHIFT)

static inline void _debug_uart_init(void)
{
	struct NS16550 *com_port = (struct NS16550 *)CONFIG_DEBUG_UART_BASE;
	int baud_divisor;

	/*
	 * We copy the code from above because it is already horribly messy.
	 * Trying to refactor to nicely remove the duplication doesn't seem
	 * feasible. The better fix is to move all users of this driver to
	 * driver model.
	 */
	baud_divisor = calc_divisor(com_port, CONFIG_DEBUG_UART_CLOCK,
				    CONFIG_BAUDRATE);
	serial_dout(&com_port->ier, CONFIG_SYS_NS16550_IER);
	serial_dout(&com_port->mcr, UART_MCRVAL);
	serial_dout(&com_port->fcr, UART_FCRVAL);

	serial_dout(&com_port->lcr, UART_LCR_BKSE | UART_LCRVAL);
	serial_dout(&com_port->dll, baud_divisor & 0xff);
	serial_dout(&com_port->dlm, (baud_divisor >> 8) & 0xff);
	serial_dout(&com_port->lcr, UART_LCRVAL);
}

static inline void _debug_uart_putc(int ch)
{
	struct NS16550 *com_port = (struct NS16550 *)CONFIG_DEBUG_UART_BASE;

	while (!(serial_din(&com_port->lsr) & UART_LSR_THRE))
		;
	serial_dout(&com_port->thr, ch);
}

DEBUG_UART_FUNCS

#endif

#ifdef CONFIG_DM_SERIAL
static int ns16550_serial_putc(struct udevice *dev, const char ch)
{
	struct NS16550 *const com_port = dev_get_priv(dev);

	if (!(serial_in(&com_port->lsr) & UART_LSR_THRE))
		return -EAGAIN;
	serial_out(ch, &com_port->thr);

	/*
	 * Call watchdog_reset() upon newline. This is done here in putc
	 * since the environment code uses a single puts() to print the complete
	 * environment upon "printenv". So we can't put this watchdog call
	 * in puts().
	 */
	if (ch == '\n')
		WATCHDOG_RESET();

	return 0;
}

static int ns16550_serial_pending(struct udevice *dev, bool input)
{
	struct NS16550 *const com_port = dev_get_priv(dev);

	if (input)
		return serial_in(&com_port->lsr) & UART_LSR_DR ? 1 : 0;
	else
		return serial_in(&com_port->lsr) & UART_LSR_THRE ? 0 : 1;
}

static int ns16550_serial_getc(struct udevice *dev)
{
	struct NS16550 *const com_port = dev_get_priv(dev);

	if (!(serial_in(&com_port->lsr) & UART_LSR_DR))
		return -EAGAIN;

	return serial_in(&com_port->rbr);
}

static int ns16550_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct NS16550 *const com_port = dev_get_priv(dev);
	struct ns16550_platdata *plat = com_port->plat;
	int clock_divisor;

	clock_divisor = ns16550_calc_divisor(com_port, plat->clock, baudrate);

	NS16550_setbrg(com_port, clock_divisor);

	return 0;
}

int ns16550_serial_probe(struct udevice *dev)
{
	struct NS16550 *const com_port = dev_get_priv(dev);

	com_port->plat = dev_get_platdata(dev);
	NS16550_init(com_port, -1);

	return 0;
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
int ns16550_serial_ofdata_to_platdata(struct udevice *dev)
{
	struct ns16550_platdata *plat = dev->platdata;
	fdt_addr_t addr;

	/* try Processor Local Bus device first */
	addr = dev_get_addr(dev);
#ifdef CONFIG_PCI
	if (addr == FDT_ADDR_T_NONE) {
		/* then try pci device */
		struct fdt_pci_addr pci_addr;
		u32 bar;
		int ret;

		/* we prefer to use a memory-mapped register */
		ret = fdtdec_get_pci_addr(gd->fdt_blob, dev->of_offset,
					  FDT_PCI_SPACE_MEM32, "reg",
					  &pci_addr);
		if (ret) {
			/* try if there is any i/o-mapped register */
			ret = fdtdec_get_pci_addr(gd->fdt_blob,
						  dev->of_offset,
						  FDT_PCI_SPACE_IO,
						  "reg", &pci_addr);
			if (ret)
				return ret;
		}

		ret = fdtdec_get_pci_bar32(gd->fdt_blob, dev->of_offset,
					   &pci_addr, &bar);
		if (ret)
			return ret;

		addr = bar;
	}
#endif

	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	plat->base = addr;
	plat->reg_shift = fdtdec_get_int(gd->fdt_blob, dev->of_offset,
					 "reg-shift", 0);
	plat->clock = fdtdec_get_int(gd->fdt_blob, dev->of_offset,
				     "clock-frequency",
				     CONFIG_SYS_NS16550_CLK);
	if (!plat->clock) {
		debug("ns16550 clock not defined\n");
		return -EINVAL;
	}

	return 0;
}
#endif

const struct dm_serial_ops ns16550_serial_ops = {
	.putc = ns16550_serial_putc,
	.pending = ns16550_serial_pending,
	.getc = ns16550_serial_getc,
	.setbrg = ns16550_serial_setbrg,
};

#if CONFIG_IS_ENABLED(OF_CONTROL)
static const struct udevice_id ns16550_serial_ids[] = {
	{ .compatible = "ns16550" },
	{ .compatible = "ns16550a" },
	{ .compatible = "nvidia,tegra20-uart" },
	{ .compatible = "rockchip,rk3036-uart" },
	{ .compatible = "snps,dw-apb-uart" },
	{ .compatible = "ti,omap2-uart" },
	{ .compatible = "ti,omap3-uart" },
	{ .compatible = "ti,omap4-uart" },
	{ .compatible = "ti,am3352-uart" },
	{ .compatible = "ti,am4372-uart" },
	{ .compatible = "ti,dra742-uart" },
	{}
};
#endif

U_BOOT_DRIVER(ns16550_serial) = {
	.name	= "ns16550_serial",
	.id	= UCLASS_SERIAL,
#if CONFIG_IS_ENABLED(OF_CONTROL)
	.of_match = ns16550_serial_ids,
	.ofdata_to_platdata = ns16550_serial_ofdata_to_platdata,
	.platdata_auto_alloc_size = sizeof(struct ns16550_platdata),
#endif
	.priv_auto_alloc_size = sizeof(struct NS16550),
	.probe = ns16550_serial_probe,
	.ops	= &ns16550_serial_ops,
	.flags	= DM_FLAG_PRE_RELOC,
};
#endif /* CONFIG_DM_SERIAL */
