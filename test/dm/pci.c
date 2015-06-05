/*
 * Copyright (C) 2015 Google, Inc
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <dm/test.h>
#include <test/ut.h>

/* Test that sandbox PCI works correctly */
static int dm_test_pci_base(struct unit_test_state *uts)
{
	struct udevice *bus;

	ut_assertok(uclass_get_device(UCLASS_PCI, 0, &bus));

	return 0;
}
DM_TEST(dm_test_pci_base, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test that sandbox PCI bus numbering works correctly */
static int dm_test_pci_busnum(struct unit_test_state *uts)
{
	struct udevice *bus;

	ut_assertok(uclass_get_device_by_seq(UCLASS_PCI, 0, &bus));

	return 0;
}
DM_TEST(dm_test_pci_busnum, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);

/* Test that we can use the swapcase device correctly */
static int dm_test_pci_swapcase(struct unit_test_state *uts)
{
	pci_dev_t pci_dev = PCI_BDF(0, 0x1f, 0);
	struct pci_controller *hose;
	struct udevice *bus, *swap;
	ulong io_addr, mem_addr;
	char *ptr;

	/* Check that asking for the device automatically fires up PCI */
	ut_assertok(uclass_get_device(UCLASS_PCI_EMUL, 0, &swap));

	ut_assertok(uclass_get_device(UCLASS_PCI, 0, &bus));
	hose = dev_get_uclass_priv(bus);

	/* First test I/O */
	io_addr = pci_read_bar32(hose, pci_dev, 0);
	outb(2, io_addr);
	ut_asserteq(2, inb(io_addr));

	/*
	 * Now test memory mapping - note we must unmap and remap to cause
	 * the swapcase emulation to see our data and response.
	 */
	mem_addr = pci_read_bar32(hose, pci_dev, 1);
	ptr = map_sysmem(mem_addr, 20);
	strcpy(ptr, "This is a TesT");
	unmap_sysmem(ptr);

	ptr = map_sysmem(mem_addr, 20);
	ut_asserteq_str("tHIS IS A tESt", ptr);
	unmap_sysmem(ptr);

	return 0;
}
DM_TEST(dm_test_pci_swapcase, DM_TESTF_SCAN_PDATA | DM_TESTF_SCAN_FDT);
