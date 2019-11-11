/*
 * (C) Copyright 2019, Foundries.IO
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#ifndef	_FIOVB_H
#define _FIOVB_H

#include <common.h>
#include <linux/types.h>
/*
 * FIOVB_IO_RESULT_OK
 * FIOVB_IO_RESULT_ERROR_IO: hardware I/O error.
 * FIOVB_IO_RESULT_ERROR_OOM:  unable to allocate memory.
 * FIOVB_IO_RESULT_ERROR_NO_SUCH_VALUE: persistent value does not exist.
 * FIOVB_IO_RESULT_ERROR_INVALID_VALUE_SIZE: named persistent value size is
 *					     not supported or does not match the
 *      				     expected size.
 * FIOVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE: buffer too small for the requested
 *					     operation.
 * FIOVB_IO_RESULT_ERROR_ACCESS_CONFLICT: persistent object already exists and
 *					  no permission to overwrite.
 */
typedef enum {
	FIOVB_IO_RESULT_OK,
	FIOVB_IO_RESULT_ERROR_OOM,
	FIOVB_IO_RESULT_ERROR_IO,
	FIOVB_IO_RESULT_ERROR_NO_SUCH_VALUE,
	FIOVB_IO_RESULT_ERROR_INVALID_VALUE_SIZE,
	FIOVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE,
	FIOVB_IO_RESULT_ERROR_ACCESS_CONFLICT,
} fiovb_io_result;

struct fiovb_ops;

struct fiovb_ops {

  void* user_data;

  fiovb_io_result (*read_persistent_value)(struct fiovb_ops* ops,
                                           const char* name,
                                           size_t buffer_size,
                                           uint8_t* out_buffer,
                                           size_t* out_num_bytes_read);

  fiovb_io_result (*write_persistent_value)(struct fiovb_ops* ops,
                                            const char* name,
                                            size_t value_size,
                                            const uint8_t* value);

  fiovb_io_result (*delete_persistent_value)(struct fiovb_ops* ops,
                                             const char* name);
};

struct fiovb_ops_data {
	struct fiovb_ops ops;
	int mmc_dev;
	struct udevice *tee;
	u32 session;
};

struct fiovb_ops *fiovb_ops_alloc(int boot_device);
void fiovb_ops_free(struct fiovb_ops *ops);

static inline int fiovb_get_boot_device(struct fiovb_ops *ops)
{
	struct fiovb_ops_data *data;

	if (ops) {
		data = ops->user_data;
		if (data)
			return data->mmc_dev;
	}

	return -1;
}

#endif /* _FIOVB_H */
