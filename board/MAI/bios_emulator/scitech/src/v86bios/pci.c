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
#include "debug.h"
#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#if defined (__alpha__) || defined (__ia64__)
#include <sys/io.h>
#endif
#include "AsmMacros.h"

#include "pci.h"

/*
 * I'm rather simple mindend - therefore I do a poor man's
 * pci scan without all the fancy stuff that is done in
 * scanpci. However that's all we need.
 */

PciStructPtr PciStruct = NULL;
PciBusPtr PciBuses = NULL;
PciStructPtr CurrentPci = NULL;
PciStructPtr PciList = NULL;
PciStructPtr BootBios = NULL;
int pciMaxBus = 0;

static CARD32 PciCfg1Addr;

static void readConfigSpaceCfg1(CARD32 bus, CARD32 dev, CARD32 func,
		CARD32 *reg);
static int checkSlotCfg1(CARD32 bus, CARD32 dev, CARD32 func);
static int checkSlotCfg2(CARD32 bus, int dev);
static void readConfigSpaceCfg2(CARD32 bus, int dev, CARD32 *reg);
static CARD8 interpretConfigSpace(CARD32 *reg, int busidx,
		  CARD8 dev, CARD8 func);
static CARD32 findBIOSMap(PciStructPtr pciP, CARD32 *biosSize);
static void restoreMem(PciStructPtr pciP);


#ifdef __alpha__
#define PCI_BUS_FROM_TAG(tag)  (((tag) & 0x00ff0000) >> 16)
#define PCI_DFN_FROM_TAG(tag) (((tag) & 0x0000ff00) >> 8)

#include <asm/unistd.h>

CARD32
axpPciCfgRead(CARD32 tag)
{
    int bus, dfn;
    CARD32 val = 0xffffffff;

    bus = PCI_BUS_FROM_TAG(tag);
    dfn = PCI_DFN_FROM_TAG(tag);

    syscall(__NR_pciconfig_read, bus, dfn, tag & 0xff, 4, &val);
    return(val);
}

void
axpPciCfgWrite(CARD32 tag, CARD32 val)
{
    int bus, dfn;

    bus = PCI_BUS_FROM_TAG(tag);
    dfn = PCI_DFN_FROM_TAG(tag);

    syscall(__NR_pciconfig_write, bus, dfn, tag & 0xff, 4, &val);
}

static CARD32 (*readPci)(CARD32 reg) = axpPciCfgRead;
static void (*writePci)(CARD32 reg, CARD32 val) = axpPciCfgWrite;
#else
static CARD32 readPciCfg1(CARD32 reg);
static void writePciCfg1(CARD32 reg, CARD32 val);
static CARD32 readPciCfg2(CARD32 reg);
static void writePciCfg2(CARD32 reg, CARD32 val);

static CARD32 (*readPci)(CARD32 reg) = readPciCfg1;
static void (*writePci)(CARD32 reg, CARD32 val) = writePciCfg1;
#endif

#if defined(__alpha__) || defined(__sparc__)
#define PCI_EN 0x00000000
#else
#define PCI_EN 0x80000000
#endif


static int numbus;
static int hostbridges = 1;
static unsigned long pciMinMemReg = ~0;


