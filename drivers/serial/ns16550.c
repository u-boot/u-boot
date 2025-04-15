/*
 * COM1 NS16550 support
 * originally from linux source (arch/powerpc/boot/ns16550.c)
 * modified to use CONFIG_SYS_ISA_MEM and new defines
 */

#include <clock_legacy.h>
#include <config.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <log.h>
#include <ns16550.h>
#include <reset.h>
#include <spl.h>
#include <watchdog.h>
#include <asm/global_data.h>
#include <linux/err.h>
#include <linux/types.h>
#include <asm/io.h>

DECLARE_GLOBAL_DATA_PTR;

#define UART_LCRVAL UART_LCR_8N1		/* 8 data, 1 stop, no parity */
#define UART_MCRVAL (UART_MCR_DTR | \
		     UART_MCR_RTS)		/* RTS/DTR */

#if !CONFIG_IS_ENABLED(DM_SERIAL)
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

#if defined(CONFIG_ARCH_KEYSTONE)
#define UART_REG_VAL_PWREMU_MGMT_UART_DISABLE   0
#define UART_REG_VAL_PWREMU_MGMT_UART_ENABLE ((1 << 14) | (1 << 13) | (1 << 0))
#undef UART_MCRVAL
#ifdef CONFIG_SERIAL_HW_FLOW_CONTROL
#define UART_MCRVAL             (UART_MCR_RTS | UART_MCR_AFE)
#else
#define UART_MCRVAL             (UART_MCR_RTS)
#endif
#endif

#ifndef CFG_SYS_NS16550_IER
#define CFG_SYS_NS16550_IER  0x00
#endif /* CFG_SYS_NS16550_IER */

