#ifndef __PCA9698_H_
#define __PCA9698_H_

int pca9698_direction_input(u8 chip, unsigned offset);
int pca9698_direction_output(u8 chip, unsigned offset);
int pca9698_get_input(u8 chip, unsigned offset);
int pca9698_set_output(u8 chip, unsigned offset, int value);

#endif /* __PCA9698_H_ */