void
scan_pci(void)
{
    unsigned short configtype;

    CARD32 reg[64];
    int busidx;
    CARD8 cardnum;
    CARD8 func;
    int idx;

    int i;
    PciStructPtr pci1;
    PciBusPtr pci_b1,pci_b2;

#if defined(__alpha__) || defined(__powerpc__) || defined(__sparc__) || defined(__ia64__)
    configtype = 1;
#else
    CARD8 tmp1, tmp2;
    CARD32 tmp32_1, tmp32_2;
    outb(PCI_MODE2_ENABLE_REG, 0x00);
    outb(PCI_MODE2_FORWARD_REG, 0x00);
    tmp1 = inb(PCI_MODE2_ENABLE_REG);
    tmp2 = inb(PCI_MODE2_FORWARD_REG);
    if ((tmp1 == 0x00) && (tmp2 == 0x00)) {
	configtype = 2;
	readPci = readPciCfg2;
	writePci = writePciCfg2;
	P_printf("PCI says configuration type 2\n");
    } else {
	tmp32_1 = inl(PCI_MODE1_ADDRESS_REG);
	outl(PCI_MODE1_ADDRESS_REG, PCI_EN);
	tmp32_2 = inl(PCI_MODE1_ADDRESS_REG);
	outl(PCI_MODE1_ADDRESS_REG, tmp32_1);
	if (tmp32_2 == PCI_EN) {
	    configtype = 1;
	    P_printf("PCI says configuration type 1\n");
	} else {
	    P_printf("No PCI !\n");
	    return;
	}
    }
#endif

    if (configtype == 1) {
	P_printf("PCI probing configuration type 1\n");
	busidx = 0;
	numbus = 1;
	idx = 0;
	do {
	    P_printf("\nProbing for devices on PCI bus %d:\n", busidx);
	    for (cardnum = 0; cardnum < MAX_DEV_PER_VENDOR_CFG1; cardnum++) {
		func = 0;
		do {
		    /* loop over the different functions, if present */
		    if (!checkSlotCfg1(busidx,cardnum,func))
			break;
		    readConfigSpaceCfg1(busidx,cardnum,func,reg);

		    func = interpretConfigSpace(reg,busidx,
						cardnum,func);

		    if (idx++ > MAX_PCI_DEVICES)
			continue;
		} while (func < 8);
	    }
	} while (++busidx < PCI_MAXBUS);
#if defined(__alpha__) || defined(__powerpc__) || defined(__sparc__) || defined(__ia64__)
	/* don't use outl()  ;-) */
#else
	outl(PCI_MODE1_ADDRESS_REG, 0);
#endif
    } else {
	int slot;

	P_printf("PCI probing configuration type 2\n");
	busidx = 0;
	numbus = 1;
	idx = 0;
	do {
	    for (slot=0xc0; slot<0xd0; i++) {
		if (!checkSlotCfg2(busidx,slot))
		    break;
		readConfigSpaceCfg2(busidx,slot,reg);

		interpretConfigSpace(reg,busidx,
				     slot,0);
		if (idx++ > MAX_PCI_DEVICES)
		    continue;
	    }
	}  while (++busidx < PCI_MAXBUS);
    }


    pciMaxBus = numbus - 1;
    P_printf("Number of buses in system: %i\n",pciMaxBus + 1);
    P_printf("Min PCI mem address: 0x%lx\n",pciMinMemReg);

    /* link buses */
    pci_b1 = PciBuses;
    while (pci_b1) {
	pci_b2 = PciBuses;
	pci_b1->pBus = NULL;
	while (pci_b2) {
	    if (pci_b1->primary == pci_b2->secondary)
		pci_b1->pBus = pci_b2;
	    pci_b2 = pci_b2->next;
	}
	pci_b1 = pci_b1->next;
    }
    pci1 = PciStruct;
    while (pci1) {
	pci_b2 = PciBuses;
	pci1->pBus = NULL;
	while (pci_b2) {
	    if (pci1->bus == pci_b2->secondary)
		pci1->pBus = pci_b2;
	    pci_b2 = pci_b2->next;
	}
	pci1 = pci1->next;
    }
    if (RESORT) {
	PciStructPtr tmp = PciStruct, tmp1;
	PciStruct = NULL;
	while (tmp) {
	    tmp1 = tmp->next;
	    tmp->next = PciStruct;
	    PciStruct = tmp;
	    tmp = tmp1;
	}
    }
    PciList = CurrentPci = PciStruct;
}

#ifndef __alpha__
static CARD32
readPciCfg1(CARD32 reg)
{
    CARD32 val;

    outl(PCI_MODE1_ADDRESS_REG, reg);
    val = inl(PCI_MODE1_DATA_REG);
    outl(PCI_MODE1_ADDRESS_REG, 0);
    P_printf("reading: 0x%x from 0x%x\n",val,reg);
    return val;
}

static void
writePciCfg1(CARD32 reg, CARD32 val)
{
    P_printf("writing: 0x%x to 0x%x\n",val,reg);
    outl(PCI_MODE1_ADDRESS_REG, reg);
    outl(PCI_MODE1_DATA_REG,val);
    outl(PCI_MODE1_ADDRESS_REG, 0);
}

