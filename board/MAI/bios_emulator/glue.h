#ifndef GLUE_H
#define GLUE_H

typedef unsigned int pci_dev_t;

int mypci_find_device(int vendor, int product, int index);
int mypci_bus(int device);
int mypci_devfn(int device);
unsigned long get_bar_size(pci_dev_t dev, int offset);

u8   mypci_read_cfg_byte(int bus, int devfn, int offset);
u16  mypci_read_cfg_word(int bus, int devfn, int offset);
u32  mypci_read_cfg_long(int bus, int devfn, int offset);

void mypci_write_cfg_byte(int bus, int devfn, int offset, u8 value);
void mypci_write_cfg_word(int bus, int devfn, int offset, u16 value);
void mypci_write_cfg_long(int bus, int devfn, int offset, u32 value);

void _printf(const char *fmt, ...);
char *_getenv(char *name);

void *malloc(size_t size);
void memset(void *addr, int value, size_t size);
void memcpy(void *to, void *from, size_t numbytes);
int  strcmp(char *, char *);

void enable_compatibility_hole(void);
void disable_compatibility_hole(void);

void map_rom(pci_dev_t dev, unsigned long address);
void unmap_rom(pci_dev_t dev);
int  attempt_map_rom(pci_dev_t dev, void *copy_address);

#define  PCI_BASE_ADDRESS_SPACE 0x01    /* 0 = memory, 1 = I/O */
#define  PCI_BASE_ADDRESS_SPACE_IO 0x01
#define  PCI_BASE_ADDRESS_SPACE_MEMORY 0x00
#define  PCI_BASE_ADDRESS_MEM_MASK      (~0x0fUL)

#define PCI_BASE_ADDRESS_0      0x10    /* 32 bits */
#define PCI_BASE_ADDRESS_1      0x14    /* 32 bits [htype 0,1 only] */
#define PCI_BASE_ADDRESS_2      0x18    /* 32 bits [htype 0 only] */
#define PCI_BASE_ADDRESS_3      0x1c    /* 32 bits */
#define PCI_BASE_ADDRESS_4      0x20    /* 32 bits */
#define PCI_BASE_ADDRESS_5      0x24    /* 32 bits */
#define PCI_BUS(d)      (((d) >> 16) & 0xff)
#define PCI_DEV(d)      (((d) >> 11) & 0x1f)
#define PCI_FUNC(d)     (((d) >> 8) & 0x7)
#define PCI_BDF(b,d,f)  ((b) << 16 | (d) << 11 | (f) << 8)

#define PCI_ANY_ID (~0)
#define PCI_ROM_ADDRESS         0x30    /* Bits 31..11 are address, 10..1 reserved */
#define PCI_ROM_ADDRESS_ENABLE 0x01

#define OFF(addr) ((addr) & 0xFFFF)
#define SEG(addr) (((addr)>>4) &0xF000)

#endif
