#ifndef __405EX_H_
#define __405EX_H_

/* functions to be provided by board implementation */
void gd405ex_init(void);
void gd405ex_set_fpga_reset(unsigned state);
void gd405ex_setup_hw(void);
int gd405ex_get_fpga_done(unsigned fpga);

#endif /* __405EX_H_ */
