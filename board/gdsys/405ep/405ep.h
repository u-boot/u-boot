#ifndef __405EP_H_
#define __405EP_H_

/* functions to be provided by board implementation */
void gd405ep_init(void);
void gd405ep_set_fpga_reset(unsigned state);
void gd405ep_setup_hw(void);
int gd405ep_get_fpga_done(unsigned fpga);

#endif /* __405EP_H_ */
