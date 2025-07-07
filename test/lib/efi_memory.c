// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Test memory functions
 *
 * Copyright (c) 2025 Heinrich Schuchardt <xypron.glpk@gmx.de>
 */

#include <efi_loader.h>
#include <test/lib.h>
#include <test/test.h>
#include <test/ut.h>

static int lib_test_efi_alloc_aligned_pages(struct unit_test_state *uts)
{
	efi_status_t ret;

	void *addr;
	unsigned long align = 0x400000;

	addr = efi_alloc_aligned_pages(4096, EFI_PERSISTENT_MEMORY_TYPE,
				       EFI_PAGE_SIZE);
	ut_asserteq_ptr(NULL, addr);

	addr = efi_alloc_aligned_pages(4096, 0x6FFFFFFF, EFI_PAGE_SIZE);
	ut_asserteq_ptr(NULL, addr);

	align = 0x200;
	addr = efi_alloc_aligned_pages(4096, EFI_ACPI_RECLAIM_MEMORY, align);
	ut_assertnonnull(addr);
	ut_asserteq_64(0, (uintptr_t)addr & (align - 1));

	ret = efi_free_pages((uintptr_t) addr, 1);
	ut_asserteq_64(ret, EFI_SUCCESS);

	align = 0x400000;
	addr = efi_alloc_aligned_pages(4096, EFI_ACPI_RECLAIM_MEMORY, align);
	ut_assertnonnull(addr);
	ut_asserteq_64(0, (uintptr_t)addr & (align - 1));

	ret = efi_free_pages((uintptr_t) addr, 1);
	ut_asserteq_64(ret, EFI_SUCCESS);

	return 0;
}
LIB_TEST(lib_test_efi_alloc_aligned_pages, 0);

static int lib_test_efi_allocate_pages(struct unit_test_state *uts)
{
	efi_status_t ret;
	u64 memory;

	ret = efi_allocate_pages(EFI_ALLOCATE_ANY_PAGES,
				 EFI_ACPI_RECLAIM_MEMORY,
				 1, &memory);
	ut_asserteq_64(ret, EFI_SUCCESS);
	ut_asserteq_64(0, memory & EFI_PAGE_MASK);

	ret = efi_free_pages(memory, 1);
	ut_asserteq_64(ret, EFI_SUCCESS);

	return 0;
}
LIB_TEST(lib_test_efi_allocate_pages, 0);
