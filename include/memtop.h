/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2024, Linaro Limited
 */

/**
 * get_mem_top() - Compute the value of ram_top
 * @ram_start:	Start of RAM
 * @ram_size:	RAM size
 * @size:	Minimum RAM size requested
 * @fdt:	FDT blob
 *
 * The function computes the top address of RAM memory that can be
 * used by U-Boot. This is being done by going through the list of
 * reserved memory regions specified in the devicetree blob passed
 * to the function. The logic used here is derived from the lmb
 * allocation function.
 *
 * Return: address of ram top on success, 0 on failure
 */
phys_addr_t get_mem_top(phys_addr_t ram_start, phys_size_t ram_size,
			phys_size_t size, void *fdt);