static inline void serial_out_shift(void *addr, int shift, int value)
{
#ifdef CONFIG_SYS_NS16550_PORT_MAPPED
	outb(value, (ulong)addr);
#elif defined(CONFIG_SYS_NS16550_MEM32) && defined(CONFIG_SYS_LITTLE_ENDIAN)
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
#elif defined(CONFIG_SYS_NS16550_MEM32) && defined(CONFIG_SYS_LITTLE_ENDIAN)
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

#if CONFIG_IS_ENABLED(DM_SERIAL)

#ifndef CFG_SYS_NS16550_CLK
#define CFG_SYS_NS16550_CLK  0
#endif

/*
 * Use this #ifdef for now since many platforms don't define in(), out(),
 * out_le32(), etc. but we don't have #defines to indicate this.
 *
 * TODO(sjg@chromium.org): Add CONFIG options to indicate what I/O is available
 * on a platform
 */
#ifdef CONFIG_NS16550_DYNAMIC
static void serial_out_dynamic(struct ns16550_plat *plat, u8 *addr,
			       int value)
{
	if (plat->flags & NS16550_FLAG_IO) {
		outb(value, addr);
	} else if (plat->reg_width == 4) {
		if (plat->flags & NS16550_FLAG_ENDIAN) {
			if (plat->flags & NS16550_FLAG_BE)
				out_be32((u32 *)addr, value);
			else
				out_le32((u32 *)addr, value);
		} else {
			writel(value, addr);
		}
	} else if (plat->flags & NS16550_FLAG_BE) {
		writeb(value, addr + (1 << plat->reg_shift) - 1);
	} else {
		writeb(value, addr);
	}
}

static int serial_in_dynamic(struct ns16550_plat *plat, u8 *addr)
{
	if (plat->flags & NS16550_FLAG_IO) {
		return inb(addr);
	} else if (plat->reg_width == 4) {
		if (plat->flags & NS16550_FLAG_ENDIAN) {
			if (plat->flags & NS16550_FLAG_BE)
				return in_be32((u32 *)addr);
			else
				return in_le32((u32 *)addr);
		} else {
			return readl(addr);
		}
	} else if (plat->flags & NS16550_FLAG_BE) {
		return readb(addr + (1 << plat->reg_shift) - 1);
	} else {
		return readb(addr);
	}
}
#else
static inline void serial_out_dynamic(struct ns16550_plat *plat, u8 *addr,
				      int value)
{
}

static inline int serial_in_dynamic(struct ns16550_plat *plat, u8 *addr)
{
	return 0;
}

#endif /* CONFIG_NS16550_DYNAMIC */

void ns16550_writeb(struct ns16550 *port, int offset, int value)
{
	struct ns16550_plat *plat = port->plat;
	unsigned char *addr;

	offset *= 1 << plat->reg_shift;
	addr = (unsigned char *)plat->base + offset + plat->reg_offset;

	if (IS_ENABLED(CONFIG_NS16550_DYNAMIC))
		serial_out_dynamic(plat, addr, value);
	else
		serial_out_shift(addr, plat->reg_shift, value);
}

static int ns16550_readb(struct ns16550 *port, int offset)
{
	struct ns16550_plat *plat = port->plat;
	unsigned char *addr;

	offset *= 1 << plat->reg_shift;
	addr = (unsigned char *)plat->base + offset + plat->reg_offset;

	if (IS_ENABLED(CONFIG_NS16550_DYNAMIC))
		return serial_in_dynamic(plat, addr);
	else
		return serial_in_shift(addr, plat->reg_shift);
}

static u32 ns16550_getfcr(struct ns16550 *port)
{
	struct ns16550_plat *plat = port->plat;

	return plat->fcr;
}

#else
static u32 ns16550_getfcr(struct ns16550 *port)
{
	return UART_FCR_DEFVAL;
}
#endif

int ns16550_calc_divisor(struct ns16550 *port, int clock, int baudrate)
{
	const unsigned int mode_x_div = 16;

	return DIV_ROUND_CLOSEST(clock, mode_x_div * baudrate);
}

void ns16550_setbrg(struct ns16550 *com_port, int baud_divisor)
{
	/* to keep serial format, read lcr before writing BKSE */
	int lcr_val = serial_in(&com_port->lcr) & ~UART_LCR_BKSE;

	serial_out(UART_LCR_BKSE | lcr_val, &com_port->lcr);
	serial_out(baud_divisor & 0xff, &com_port->dll);
	serial_out((baud_divisor >> 8) & 0xff, &com_port->dlm);
	serial_out(lcr_val, &com_port->lcr);
}

void ns16550_init(struct ns16550 *com_port, int baud_divisor)
{
#if defined(CONFIG_XPL_BUILD) && defined(CONFIG_OMAP34XX)
	/*
	 * On some OMAP3/OMAP4 devices when UART3 is configured for boot mode
	 * before SPL starts only THRE bit is set. We have to empty the
	 * transmitter before initialization starts.
	 */
	if ((serial_in(&com_port->lsr) & (UART_LSR_TEMT | UART_LSR_THRE))
	     == UART_LSR_THRE) {
		if (baud_divisor != -1)
			ns16550_setbrg(com_port, baud_divisor);
		else {
			// Re-use old baud rate divisor to flush transmit reg.
			const int dll = serial_in(&com_port->dll);
			const int dlm = serial_in(&com_port->dlm);
			const int divisor = dll | (dlm << 8);
			ns16550_setbrg(com_port, divisor);
		}
		serial_out(0, &com_port->mdr1);
	}
#endif

	while (!(serial_in(&com_port->lsr) & UART_LSR_TEMT))
		;

	serial_out(CFG_SYS_NS16550_IER, &com_port->ier);
#if defined(CONFIG_ARCH_OMAP2PLUS) || defined(CONFIG_OMAP_SERIAL)
	serial_out(0x7, &com_port->mdr1);	/* mode select reset TL16C750*/
#endif

	serial_out(UART_MCRVAL, &com_port->mcr);
	serial_out(ns16550_getfcr(com_port), &com_port->fcr);
	/* initialize serial config to 8N1 before writing baudrate */
	serial_out(UART_LCRVAL, &com_port->lcr);
	if (baud_divisor != -1)
		ns16550_setbrg(com_port, baud_divisor);
#if defined(CONFIG_ARCH_OMAP2PLUS) || defined(CONFIG_SOC_DA8XX) || \
	defined(CONFIG_OMAP_SERIAL)
	/* /16 is proper to hit 115200 with 48MHz */
	serial_out(0, &com_port->mdr1);
#endif
#if defined(CONFIG_ARCH_KEYSTONE)
	serial_out(UART_REG_VAL_PWREMU_MGMT_UART_ENABLE, &com_port->regC);
#endif
}

#if !CONFIG_IS_ENABLED(NS16550_MIN_FUNCTIONS)
void ns16550_reinit(struct ns16550 *com_port, int baud_divisor)
{
	serial_out(CFG_SYS_NS16550_IER, &com_port->ier);
	ns16550_setbrg(com_port, 0);
	serial_out(UART_MCRVAL, &com_port->mcr);
	serial_out(ns16550_getfcr(com_port), &com_port->fcr);
	ns16550_setbrg(com_port, baud_divisor);
}
#endif /* !CONFIG_IS_ENABLED(NS16550_MIN_FUNCTIONS) */

void ns16550_putc(struct ns16550 *com_port, char c)
{
	while ((serial_in(&com_port->lsr) & UART_LSR_THRE) == 0)
		;
	serial_out(c, &com_port->thr);

	/*
	 * Call schedule() upon newline. This is done here in putc
	 * since the environment code uses a single puts() to print the complete
	 * environment upon "printenv". So we can't put this schedule call
	 * in puts().
	 */
	if (c == '\n')
		schedule();
}

#if !CONFIG_IS_ENABLED(NS16550_MIN_FUNCTIONS)
char ns16550_getc(struct ns16550 *com_port)
{
	while ((serial_in(&com_port->lsr) & UART_LSR_DR) == 0)
		schedule();

	return serial_in(&com_port->rbr);
}

int ns16550_tstc(struct ns16550 *com_port)
{
	return (serial_in(&com_port->lsr) & UART_LSR_DR) != 0;
}

#endif /* !CONFIG_IS_ENABLED(NS16550_MIN_FUNCTIONS) */

#ifdef CONFIG_DEBUG_UART_NS16550

#include <debug_uart.h>

static inline void _debug_uart_init(void)
{
	struct ns16550 *com_port = (struct ns16550 *)CONFIG_VAL(DEBUG_UART_BASE);
	int baud_divisor;

	/* Wait until tx buffer is empty */
	while (!(serial_din(&com_port->lsr) & UART_LSR_TEMT))
		;

	/*
	 * We copy the code from above because it is already horribly messy.
	 * Trying to refactor to nicely remove the duplication doesn't seem
	 * feasible. The better fix is to move all users of this driver to
	 * driver model.
	 */
	baud_divisor = ns16550_calc_divisor(com_port, CONFIG_DEBUG_UART_CLOCK,
					    CONFIG_BAUDRATE);
	serial_dout(&com_port->ier, CFG_SYS_NS16550_IER);
	serial_dout(&com_port->mcr, UART_MCRVAL);
	serial_dout(&com_port->fcr, UART_FCR_DEFVAL);

	serial_dout(&com_port->lcr, UART_LCR_BKSE | UART_LCRVAL);
	serial_dout(&com_port->dll, baud_divisor & 0xff);
	serial_dout(&com_port->dlm, (baud_divisor >> 8) & 0xff);
	serial_dout(&com_port->lcr, UART_LCRVAL);
}

static inline int NS16550_read_baud_divisor(struct ns16550 *com_port)
{
	int ret;

	serial_dout(&com_port->lcr, UART_LCR_BKSE | UART_LCRVAL);
	ret = serial_din(&com_port->dll) & 0xff;
	ret |= (serial_din(&com_port->dlm) & 0xff) << 8;
	serial_dout(&com_port->lcr, UART_LCRVAL);

	return ret;
}

static inline void _debug_uart_putc(int ch)
{
	struct ns16550 *com_port = (struct ns16550 *)CONFIG_VAL(DEBUG_UART_BASE);

	while (!(serial_din(&com_port->lsr) & UART_LSR_THRE)) {
#ifdef CONFIG_DEBUG_UART_NS16550_CHECK_ENABLED
		if (!NS16550_read_baud_divisor(com_port))
			return;
#endif
	}
	serial_dout(&com_port->thr, ch);
}

DEBUG_UART_FUNCS

#endif

#if CONFIG_IS_ENABLED(DM_SERIAL)
int ns16550_serial_putc(struct udevice *dev, const char ch)
{
	struct ns16550 *const com_port = dev_get_priv(dev);

	if (!(serial_in(&com_port->lsr) & UART_LSR_THRE))
		return -EAGAIN;
	serial_out(ch, &com_port->thr);

	/*
	 * Call schedule() upon newline. This is done here in putc
	 * since the environment code uses a single puts() to print the complete
	 * environment upon "printenv". So we can't put this schedule call
	 * in puts().
	 */
	if (ch == '\n')
		schedule();

	return 0;
}

int ns16550_serial_pending(struct udevice *dev, bool input)
{
	struct ns16550 *const com_port = dev_get_priv(dev);

	if (input)
		return (serial_in(&com_port->lsr) & UART_LSR_DR) ? 1 : 0;
	else
		return (serial_in(&com_port->lsr) & UART_LSR_THRE) ? 0 : 1;
}

int ns16550_serial_getc(struct udevice *dev)
{
	struct ns16550 *const com_port = dev_get_priv(dev);

	if (!(serial_in(&com_port->lsr) & UART_LSR_DR))
		return -EAGAIN;

	return serial_in(&com_port->rbr);
}

int ns16550_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct ns16550 *const com_port = dev_get_priv(dev);
	struct ns16550_plat *plat = com_port->plat;
	int clock_divisor;

	clock_divisor = ns16550_calc_divisor(com_port, plat->clock, baudrate);

	ns16550_setbrg(com_port, clock_divisor);

	return 0;
}

