#ifndef VIA686_H_
#define VIA686_H_


#define CMOS_ADDR         0x70
#define CMOS_DATA         0x71

#define I8259_MASTER_CONTROL 0x20
#define I8259_MASTER_MASK    0x21

#define I8259_SLAVE_CONTROL  0xA0
#define I8259_SLAVE_MASK     0xA1

#define SIO_CONFIG_ADDR 0x3F0
#define SIO_CONFIG_DATA 0x3F1

#define SIO_WRITE_CONFIG(addr, byte) \
   out_byte(SIO_CONFIG_ADDR, addr);  \
   out_byte(SIO_CONFIG_DATA, byte);

#define SIO_READ_CONFIG(addr, byte) \
   out_byte(SIO_CONFIG_ADDR, addr); \
   byte = in_byte(SIO_CONFIG_DATA);

void via_init(void);

void via_calibrate_bus_freq(void);

#endif
