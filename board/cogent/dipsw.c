#include <common.h>
#include <board/cogent/dipsw.h>

unsigned char
dipsw_raw(void)
{
    return cma_mb_reg_read(&((cma_mb_dipsw *)CMA_MB_DIPSW_BASE)->dip_val);
}

unsigned char
dipsw_cooked(void)
{
    unsigned char val1, val2, mask1, mask2;

    val1 = dipsw_raw();

    /*
     * we want to mirror the bits because the low bit is switch 1 and high
     * bit is switch 8 and also invert them because 1=off and 0=on, according
     * to manual.
     *
     * this makes the value more intuitive i.e.
     * - left most, or high, or top, bit is left most switch (1);
     * - right most, or low, or bottom, bit is right most switch (8)
     * - a set bit means "on" and a clear bit means "off"
     */

    val2 = 0;
    for (mask1 = 1 << 7, mask2 = 1; mask1 > 0; mask1 >>= 1, mask2 <<= 1)
	if ((val1 & mask1) == 0)
	    val2 |= mask2;

    return (val2);
}

void
dipsw_init(void)
{
    unsigned char val, mask;

    val = dipsw_cooked();

    printf("|");
    for (mask = 1 << 7; mask > 0; mask >>= 1)
	if (val & mask)
	    printf("on |");
	else
	    printf("off|");
    printf("\n");
}
