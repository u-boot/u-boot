// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020, Linaro Limited
 *
 * Tests scmi_agent uclass and the SCMI drivers implemented in other
 * uclass devices probe when a SCMI server exposes resources.
 *
 * Note in test.dts the protocol@10 node in scmi node. Protocol 0x10 is not
 * implemented in U-Boot SCMI components but the implementation is exepected
 * to not complain on unknown protocol IDs, as long as it is not used. Note
 * in test.dts tests that SCMI drivers probing does not fail for such an
 * unknown SCMI protocol ID.
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <reset.h>
#include <asm/scmi_test.h>
#include <dm/device-internal.h>
#include <dm/test.h>
#include <linux/kconfig.h>
#include <power/regulator.h>
#include <test/ut.h>

static int ut_assert_scmi_state_preprobe(struct unit_test_state *uts)
{
	struct sandbox_scmi_service *scmi_ctx = sandbox_scmi_service_ctx();

	ut_assertnonnull(scmi_ctx);
	ut_assertnull(scmi_ctx->agent);

	return 0;
}

static int ut_assert_scmi_state_postprobe(struct unit_test_state *uts,
					  struct udevice *dev)
{
	struct sandbox_scmi_devices *scmi_devices;
	struct sandbox_scmi_service *scmi_ctx;
	struct sandbox_scmi_agent *agent;

	/* Device references to check context against test sequence */
	scmi_devices = sandbox_scmi_devices_ctx(dev);
	ut_assertnonnull(scmi_devices);
	ut_asserteq(2, scmi_devices->clk_count);
	ut_asserteq(1, scmi_devices->reset_count);
	ut_asserteq(2, scmi_devices->regul_count);

	/* State of the simulated SCMI server exposed */
	scmi_ctx = sandbox_scmi_service_ctx();
	ut_assertnonnull(scmi_ctx);
	agent = scmi_ctx->agent;
	ut_assertnonnull(agent);
	ut_asserteq(3, agent->clk_count);
	ut_assertnonnull(agent->clk);
	ut_asserteq(1, agent->reset_count);
	ut_assertnonnull(agent->reset);
	ut_asserteq(2, agent->voltd_count);
	ut_assertnonnull(agent->voltd);

	return 0;
}

static int load_sandbox_scmi_test_devices(struct unit_test_state *uts,
					  struct udevice **dev)
{
	int ret;

	ret = ut_assert_scmi_state_preprobe(uts);
	if (ret)
		return ret;

	ut_assertok(uclass_get_device_by_name(UCLASS_MISC, "sandbox_scmi",
					      dev));
	ut_assertnonnull(*dev);

	return ut_assert_scmi_state_postprobe(uts, *dev);
}

static int release_sandbox_scmi_test_devices(struct unit_test_state *uts,
					     struct udevice *dev)
{
	ut_assertok(device_remove(dev, DM_REMOVE_NORMAL));

	/* Not sure test devices are fully removed, agent may not be visible */
	return 0;
}

/*
 * Test SCMI states when loading and releasing resources
 * related to SCMI drivers.
 */
static int dm_test_scmi_sandbox_agent(struct unit_test_state *uts)
{
	struct udevice *dev = NULL;
	int ret;

	ret = load_sandbox_scmi_test_devices(uts, &dev);
	if (!ret)
		ret = release_sandbox_scmi_test_devices(uts, dev);

	return ret;
}
DM_TEST(dm_test_scmi_sandbox_agent, UT_TESTF_SCAN_FDT);

static int dm_test_scmi_clocks(struct unit_test_state *uts)
{
	struct sandbox_scmi_devices *scmi_devices;
	struct sandbox_scmi_service *scmi_ctx;
	struct sandbox_scmi_agent *agent;
	struct udevice *dev;
	int ret_dev;
	int ret;

	ret = load_sandbox_scmi_test_devices(uts, &dev);
	if (ret)
		return ret;

	scmi_devices = sandbox_scmi_devices_ctx(dev);
	ut_assertnonnull(scmi_devices);
	scmi_ctx = sandbox_scmi_service_ctx();
	ut_assertnonnull(scmi_ctx);
	agent = scmi_ctx->agent;
	ut_assertnonnull(agent);

	/* Test SCMI clocks rate manipulation */
	ut_asserteq(333, agent->clk[0].rate);
	ut_asserteq(200, agent->clk[1].rate);
	ut_asserteq(1000, agent->clk[2].rate);

	ut_asserteq(1000, clk_get_rate(&scmi_devices->clk[0]));
	ut_asserteq(333, clk_get_rate(&scmi_devices->clk[1]));

	ret_dev = clk_set_rate(&scmi_devices->clk[1], 1088);
	ut_assert(!ret_dev || ret_dev == 1088);

	ut_asserteq(1088, agent->clk[0].rate);
	ut_asserteq(200, agent->clk[1].rate);
	ut_asserteq(1000, agent->clk[2].rate);

	ut_asserteq(1000, clk_get_rate(&scmi_devices->clk[0]));
	ut_asserteq(1088, clk_get_rate(&scmi_devices->clk[1]));

	/* restore original rate for further tests */
	ret_dev = clk_set_rate(&scmi_devices->clk[1], 333);
	ut_assert(!ret_dev || ret_dev == 333);

	/* Test SCMI clocks gating manipulation */
	ut_assert(!agent->clk[0].enabled);
	ut_assert(!agent->clk[1].enabled);
	ut_assert(!agent->clk[2].enabled);

	ut_asserteq(0, clk_enable(&scmi_devices->clk[1]));

	ut_assert(agent->clk[0].enabled);
	ut_assert(!agent->clk[1].enabled);
	ut_assert(!agent->clk[2].enabled);

	ut_assertok(clk_disable(&scmi_devices->clk[1]));

	ut_assert(!agent->clk[0].enabled);
	ut_assert(!agent->clk[1].enabled);
	ut_assert(!agent->clk[2].enabled);

	return release_sandbox_scmi_test_devices(uts, dev);
}
DM_TEST(dm_test_scmi_clocks, UT_TESTF_SCAN_FDT);

