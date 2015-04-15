#include <common.h>
#include <exports.h>
#include <spi.h>
#include <i2c.h>

DECLARE_GLOBAL_DATA_PTR;

__attribute__((unused)) static void dummy(void)
{
}

unsigned long get_version(void)
{
	return XF_VERSION;
}

#define EXPORT_FUNC(f, a, x, ...)  gd->jt->x = f;

void jumptable_init(void)
{
	gd->jt = malloc(sizeof(struct jt_funcs));
#include <_exports.h>
}
