#include <common.h>
#include <exports.h>

DECLARE_GLOBAL_DATA_PTR;

static void dummy(void)
{
}

unsigned long get_version(void)
{
	return XF_VERSION;
}

void jumptable_init (void)
{
	int i;

	gd->jt = (void **) malloc (XF_MAX * sizeof (void *));
	for (i = 0; i < XF_MAX; i++)
		gd->jt[i] = (void *) dummy;

	gd->jt[XF_get_version] = (void *) get_version;
	gd->jt[XF_malloc] = (void *) malloc;
	gd->jt[XF_free] = (void *) free;
	gd->jt[XF_getenv] = (void *) getenv;
	gd->jt[XF_setenv] = (void *) setenv;
	gd->jt[XF_get_timer] = (void *) get_timer;
	gd->jt[XF_simple_strtoul] = (void *) simple_strtoul;
	gd->jt[XF_udelay] = (void *) udelay;
	gd->jt[XF_simple_strtol] = (void *) simple_strtol;
	gd->jt[XF_strcmp] = (void *) strcmp;
#if defined(CONFIG_I386) || defined(CONFIG_PPC)
	gd->jt[XF_install_hdlr] = (void *) irq_install_handler;
	gd->jt[XF_free_hdlr] = (void *) irq_free_handler;
#endif	/* I386 || PPC */
#if defined(CONFIG_CMD_I2C)
	gd->jt[XF_i2c_write] = (void *) i2c_write;
	gd->jt[XF_i2c_read] = (void *) i2c_read;
#endif
#ifdef CONFIG_CMD_SPI
	gd->jt[XF_spi_init] = (void *) spi_init;
	gd->jt[XF_spi_setup_slave] = (void *) spi_setup_slave;
	gd->jt[XF_spi_free_slave] = (void *) spi_free_slave;
	gd->jt[XF_spi_claim_bus] = (void *) spi_claim_bus;
	gd->jt[XF_spi_release_bus] = (void *) spi_release_bus;
	gd->jt[XF_spi_xfer] = (void *) spi_xfer;
#endif
}
