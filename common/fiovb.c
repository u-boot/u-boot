/*
 * (C) Copyright 2019, Foundries.IO 
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */
#include <fiovb.h>
#include <blk.h>
#include <fastboot.h>
#include <image.h>
#include <malloc.h>
#include <part.h>
#include <tee.h>
#include <tee/optee_ta_fiovb.h>

static int get_open_session(struct fiovb_ops_data *ops_data)
{
	struct udevice *tee = NULL;

	while (!ops_data->tee) {
		const struct tee_optee_ta_uuid uuid = TA_FIOVB_UUID;
		struct tee_open_session_arg arg;
		int rc;

		tee = tee_find_device(tee, NULL, NULL, NULL);
		if (!tee)
			return -ENODEV;

		memset(&arg, 0, sizeof(arg));
		tee_optee_ta_uuid_to_octets(arg.uuid, &uuid);
		rc = tee_open_session(tee, &arg, 0, NULL);
		if (!rc) {
			ops_data->tee = tee;
			ops_data->session = arg.session;
		}
	}

	return 0;
}

static fiovb_io_result invoke_func(struct fiovb_ops_data *ops_data, u32 func,
			           ulong num_param, struct tee_param *param)
{
	struct tee_invoke_arg arg;

	if (get_open_session(ops_data))
		return FIOVB_IO_RESULT_ERROR_IO;

	memset(&arg, 0, sizeof(arg));
	arg.func = func;
	arg.session = ops_data->session;

	if (tee_invoke_func(ops_data->tee, &arg, num_param, param))
		return FIOVB_IO_RESULT_ERROR_IO;
	switch (arg.ret) {
	case TEE_SUCCESS:
		return FIOVB_IO_RESULT_OK;
	case TEE_ERROR_OUT_OF_MEMORY:
		return FIOVB_IO_RESULT_ERROR_OOM;
	case TEE_ERROR_STORAGE_NO_SPACE:
		return FIOVB_IO_RESULT_ERROR_INSUFFICIENT_SPACE;
	case TEE_ERROR_ITEM_NOT_FOUND:
		return FIOVB_IO_RESULT_ERROR_NO_SUCH_VALUE;
	case TEE_ERROR_ACCESS_CONFLICT:
		return FIOVB_IO_RESULT_ERROR_ACCESS_CONFLICT;
	case TEE_ERROR_TARGET_DEAD:
		/*
		 * The TA has paniced, close the session to reload the TA
		 * for the next request.
		 */
		tee_close_session(ops_data->tee, ops_data->session);
		ops_data->tee = NULL;
		return FIOVB_IO_RESULT_ERROR_IO;
	default:
		return FIOVB_IO_RESULT_ERROR_IO;
	}
}