static CARD32
readPciCfg2(CARD32 reg)
{
    CARD32 val;
    CARD8 bus = (reg >> 16) & 0xff;
    CARD8 dev = (reg >> 11) & 0x1f;
    CARD8 num = reg & 0xff;

    outb(PCI_MODE2_ENABLE_REG, 0xF1);
    outb(PCI_MODE2_FORWARD_REG, bus);
    val = inl((dev << 8) + num);
    outb(PCI_MODE2_ENABLE_REG, 0x00);
    P_printf("reading: 0x%x from 0x%x\n",val,reg);
    return val;
}

static void
writePciCfg2(CARD32 reg, CARD32 val)
{
    CARD8 bus = (reg >> 16) & 0xff;
    CARD8 dev = (reg >> 11) & 0x1f;
    CARD8 num = reg & 0xff;

    P_printf("writing: 0x%x to 0x%x\n",val,reg);
    outb(PCI_MODE2_ENABLE_REG, 0xF1);
    outb(PCI_MODE2_FORWARD_REG, bus);
    outl((dev << 8) + num,val);
    outb(PCI_MODE2_ENABLE_REG, 0x00);
}
#endif

void
pciVideoDisable(void)
{
    /* disable VGA routing on bridges */
    PciBusPtr pbp = PciBuses;
    PciStructPtr pcp = PciStruct;

    while (pbp) {
	writePci(pbp->Slot.l | 0x3c, pbp->bctl & ~(CARD32)(8<<16));
	pbp = pbp->next;
    }
    /* disable display devices */
    while (pcp) {
	writePci(pcp->Slot.l | 0x04, pcp->cmd_st & ~(CARD32)3);
	writePci(pcp->Slot.l | 0x30, pcp->RomBase & ~(CARD32)1);
	pcp = pcp->next;
    }
}

void
pciVideoRestore(void)
{
    /* disable VGA routing on bridges */
    PciBusPtr pbp = PciBuses;
    PciStructPtr pcp = PciStruct;

    while (pbp) {
	writePci(pbp->Slot.l | 0x3c, pbp->bctl);
	pbp = pbp->next;
    }
    /* disable display devices */
    while (pcp) {
	writePci(pcp->Slot.l | 0x04, pcp->cmd_st);
	writePci(pcp->Slot.l | 0x30, pcp->RomBase);
	pcp = pcp->next;
    }
}

void
EnableCurrent()
{
    PciBusPtr pbp;
    PciStructPtr pcp = CurrentPci;

    pciVideoDisable();

    pbp = pcp->pBus;
    while (pbp) { /* enable bridges */
	writePci(pbp->Slot.l | 0x3c, pbp->bctl | (CARD32)(8<<16));
	pbp = pbp->pBus;
    }
    writePci(pcp->Slot.l | 0x04, pcp->cmd_st | (CARD32)3);
    writePci(pcp->Slot.l | 0x30, pcp->RomBase | (CARD32)1);
}

CARD8
PciRead8(int offset, CARD32 Slot)
{
    int shift = offset & 0x3;
    offset = offset & 0xFC;
    return ((readPci(Slot | offset) >> (shift << 3)) & 0xff);
}

CARD16
PciRead16(int offset, CARD32 Slot)
{
    int shift = offset & 0x2;
    offset = offset & 0xFC;
    return ((readPci(Slot | offset) >> (shift << 3)) & 0xffff);
}

CARD32
PciRead32(int offset, CARD32 Slot)
{
    offset = offset & 0xFC;
    return (readPci(Slot | offset));
}

void
PciWrite8(int offset, CARD8 byte, CARD32 Slot)
{
    CARD32 val;
    int shift = offset & 0x3;
    offset = offset & 0xFC;
    val = readPci(Slot | offset);
    val &= ~(CARD32)(0xff << (shift << 3));
    val |= byte << (shift << 3);
    writePci(Slot | offset, val);
}

void
PciWrite16(int offset, CARD16 word, CARD32 Slot)
{
    CARD32 val;
    int shift = offset & 0x2;
    offset = offset & 0xFC;
    val = readPci(Slot | offset);
    val &= ~(CARD32)(0xffff << (shift << 3));
    val |= word << (shift << 3);
    writePci(Slot | offset, val);
}

