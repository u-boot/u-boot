#ifndef __SATA_H__
#define __SATA_H__
#include <part.h>

int init_sata(int dev);
int scan_sata(int dev);
ulong sata_read(int dev, ulong blknr, lbaint_t blkcnt, void *buffer);
ulong sata_write(int dev, ulong blknr, lbaint_t blkcnt, const void *buffer);

int sata_initialize(void);
int __sata_initialize(void);
int sata_port_status(int dev, int port);

extern block_dev_desc_t sata_dev_desc[];

#endif
