// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Linaro Limited
 */

#include <common.h>
#include <dm.h>
#include <dm/test.h>
#include <sandboxtee.h>
#include <tee.h>
#include <test/ut.h>
#include <tee/optee_ta_avb.h>

static int open_session(struct udevice *dev, u32 *session)
{
	struct tee_open_session_arg arg;
	const struct tee_optee_ta_uuid uuid = TA_AVB_UUID;
	int rc;

	memset(&arg, 0, sizeof(arg));
	tee_optee_ta_uuid_to_octets(arg.uuid, &uuid);
	rc = tee_open_session(dev, &arg, 0, NULL);
	if (rc)
		return rc;
	if (arg.ret)
		return -EIO;
	*session = arg.session;

	return 0;
}

static int invoke_func(struct udevice *dev, u32 session)
{
	struct tee_param param = { .attr = TEE_PARAM_ATTR_TYPE_VALUE_OUTPUT };
	struct tee_invoke_arg arg;

	memset(&arg, 0, sizeof(arg));
	arg.session = session;
	arg.func = TA_AVB_CMD_READ_LOCK_STATE;

	if (tee_invoke_func(dev, &arg, 1, &param) || arg.ret)
		return -1;

	return 0;
}

static int match(struct tee_version_data *vers, const void *data)
{
	return vers->gen_caps & TEE_GEN_CAP_GP;
}

static int dm_test_tee(struct unit_test_state *uts)
{
	struct tee_version_data vers;
	struct udevice *dev;
	struct sandbox_tee_state *state;
	u32 session = 0;
	int rc;
	u8 data[128];
	struct tee_shm *reg_shm;
	struct tee_shm *alloc_shm;

	dev = tee_find_device(NULL, match, NULL, &vers);
	ut_assert(dev);
	state = dev_get_priv(dev);
	ut_assert(!state->session);

	rc = open_session(dev, &session);
	ut_assert(!rc);
	ut_assert(session == state->session);

	rc = invoke_func(dev, session);
	ut_assert(!rc);

	rc = tee_close_session(dev, session);
	ut_assert(!rc);
	ut_assert(!state->session);

	ut_assert(!state->num_shms);
	rc = tee_shm_register(dev, data, sizeof(data), 0, &reg_shm);
	ut_assert(!rc);
	ut_assert(state->num_shms == 1);

	rc = tee_shm_alloc(dev, 256, 0, &alloc_shm);
	ut_assert(!rc);
	ut_assert(state->num_shms == 2);

	ut_assert(tee_shm_is_registered(reg_shm, dev));
	ut_assert(tee_shm_is_registered(alloc_shm, dev));

	tee_shm_free(reg_shm);
	tee_shm_free(alloc_shm);
	ut_assert(!state->num_shms);

	return 0;
}

DM_TEST(dm_test_tee, DM_TESTF_SCAN_FDT);