void
PciWrite32(int offset, CARD32 lg, CARD32 Slot)
{
    offset = offset & 0xFC;
    writePci(Slot | offset, lg);
}

int
mapPciRom(PciStructPtr pciP)
{
    unsigned long RomBase = 0;
    int mem_fd;
    unsigned char *mem, *ptr;
    unsigned char *scratch = NULL;
    int length = 0;
    CARD32 biosSize = 0x1000000;
    CARD32 enablePci;

    if (!pciP)
      pciP = CurrentPci;

    if (FIX_ROM) {
	RomBase = findBIOSMap(pciP, &biosSize);
	if (!RomBase) {
	    fprintf(stderr,"Cannot remap BIOS of %i:%i:%i "
		    "- trying preset address\n",pciP->bus,pciP->dev,
		    pciP->func);
	    RomBase = pciP->RomBase & ~(CARD32)0xFF;
	}
    }  else {
	RomBase = pciP->RomBase & ~(CARD32)0xFF;
	if (~RomBase + 1 < biosSize || !RomBase)
	    RomBase = findBIOSMap(pciP, &biosSize);
    }

    P_printf("RomBase: 0x%lx\n",RomBase);

    if ((mem_fd = open(MEM_FILE,O_RDONLY))<0) {
	perror("opening memory");
	restoreMem(pciP);
	return (0);
    }

    PciWrite32(0x30,RomBase | 1,pciP->Slot.l);

#ifdef __alpha__
    mem = ptr = (unsigned char *)mmap(0, biosSize, PROT_READ,
				      MAP_SHARED, mem_fd, RomBase | _bus_base());
#else
    mem = ptr = (unsigned char *)mmap(0, biosSize, PROT_READ,
				      MAP_SHARED, mem_fd, RomBase);
#endif
    if (pciP != CurrentPci) {
      enablePci = PciRead32(0x4,pciP->Slot.l);
      PciWrite32(0x4,enablePci | 0x2,pciP->Slot.l);
    }

#ifdef PRINT_PCI
    dprint((unsigned long)ptr,0x30);
#endif
    while ( *ptr == 0x55 && *(ptr+1) == 0xAA) {
	unsigned short data_off = *(ptr+0x18) | (*(ptr+0x19)<< 8);
	unsigned char *data = ptr + data_off;
	unsigned char type;
	int i;

	if (*data!='P' || *(data+1)!='C' || *(data+2)!='I' || *(data+3)!='R') {
	    break;
	}
	type = *(data + 0x14);
	P_printf("data segment in BIOS: 0x%x, type: 0x%x ",data_off,type);

	if (type != 0)  { /* not PC-AT image: find next one */
	    unsigned int image_length;
	    unsigned char indicator = *(data + 0x15);
	    if (indicator & 0x80) /* last image */
		break;
	    image_length = (*(data + 0x10)
			    | (*(data + 0x11) << 8)) << 9;
	    P_printf("data image length: 0x%x, ind: 0x%x\n",
		     image_length,indicator);
	    ptr = ptr + image_length;
	    continue;
	}
	/* OK, we have a PC Image */
	length = (*(ptr + 2) << 9);
	P_printf("BIOS length: 0x%x\n",length);
	scratch = (unsigned char *)malloc(length);
	/* don't use memcpy() here: Reading from bus! */
	for (i=0;i<length;i++)
	    *(scratch + i)=*(ptr + i);
	break;
    }

    if (pciP != CurrentPci)
      PciWrite32(0x4,enablePci,pciP->Slot.l);

    /* unmap/close/disable PCI bios mem */
    munmap(mem, biosSize);
    close(mem_fd);
    /* disable and restore mapping */
    writePci(pciP->Slot.l | 0x30, pciP->RomBase & ~(CARD32)1);

    if (scratch && length) {
	memcpy((unsigned char *)V_BIOS, scratch, length);
	free(scratch);
    }

    restoreMem(pciP);
    return length;
}

