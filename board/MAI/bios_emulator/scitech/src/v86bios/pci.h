/*
 * Copyright 1999 Egbert Eich
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the authors not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  The authors makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THE AUTHORS DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE AUTHORS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
#include "v86bios.h"

#ifndef V86_PCI_H
#define V86_PCI_H

typedef union {
    struct {
	unsigned int    zero:2;
	unsigned int    reg:6;
	unsigned int    func:3;
	unsigned int    dev:5;
	unsigned int    bus:8;
	unsigned int    reserved:7;
	unsigned int    enable:1;
    } pci;
    CARD32 l;
} PciSlot;

typedef struct pciBusRec {
    CARD8 primary;
    CARD8 secondary;
    CARD32 bctl;
    PciSlot Slot;
    struct pciBusRec *next;
    struct pciBusRec *pBus;
} PciBusRec, *PciBusPtr;

typedef struct pciStructRec {
    CARD16 VendorID;
    CARD16 DeviceID;
    CARD8 Interface;
    CARD8 BaseClass;
    CARD8 SubClass;
    CARD32 RomBase;
    CARD32 bus;
    CARD8 dev;
    CARD8 func;
    CARD32 cmd_st;
    int active;
    PciSlot Slot;
    struct pciStructRec *next;
    PciBusPtr pBus;
} PciStructRec , *PciStructPtr;


extern PciStructPtr CurrentPci;
extern PciStructPtr PciList;
extern PciStructPtr BootBios;
extern int pciMaxBus;

extern CARD32 findPci(CARD16 slotBX);
extern CARD16 pciSlotBX(PciStructPtr);
PciStructPtr findPciDevice(CARD16 vendorID, CARD16 deviceID, char n);
PciStructPtr findPciClass(CARD8 intf, CARD8 subClass, CARD16 class, char n);

extern CARD8 PciRead8(int offset, CARD32 slot);
extern CARD16 PciRead16(int offset, CARD32 slot);
extern CARD32 PciRead32(int offset, CARD32 slot);

extern void PciWrite8(int offset,CARD8 byte, CARD32 slot);
extern void PciWrite16(int offset,CARD16 word, CARD32 slot);
extern void PciWrite32(int offset,CARD32 lg, CARD32 slot);

extern void scan_pci(void);
extern void pciVideoDisable(void);
extern void pciVideoRestore(void);
extern void EnableCurrent(void);
extern int mapPciRom(PciStructPtr pciP);
extern int cfg1out(CARD16 addr, CARD32 val);
extern int cfg1in(CARD16 addr, CARD32 *val);
extern void list_pci(void);
extern PciStructPtr findPciByIDs(int bus, int dev, int func);

#define PCI_MODE2_ENABLE_REG  0xCF8
#define PCI_MODE2_FORWARD_REG  0xCFA
#define PCI_MODE1_ADDRESS_REG  0xCF8
#define PCI_MODE1_DATA_REG  0xCFC
#if defined(__alpha__) || defined(__sparc__)
#define PCI_EN 0x00000000
#else
#define PCI_EN 0x80000000
#endif
#define MAX_DEV_PER_VENDOR_CFG1  32
#define BRIDGE_CLASS(x) (x == 0x06)
#define BRIDGE_PCI_CLASS(x) (x == 0x04)
#define BRIDGE_HOST_CLASS(x) (x == 0x00)
#define PCI_CLASS_PREHISTORIC 0x00
#define PCI_SUBCLASS_PREHISTORIC_VGA 0x01
#define PCI_CLASS_DISPLAY 0x03
#define PCI_SUBCLASS_DISPLAY_VGA 0x00
#define PCI_SUBCLASS_DISPLAY_XGA 0x01
#define PCI_SUBCLASS_DISPLAY_MISC 0x80
#define VIDEO_CLASS(b,s) \
    (((b) == PCI_CLASS_PREHISTORIC && (s) == PCI_SUBCLASS_PREHISTORIC_VGA) || \
     ((b) == PCI_CLASS_DISPLAY && (s) == PCI_SUBCLASS_DISPLAY_VGA) ||\
     ((b) == PCI_CLASS_DISPLAY && (s) == PCI_SUBCLASS_DISPLAY_XGA) ||\
     ((b) == PCI_CLASS_DISPLAY && (s) == PCI_SUBCLASS_DISPLAY_MISC))
#define PCI_MULTIFUNC_DEV  0x80
#define MAX_PCI_DEVICES   64
#define PCI_MAXBUS 16
#define PCI_IS_MEM 0x00000001
#define MAX_PCI_ROM_SIZE (1024 * 1024 * 16)

#define IS_MEM32(x) ((x & 0x7) == 0 && x != 0)
#define IS_MEM64(x) ((x & 0x7) == 0x4)
#endif