static int dm_test_scmi_resets(struct unit_test_state *uts)
{
	struct sandbox_scmi_devices *scmi_devices;
	struct sandbox_scmi_service *scmi_ctx;
	struct sandbox_scmi_agent *agent;
	struct udevice *dev = NULL;
	int ret;

	ret = load_sandbox_scmi_test_devices(uts, &dev);
	if (ret)
		return ret;

	scmi_devices = sandbox_scmi_devices_ctx(dev);
	ut_assertnonnull(scmi_devices);
	scmi_ctx = sandbox_scmi_service_ctx();
	ut_assertnonnull(scmi_ctx);
	agent = scmi_ctx->agent;
	ut_assertnonnull(agent);

	/* Test SCMI resect controller manipulation */
	ut_assert(!agent->reset[0].asserted)

	ut_assertok(reset_assert(&scmi_devices->reset[0]));
	ut_assert(agent->reset[0].asserted)

	ut_assertok(reset_deassert(&scmi_devices->reset[0]));
	ut_assert(!agent->reset[0].asserted);

	return release_sandbox_scmi_test_devices(uts, dev);
}
DM_TEST(dm_test_scmi_resets, UT_TESTF_SCAN_FDT);

static int dm_test_scmi_voltage_domains(struct unit_test_state *uts)
{
	struct sandbox_scmi_devices *scmi_devices;
	struct sandbox_scmi_service *scmi_ctx;
	struct sandbox_scmi_agent *agent;
	struct dm_regulator_uclass_plat *uc_pdata;
	struct udevice *dev;
	struct udevice *regul0_dev;

	ut_assertok(load_sandbox_scmi_test_devices(uts, &dev));

	scmi_devices = sandbox_scmi_devices_ctx(dev);
	ut_assertnonnull(scmi_devices);
	scmi_ctx = sandbox_scmi_service_ctx();
	ut_assertnonnull(scmi_ctx);
	agent = scmi_ctx->agent;
	ut_assertnonnull(agent);

	/* Set/Get an SCMI voltage domain level */
	regul0_dev = scmi_devices->regul[0];
	ut_assert(regul0_dev);

	uc_pdata = dev_get_uclass_plat(regul0_dev);
	ut_assert(uc_pdata);

	ut_assertok(regulator_set_value(regul0_dev, uc_pdata->min_uV));
	ut_asserteq(agent->voltd[0].voltage_uv, uc_pdata->min_uV);

	ut_assert(regulator_get_value(regul0_dev) == uc_pdata->min_uV);

	ut_assertok(regulator_set_value(regul0_dev, uc_pdata->max_uV));
	ut_asserteq(agent->voltd[0].voltage_uv, uc_pdata->max_uV);

	ut_assert(regulator_get_value(regul0_dev) == uc_pdata->max_uV);

	/* Enable/disable SCMI voltage domains */
	ut_assertok(regulator_set_enable(scmi_devices->regul[0], false));
	ut_assertok(regulator_set_enable(scmi_devices->regul[1], false));
	ut_assert(!agent->voltd[0].enabled);
	ut_assert(!agent->voltd[1].enabled);

	ut_assertok(regulator_set_enable(scmi_devices->regul[0], true));
	ut_assert(agent->voltd[0].enabled);
	ut_assert(!agent->voltd[1].enabled);

	ut_assertok(regulator_set_enable(scmi_devices->regul[1], true));
	ut_assert(agent->voltd[0].enabled);
	ut_assert(agent->voltd[1].enabled);

	ut_assertok(regulator_set_enable(scmi_devices->regul[0], false));
	ut_assert(!agent->voltd[0].enabled);
	ut_assert(agent->voltd[1].enabled);

	return release_sandbox_scmi_test_devices(uts, dev);
}
DM_TEST(dm_test_scmi_voltage_domains, UT_TESTF_SCAN_FDT);