CARD32
findPci(CARD16 slotBX)
{
    CARD32 slot = slotBX << 8;

    if (slot == (CurrentPci->Slot.l & ~PCI_EN))
	return (CurrentPci->Slot.l | PCI_EN);
    else {
#if !SHOW_ALL_DEV
	PciBusPtr pBus = CurrentPci->pBus;
	while (pBus) {
	  /*      fprintf(stderr,"slot: 0x%x  bridge: 0x%x\n",slot, pBus->Slot.l); */
	    if (slot == (pBus->Slot.l & ~PCI_EN))
		return pBus->Slot.l | PCI_EN;
	    pBus = pBus->next;
	}
#else
	PciStructPtr pPci = PciStruct;
	while (pPci) {
	  /*fprintf(stderr,"slot: 0x%x  bridge: 0x%x\n",slot, pPci->Slot.l); */
	    if (slot == (pPci->Slot.l & ~PCI_EN))
		return pPci->Slot.l | PCI_EN;
	    pPci = pPci->next;
	}
#endif
    }
    return 0;
}

CARD16
pciSlotBX(PciStructPtr pPci)
{
    return (CARD16)((pPci->Slot.l >> 8) & 0xFFFF);
}

PciStructPtr
findPciDevice(CARD16 vendorID, CARD16 deviceID, char n)
{
    PciStructPtr pPci = CurrentPci;
    n++;

    while (pPci)  {
	if ((pPci->VendorID == vendorID) && (pPci->DeviceID == deviceID)) {
	if (!(--n)) break;
	}
    pPci = pPci->next;
    }
    return pPci;
}

PciStructPtr
findPciClass(CARD8 intf, CARD8 subClass, CARD16 class, char n)
{
    PciStructPtr pPci = CurrentPci;
    n++;

    while (pPci)  {
	if ((pPci->Interface == intf) && (pPci->SubClass == subClass)
	 && (pPci->BaseClass == class)) {
	if (!(--n)) break;
	}
    pPci = pPci->next;
    }
    return pPci;
}

static void
readConfigSpaceCfg1(CARD32 bus, CARD32 dev, CARD32 func, CARD32 *reg)
{
    CARD32   config_cmd = PCI_EN | (bus<<16) |
      (dev<<11) | (func<<8);
    int i;

    for (i = 0; i<64;i+=4) {
#ifdef __alpha__
	reg[i] = axpPciCfgRead(config_cmd | i);
#else
	outl(PCI_MODE1_ADDRESS_REG, config_cmd | i);
	reg[i] = inl(PCI_MODE1_DATA_REG);
#endif

#ifdef V86BIOS_DEBUG
	P_printf("0x%lx\n",reg[i]);
#endif
    }
}

static int
checkSlotCfg1(CARD32 bus, CARD32 dev, CARD32 func)
{
    CARD32    config_cmd = PCI_EN | (bus<<16) |
      (dev<<11) | (func<<8);
    CARD32 reg;
#ifdef __alpha__
	reg = axpPciCfgRead(config_cmd);
#else
	outl(PCI_MODE1_ADDRESS_REG, config_cmd);
	reg = inl(PCI_MODE1_DATA_REG);
#endif
    if (reg != 0xFFFFFFFF)
	return 1;
    else
	return 0;
}

static int
checkSlotCfg2(CARD32 bus, int dev)
{
    CARD32 val;

    outb(PCI_MODE2_ENABLE_REG, 0xF1);
    outb(PCI_MODE2_FORWARD_REG, bus);
    val = inl(dev << 8);
    outb(PCI_MODE2_FORWARD_REG, 0x00);
    outb(PCI_MODE2_ENABLE_REG, 0x00);
    if (val == 0xFFFFFFFF)
	return 0;
    if (val == 0xF0F0F0F0)
	return 0;
    return 1;
}

static void
readConfigSpaceCfg2(CARD32 bus, int dev, CARD32 *reg)
{
    int i;

    outb(PCI_MODE2_ENABLE_REG, 0xF1);
    outb(PCI_MODE2_FORWARD_REG, bus);
    for (i = 0; i<64;i+=4) {
	reg[i] = inl((dev << 8) + i);
#ifdef V86BIOS_DEBUG
	P_printf("0x%lx\n",reg[i]);
#endif
    }
    outb(PCI_MODE2_ENABLE_REG, 0x00);
}