static fiovb_io_result read_persistent_value(struct fiovb_ops *ops,
					     const char *name,
					     size_t buffer_size,
					     u8 *out_buffer,
					     size_t *out_num_bytes_read)
{
	fiovb_io_result rc;
	struct tee_shm *shm_name;
	struct tee_shm *shm_buf;
	struct tee_param param[2];
	struct udevice *tee;
	size_t name_size = strlen(name) + 1;

	if (get_open_session(ops->user_data))
		return FIOVB_IO_RESULT_ERROR_IO;

	tee = ((struct fiovb_ops_data *)ops->user_data)->tee;

	rc = tee_shm_alloc(tee, name_size,
			   TEE_SHM_ALLOC, &shm_name);
	if (rc)
		return FIOVB_IO_RESULT_ERROR_OOM;

	rc = tee_shm_alloc(tee, buffer_size,
			   TEE_SHM_ALLOC, &shm_buf);
	if (rc) {
		rc = FIOVB_IO_RESULT_ERROR_OOM;
		goto free_name;
	}

	memcpy(shm_name->addr, name, name_size);

	memset(param, 0, sizeof(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INPUT;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.size = name_size;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INOUT;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.size = buffer_size;

	rc = invoke_func(ops->user_data, TA_FIOVB_CMD_READ_PERSIST_VALUE,
			 2, param);
	if (rc)
		goto out;

	if (param[1].u.memref.size > buffer_size) {
		rc = FIOVB_IO_RESULT_ERROR_NO_SUCH_VALUE;
		goto out;
	}

	*out_num_bytes_read = param[1].u.memref.size;

	memcpy(out_buffer, shm_buf->addr, *out_num_bytes_read);

out:
	tee_shm_free(shm_buf);
free_name:
	tee_shm_free(shm_name);

	return rc;
}

static fiovb_io_result write_persistent_value(struct fiovb_ops *ops,
					      const char *name,
					      size_t value_size,
					      const u8 *value)
{
	fiovb_io_result rc;
	struct tee_shm *shm_name;
	struct tee_shm *shm_buf;
	struct tee_param param[2];
	struct udevice *tee;
	size_t name_size = strlen(name) + 1;

	if (get_open_session(ops->user_data))
		return FIOVB_IO_RESULT_ERROR_IO;

	tee = ((struct fiovb_ops_data *)ops->user_data)->tee;

	if (!value_size)
		return FIOVB_IO_RESULT_ERROR_NO_SUCH_VALUE;

	rc = tee_shm_alloc(tee, name_size,
			   TEE_SHM_ALLOC, &shm_name);
	if (rc)
		return FIOVB_IO_RESULT_ERROR_OOM;

	rc = tee_shm_alloc(tee, value_size,
			   TEE_SHM_ALLOC, &shm_buf);
	if (rc) {
		rc = FIOVB_IO_RESULT_ERROR_OOM;
		goto free_name;
	}

	memcpy(shm_name->addr, name, name_size);
	memcpy(shm_buf->addr, value, value_size);

	memset(param, 0, sizeof(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INPUT;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.size = name_size;
	param[1].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INPUT;
	param[1].u.memref.shm = shm_buf;
	param[1].u.memref.size = value_size;

	rc = invoke_func(ops->user_data, TA_FIOVB_CMD_WRITE_PERSIST_VALUE,
			 2, param);
	if (rc)
		goto out;

out:
	tee_shm_free(shm_buf);
free_name:
	tee_shm_free(shm_name);

	return rc;
}

static fiovb_io_result delete_persistent_value(struct fiovb_ops *ops,
					       const char *name)
{
	fiovb_io_result rc;
	struct tee_shm *shm_name;
	struct tee_param param[1];
	struct udevice *tee;
	size_t name_size = strlen(name) + 1;

	if (get_open_session(ops->user_data))
		return FIOVB_IO_RESULT_ERROR_IO;

	tee = ((struct fiovb_ops_data *)ops->user_data)->tee;

	rc = tee_shm_alloc(tee, name_size,
			   TEE_SHM_ALLOC, &shm_name);
	if (rc)
		return FIOVB_IO_RESULT_ERROR_OOM;

	memcpy(shm_name->addr, name, name_size);

	memset(param, 0, sizeof(param));
	param[0].attr = TEE_PARAM_ATTR_TYPE_MEMREF_INPUT;
	param[0].u.memref.shm = shm_name;
	param[0].u.memref.size = name_size;

	rc = invoke_func(ops->user_data, TA_FIOVB_CMD_DELETE_PERSIST_VALUE,
			 1, param);

	tee_shm_free(shm_name);

	return rc;
}

struct fiovb_ops *fiovb_ops_alloc(int boot_device)
{
	struct fiovb_ops_data *ops_data;

	ops_data = calloc(1, sizeof(struct fiovb_ops_data));
	if (!ops_data)
		return NULL;

	ops_data->ops.user_data = ops_data;

	ops_data->ops.delete_persistent_value = delete_persistent_value;
	ops_data->ops.write_persistent_value = write_persistent_value;
	ops_data->ops.read_persistent_value = read_persistent_value;
	ops_data->mmc_dev = boot_device;

	return &ops_data->ops;
}

void fiovb_ops_free(struct fiovb_ops *ops)
{
	struct fiovb_ops_data *ops_data;

	if (!ops)
		return;

	ops_data = ops->user_data;

	if (ops_data) {
		if (ops_data->tee)
			tee_close_session(ops_data->tee, ops_data->session);
		free(ops_data);
	}
}
