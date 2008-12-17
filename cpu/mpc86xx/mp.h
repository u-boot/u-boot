#ifndef __MPC86XX_MP_H_
#define __MPC86XX_MP_H_

void setup_mp(void);
void cpu_mp_lmb_reserve(struct lmb *lmb);

#endif