int ns16550_serial_setconfig(struct udevice *dev, uint serial_config)
{
	struct ns16550 *const com_port = dev_get_priv(dev);
	int lcr_val = UART_LCR_WLS_8;
	uint parity = SERIAL_GET_PARITY(serial_config);
	uint bits = SERIAL_GET_BITS(serial_config);
	uint stop = SERIAL_GET_STOP(serial_config);

	/*
	 * only parity config is implemented, check if other serial settings
	 * are the default one.
	 */
	if (bits != SERIAL_8_BITS || stop != SERIAL_ONE_STOP)
		return -ENOTSUPP; /* not supported in driver*/

	switch (parity) {
	case SERIAL_PAR_NONE:
		/* no bits to add */
		break;
	case SERIAL_PAR_ODD:
		lcr_val |= UART_LCR_PEN;
		break;
	case SERIAL_PAR_EVEN:
		lcr_val |= UART_LCR_PEN | UART_LCR_EPS;
		break;
	default:
		return -ENOTSUPP; /* not supported in driver*/
	}

	serial_out(lcr_val, &com_port->lcr);
	return 0;
}

int ns16550_serial_getinfo(struct udevice *dev, struct serial_device_info *info)
{
	struct ns16550 *const com_port = dev_get_priv(dev);
	struct ns16550_plat *plat = com_port->plat;

	/* save code size */
	if (!not_xpl() && !CONFIG_IS_ENABLED(UPL_OUT))
		return -ENOSYS;

	info->type = SERIAL_CHIP_16550_COMPATIBLE;
#ifdef CONFIG_SYS_NS16550_PORT_MAPPED
	info->addr_space = SERIAL_ADDRESS_SPACE_IO;
#else
	info->addr_space = SERIAL_ADDRESS_SPACE_MEMORY;
#endif
	info->addr = plat->base;
	info->size = plat->size;
	info->reg_width = plat->reg_width;
	info->reg_shift = plat->reg_shift;
	info->reg_offset = plat->reg_offset;
	info->clock = plat->clock;

	return 0;
}

