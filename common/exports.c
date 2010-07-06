#include <common.h>
#include <exports.h>

DECLARE_GLOBAL_DATA_PTR;

__attribute__((unused)) static void dummy(void)
{
}

unsigned long get_version(void)
{
	return XF_VERSION;
}

/* Reuse _exports.h with a little trickery to avoid bitrot */
#define EXPORT_FUNC(sym) gd->jt[XF_##sym] = (void *)sym;

#if !defined(CONFIG_I386) && !defined(CONFIG_PPC)
# define install_hdlr      dummy
# define free_hdlr         dummy
#else /* kludge for non-standard function naming */
# define install_hdlr      irq_install_handler
# define free_hdlr         irq_free_handler
#endif
#ifndef CONFIG_CMD_I2C
# define i2c_write         dummy
# define i2c_read          dummy
#endif
#ifndef CONFIG_CMD_SPI
# define spi_init          dummy
# define spi_setup_slave   dummy
# define spi_free_slave    dummy
# define spi_claim_bus     dummy
# define spi_release_bus   dummy
# define spi_xfer          dummy
#endif
#ifndef CONFIG_HAS_UID
# define forceenv          dummy
#endif

void jumptable_init(void)
{
	gd->jt = malloc(XF_MAX * sizeof(void *));
#include <_exports.h>
}
