#include <config.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/clk.h>
#include <asm/arch/io.h>

int arch_cpu_init(void)
{
#ifdef AT91_MAIN_CLOCK
	return at91_clock_init(AT91_MAIN_CLOCK);
#else
	return at91_clock_init(0);
#endif
}
