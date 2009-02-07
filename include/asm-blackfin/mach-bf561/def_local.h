#define SWRST SICA_SWRST
#define SYSCR SICA_SYSCR
#define bfin_write_SWRST(val) bfin_write_SICA_SWRST(val)
#define bfin_write_SYSCR(val) bfin_write_SICA_SYSCR(val)

#define WDOG_CNT WDOGA_CNT
#define WDOG_CTL WDOGA_CTL
#define bfin_write_WDOG_CNT(val) bfin_write_WDOGA_CNT(val)
#define bfin_write_WDOG_CTL(val) bfin_write_WDOGA_CTL(val)
#define bfin_write_WDOG_STAT(val) bfin_write_WDOGA_STAT(val)

#include "ports.h"