static int ns16550_serial_assign_base(struct ns16550_plat *plat,
				      fdt_addr_t base, fdt_size_t size)
{
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

#ifdef CONFIG_SYS_NS16550_PORT_MAPPED
	plat->base = base;
#else
	plat->base = (unsigned long)map_physmem(base, 0, MAP_NOCACHE);
#endif
	plat->size = size;

	return 0;
}

int ns16550_serial_probe(struct udevice *dev)
{
	struct ns16550_plat *plat = dev_get_plat(dev);
	struct ns16550 *const com_port = dev_get_priv(dev);
	struct reset_ctl_bulk reset_bulk;
	fdt_addr_t addr;
	fdt_addr_t size;
	int ret;

	/*
	 * If we are on PCI bus, either directly attached to a PCI root port,
	 * or via a PCI bridge, assign plat->base before probing hardware.
	 */
	if (device_is_on_pci_bus(dev)) {
		addr = devfdt_get_addr_pci(dev, &size);
		ret = ns16550_serial_assign_base(plat, addr, size);
		if (ret)
			return ret;
	}

	ret = reset_get_bulk(dev, &reset_bulk);
	if (!ret)
		reset_deassert_bulk(&reset_bulk);

	com_port->plat = dev_get_plat(dev);
	ns16550_init(com_port, -1);

	return 0;
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
enum {
	PORT_NS16550 = 0,
	PORT_JZ4780,
};
#endif

#if CONFIG_IS_ENABLED(OF_REAL)
int ns16550_serial_of_to_plat(struct udevice *dev)
{
	struct ns16550_plat *plat = dev_get_plat(dev);
	const u32 port_type = dev_get_driver_data(dev);
	fdt_size_t size = 0;
	fdt_addr_t addr;
	struct clk clk;
	int err;

	addr = not_xpl() ? dev_read_addr_size(dev, &size) :
		dev_read_addr(dev);
	err = ns16550_serial_assign_base(plat, addr, size);
	if (err && !device_is_on_pci_bus(dev))
		return err;

	plat->reg_offset = dev_read_u32_default(dev, "reg-offset", 0);
	plat->reg_shift = dev_read_u32_default(dev, "reg-shift", 0);
	plat->reg_width = dev_read_u32_default(dev, "reg-io-width", 1);

	if (!plat->clock)
		plat->clock = dev_read_u32_default(dev, "clock-frequency", 0);
	if (!plat->clock) {
		err = clk_get_by_index(dev, 0, &clk);
		if (!err) {
			err = clk_get_rate(&clk);
			if (!IS_ERR_VALUE(err))
				plat->clock = err;
		} else if (err != -ENOENT && err != -ENODEV && err != -ENOSYS) {
			debug("ns16550 failed to get clock\n");
			return err;
		}
	}
	if (!plat->clock)
		plat->clock = CFG_SYS_NS16550_CLK;
	if (!plat->clock) {
		debug("ns16550 clock not defined\n");
		return -EINVAL;
	}

	plat->fcr = UART_FCR_DEFVAL;
	if (port_type == PORT_JZ4780)
		plat->fcr |= UART_FCR_UME;

	return 0;
}
#endif

const struct dm_serial_ops ns16550_serial_ops = {
	.putc = ns16550_serial_putc,
	.pending = ns16550_serial_pending,
	.getc = ns16550_serial_getc,
	.setbrg = ns16550_serial_setbrg,
	.setconfig = ns16550_serial_setconfig,
	.getinfo = ns16550_serial_getinfo,
};

#if CONFIG_IS_ENABLED(OF_REAL)
/*
 * Please consider existing compatible strings before adding a new
 * one to keep this table compact. Or you may add a generic "ns16550"
 * compatible string to your dts.
 */
static const struct udevice_id ns16550_serial_ids[] = {
	{ .compatible = "ns16550",		.data = PORT_NS16550 },
	{ .compatible = "ns16550a",		.data = PORT_NS16550 },
	{ .compatible = "ingenic,jz4780-uart",	.data = PORT_JZ4780  },
	{ .compatible = "nvidia,tegra20-uart",	.data = PORT_NS16550 },
	{ .compatible = "snps,dw-apb-uart",	.data = PORT_NS16550 },
	{ .compatible = "intel,xscale-uart",	.data = PORT_NS16550 },
	{}
};
#endif /* OF_REAL */

#if CONFIG_IS_ENABLED(SERIAL_PRESENT)

/* TODO(sjg@chromium.org): Integrate this into a macro like CONFIG_IS_ENABLED */
#if !defined(CONFIG_TPL_BUILD) || defined(CONFIG_TPL_DM_SERIAL)
U_BOOT_DRIVER(ns16550_serial) = {
	.name	= "ns16550_serial",
	.id	= UCLASS_SERIAL,
#if CONFIG_IS_ENABLED(OF_REAL)
	.of_match = ns16550_serial_ids,
	.of_to_plat = ns16550_serial_of_to_plat,
	.plat_auto	= sizeof(struct ns16550_plat),
#endif
	.priv_auto	= sizeof(struct ns16550),
	.probe = ns16550_serial_probe,
	.ops	= &ns16550_serial_ops,
#if !CONFIG_IS_ENABLED(OF_CONTROL)
	.flags	= DM_FLAG_PRE_RELOC,
#endif
};

DM_DRIVER_ALIAS(ns16550_serial, ti_da830_uart)
#endif
#endif /* SERIAL_PRESENT */

#endif /* CONFIG_DM_SERIAL */
