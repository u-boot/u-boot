/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2022 Sean Anderson <sean.anderson@seco.com>
 */

#ifndef NVMEM_H
#define NVMEM_H

/**
 * typedef nvmem_reg_read_t - Read a register from an nvmem device
 *
 * @dev: The device to read from
 * @offset: The offset of the register from the beginning of @dev
 * @buf: The buffer to read into
 * @size: The size of @buf, in bytes
 *
 * Return:
 * * 0 on success
 * * A negative error on failure
 */
typedef int (*nvmem_reg_read_t)(struct udevice *dev, unsigned int offset,
				void *buf, size_t size);

/**
 * typedef nvmem_reg_write_t - Write a register to an nvmem device
 * @dev: The device to write
 * @offset: The offset of the register from the beginning of @dev
 * @buf: The buffer to write
 * @size: The size of @buf, in bytes
 *
 * Return:
 * * 0 on success
 * * -ENOSYS if the device is read-only
 * * A negative error on other failures
 */
typedef int (*nvmem_reg_write_t)(struct udevice *dev, unsigned int offset,
				 const void *buf, size_t size);

/**
 * struct nvmem_cell - One datum within non-volatile memory
 * @nvmem: The backing storage device
 * @offset: The offset of the cell from the start of @nvmem
 * @size: The size of the cell, in bytes
 */
struct nvmem_cell {
	struct udevice *nvmem;
	unsigned int offset;
	size_t size;
};

struct udevice;

#if CONFIG_IS_ENABLED(NVMEM)

/**
 * nvmem_cell_read() - Read the value of an nvmem cell
 * @cell: The nvmem cell to read
 * @buf: The buffer to read into
 * @size: The size of @buf
 *
 * Return:
 * * 0 on success
 * * -EINVAL if @buf is not the same size as @cell.
 * * -ENOSYS if CONFIG_NVMEM is disabled
 * * A negative error if there was a problem reading the underlying storage
 */
int nvmem_cell_read(struct nvmem_cell *cell, void *buf, size_t size);

/**
 * nvmem_cell_write() - Write a value to an nvmem cell
 * @cell: The nvmem cell to write
 * @buf: The buffer to write from
 * @size: The size of @buf
 *
 * Return:
 * * 0 on success
 * * -EINVAL if @buf is not the same size as @cell
 * * -ENOSYS if @cell is read-only, or if CONFIG_NVMEM is disabled
 * * A negative error if there was a problem writing the underlying storage
 */
int nvmem_cell_write(struct nvmem_cell *cell, const void *buf, size_t size);

/**
 * nvmem_cell_get_by_index() - Get an nvmem cell from a given device and index
 * @dev: The device that uses the nvmem cell
 * @index: The index of the cell in nvmem-cells
 * @cell: The cell to initialize
 *
 * Look up the nvmem cell referenced by the phandle at @index in nvmem-cells in
 * @dev.
 *
 * Return:
 * * 0 on success
 * * -EINVAL if the regs property is missing, empty, or undersized
 * * -ENODEV if the nvmem device is missing or unimplemented
 * * -ENOSYS if CONFIG_NVMEM is disabled
 * * A negative error if there was a problem reading nvmem-cells or getting the
 *   device
 */
int nvmem_cell_get_by_index(struct udevice *dev, int index,
			    struct nvmem_cell *cell);

/**
 * nvmem_cell_get_by_name() - Get an nvmem cell from a given device and name
 * @dev: The device that uses the nvmem cell
 * @name: The name of the nvmem cell
 * @cell: The cell to initialize
 *
 * Look up the nvmem cell referenced by @name in the nvmem-cell-names property
 * of @dev.
 *
 * Return:
 * * 0 on success
 * * -EINVAL if the regs property is missing, empty, or undersized
 * * -ENODEV if the nvmem device is missing or unimplemented
 * * -ENODATA if @name is not in nvmem-cell-names
 * * -ENOSYS if CONFIG_NVMEM is disabled
 * * A negative error if there was a problem reading nvmem-cell-names,
 *   nvmem-cells, or getting the device
 */
int nvmem_cell_get_by_name(struct udevice *dev, const char *name,
			   struct nvmem_cell *cell);

#else /* CONFIG_NVMEM */

static inline int nvmem_cell_read(struct nvmem_cell *cell, void *buf, int size)
{
	return -ENOSYS;
}

static inline int nvmem_cell_write(struct nvmem_cell *cell, const void *buf,
				   int size)
{
	return -ENOSYS;
}

static inline int nvmem_cell_get_by_index(struct udevice *dev, int index,
					  struct nvmem_cell *cell)
{
	return -ENOSYS;
}

static inline int nvmem_cell_get_by_name(struct udevice *dev, const char *name,
					 struct nvmem_cell *cell)
{
	return -ENOSYS;
}

#endif /* CONFIG_NVMEM */

#endif /* NVMEM_H */
