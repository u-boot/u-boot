#include <config.h>

#include <common.h>

void
signal_delay(unsigned int n)
{
  while (n--);
}

void
signal_on(void)
{
  *((volatile uint *)BCSR4) &= ~(1<<(31-3)); /* led on */
}

void
signal_off(void)
{
  *((volatile uint *)BCSR4) |= (1<<(31-3)); /* led off */
}

void
slow_blink(unsigned int n)
{
  while (n--) {
    signal_on();
    signal_delay(0x00400000);
    signal_off();
    signal_delay(0x00400000);
  }
}

void
fast_blink(unsigned int n)
{
  while (n--) {
    signal_on();
    signal_delay(0x00100000);
    signal_off();
    signal_delay(0x00100000);
  }
}
