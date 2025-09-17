/*
 * You need to use #ifdef around functions that may not exist
 * in the final configuration (such as i2c).
 * As an example see the CONFIG_CMD_I2C section below
 */
#ifndef EXPORT_FUNC
#define EXPORT_FUNC(a, b, c, ...)
#endif
	EXPORT_FUNC(get_version, unsigned long, get_version, void)
	EXPORT_FUNC(getchar, int, getc, void)
	EXPORT_FUNC(tstc, int, tstc, void)
	EXPORT_FUNC(putc, void, putc, const char)
	EXPORT_FUNC(puts, void, puts, const char *)
#ifdef CONFIG_CONSOLE_FLUSH_SUPPORT
	EXPORT_FUNC(flush, void, flush, void)
#endif
	EXPORT_FUNC(printf, int, printf, const char*, ...)
	EXPORT_FUNC(vprintf, int, vprintf, const char *, va_list)
#if (defined(CONFIG_X86) && !defined(CONFIG_X86_64)) || defined(CONFIG_PPC)
	EXPORT_FUNC(irq_install_handler, void, install_hdlr,
		    int, interrupt_handler_t, void*)

	EXPORT_FUNC(irq_free_handler, void, free_hdlr, int)
#endif
	EXPORT_FUNC(malloc, void *, malloc, size_t)
	EXPORT_FUNC(realloc, void *, realloc, void *, size_t)
	EXPORT_FUNC(calloc, void *, calloc, size_t, size_t)
#if !CONFIG_IS_ENABLED(SYS_MALLOC_SIMPLE)
	EXPORT_FUNC(free, void, free, void *)
#endif
	EXPORT_FUNC(mdelay, void, mdelay, unsigned long msec)
	EXPORT_FUNC(udelay, void, udelay, unsigned long)
	EXPORT_FUNC(get_timer, unsigned long, get_timer, unsigned long)
	EXPORT_FUNC(do_reset, int, do_reset, struct cmd_tbl *,
		    int , int , char * const [])
	EXPORT_FUNC(env_get, char  *, env_get, const char*)
	EXPORT_FUNC(env_set, int, env_set, const char *, const char *)
	EXPORT_FUNC(simple_strtoul, unsigned long, simple_strtoul,
		    const char *, char **, unsigned int)
	EXPORT_FUNC(strict_strtoul, int, strict_strtoul,
		    const char *, unsigned int , unsigned long *)
	EXPORT_FUNC(simple_strtol, long, simple_strtol,
		    const char *, char **, unsigned int)
	EXPORT_FUNC(ustrtoul, unsigned long, ustrtoul,
		    const char *, char **, unsigned int)
	EXPORT_FUNC(ustrtoull, unsigned long long, ustrtoull,
		    const char *, char **, unsigned int)
	EXPORT_FUNC(memcmp, int, memcmp, const void *, const void *, size_t)
	EXPORT_FUNC(memcpy, void *, memcpy, void *, const void *, size_t)
	EXPORT_FUNC(memmove, void *, memmove, void *, const void *, size_t)
	EXPORT_FUNC(memset, void *, memset, void *, int, size_t)
	EXPORT_FUNC(strchr, char *, strchr, const char *cs, int c)
	EXPORT_FUNC(strlen, size_t, strlen, const char *s)
	EXPORT_FUNC(strncmp, int, strncmp, const char *cs, const char *ct, size_t n)
	EXPORT_FUNC(strncpy, char *, strncpy, char *dest, const char *src, size_t n)
	EXPORT_FUNC(strnlen, size_t, strnlen, const char *s, size_t n)
	EXPORT_FUNC(strcmp, int, strcmp, const char *cs, const char *ct)
	EXPORT_FUNC(strcpy, char *, strcpy, char *dest, const char *src)
	EXPORT_FUNC(sprintf, int, sprintf, char *, const char *, ...)
	EXPORT_FUNC(snprintf, int, snprintf, char *, size_t, const char *, ...)
	EXPORT_FUNC(vsprintf, int, vsprintf, char *, const char *, va_list)
	EXPORT_FUNC(vsnprintf, int, vsnprintf, char *, size_t, const char *, va_list)
#if defined(CONFIG_CMD_I2C) && CONFIG_IS_ENABLED(SYS_I2C_LEGACY)
	EXPORT_FUNC(i2c_write, int, i2c_write, uchar, uint, int , uchar * , int)
	EXPORT_FUNC(i2c_read, int, i2c_read, uchar, uint, int , uchar * , int)
#endif

#if defined(CONFIG_CMD_SPI)
	EXPORT_FUNC(spi_setup_slave, struct spi_slave *, spi_setup_slave,
		    unsigned int, unsigned int, unsigned int, unsigned int)
	EXPORT_FUNC(spi_free_slave, void, spi_free_slave, struct spi_slave *)
	EXPORT_FUNC(spi_claim_bus, int, spi_claim_bus, struct spi_slave *)
	EXPORT_FUNC(spi_release_bus, void, spi_release_bus, struct spi_slave *)
	EXPORT_FUNC(spi_xfer, int, spi_xfer, struct spi_slave *,
		    unsigned int, const void *, void *, unsigned long)
#endif
#ifdef CONFIG_PHY_AQUANTIA
	EXPORT_FUNC(mdio_get_current_dev, struct mii_dev *,
		    mdio_get_current_dev, void)
	EXPORT_FUNC(phy_find_by_mask, struct phy_device *, phy_find_by_mask,
		    struct mii_dev *bus, unsigned phy_mask)
	EXPORT_FUNC(mdio_phydev_for_ethname, struct phy_device *,
		    mdio_phydev_for_ethname, const char *ethname)
	EXPORT_FUNC(miiphy_set_current_dev, int, miiphy_set_current_dev,
		    const char *devname)
#endif