static CARD8
interpretConfigSpace(CARD32 *reg, int busidx, CARD8 dev, CARD8 func)
{
    CARD32 config_cmd;
    CARD16 vendor, device;
    CARD8 baseclass, subclass;
    CARD8 primary, secondary;
    CARD8 header, interface;
    int i;

    config_cmd = PCI_EN | busidx<<16 |
	(dev<<11) | (func<<8);

    for (i = 0x10; i < 0x28; i+=4) {
	if (IS_MEM32(reg[i]))
	    if ((reg[i] & 0xFFFFFFF0) < pciMinMemReg)
		pciMinMemReg = (reg[i] & 0xFFFFFFF0);
#ifdef __alpha__
	if (IS_MEM64(reg[i])) {
		unsigned long addr = reg[i] |
	      (unsigned long)(reg[i+4]) << 32;
	    if ((addr & ~0xfL) < pciMinMemReg)
		pciMinMemReg = (addr & ~0xfL);
	    i+=4;
	}
#endif
    }
    vendor = reg[0] & 0xFFFF;
    device = reg[0] >> 16;
    P_printf("bus: %i card: %i func %i reg0: 0x%x ", busidx,dev,func,reg[0]);
    baseclass = reg[8] >> 24;
    subclass = (reg[8] >> 16) & 0xFF;
    interface = (reg[8] >> 8) & 0xFF;

    header = (reg[0x0c] >> 16) & 0xff;
    P_printf("bc 0x%x, sub 0x%x, if 0x%x, hdr 0x%x\n",
	     baseclass,subclass,interface,header);
    if (BRIDGE_CLASS(baseclass)) {
	if (BRIDGE_PCI_CLASS(subclass)) {
	    PciBusPtr pbp = malloc(sizeof(PciBusRec));
	    P_printf("Pci-Pci Bridge found; ");
	    primary = reg[0x18] & 0xFF;
	    secondary = (reg[0x18] >> 8) & 0xFF;
	    P_printf("primary: 0x%x secondary: 0x%x\n",
		     primary,secondary);
	    pbp->bctl = reg[0x3c];
	    pbp->primary = primary;
	    pbp->secondary = secondary;
	    pbp->Slot.l = config_cmd;
	    pbp->next = PciBuses;
	    PciBuses = pbp;
	    numbus++;
	} else if (BRIDGE_HOST_CLASS(subclass)
		   && (hostbridges++ > 1)) {
	    numbus++;
	}
    } else if (VIDEO_CLASS(baseclass,subclass)) {
	PciStructPtr pcp = malloc(sizeof(PciStructRec));
	P_printf("Display adapter found\n");
	pcp->RomBase = reg[0x30];
	pcp->cmd_st = reg[4];
	pcp->active = (reg[4] & 0x03) == 3 ? 1 : 0;
	pcp->VendorID = vendor;
	pcp->DeviceID = device;
	pcp->Interface = interface;
	pcp->BaseClass = baseclass;
	pcp->SubClass = subclass;
	pcp->Slot.l = config_cmd;
	pcp->bus = busidx;
	pcp->dev = dev;
	pcp->func = func;
	pcp->next = PciStruct;
	PciStruct = pcp;
    }
    if ((func == 0)
	&& ((header & PCI_MULTIFUNC_DEV) == 0))
	func = 8;
    else
	func++;
    return func;
}

static CARD32 remapMEM_val;
static int remapMEM_num;

static int /* map it on some other video device */
remapMem(PciStructPtr pciP, int num, CARD32 size)
{
    PciStructPtr pciPtr = PciStruct;
    int i;
    CARD32 org;
    CARD32 val;
    CARD32 size_n;

    org = PciRead32(num + 0x10,pciP->Slot.l);

    while (pciPtr) {
	for (i = 0; i < 20; i=i+4) {

	    val = PciRead32(i + 0x10,pciPtr->Slot.l);
	    /* don't map it on itself */
	    if ((org & 0xfffffff0) == (val & 0xfffffff0))
		continue;
	    if (val && !(val & 1))
		PciWrite32(i + 0x10,0xffffffff,pciPtr->Slot.l);
	    else
		continue;
	    size_n = PciRead32(i + 0x10,pciPtr->Slot.l);
	    PciWrite32(i + 0x10,val,pciPtr->Slot.l);
	    size_n = ~(CARD32)(size_n  & 0xfffffff0) + 1;

	    if (size_n >= size) {
		PciWrite32(num + 0x10,val,pciP->Slot.l);
		return 1;
	    }
	}
	pciPtr = pciPtr->next;
    }
    /* last resort: try to go below lowest PCI mem address */
    val = ((pciMinMemReg & ~(CARD32)(size - 1)) - size);
    if (val > 0x7fffffff) {
	PciWrite32(num + 0x10,val, pciP->Slot.l);
	return 1;
    }

    return 0;
}

