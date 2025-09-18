#ifndef __EXPORTS_H__
#define __EXPORTS_H__

#include <irq_func.h>
#include <asm/global_data.h>
#include <linux/delay.h>

#ifndef __ASSEMBLY__
#ifdef CONFIG_PHY_AQUANTIA
#include <env.h>
#include <phy_interface.h>
#endif

#include <irq_func.h>

struct cmd_tbl;
struct spi_slave;

/**
 * jumptable_init() - Set up the jump table for use by the API
 *
 * It is called during the generic post-relocation init sequence.
 *
 * Return: 0 if OK
 */
int jumptable_init(void);

/* These are declarations of exported functions available in C code */
#define EXPORT_FUNC(impl, res, func, ...) res func(__VA_ARGS__);
#include <_exports.h>
#undef EXPORT_FUNC

void app_startup(char * const *);

#endif    /* ifndef __ASSEMBLY__ */

struct jt_funcs {
#define EXPORT_FUNC(impl, res, func, ...) res(*func)(__VA_ARGS__);
#include <_exports.h>
#undef EXPORT_FUNC
};

#define XF_VERSION	10

#if defined(CONFIG_X86)
extern gd_t *global_data;
#endif

#endif	/* __EXPORTS_H__ */
