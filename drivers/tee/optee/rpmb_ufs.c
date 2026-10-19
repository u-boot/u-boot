// SPDX-License-Identifier: GPL-2.0+

#include <dm.h>
#include <tee.h>
#include <ufs.h>

#include "optee_msg.h"
#include "optee_msg_supplicant.h"
#include "optee_private.h"

static int optee_rpmb_get_dev(struct udevice **scsi_devp)
{
	struct udevice *ufs_dev, *scsi_dev;
	int ret;

	ret = uclass_get_device(UCLASS_UFS, CONFIG_UFS_RPMB_CONTROLLER,
				&ufs_dev);
	if (ret)
		return ret;

	ret = device_get_child(ufs_dev, 0, &scsi_dev);
	if (ret)
		return ret;

	*scsi_devp = scsi_dev;

	return 0;
}

void optee_suppl_cmd_rpmb_probe_reset(struct udevice *dev,
				      struct optee_msg_arg *arg)
{
	struct optee_private *priv = dev_get_priv(dev);

	if (arg->num_params != 1 ||
	    arg->params[0].attr != OPTEE_MSG_ATTR_TYPE_VALUE_OUTPUT) {
		arg->ret = TEE_ERROR_BAD_PARAMETERS;
		return;
	}

	priv->rpmb_next_region = 0;
	priv->rpmb_cur_region = 0;

	arg->params[0].u.value.a = OPTEE_MSG_RPC_SHM_TYPE_APPL;
	arg->ret = TEE_SUCCESS;
}

void optee_suppl_cmd_rpmb_probe_next(struct udevice *dev,
				     struct optee_msg_arg *arg)
{
	struct optee_private *priv = dev_get_priv(dev);
	struct udevice *scsi_dev;
	struct tee_shm *cid_shm;
	u8 size_mult = 0;
	u8 rel_wr = 0;
	void *cid_buf;
	ulong cid_size;
	int ret;

	if (arg->num_params != 2 ||
	    arg->params[0].attr != OPTEE_MSG_ATTR_TYPE_VALUE_OUTPUT ||
	    arg->params[1].attr != OPTEE_MSG_ATTR_TYPE_RMEM_OUTPUT) {
		arg->ret = TEE_ERROR_BAD_PARAMETERS;
		return;
	}

	cid_shm = (struct tee_shm *)(ulong)arg->params[1].u.rmem.shm_ref;
	cid_buf = (u8 *)cid_shm->addr + arg->params[1].u.rmem.offs;
	cid_size = arg->params[1].u.rmem.size;
	if (cid_size < UFS_RPMB_CID_SIZE) {
		arg->ret = TEE_ERROR_SHORT_BUFFER;
		return;
	}

	if (optee_rpmb_get_dev(&scsi_dev)) {
		arg->ret = TEE_ERROR_ITEM_NOT_FOUND;
		return;
	}

	while (priv->rpmb_next_region < UFS_RPMB_NUM_REGIONS) {
		unsigned int region = priv->rpmb_next_region++;

		ret = ufs_rpmb_get_region_info(scsi_dev, region, &size_mult,
					       &rel_wr, cid_buf);
		if (ret < 0) {
			arg->ret = TEE_ERROR_GENERIC;
			return;
		}
		if (!ret)
			continue;

		priv->rpmb_cur_region = region;
		arg->params[0].u.value.a = OPTEE_RPC_RPMB_UFS;
		arg->params[0].u.value.b = size_mult;
		arg->params[0].u.value.c = rel_wr;
		arg->params[1].u.rmem.size = UFS_RPMB_CID_SIZE;
		arg->ret = TEE_SUCCESS;
		return;
	}

	arg->ret = TEE_ERROR_ITEM_NOT_FOUND;
}

void optee_suppl_cmd_rpmb_frames(struct udevice *dev,
				 struct optee_msg_arg *arg)
{
	struct optee_private *priv = dev_get_priv(dev);
	struct tee_shm *req_shm;
	struct tee_shm *rsp_shm;
	struct udevice *scsi_dev;
	void *req_buf;
	void *rsp_buf;
	ulong req_size;
	ulong rsp_size;

	if (arg->num_params != 2 ||
	    arg->params[0].attr != OPTEE_MSG_ATTR_TYPE_RMEM_INPUT ||
	    arg->params[1].attr != OPTEE_MSG_ATTR_TYPE_RMEM_OUTPUT) {
		arg->ret = TEE_ERROR_BAD_PARAMETERS;
		return;
	}

	if (optee_rpmb_get_dev(&scsi_dev)) {
		arg->ret = TEE_ERROR_ITEM_NOT_FOUND;
		return;
	}

	req_shm = (struct tee_shm *)(ulong)arg->params[0].u.rmem.shm_ref;
	req_buf = (u8 *)req_shm->addr + arg->params[0].u.rmem.offs;
	req_size = arg->params[0].u.rmem.size;

	rsp_shm = (struct tee_shm *)(ulong)arg->params[1].u.rmem.shm_ref;
	rsp_buf = (u8 *)rsp_shm->addr + arg->params[1].u.rmem.offs;
	rsp_size = arg->params[1].u.rmem.size;

	if (ufs_rpmb_route_frames(scsi_dev, priv->rpmb_cur_region, req_buf,
				  req_size, rsp_buf, rsp_size))
		arg->ret = TEE_ERROR_BAD_PARAMETERS;
	else
		arg->ret = TEE_SUCCESS;
}