static void
restoreMem(PciStructPtr pciP)
{
    if (remapMEM_val == 0) return;
    PciWrite32(remapMEM_num + 0x10,remapMEM_val,pciP->Slot.l);
    return;
}

static CARD32
findBIOSMap(PciStructPtr pciP, CARD32 *biosSize)
{
    PciStructPtr pciPtr = PciStruct;
    int i;
    CARD32 val;
    CARD32 size;

    PciWrite32(0x30,0xffffffff,pciP->Slot.l);
    *biosSize = PciRead32(0x30,pciP->Slot.l);
    P_printf("bios size: 0x%x\n",*biosSize);
    PciWrite32(0x30,pciP->RomBase,pciP->Slot.l);
    *biosSize = ~(*biosSize & 0xFFFFFF00) + 1;
    P_printf("bios size masked: 0x%x\n",*biosSize);
    if (*biosSize > (1024 * 1024 * 16)) {
      *biosSize = 1024 * 1024 * 16;
      P_printf("fixing broken BIOS size: 0x%x\n",*biosSize);
    }
    while (pciPtr) {
	if (pciPtr->bus != pciP->bus) {
	    pciPtr = pciPtr->next;
	    continue;
	}
	for (i = 0; i < 20; i=i+4) {

	    val = PciRead32(i + 0x10,pciPtr->Slot.l);
	    if (!(val & 1))

	    PciWrite32(i + 0x10,0xffffffff,pciPtr->Slot.l);
	    else
		continue;
	    size = PciRead32(i + 0x10,pciPtr->Slot.l);
	    PciWrite32(i + 0x10,val,pciPtr->Slot.l);
	    size = ~(CARD32)(size & 0xFFFFFFF0) + 1;
#ifdef V86_BIOS_DEBUG
	    P_printf("size: 0x%x\n",size);
#endif
	    if (size >= *biosSize) {
		if (pciP == pciPtr) { /* if same device remap ram*/
		    if (!(remapMem(pciP,i,size)))
			continue;
		    remapMEM_val = val;
		    remapMEM_num = i;
		} else {
		    remapMEM_val = 0;
		}
		return val & 0xFFFFFF00;
	    }
	}
	pciPtr = pciPtr->next;
    }
    remapMEM_val = 0;
    /* very last resort */
    if (pciP->bus == 0 && (pciMinMemReg > *biosSize))
      return (pciMinMemReg - size) & ~(size - 1);

    return 0;
}

int
cfg1out(CARD16 addr, CARD32 val)
{
  if (addr == 0xCF8) {
    PciCfg1Addr = val;
    return 1;
  } else if (addr == 0xCFC) {
    writePci(PciCfg1Addr, val);
    return 1;
  }
  return 0;
}

int
cfg1in(CARD16 addr, CARD32 *val)
{
  if (addr == 0xCF8) {
    *val = PciCfg1Addr;
    return 1;
  } else if (addr == 0xCFC) {
    *val = readPci(PciCfg1Addr);
    return 1;
  }
  return 0;
}

void
list_pci(void)
{
    PciStructPtr pci = PciList;

    while (pci) {
    printf("[0x%x:0x%x:0x%x] vendor: 0x%4.4x dev: 0x%4.4x class: 0x%4.4x"
	   " subclass: 0x%4.4x\n",pci->bus,pci->dev,pci->func,
	   pci->VendorID,pci->DeviceID,pci->BaseClass,pci->SubClass);
    pci = pci->next;
    }
}

PciStructPtr
findPciByIDs(int bus, int dev, int func)
{
  PciStructPtr pciP = PciList;

  while (pciP) {
    if (pciP->bus == bus && pciP->dev == dev && pciP->func == func)
      return pciP;
    pciP = pciP->next;
  }
  return NULL;
}
