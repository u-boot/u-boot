// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2015-2017 Google, Inc
 *
 * USB Power Delivery protocol stack.
 */

#include <asm/gpio.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <dm/devres.h>
#include <linux/compat.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/iopoll.h>
#include <time.h>
#include <usb/tcpm.h>
#include "tcpm-internal.h"

DECLARE_GLOBAL_DATA_PTR;

const char * const tcpm_states[] = {
	FOREACH_TCPM_STATE(GENERATE_TCPM_STRING)
};

const char * const typec_pd_rev_name[] = {
	[PD_REV10]		= "rev1",
	[PD_REV20]		= "rev2",
	[PD_REV30]		= "rev3",
};

const char * const typec_role_name[] = {
	[TYPEC_SINK]		= "sink",
	[TYPEC_SOURCE]		= "source",
};

const char * const typec_data_role_name[] = {
	[TYPEC_DEVICE]		= "device",
	[TYPEC_HOST]		= "host",
};

const char * const typec_orientation_name[] = {
	[TYPEC_ORIENTATION_NONE]	= "none",
	[TYPEC_ORIENTATION_NORMAL]	= "normal",
	[TYPEC_ORIENTATION_REVERSE]	= "reverse",
};

const char * const typec_cc_status_name[] = {
	[TYPEC_CC_OPEN]		= "open",
	[TYPEC_CC_RA]		= "ra",
	[TYPEC_CC_RD]		= "rd",
	[TYPEC_CC_RP_DEF]	= "rp-def",
	[TYPEC_CC_RP_1_5]	= "rp-1.5",
	[TYPEC_CC_RP_3_0]	= "rp-3.0",
};

static inline bool tcpm_cc_is_sink(enum typec_cc_status cc)
{
	return cc == TYPEC_CC_RP_DEF ||
	       cc == TYPEC_CC_RP_1_5 ||
	       cc == TYPEC_CC_RP_3_0;
}

static inline bool tcpm_port_is_sink(struct tcpm_port *port)
{
	bool cc1_is_snk = tcpm_cc_is_sink(port->cc1);
	bool cc2_is_snk = tcpm_cc_is_sink(port->cc2);

	return (cc1_is_snk && !cc2_is_snk) ||
	       (cc2_is_snk && !cc1_is_snk);
}

static inline bool tcpm_cc_is_source(enum typec_cc_status cc)
{
	return cc == TYPEC_CC_RD;
}

static inline bool tcpm_port_is_source(struct tcpm_port *port)
{
	bool cc1_is_src = tcpm_cc_is_source(port->cc1);
	bool cc2_is_src = tcpm_cc_is_source(port->cc2);

	return (cc1_is_src && !cc2_is_src) ||
	       (cc2_is_src && !cc1_is_src);
}

static inline bool tcpm_try_src(struct tcpm_port *port)
{
	return port->try_role == TYPEC_SOURCE &&
	       port->port_type == TYPEC_PORT_DRP;
}

static inline void tcpm_reset_event_cnt(struct udevice *dev)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);

	port->poll_event_cnt = 0;
}

static enum tcpm_state tcpm_default_state(struct tcpm_port *port)
{
	if (port->port_type == TYPEC_PORT_DRP) {
		if (port->try_role == TYPEC_SINK)
			return SNK_UNATTACHED;
		else if (port->try_role == TYPEC_SOURCE)
			return SRC_UNATTACHED;
	} else if (port->port_type == TYPEC_PORT_SNK) {
		return SNK_UNATTACHED;
	}
	return SRC_UNATTACHED;
}

static bool tcpm_port_is_disconnected(struct tcpm_port *port)
{
	return (!port->attached && port->cc1 == TYPEC_CC_OPEN &&
		port->cc2 == TYPEC_CC_OPEN) ||
	       (port->attached && ((port->polarity == TYPEC_POLARITY_CC1 &&
				    port->cc1 == TYPEC_CC_OPEN) ||
				   (port->polarity == TYPEC_POLARITY_CC2 &&
				    port->cc2 == TYPEC_CC_OPEN)));
}

static void tcpm_set_cc(struct udevice *dev, enum typec_cc_status cc)
{
	const struct dm_tcpm_ops *drvops = dev_get_driver_ops(dev);
	struct tcpm_port *port = dev_get_uclass_plat(dev);

	dev_dbg(dev, "TCPM: set cc = %d\n", cc);
	port->cc_req = cc;
	drvops->set_cc(dev, cc);
}

/*
 * Determine RP value to set based on maximum current supported
 * by a port if configured as source.
 * Returns CC value to report to link partner.
 */
static enum typec_cc_status tcpm_rp_cc(struct tcpm_port *port)
{
	const u32 *src_pdo = port->src_pdo;
	int nr_pdo = port->nr_src_pdo;
	int i;

	/*
	 * Search for first entry with matching voltage.
	 * It should report the maximum supported current.
	 */
	for (i = 0; i < nr_pdo; i++) {
		const u32 pdo = src_pdo[i];

		if (pdo_type(pdo) == PDO_TYPE_FIXED &&
		    pdo_fixed_voltage(pdo) == 5000) {
			unsigned int curr = pdo_max_current(pdo);

			if (curr >= 3000)
				return TYPEC_CC_RP_3_0;
			else if (curr >= 1500)
				return TYPEC_CC_RP_1_5;
			return TYPEC_CC_RP_DEF;
		}
	}

	return TYPEC_CC_RP_DEF;
}

static void tcpm_check_and_run_delayed_work(struct udevice *dev);

static bool tcpm_transmit_helper(struct udevice *dev)
{
	const struct dm_tcpm_ops *drvops = dev_get_driver_ops(dev);
	struct tcpm_port *port = dev_get_uclass_plat(dev);

	drvops->poll_event(dev);
	udelay(500);
	tcpm_check_and_run_delayed_work(dev);
	return port->tx_complete;
}

static int tcpm_pd_transmit(struct udevice *dev,
			    enum tcpm_transmit_type type,
			    const struct pd_message *msg)
{
	const struct dm_tcpm_ops *drvops = dev_get_driver_ops(dev);
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	u32 timeout_us = PD_T_TCPC_TX_TIMEOUT * 1000;
	bool tx_complete;
	int ret;

	if (msg)
		dev_dbg(dev, "TCPM: PD TX, header: %#x\n",
			le16_to_cpu(msg->header));
	else
		dev_dbg(dev, "TCPM: PD TX, type: %#x\n", type);

	port->tx_complete = false;
	ret = drvops->pd_transmit(dev, type, msg, port->negotiated_rev);
	if (ret < 0)
		return ret;

	/*
	 * At this point we basically need to block until the TCPM controller
	 * returns successful transmission. Since this is usually done using
	 * the generic interrupt status bits, we poll for any events. That
	 * will clear the interrupt status, so we also need to process any
	 * of the incoming events. This means we will do more processing and
	 * thus let's give everything a bit more time.
	 */
	timeout_us *= 5;
	ret = read_poll_timeout(tcpm_transmit_helper, tx_complete,
				!tx_complete, false, timeout_us, dev);
	if (ret < 0) {
		dev_err(dev, "TCPM: PD transmit data failed: %d\n", ret);
		return ret;
	}

	switch (port->tx_status) {
	case TCPC_TX_SUCCESS:
		port->message_id = (port->message_id + 1) & PD_HEADER_ID_MASK;
		break;
	case TCPC_TX_DISCARDED:
		ret = -EAGAIN;
		break;
	case TCPC_TX_FAILED:
	default:
		ret = -EIO;
		break;
	}

	return ret;
}

void tcpm_pd_transmit_complete(struct udevice *dev,
			       enum tcpm_transmit_status status)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);

	dev_dbg(dev, "TCPM: PD TX complete, status: %u\n", status);
	tcpm_reset_event_cnt(dev);
	port->tx_status = status;
	port->tx_complete = true;
}

static int tcpm_set_polarity(struct udevice *dev,
			     enum typec_cc_polarity polarity)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	const struct dm_tcpm_ops *drvops = dev_get_driver_ops(dev);
	int ret;

	dev_dbg(dev, "TCPM: set polarity = %d\n", polarity);

	if (drvops->set_polarity) {
		ret = drvops->set_polarity(dev, polarity);
		if (ret < 0)
			return ret;
	}

	port->polarity = polarity;

	return 0;
}

static int tcpm_set_vconn(struct udevice *dev, bool enable)
{
	const struct dm_tcpm_ops *drvops = dev_get_driver_ops(dev);
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	int ret;

	dev_dbg(dev, "TCPM: set vconn = %d\n", enable);

	ret = drvops->set_vconn(dev, enable);
	if (!ret)
		port->vconn_role = enable ? TYPEC_SOURCE : TYPEC_SINK;

	return ret;
}

static inline u32 tcpm_get_current_limit(struct tcpm_port *port)
{
	switch (port->polarity ? port->cc2 : port->cc1) {
	case TYPEC_CC_RP_1_5:
		return 1500;
	case TYPEC_CC_RP_3_0:
		return 3000;
	case TYPEC_CC_RP_DEF:
	default:
		return 0;
	}
}

static int tcpm_set_current_limit(struct udevice *dev, u32 max_ma, u32 mv)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	int ret = -EOPNOTSUPP;

	dev_info(dev, "TCPM: set voltage limit = %u mV\n", mv);
	dev_info(dev, "TCPM: set current limit = %u mA\n", max_ma);

	port->supply_voltage = mv;
	port->current_limit = max_ma;

	return ret;
}

static int tcpm_set_attached_state(struct udevice *dev, bool attached)
{
	const struct dm_tcpm_ops *drvops = dev_get_driver_ops(dev);
	struct tcpm_port *port = dev_get_uclass_plat(dev);

	return drvops->set_roles(dev, attached, port->pwr_role,
				 port->data_role);
}

static int tcpm_set_roles(struct udevice *dev, bool attached,
			  enum typec_role role, enum typec_data_role data)
{
	const struct dm_tcpm_ops *drvops = dev_get_driver_ops(dev);
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	int ret;

	ret = drvops->set_roles(dev, attached, role, data);
	if (ret < 0)
		return ret;

	port->pwr_role = role;
	port->data_role = data;

	return 0;
}

static int tcpm_pd_send_source_caps(struct udevice *dev)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	struct pd_message msg;
	int i;

	memset(&msg, 0, sizeof(msg));

	if (!port->nr_src_pdo) {
		/* No source capabilities defined, sink only */
		msg.header = PD_HEADER_LE(PD_CTRL_REJECT,
					  port->pwr_role,
					  port->data_role,
					  port->negotiated_rev,
					  port->message_id, 0);
	} else {
		msg.header = PD_HEADER_LE(PD_DATA_SOURCE_CAP,
					  port->pwr_role,
					  port->data_role,
					  port->negotiated_rev,
					  port->message_id,
					  port->nr_src_pdo);
	}

	for (i = 0; i < port->nr_src_pdo; i++)
		msg.payload[i] = cpu_to_le32(port->src_pdo[i]);

	return tcpm_pd_transmit(dev, TCPC_TX_SOP, &msg);
}

static int tcpm_pd_send_sink_caps(struct udevice *dev)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	struct pd_message msg;
	unsigned int i;

	memset(&msg, 0, sizeof(msg));

	if (!port->nr_snk_pdo) {
		/* No sink capabilities defined, source only */
		msg.header = PD_HEADER_LE(PD_CTRL_REJECT,
					  port->pwr_role,
					  port->data_role,
					  port->negotiated_rev,
					  port->message_id, 0);
	} else {
		msg.header = PD_HEADER_LE(PD_DATA_SINK_CAP,
					  port->pwr_role,
					  port->data_role,
					  port->negotiated_rev,
					  port->message_id,
					  port->nr_snk_pdo);
	}

	for (i = 0; i < port->nr_snk_pdo; i++)
		msg.payload[i] = cpu_to_le32(port->snk_pdo[i]);

	return tcpm_pd_transmit(dev, TCPC_TX_SOP, &msg);
}

static void tcpm_state_machine(struct udevice *dev);

static inline void tcpm_timer_uninit(struct udevice *dev)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);

	port->delay_target = 0;
}

static void tcpm_timer_init(struct udevice *dev, uint32_t ms)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	unsigned long time_us = ms * 1000;

	port->delay_target = timer_get_us() + time_us;
}

static void tcpm_check_and_run_delayed_work(struct udevice *dev)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);

	/* no delayed state changes scheduled */
	if (port->delay_target == 0)
		return;

	/* it's not yet time */
	if (timer_get_us() < port->delay_target)
		return;

	tcpm_timer_uninit(dev);
	tcpm_state_machine(dev);
}

static void mod_tcpm_delayed_work(struct udevice *dev, unsigned int delay_ms)
{
	if (delay_ms) {
		tcpm_timer_init(dev, delay_ms);
	} else {
		tcpm_timer_uninit(dev);
		tcpm_state_machine(dev);
	}
}

static void tcpm_set_state(struct udevice *dev, enum tcpm_state state,
			   unsigned int delay_ms)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);

	if (delay_ms) {
		dev_dbg(dev, "TCPM: pending state change %s -> %s @ %u ms [%s]\n",
			tcpm_states[port->state], tcpm_states[state], delay_ms,
			typec_pd_rev_name[port->negotiated_rev]);
		port->delayed_state = state;
		mod_tcpm_delayed_work(dev, delay_ms);
		port->delay_ms = delay_ms;
	} else {
		dev_dbg(dev, "TCPM: state change %s -> %s\n",
			tcpm_states[port->state], tcpm_states[state]);
		port->delayed_state = INVALID_STATE;
		port->prev_state = port->state;
		port->state = state;
		/*
		 * Don't re-queue the state machine work item if we're currently
		 * in the state machine and we're immediately changing states.
		 * tcpm_state_machine_work() will continue running the state
		 * machine.
		 */
		if (!port->state_machine_running)
			mod_tcpm_delayed_work(dev, 0);
	}
}

static void tcpm_set_state_cond(struct udevice *dev, enum tcpm_state state,
				unsigned int delay_ms)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);

	if (port->enter_state == port->state)
		tcpm_set_state(dev, state, delay_ms);
	else
		dev_dbg(dev, "TCPM: skipped %sstate change %s -> %s [%u ms], context state %s [%s]\n",
			delay_ms ? "delayed " : "",
			tcpm_states[port->state], tcpm_states[state],
			delay_ms, tcpm_states[port->enter_state],
			typec_pd_rev_name[port->negotiated_rev]);
}

static void tcpm_queue_message(struct udevice *dev,
			       enum pd_msg_request message)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);

	port->queued_message = message;
	mod_tcpm_delayed_work(dev, 0);
}

enum pdo_err {
	PDO_NO_ERR,
	PDO_ERR_NO_VSAFE5V,
	PDO_ERR_VSAFE5V_NOT_FIRST,
	PDO_ERR_PDO_TYPE_NOT_IN_ORDER,
	PDO_ERR_FIXED_NOT_SORTED,
	PDO_ERR_VARIABLE_BATT_NOT_SORTED,
	PDO_ERR_DUPE_PDO,
	PDO_ERR_PPS_APDO_NOT_SORTED,
	PDO_ERR_DUPE_PPS_APDO,
};

static const char * const pdo_err_msg[] = {
	[PDO_ERR_NO_VSAFE5V] =
	" err: source/sink caps should at least have vSafe5V",
	[PDO_ERR_VSAFE5V_NOT_FIRST] =
	" err: vSafe5V Fixed Supply Object Shall always be the first object",
	[PDO_ERR_PDO_TYPE_NOT_IN_ORDER] =
	" err: PDOs should be in the following order: Fixed; Battery; Variable",
	[PDO_ERR_FIXED_NOT_SORTED] =
	" err: Fixed supply pdos should be in increasing order of their fixed voltage",
	[PDO_ERR_VARIABLE_BATT_NOT_SORTED] =
	" err: Variable/Battery supply pdos should be in increasing order of their minimum voltage",
	[PDO_ERR_DUPE_PDO] =
	" err: Variable/Batt supply pdos cannot have same min/max voltage",
	[PDO_ERR_PPS_APDO_NOT_SORTED] =
	" err: Programmable power supply apdos should be in increasing order of their maximum voltage",
	[PDO_ERR_DUPE_PPS_APDO] =
	" err: Programmable power supply apdos cannot have same min/max voltage and max current",
};

static enum pdo_err tcpm_caps_err(struct udevice *dev, const u32 *pdo,
				  unsigned int nr_pdo)
{
	unsigned int i;

	/* Should at least contain vSafe5v */
	if (nr_pdo < 1)
		return PDO_ERR_NO_VSAFE5V;

	/* The vSafe5V Fixed Supply Object Shall always be the first object */
	if (pdo_type(pdo[0]) != PDO_TYPE_FIXED ||
	    pdo_fixed_voltage(pdo[0]) != VSAFE5V)
		return PDO_ERR_VSAFE5V_NOT_FIRST;

	for (i = 1; i < nr_pdo; i++) {
		if (pdo_type(pdo[i]) < pdo_type(pdo[i - 1])) {
			return PDO_ERR_PDO_TYPE_NOT_IN_ORDER;
		} else if (pdo_type(pdo[i]) == pdo_type(pdo[i - 1])) {
			enum pd_pdo_type type = pdo_type(pdo[i]);

			switch (type) {
			/*
			 * The remaining Fixed Supply Objects, if
			 * present, shall be sent in voltage order;
			 * lowest to highest.
			 */
			case PDO_TYPE_FIXED:
				if (pdo_fixed_voltage(pdo[i]) <=
				    pdo_fixed_voltage(pdo[i - 1]))
					return PDO_ERR_FIXED_NOT_SORTED;
				break;
			/*
			 * The Battery Supply Objects and Variable
			 * supply, if present shall be sent in Minimum
			 * Voltage order; lowest to highest.
			 */
			case PDO_TYPE_VAR:
			case PDO_TYPE_BATT:
				if (pdo_min_voltage(pdo[i]) <
				    pdo_min_voltage(pdo[i - 1]))
					return PDO_ERR_VARIABLE_BATT_NOT_SORTED;
				else if ((pdo_min_voltage(pdo[i]) ==
					  pdo_min_voltage(pdo[i - 1])) &&
					 (pdo_max_voltage(pdo[i]) ==
					  pdo_max_voltage(pdo[i - 1])))
					return PDO_ERR_DUPE_PDO;
				break;
			/*
			 * The Programmable Power Supply APDOs, if present,
			 * shall be sent in Maximum Voltage order;
			 * lowest to highest.
			 */
			case PDO_TYPE_APDO:
				if (pdo_apdo_type(pdo[i]) != APDO_TYPE_PPS)
					break;

				if (pdo_pps_apdo_max_voltage(pdo[i]) <
				    pdo_pps_apdo_max_voltage(pdo[i - 1]))
					return PDO_ERR_PPS_APDO_NOT_SORTED;
				else if (pdo_pps_apdo_min_voltage(pdo[i]) ==
					  pdo_pps_apdo_min_voltage(pdo[i - 1]) &&
					 pdo_pps_apdo_max_voltage(pdo[i]) ==
					  pdo_pps_apdo_max_voltage(pdo[i - 1]) &&
					 pdo_pps_apdo_max_current(pdo[i]) ==
					  pdo_pps_apdo_max_current(pdo[i - 1]))
					return PDO_ERR_DUPE_PPS_APDO;
				break;
			default:
				dev_err(dev, "TCPM: Unknown pdo type\n");
			}
		}
	}

	return PDO_NO_ERR;
}

static int tcpm_validate_caps(struct udevice *dev, const u32 *pdo,
			      unsigned int nr_pdo)
{
	enum pdo_err err_index = tcpm_caps_err(dev, pdo, nr_pdo);

	if (err_index != PDO_NO_ERR) {
		dev_err(dev, "TCPM:%s\n", pdo_err_msg[err_index]);
		return -EINVAL;
	}

	return 0;
}

/*
 * PD (data, control) command handling functions
 */
static inline enum tcpm_state ready_state(struct tcpm_port *port)
{
	if (port->pwr_role == TYPEC_SOURCE)
		return SRC_READY;
	else
		return SNK_READY;
}

static void tcpm_pd_data_request(struct udevice *dev,
				 const struct pd_message *msg)
{
	enum pd_data_msg_type type = pd_header_type_le(msg->header);
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	unsigned int cnt = pd_header_cnt_le(msg->header);
	unsigned int rev = pd_header_rev_le(msg->header);
	unsigned int i;

	switch (type) {
	case PD_DATA_SOURCE_CAP:
		for (i = 0; i < cnt; i++)
			port->source_caps[i] = le32_to_cpu(msg->payload[i]);

		port->nr_source_caps = cnt;

		tcpm_validate_caps(dev, port->source_caps,
				   port->nr_source_caps);

		/*
		 * Adjust revision in subsequent message headers, as required,
		 * to comply with 6.2.1.1.5 of the USB PD 3.0 spec. We don't
		 * support Rev 1.0 so just do nothing in that scenario.
		 */
		if (rev == PD_REV10)
			break;

		if (rev < PD_MAX_REV)
			port->negotiated_rev = rev;

		if ((pdo_type(port->source_caps[0]) == PDO_TYPE_FIXED) &&
		    (port->source_caps[0] & PDO_FIXED_DUAL_ROLE) &&
		    (port->source_caps[0] & PDO_FIXED_DATA_SWAP)) {
			/* Dual role power and data, eg: self-powered Type-C */
			port->wait_dr_swap_message = true;
		} else {
			/* Non-Dual role power, eg: adapter */
			port->wait_dr_swap_message = false;
		}

		/*
		 * This message may be received even if VBUS is not
		 * present. This is quite unexpected; see USB PD
		 * specification, sections 8.3.3.6.3.1 and 8.3.3.6.3.2.
		 * However, at the same time, we must be ready to
		 * receive this message and respond to it 15ms after
		 * receiving PS_RDY during power swap operations, no matter
		 * if VBUS is available or not (USB PD specification,
		 * section 6.5.9.2).
		 * So we need to accept the message either way,
		 * but be prepared to keep waiting for VBUS after it was
		 * handled.
		 */
		tcpm_set_state(dev, SNK_NEGOTIATE_CAPABILITIES, 0);
		break;
	case PD_DATA_REQUEST:
		/*
		 * Adjust revision in subsequent message headers, as required,
		 * to comply with 6.2.1.1.5 of the USB PD 3.0 spec. We don't
		 * support Rev 1.0 so just reject in that scenario.
		 */
		if (rev == PD_REV10) {
			tcpm_queue_message(dev, PD_MSG_CTRL_REJECT);
			break;
		}

		if (rev < PD_MAX_REV)
			port->negotiated_rev = rev;

		port->sink_request = le32_to_cpu(msg->payload[0]);

		tcpm_set_state(dev, SRC_NEGOTIATE_CAPABILITIES, 0);
		break;
	case PD_DATA_SINK_CAP:
		/* We don't do anything with this at the moment... */
		for (i = 0; i < cnt; i++)
			port->sink_caps[i] = le32_to_cpu(msg->payload[i]);

		port->nr_sink_caps = cnt;
		break;
	default:
		break;
	}
}

static void tcpm_pd_ctrl_request(struct udevice *dev,
				 const struct pd_message *msg)
{
	enum pd_ctrl_msg_type type = pd_header_type_le(msg->header);
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	enum tcpm_state next_state;

	switch (type) {
	case PD_CTRL_GOOD_CRC:
	case PD_CTRL_PING:
		break;
	case PD_CTRL_GET_SOURCE_CAP:
		switch (port->state) {
		case SRC_READY:
		case SNK_READY:
			tcpm_queue_message(dev, PD_MSG_DATA_SOURCE_CAP);
			break;
		default:
			tcpm_queue_message(dev, PD_MSG_CTRL_REJECT);
			break;
		}
		break;
	case PD_CTRL_GET_SINK_CAP:
		switch (port->state) {
		case SRC_READY:
		case SNK_READY:
			tcpm_queue_message(dev, PD_MSG_DATA_SINK_CAP);
			break;
		default:
			tcpm_queue_message(dev, PD_MSG_CTRL_REJECT);
			break;
		}
		break;
	case PD_CTRL_GOTO_MIN:
		break;
	case PD_CTRL_PS_RDY:
		switch (port->state) {
		case SNK_TRANSITION_SINK:
			if (port->vbus_present) {
				tcpm_set_current_limit(dev,
						       port->req_current_limit,
						       port->req_supply_voltage);
				port->explicit_contract = true;
				tcpm_set_state(dev, SNK_READY, 0);
			} else {
				/*
				 * Seen after power swap. Keep waiting for VBUS
				 * in a transitional state.
				 */
				tcpm_set_state(dev,
					       SNK_TRANSITION_SINK_VBUS, 0);
			}
			break;
		default:
			break;
		}
		break;
	case PD_CTRL_REJECT:
	case PD_CTRL_WAIT:
	case PD_CTRL_NOT_SUPP:
		switch (port->state) {
		case SNK_NEGOTIATE_CAPABILITIES:
			/* USB PD specification, Figure 8-43 */
			if (port->explicit_contract)
				next_state = SNK_READY;
			else
				next_state = SNK_WAIT_CAPABILITIES;

			tcpm_set_state(dev, next_state, 0);
			break;
		default:
			break;
		}
		break;
	case PD_CTRL_ACCEPT:
		switch (port->state) {
		case SNK_NEGOTIATE_CAPABILITIES:
			tcpm_set_state(dev, SNK_TRANSITION_SINK, 0);
			break;
		case SOFT_RESET_SEND:
			port->message_id = 0;
			port->rx_msgid = -1;
			if (port->pwr_role == TYPEC_SOURCE)
				next_state = SRC_SEND_CAPABILITIES;
			else
				next_state = SNK_WAIT_CAPABILITIES;
			tcpm_set_state(dev, next_state, 0);
			break;
		default:
			break;
		}
		break;
	case PD_CTRL_SOFT_RESET:
		tcpm_set_state(dev, SOFT_RESET, 0);
		break;
	case PD_CTRL_DR_SWAP:
		if (port->port_type != TYPEC_PORT_DRP) {
			tcpm_queue_message(dev, PD_MSG_CTRL_REJECT);
			break;
		}
		/*
		 * 6.3.9: If an alternate mode is active, a request to swap
		 * alternate modes shall trigger a port reset.
		 */
		switch (port->state) {
		case SRC_READY:
		case SNK_READY:
			tcpm_set_state(dev, DR_SWAP_ACCEPT, 0);
			break;
		default:
			tcpm_queue_message(dev, PD_MSG_CTRL_WAIT);
			break;
		}
		break;
	case PD_CTRL_PR_SWAP:
	case PD_CTRL_VCONN_SWAP:
	case PD_CTRL_GET_SOURCE_CAP_EXT:
	case PD_CTRL_GET_STATUS:
	case PD_CTRL_FR_SWAP:
	case PD_CTRL_GET_PPS_STATUS:
	case PD_CTRL_GET_COUNTRY_CODES:
		/* Currently not supported */
		dev_err(dev, "TCPM: Currently not supported type %#x\n", type);
		tcpm_queue_message(dev, PD_MSG_CTRL_NOT_SUPP);
		break;
	default:
		dev_err(dev, "TCPM: Unrecognized ctrl message type %#x\n", type);
		break;
	}
}

static void tcpm_pd_rx_handler(struct udevice *dev,
			       const struct pd_message *msg)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	unsigned int cnt = pd_header_cnt_le(msg->header);
	bool remote_is_host, local_is_host;

	dev_dbg(dev, "TCPM: PD RX, header: %#x [%d]\n",
		le16_to_cpu(msg->header), port->attached);

	if (port->attached) {
		enum pd_ctrl_msg_type type = pd_header_type_le(msg->header);
		unsigned int msgid = pd_header_msgid_le(msg->header);

		/*
		 * USB PD standard, 6.6.1.2:
		 * "... if MessageID value in a received Message is the
		 * same as the stored value, the receiver shall return a
		 * GoodCRC Message with that MessageID value and drop
		 * the Message (this is a retry of an already received
		 * Message). Note: this shall not apply to the Soft_Reset
		 * Message which always has a MessageID value of zero."
		 */
		if (msgid == port->rx_msgid && type != PD_CTRL_SOFT_RESET)
			return;
		port->rx_msgid = msgid;

		/*
		 * If both ends believe to be DFP/host, we have a data role
		 * mismatch.
		 */
		remote_is_host = !!(le16_to_cpu(msg->header) & PD_HEADER_DATA_ROLE);
		local_is_host = port->data_role == TYPEC_HOST;
		if (remote_is_host == local_is_host) {
			dev_err(dev, "TCPM: data role mismatch, initiating error recovery\n");
			tcpm_set_state(dev, ERROR_RECOVERY, 0);
		} else {
			if (cnt)
				tcpm_pd_data_request(dev, msg);
			else
				tcpm_pd_ctrl_request(dev, msg);
		}
	}
}

void tcpm_pd_receive(struct udevice *dev, const struct pd_message *msg)
{
	tcpm_reset_event_cnt(dev);
	tcpm_pd_rx_handler(dev, msg);
}

static int tcpm_pd_send_control(struct udevice *dev,
				enum pd_ctrl_msg_type type)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	struct pd_message msg;

	memset(&msg, 0, sizeof(msg));
	msg.header = PD_HEADER_LE(type, port->pwr_role,
				  port->data_role,
				  port->negotiated_rev,
				  port->message_id, 0);

	return tcpm_pd_transmit(dev, TCPC_TX_SOP, &msg);
}

/*
 * Send queued message without affecting state.
 * Return true if state machine should go back to sleep,
 * false otherwise.
 */
static bool tcpm_send_queued_message(struct udevice *dev)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	enum pd_msg_request queued_message;
	int max_messages = 100;

	do {
		queued_message = port->queued_message;
		port->queued_message = PD_MSG_NONE;
		max_messages--;

		switch (queued_message) {
		case PD_MSG_CTRL_WAIT:
			tcpm_pd_send_control(dev, PD_CTRL_WAIT);
			break;
		case PD_MSG_CTRL_REJECT:
			tcpm_pd_send_control(dev, PD_CTRL_REJECT);
			break;
		case PD_MSG_CTRL_NOT_SUPP:
			tcpm_pd_send_control(dev, PD_CTRL_NOT_SUPP);
			break;
		case PD_MSG_DATA_SINK_CAP:
			tcpm_pd_send_sink_caps(dev);
			break;
		case PD_MSG_DATA_SOURCE_CAP:
			tcpm_pd_send_source_caps(dev);
			break;
		default:
			break;
		}
	} while (max_messages > 0 && port->queued_message != PD_MSG_NONE);

	if (!max_messages)
		dev_err(dev, "Aborted sending of too many queued messages\n");

	return false;
}

static int tcpm_pd_check_request(struct udevice *dev)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	u32 pdo, rdo = port->sink_request;
	unsigned int max, op, pdo_max, index;
	enum pd_pdo_type type;

	index = rdo_index(rdo);
	if (!index || index > port->nr_src_pdo)
		return -EINVAL;

	pdo = port->src_pdo[index - 1];
	type = pdo_type(pdo);
	switch (type) {
	case PDO_TYPE_FIXED:
	case PDO_TYPE_VAR:
		max = rdo_max_current(rdo);
		op = rdo_op_current(rdo);
		pdo_max = pdo_max_current(pdo);

		if (op > pdo_max)
			return -EINVAL;
		if (max > pdo_max && !(rdo & RDO_CAP_MISMATCH))
			return -EINVAL;

		if (type == PDO_TYPE_FIXED)
			dev_dbg(dev, "TCPM: Requested %u mV, %u mA for %u / %u mA\n",
				pdo_fixed_voltage(pdo), pdo_max, op, max);
		else
			dev_dbg(dev, "TCPM: Requested %u -> %u mV, %u mA for %u / %u mA\n",
				pdo_min_voltage(pdo), pdo_max_voltage(pdo),
				pdo_max, op, max);
		break;
	case PDO_TYPE_BATT:
		max = rdo_max_power(rdo);
		op = rdo_op_power(rdo);
		pdo_max = pdo_max_power(pdo);

		if (op > pdo_max)
			return -EINVAL;
		if (max > pdo_max && !(rdo & RDO_CAP_MISMATCH))
			return -EINVAL;
		dev_info(dev, "TCPM: Requested %u -> %u mV, %u mW for %u / %u mW\n",
			 pdo_min_voltage(pdo), pdo_max_voltage(pdo),
			 pdo_max, op, max);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

#define min_power(x, y) min(pdo_max_power(x), pdo_max_power(y))
#define min_current(x, y) min(pdo_max_current(x), pdo_max_current(y))

static int tcpm_pd_select_pdo(struct udevice *dev, int *sink_pdo,
			      int *src_pdo)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	unsigned int i, j, max_src_mv = 0, min_src_mv = 0, max_mw = 0,
		     max_mv = 0, src_mw = 0, src_ma = 0, max_snk_mv = 0,
		     min_snk_mv = 0;
	int ret = -EINVAL;

	/*
	 * Select the source PDO providing the most power which has a
	 * matchig sink cap.
	 */
	for (i = 0; i < port->nr_source_caps; i++) {
		u32 pdo = port->source_caps[i];
		enum pd_pdo_type type = pdo_type(pdo);

		switch (type) {
		case PDO_TYPE_FIXED:
			max_src_mv = pdo_fixed_voltage(pdo);
			min_src_mv = max_src_mv;
			break;
		case PDO_TYPE_BATT:
		case PDO_TYPE_VAR:
			max_src_mv = pdo_max_voltage(pdo);
			min_src_mv = pdo_min_voltage(pdo);
			break;
		case PDO_TYPE_APDO:
			continue;
		default:
			dev_err(dev, "TCPM: Invalid source PDO type, ignoring\n");
			continue;
		}

		switch (type) {
		case PDO_TYPE_FIXED:
		case PDO_TYPE_VAR:
			src_ma = pdo_max_current(pdo);
			src_mw = src_ma * min_src_mv / 1000;
			break;
		case PDO_TYPE_BATT:
			src_mw = pdo_max_power(pdo);
			break;
		case PDO_TYPE_APDO:
			continue;
		default:
			dev_err(dev, "TCPM: Invalid source PDO type, ignoring\n");
			continue;
		}

		for (j = 0; j < port->nr_snk_pdo; j++) {
			pdo = port->snk_pdo[j];

			switch (pdo_type(pdo)) {
			case PDO_TYPE_FIXED:
				max_snk_mv = pdo_fixed_voltage(pdo);
				min_snk_mv = max_snk_mv;
				break;
			case PDO_TYPE_BATT:
			case PDO_TYPE_VAR:
				max_snk_mv = pdo_max_voltage(pdo);
				min_snk_mv = pdo_min_voltage(pdo);
				break;
			case PDO_TYPE_APDO:
				continue;
			default:
				dev_err(dev, "TCPM: Invalid sink PDO type, ignoring\n");
				continue;
			}

			if (max_src_mv <= max_snk_mv && min_src_mv >= min_snk_mv) {
				/* Prefer higher voltages if available */
				if ((src_mw == max_mw && min_src_mv > max_mv) ||
				    src_mw > max_mw) {
					*src_pdo = i;
					*sink_pdo = j;
					max_mw = src_mw;
					max_mv = min_src_mv;
					ret = 0;
				}
			}
		}
	}

	return ret;
}

static int tcpm_pd_build_request(struct udevice *dev, u32 *rdo)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	unsigned int mv, ma, mw, flags;
	unsigned int max_ma, max_mw;
	enum pd_pdo_type type;
	u32 pdo, matching_snk_pdo;
	int src_pdo_index = 0;
	int snk_pdo_index = 0;
	int ret;

	ret = tcpm_pd_select_pdo(dev, &snk_pdo_index, &src_pdo_index);
	if (ret < 0)
		return ret;

	pdo = port->source_caps[src_pdo_index];
	matching_snk_pdo = port->snk_pdo[snk_pdo_index];
	type = pdo_type(pdo);

	switch (type) {
	case PDO_TYPE_FIXED:
		mv = pdo_fixed_voltage(pdo);
		break;
	case PDO_TYPE_BATT:
	case PDO_TYPE_VAR:
		mv = pdo_min_voltage(pdo);
		break;
	default:
		dev_err(dev, "TCPM: Invalid PDO selected!\n");
		return -EINVAL;
	}

	/* Select maximum available current within the sink pdo's limit */
	if (type == PDO_TYPE_BATT) {
		mw = min_power(pdo, matching_snk_pdo);
		ma = 1000 * mw / mv;
	} else {
		ma = min_current(pdo, matching_snk_pdo);
		mw = ma * mv / 1000;
	}

	flags = RDO_USB_COMM | RDO_NO_SUSPEND;

	/* Set mismatch bit if offered power is less than operating power */
	max_ma = ma;
	max_mw = mw;
	if (mw < port->operating_snk_mw) {
		flags |= RDO_CAP_MISMATCH;
		if (type == PDO_TYPE_BATT &&
		    (pdo_max_power(matching_snk_pdo) > pdo_max_power(pdo)))
			max_mw = pdo_max_power(matching_snk_pdo);
		else if (pdo_max_current(matching_snk_pdo) >
			 pdo_max_current(pdo))
			max_ma = pdo_max_current(matching_snk_pdo);
	}

	dev_dbg(dev, "TCPM: cc=%d cc1=%d cc2=%d vbus=%d vconn=%s polarity=%d\n",
		port->cc_req, port->cc1, port->cc2, port->vbus_source,
		port->vconn_role == TYPEC_SOURCE ? "source" : "sink",
		port->polarity);

	if (type == PDO_TYPE_BATT) {
		*rdo = RDO_BATT(src_pdo_index + 1, mw, max_mw, flags);

		dev_info(dev, "TCPM: requesting PDO %d: %u mV, %u mW%s\n",
			 src_pdo_index, mv, mw,
			 flags & RDO_CAP_MISMATCH ? " [mismatch]" : "");
	} else {
		*rdo = RDO_FIXED(src_pdo_index + 1, ma, max_ma, flags);

		dev_info(dev, "TCPM: requesting PDO %d: %u mV, %u mA%s\n",
			 src_pdo_index, mv, ma,
			 flags & RDO_CAP_MISMATCH ? " [mismatch]" : "");
	}

	port->req_current_limit = ma;
	port->req_supply_voltage = mv;

	return 0;
}

static int tcpm_pd_send_request(struct udevice *dev)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	struct pd_message msg;
	int ret;
	u32 rdo;

	ret = tcpm_pd_build_request(dev, &rdo);
	if (ret < 0)
		return ret;

	memset(&msg, 0, sizeof(msg));
	msg.header = PD_HEADER_LE(PD_DATA_REQUEST,
				  port->pwr_role,
				  port->data_role,
				  port->negotiated_rev,
				  port->message_id, 1);
	msg.payload[0] = cpu_to_le32(rdo);

	return tcpm_pd_transmit(dev, TCPC_TX_SOP, &msg);
}

static int tcpm_set_vbus(struct udevice *dev, bool enable)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	const struct dm_tcpm_ops *drvops = dev_get_driver_ops(dev);
	int ret;

	if (enable && port->vbus_charge)
		return -EINVAL;

	dev_dbg(dev, "TCPM: set vbus = %d charge = %d\n",
		enable, port->vbus_charge);

	ret = drvops->set_vbus(dev, enable, port->vbus_charge);
	if (ret < 0)
		return ret;

	port->vbus_source = enable;
	return 0;
}

static int tcpm_set_charge(struct udevice *dev, bool charge)
{
	const struct dm_tcpm_ops *drvops = dev_get_driver_ops(dev);
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	int ret;

	if (charge && port->vbus_source)
		return -EINVAL;

	if (charge != port->vbus_charge) {
		dev_dbg(dev, "TCPM: set vbus = %d charge = %d\n",
			port->vbus_source, charge);
		ret = drvops->set_vbus(dev, port->vbus_source,
					   charge);
		if (ret < 0)
			return ret;
	}
	port->vbus_charge = charge;
	return 0;
}

static bool tcpm_start_toggling(struct udevice *dev, enum typec_cc_status cc)
{
	const struct dm_tcpm_ops *drvops = dev_get_driver_ops(dev);
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	int ret;

	if (!drvops->start_toggling)
		return false;

	dev_dbg(dev, "TCPM: Start toggling\n");
	ret = drvops->start_toggling(dev, port->port_type, cc);
	return ret == 0;
}

static int tcpm_init_vbus(struct udevice *dev)
{
	const struct dm_tcpm_ops *drvops = dev_get_driver_ops(dev);
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	int ret;

	ret = drvops->set_vbus(dev, false, false);
	port->vbus_source = false;
	port->vbus_charge = false;
	return ret;
}

static int tcpm_init_vconn(struct udevice *dev)
{
	const struct dm_tcpm_ops *drvops = dev_get_driver_ops(dev);
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	int ret;

	ret = drvops->set_vconn(dev, false);
	port->vconn_role = TYPEC_SINK;
	return ret;
}

static inline void tcpm_typec_connect(struct tcpm_port *port)
{
	if (!port->connected)
		port->connected = true;
}

static int tcpm_src_attach(struct udevice *dev)
{
	const struct dm_tcpm_ops *drvops = dev_get_driver_ops(dev);
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	enum typec_cc_polarity polarity =
				port->cc2 == TYPEC_CC_RD ? TYPEC_POLARITY_CC2
							 : TYPEC_POLARITY_CC1;
	int ret;

	if (port->attached)
		return 0;

	ret = tcpm_set_polarity(dev, polarity);
	if (ret < 0)
		return ret;

	ret = tcpm_set_roles(dev, true, TYPEC_SOURCE, TYPEC_HOST);
	if (ret < 0)
		return ret;

	ret = drvops->set_pd_rx(dev, true);
	if (ret < 0)
		goto out_disable_mux;

	/*
	 * USB Type-C specification, version 1.2,
	 * chapter 4.5.2.2.8.1 (Attached.SRC Requirements)
	 * Enable VCONN only if the non-RD port is set to RA.
	 */
	if ((polarity == TYPEC_POLARITY_CC1 && port->cc2 == TYPEC_CC_RA) ||
	    (polarity == TYPEC_POLARITY_CC2 && port->cc1 == TYPEC_CC_RA)) {
		ret = tcpm_set_vconn(dev, true);
		if (ret < 0)
			goto out_disable_pd;
	}

	ret = tcpm_set_vbus(dev, true);
	if (ret < 0)
		goto out_disable_vconn;

	port->pd_capable = false;

	port->partner = NULL;

	port->attached = true;

	return 0;

out_disable_vconn:
	tcpm_set_vconn(dev, false);
out_disable_pd:
	drvops->set_pd_rx(dev, false);
out_disable_mux:
	dev_err(dev, "TCPM: CC connected in %s as DFP\n",
		polarity ? "CC2" : "CC1");
	return 0;
}

static inline void tcpm_typec_disconnect(struct tcpm_port *port)
{
	if (port->connected) {
		port->partner = NULL;
		port->connected = false;
	}
}

static void tcpm_reset_port(struct udevice *dev)
{
	const struct dm_tcpm_ops *drvops = dev_get_driver_ops(dev);
	struct tcpm_port *port = dev_get_uclass_plat(dev);

	tcpm_timer_uninit(dev);
	tcpm_typec_disconnect(port);
	tcpm_reset_event_cnt(dev);
	port->wait_dr_swap_message = false;
	port->attached = false;
	port->pd_capable = false;

	/*
	 * First Rx ID should be 0; set this to a sentinel of -1 so that
	 * we can check tcpm_pd_rx_handler() if we had seen it before.
	 */
	port->rx_msgid = -1;

	drvops->set_pd_rx(dev, false);
	tcpm_init_vbus(dev);	/* also disables charging */
	tcpm_init_vconn(dev);
	tcpm_set_current_limit(dev, 0, 0);
	tcpm_set_polarity(dev, TYPEC_POLARITY_CC1);
	tcpm_set_attached_state(dev, false);
	port->nr_sink_caps = 0;
}

static void tcpm_detach(struct udevice *dev)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);

	if (tcpm_port_is_disconnected(port))
		port->hard_reset_count = 0;

	if (!port->attached)
		return;

	tcpm_reset_port(dev);
}

static void tcpm_src_detach(struct udevice *dev)
{
	tcpm_detach(dev);
}

static int tcpm_snk_attach(struct udevice *dev)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	int ret;

	if (port->attached)
		return 0;

	ret = tcpm_set_polarity(dev, port->cc2 != TYPEC_CC_OPEN ?
				TYPEC_POLARITY_CC2 : TYPEC_POLARITY_CC1);
	if (ret < 0)
		return ret;

	ret = tcpm_set_roles(dev, true, TYPEC_SINK, TYPEC_DEVICE);
	if (ret < 0)
		return ret;

	port->pd_capable = false;

	port->partner = NULL;

	port->attached = true;
	dev_info(dev, "TCPM: CC connected in %s as UFP\n",
		 port->cc1 != TYPEC_CC_OPEN ? "CC1" : "CC2");

	return 0;
}

static void tcpm_snk_detach(struct udevice *dev)
{
	tcpm_detach(dev);
}

static inline enum tcpm_state hard_reset_state(struct tcpm_port *port)
{
	if (port->hard_reset_count < PD_N_HARD_RESET_COUNT)
		return HARD_RESET_SEND;
	if (port->pd_capable)
		return ERROR_RECOVERY;
	if (port->pwr_role == TYPEC_SOURCE)
		return SRC_UNATTACHED;
	if (port->state == SNK_WAIT_CAPABILITIES)
		return SNK_READY;
	return SNK_UNATTACHED;
}

static inline enum tcpm_state unattached_state(struct tcpm_port *port)
{
	if (port->port_type == TYPEC_PORT_DRP) {
		if (port->pwr_role == TYPEC_SOURCE)
			return SRC_UNATTACHED;
		else
			return SNK_UNATTACHED;
	} else if (port->port_type == TYPEC_PORT_SRC) {
		return SRC_UNATTACHED;
	}

	return SNK_UNATTACHED;
}

static void run_state_machine(struct udevice *dev)
{
	const struct dm_tcpm_ops *drvops = dev_get_driver_ops(dev);
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	int ret;

	port->enter_state = port->state;
	switch (port->state) {
	case TOGGLING:
		break;
	/* SRC states */
	case SRC_UNATTACHED:
		tcpm_src_detach(dev);
		if (tcpm_start_toggling(dev, tcpm_rp_cc(port))) {
			tcpm_set_state(dev, TOGGLING, 0);
			break;
		}
		tcpm_set_cc(dev, tcpm_rp_cc(port));
		if (port->port_type == TYPEC_PORT_DRP)
			tcpm_set_state(dev, SNK_UNATTACHED, PD_T_DRP_SNK);
		break;
	case SRC_ATTACH_WAIT:
		if (tcpm_port_is_source(port))
			tcpm_set_state(dev, SRC_ATTACHED, PD_T_CC_DEBOUNCE);
		break;

	case SRC_ATTACHED:
		ret = tcpm_src_attach(dev);
		/*
		 * Currently, vbus control is not implemented,
		 * and the SRC detection process cannot be fully implemented.
		 */
		tcpm_set_state(dev, SRC_READY, 0);
		break;
	case SRC_STARTUP:
		port->caps_count = 0;
		port->negotiated_rev = PD_MAX_REV;
		port->message_id = 0;
		port->rx_msgid = -1;
		port->explicit_contract = false;
		tcpm_set_state(dev, SRC_SEND_CAPABILITIES, 0);
		break;
	case SRC_SEND_CAPABILITIES:
		port->caps_count++;
		if (port->caps_count > PD_N_CAPS_COUNT) {
			tcpm_set_state(dev, SRC_READY, 0);
			break;
		}
		ret = tcpm_pd_send_source_caps(dev);
		if (ret < 0) {
			tcpm_set_state(dev, SRC_SEND_CAPABILITIES,
				       PD_T_SEND_SOURCE_CAP);
		} else {
			/*
			 * Per standard, we should clear the reset counter here.
			 * However, that can result in state machine hang-ups.
			 * Reset it only in READY state to improve stability.
			 */
			/* port->hard_reset_count = 0; */
			port->caps_count = 0;
			port->pd_capable = true;
			tcpm_set_state_cond(dev, SRC_SEND_CAPABILITIES_TIMEOUT,
					    PD_T_SEND_SOURCE_CAP);
		}
		break;
	case SRC_SEND_CAPABILITIES_TIMEOUT:
		/*
		 * Error recovery for a PD_DATA_SOURCE_CAP reply timeout.
		 *
		 * PD 2.0 sinks are supposed to accept src-capabilities with a
		 * 3.0 header and simply ignore any src PDOs which the sink does
		 * not understand such as PPS but some 2.0 sinks instead ignore
		 * the entire PD_DATA_SOURCE_CAP message, causing contract
		 * negotiation to fail.
		 *
		 * After PD_N_HARD_RESET_COUNT hard-reset attempts, we try
		 * sending src-capabilities with a lower PD revision to
		 * make these broken sinks work.
		 */
		if (port->hard_reset_count < PD_N_HARD_RESET_COUNT) {
			tcpm_set_state(dev, HARD_RESET_SEND, 0);
		} else if (port->negotiated_rev > PD_REV20) {
			port->negotiated_rev--;
			port->hard_reset_count = 0;
			tcpm_set_state(dev, SRC_SEND_CAPABILITIES, 0);
		} else {
			tcpm_set_state(dev, hard_reset_state(port), 0);
		}
		break;
	case SRC_NEGOTIATE_CAPABILITIES:
		ret = tcpm_pd_check_request(dev);
		if (ret < 0) {
			tcpm_pd_send_control(dev, PD_CTRL_REJECT);
			if (!port->explicit_contract) {
				tcpm_set_state(dev,
					       SRC_WAIT_NEW_CAPABILITIES, 0);
			} else {
				tcpm_set_state(dev, SRC_READY, 0);
			}
		} else {
			tcpm_pd_send_control(dev, PD_CTRL_ACCEPT);
			tcpm_set_state(dev, SRC_TRANSITION_SUPPLY,
				       PD_T_SRC_TRANSITION);
		}
		break;
	case SRC_TRANSITION_SUPPLY:
		/* XXX: regulator_set_voltage(vbus, ...) */
		tcpm_pd_send_control(dev, PD_CTRL_PS_RDY);
		port->explicit_contract = true;
		tcpm_set_state_cond(dev, SRC_READY, 0);
		break;
	case SRC_READY:
		port->hard_reset_count = 0;

		tcpm_typec_connect(port);
		break;
	case SRC_WAIT_NEW_CAPABILITIES:
		/* Nothing to do... */
		break;

	/* SNK states */
	case SNK_UNATTACHED:
		tcpm_snk_detach(dev);
		if (tcpm_start_toggling(dev, TYPEC_CC_RD)) {
			tcpm_set_state(dev, TOGGLING, 0);
			break;
		}
		tcpm_set_cc(dev, TYPEC_CC_RD);
		if (port->port_type == TYPEC_PORT_DRP)
			tcpm_set_state(dev, SRC_UNATTACHED, PD_T_DRP_SRC);
		break;
	case SNK_ATTACH_WAIT:
		if ((port->cc1 == TYPEC_CC_OPEN &&
		     port->cc2 != TYPEC_CC_OPEN) ||
		    (port->cc1 != TYPEC_CC_OPEN &&
		     port->cc2 == TYPEC_CC_OPEN))
			tcpm_set_state(dev, SNK_DEBOUNCED,
				       PD_T_CC_DEBOUNCE);
		else if (tcpm_port_is_disconnected(port))
			tcpm_set_state(dev, SNK_UNATTACHED,
				       PD_T_CC_DEBOUNCE);
		break;
	case SNK_DEBOUNCED:
		if (tcpm_port_is_disconnected(port))
			tcpm_set_state(dev, SNK_UNATTACHED, PD_T_PD_DEBOUNCE);
		else if (port->vbus_present)
			tcpm_set_state(dev, SNK_ATTACHED, 0);
		else
			/* Wait for VBUS, but not forever */
			tcpm_set_state(dev, PORT_RESET, PD_T_PS_SOURCE_ON);
		break;
	case SNK_ATTACHED:
		ret = tcpm_snk_attach(dev);
		if (ret < 0)
			tcpm_set_state(dev, SNK_UNATTACHED, 0);
		else
			tcpm_set_state(dev, SNK_STARTUP, 0);
		break;
	case SNK_STARTUP:
		port->negotiated_rev = PD_MAX_REV;
		port->message_id = 0;
		port->rx_msgid = -1;
		port->explicit_contract = false;
		tcpm_set_state(dev, SNK_DISCOVERY, 0);
		break;
	case SNK_DISCOVERY:
		if (port->vbus_present) {
			tcpm_set_current_limit(dev,
					       tcpm_get_current_limit(port),
					       5000);
			tcpm_set_charge(dev, true);
			tcpm_set_state(dev, SNK_WAIT_CAPABILITIES, 0);
			break;
		}
		/*
		 * For DRP, timeouts differ. Also, handling is supposed to be
		 * different and much more complex (dead battery detection;
		 * see USB power delivery specification, section 8.3.3.6.1.5.1).
		 */
		tcpm_set_state(dev, hard_reset_state(port),
			       port->port_type == TYPEC_PORT_DRP ?
					PD_T_DB_DETECT : PD_T_NO_RESPONSE);
		break;
	case SNK_DISCOVERY_DEBOUNCE:
		tcpm_set_state(dev, SNK_DISCOVERY_DEBOUNCE_DONE,
			       PD_T_CC_DEBOUNCE);
		break;
	case SNK_DISCOVERY_DEBOUNCE_DONE:
		tcpm_set_state(dev, unattached_state(port), 0);
		break;
	case SNK_WAIT_CAPABILITIES:
		ret = drvops->set_pd_rx(dev, true);
		if (ret < 0) {
			tcpm_set_state(dev, SNK_READY, 0);
			break;
		}
		/*
		 * If VBUS has never been low, and we time out waiting
		 * for source cap, try a soft reset first, in case we
		 * were already in a stable contract before this boot.
		 * Do this only once.
		 */
		if (port->vbus_never_low) {
			port->vbus_never_low = false;
			tcpm_set_state(dev, SOFT_RESET_SEND,
				       PD_T_SINK_WAIT_CAP);
		} else {
			tcpm_set_state(dev, hard_reset_state(port),
				       PD_T_SINK_WAIT_CAP);
		}
		break;
	case SNK_NEGOTIATE_CAPABILITIES:
		port->pd_capable = true;
		port->hard_reset_count = 0;
		ret = tcpm_pd_send_request(dev);
		if (ret < 0) {
			/* Let the Source send capabilities again. */
			tcpm_set_state(dev, SNK_WAIT_CAPABILITIES, 0);
		} else {
			tcpm_set_state_cond(dev, hard_reset_state(port),
					    PD_T_SENDER_RESPONSE);
		}
		break;
	case SNK_TRANSITION_SINK:
	case SNK_TRANSITION_SINK_VBUS:
		tcpm_set_state(dev, hard_reset_state(port),
			       PD_T_PS_TRANSITION);
		break;
	case SNK_READY:
		port->update_sink_caps = false;
		tcpm_typec_connect(port);
		/*
		 * Here poll_event_cnt is cleared, waiting for self-powered Type-C devices
		 * to send DR_swap Messge until 1s (TCPM_POLL_EVENT_TIME_OUT * 500us)timeout
		 */
		if (port->wait_dr_swap_message)
			tcpm_reset_event_cnt(dev);

		break;

	/* Hard_Reset states */
	case HARD_RESET_SEND:
		tcpm_pd_transmit(dev, TCPC_TX_HARD_RESET, NULL);
		tcpm_set_state(dev, HARD_RESET_START, 0);
		port->wait_dr_swap_message = false;
		break;
	case HARD_RESET_START:
		port->hard_reset_count++;
		drvops->set_pd_rx(dev, false);
		port->nr_sink_caps = 0;
		if (port->pwr_role == TYPEC_SOURCE)
			tcpm_set_state(dev, SRC_HARD_RESET_VBUS_OFF,
				       PD_T_PS_HARD_RESET);
		else
			tcpm_set_state(dev, SNK_HARD_RESET_SINK_OFF, 0);
		break;
	case SRC_HARD_RESET_VBUS_OFF:
		tcpm_set_vconn(dev, true);
		tcpm_set_vbus(dev, false);
		tcpm_set_roles(dev, port->self_powered, TYPEC_SOURCE,
			       TYPEC_HOST);
		tcpm_set_state(dev, SRC_HARD_RESET_VBUS_ON, PD_T_SRC_RECOVER);
		break;
	case SRC_HARD_RESET_VBUS_ON:
		tcpm_set_vconn(dev, true);
		tcpm_set_vbus(dev, true);
		drvops->set_pd_rx(dev, true);
		tcpm_set_attached_state(dev, true);
		tcpm_set_state(dev, SRC_UNATTACHED, PD_T_PS_SOURCE_ON);
		break;
	case SNK_HARD_RESET_SINK_OFF:
		tcpm_set_vconn(dev, false);
		if (port->pd_capable)
			tcpm_set_charge(dev, false);
		tcpm_set_roles(dev, port->self_powered, TYPEC_SINK,
			       TYPEC_DEVICE);
		/*
		 * VBUS may or may not toggle, depending on the adapter.
		 * If it doesn't toggle, transition to SNK_HARD_RESET_SINK_ON
		 * directly after timeout.
		 */
		tcpm_set_state(dev, SNK_HARD_RESET_SINK_ON, PD_T_SAFE_0V);
		break;
	case SNK_HARD_RESET_WAIT_VBUS:
		/* Assume we're disconnected if VBUS doesn't come back. */
		tcpm_set_state(dev, SNK_UNATTACHED,
			       PD_T_SRC_RECOVER_MAX + PD_T_SRC_TURN_ON);
		break;
	case SNK_HARD_RESET_SINK_ON:
		/* Note: There is no guarantee that VBUS is on in this state */
		/*
		 * XXX:
		 * The specification suggests that dual mode ports in sink
		 * mode should transition to state PE_SRC_Transition_to_default.
		 * See USB power delivery specification chapter 8.3.3.6.1.3.
		 * This would mean to
		 * - turn off VCONN, reset power supply
		 * - request hardware reset
		 * - turn on VCONN
		 * - Transition to state PE_Src_Startup
		 * SNK only ports shall transition to state Snk_Startup
		 * (see chapter 8.3.3.3.8).
		 * Similar, dual-mode ports in source mode should transition
		 * to PE_SNK_Transition_to_default.
		 */
		if (port->pd_capable) {
			tcpm_set_current_limit(dev,
					       tcpm_get_current_limit(port),
					       5000);
			tcpm_set_charge(dev, true);
		}
		tcpm_set_attached_state(dev, true);
		tcpm_set_state(dev, SNK_STARTUP, 0);
		break;

	/* Soft_Reset states */
	case SOFT_RESET:
		port->message_id = 0;
		port->rx_msgid = -1;
		tcpm_pd_send_control(dev, PD_CTRL_ACCEPT);
		if (port->pwr_role == TYPEC_SOURCE)
			tcpm_set_state(dev, SRC_SEND_CAPABILITIES, 0);
		else
			tcpm_set_state(dev, SNK_WAIT_CAPABILITIES, 0);
		break;
	case SOFT_RESET_SEND:
		port->message_id = 0;
		port->rx_msgid = -1;
		if (tcpm_pd_send_control(dev, PD_CTRL_SOFT_RESET))
			tcpm_set_state_cond(dev, hard_reset_state(port), 0);
		else
			tcpm_set_state_cond(dev, hard_reset_state(port),
					    PD_T_SENDER_RESPONSE);
		break;

	/* DR_Swap states */
	case DR_SWAP_ACCEPT:
		tcpm_pd_send_control(dev, PD_CTRL_ACCEPT);
		tcpm_set_state_cond(dev, DR_SWAP_CHANGE_DR, 0);
		break;
	case DR_SWAP_CHANGE_DR:
		if (port->data_role == TYPEC_HOST) {
			tcpm_set_roles(dev, true, port->pwr_role,
				       TYPEC_DEVICE);
		} else {
			tcpm_set_roles(dev, true, port->pwr_role,
				       TYPEC_HOST);
		}
		/* DR_swap process complete, wait_dr_swap_message is cleared */
		port->wait_dr_swap_message = false;
		tcpm_set_state(dev, ready_state(port), 0);
		break;
	case ERROR_RECOVERY:
		tcpm_set_state(dev, PORT_RESET, 0);
		break;
	case PORT_RESET:
		tcpm_reset_port(dev);
		if (port->self_powered)
			tcpm_set_cc(dev, TYPEC_CC_OPEN);
		else
			tcpm_set_cc(dev, tcpm_default_state(port) == SNK_UNATTACHED ?
				    TYPEC_CC_RD : tcpm_rp_cc(port));
		tcpm_set_state(dev, PORT_RESET_WAIT_OFF,
			       PD_T_ERROR_RECOVERY);
		break;
	case PORT_RESET_WAIT_OFF:
		tcpm_set_state(dev,
			       tcpm_default_state(port),
			       port->vbus_present ? PD_T_PS_SOURCE_OFF : 0);
		break;
	default:
		dev_err(dev, "TCPM: Unexpected port state %d\n", port->state);
		break;
	}
}

static void tcpm_state_machine(struct udevice *dev)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	enum tcpm_state prev_state;

	mutex_lock(&port->lock);
	port->state_machine_running = true;

	if (port->queued_message && tcpm_send_queued_message(dev))
		goto done;

	/* If we were queued due to a delayed state change, update it now */
	if (port->delayed_state) {
		dev_dbg(dev, "TCPM: state change %s -> %s [delayed %ld ms]\n",
			tcpm_states[port->state],
			tcpm_states[port->delayed_state], port->delay_ms);
		port->prev_state = port->state;
		port->state = port->delayed_state;
		port->delayed_state = INVALID_STATE;
	}

	/*
	 * Continue running as long as we have (non-delayed) state changes
	 * to make.
	 */
	do {
		prev_state = port->state;
		run_state_machine(dev);
		if (port->queued_message)
			tcpm_send_queued_message(dev);
	} while (port->state != prev_state && !port->delayed_state);

done:
	port->state_machine_running = false;
	mutex_unlock(&port->lock);
}

static void _tcpm_cc_change(struct udevice *dev, enum typec_cc_status cc1,
			    enum typec_cc_status cc2)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	enum typec_cc_status old_cc1, old_cc2;
	enum tcpm_state new_state;

	old_cc1 = port->cc1;
	old_cc2 = port->cc2;
	port->cc1 = cc1;
	port->cc2 = cc2;

	dev_dbg(dev, "TCPM: CC1: %u -> %u, CC2: %u -> %u [state %s, polarity %d, %s]\n",
		old_cc1, cc1, old_cc2, cc2, tcpm_states[port->state],
		port->polarity,
		tcpm_port_is_disconnected(port) ? "disconnected" : "connected");

	switch (port->state) {
	case TOGGLING:
		if (tcpm_port_is_source(port))
			tcpm_set_state(dev, SRC_ATTACH_WAIT, 0);
		else if (tcpm_port_is_sink(port))
			tcpm_set_state(dev, SNK_ATTACH_WAIT, 0);
		break;
	case SRC_UNATTACHED:
	case SRC_ATTACH_WAIT:
		if (tcpm_port_is_disconnected(port))
			tcpm_set_state(dev, SRC_UNATTACHED, 0);
		else if (cc1 != old_cc1 || cc2 != old_cc2)
			tcpm_set_state(dev, SRC_ATTACH_WAIT, 0);
		break;
	case SRC_ATTACHED:
	case SRC_SEND_CAPABILITIES:
	case SRC_READY:
		if (tcpm_port_is_disconnected(port) ||
		    !tcpm_port_is_source(port))
			tcpm_set_state(dev, SRC_UNATTACHED, 0);
		break;
	case SNK_UNATTACHED:
		if (tcpm_port_is_sink(port))
			tcpm_set_state(dev, SNK_ATTACH_WAIT, 0);
		break;
	case SNK_ATTACH_WAIT:
		if ((port->cc1 == TYPEC_CC_OPEN &&
		     port->cc2 != TYPEC_CC_OPEN) ||
		    (port->cc1 != TYPEC_CC_OPEN &&
		     port->cc2 == TYPEC_CC_OPEN))
			new_state = SNK_DEBOUNCED;
		else if (tcpm_port_is_disconnected(port))
			new_state = SNK_UNATTACHED;
		else
			break;
		if (new_state != port->delayed_state)
			tcpm_set_state(dev, SNK_ATTACH_WAIT, 0);
		break;
	case SNK_DEBOUNCED:
		if (tcpm_port_is_disconnected(port))
			new_state = SNK_UNATTACHED;
		else if (port->vbus_present)
			new_state = tcpm_try_src(port) ? INVALID_STATE : SNK_ATTACHED;
		else
			new_state = SNK_UNATTACHED;
		if (new_state != port->delayed_state)
			tcpm_set_state(dev, SNK_DEBOUNCED, 0);
		break;
	case SNK_READY:
		if (tcpm_port_is_disconnected(port))
			tcpm_set_state(dev, unattached_state(port), 0);
		else if (!port->pd_capable &&
			 (cc1 != old_cc1 || cc2 != old_cc2))
			tcpm_set_current_limit(dev,
					       tcpm_get_current_limit(port),
					       5000);
		break;

	case SNK_DISCOVERY:
		/* CC line is unstable, wait for debounce */
		if (tcpm_port_is_disconnected(port))
			tcpm_set_state(dev, SNK_DISCOVERY_DEBOUNCE, 0);
		break;
	case SNK_DISCOVERY_DEBOUNCE:
		break;

	case PORT_RESET:
	case PORT_RESET_WAIT_OFF:
		/*
		 * State set back to default mode once the timer completes.
		 * Ignore CC changes here.
		 */
		break;
	default:
		/*
		 * While acting as sink and auto vbus discharge is enabled, Allow disconnect
		 * to be driven by vbus disconnect.
		 */
		if (tcpm_port_is_disconnected(port))
			tcpm_set_state(dev, unattached_state(port), 0);
		break;
	}
}

static void _tcpm_pd_vbus_on(struct udevice *dev)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);

	dev_dbg(dev, "TCPM: VBUS on event\n");
	port->vbus_present = true;
	/*
	 * When vbus_present is true i.e. Voltage at VBUS is greater than VSAFE5V implicitly
	 * states that vbus is not at VSAFE0V, hence clear the vbus_vsafe0v flag here.
	 */
	port->vbus_vsafe0v = false;

	switch (port->state) {
	case SNK_TRANSITION_SINK_VBUS:
		port->explicit_contract = true;
		tcpm_set_state(dev, SNK_READY, 0);
		break;
	case SNK_DISCOVERY:
		tcpm_set_state(dev, SNK_DISCOVERY, 0);
		break;
	case SNK_DEBOUNCED:
		tcpm_set_state(dev, SNK_ATTACHED, 0);
		break;
	case SNK_HARD_RESET_WAIT_VBUS:
		tcpm_set_state(dev, SNK_HARD_RESET_SINK_ON, 0);
		break;
	case SRC_ATTACHED:
		tcpm_set_state(dev, SRC_STARTUP, 0);
		break;
	case SRC_HARD_RESET_VBUS_ON:
		tcpm_set_state(dev, SRC_STARTUP, 0);
		break;

	case PORT_RESET:
	case PORT_RESET_WAIT_OFF:
		/*
		 * State set back to default mode once the timer completes.
		 * Ignore vbus changes here.
		 */
		break;

	default:
		break;
	}
}

static void _tcpm_pd_vbus_off(struct udevice *dev)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);

	dev_dbg(dev, "TCPM: VBUS off event\n");
	port->vbus_present = false;
	port->vbus_never_low = false;
	switch (port->state) {
	case SNK_HARD_RESET_SINK_OFF:
		tcpm_set_state(dev, SNK_HARD_RESET_WAIT_VBUS, 0);
		break;
	case HARD_RESET_SEND:
		break;
	case SNK_ATTACH_WAIT:
		tcpm_set_state(dev, SNK_UNATTACHED, 0);
		break;

	case SNK_NEGOTIATE_CAPABILITIES:
		break;

	case PORT_RESET_WAIT_OFF:
		tcpm_set_state(dev, tcpm_default_state(port), 0);
		break;

	case PORT_RESET:
		/*
		 * State set back to default mode once the timer completes.
		 * Ignore vbus changes here.
		 */
		break;

	default:
		if (port->pwr_role == TYPEC_SINK && port->attached)
			tcpm_set_state(dev, SNK_UNATTACHED, 0);
		break;
	}
}

void tcpm_cc_change(struct udevice *dev)
{
	const struct dm_tcpm_ops *drvops = dev_get_driver_ops(dev);
	enum typec_cc_status cc1, cc2;

	tcpm_reset_event_cnt(dev);
	if (drvops->get_cc(dev, &cc1, &cc2) == 0)
		_tcpm_cc_change(dev, cc1, cc2);
}

void tcpm_vbus_change(struct udevice *dev)
{
	const struct dm_tcpm_ops *drvops = dev_get_driver_ops(dev);
	bool vbus;

	tcpm_reset_event_cnt(dev);
	vbus = drvops->get_vbus(dev);
	if (vbus)
		_tcpm_pd_vbus_on(dev);
	else
		_tcpm_pd_vbus_off(dev);
}

void tcpm_pd_hard_reset(struct udevice *dev)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);

	tcpm_reset_event_cnt(dev);
	dev_dbg(dev, "TCPM: Received hard reset\n");

	/* If a hard reset message is received during the port reset process,
	 * we should ignore it, that is, do not set port->state to HARD_RESET_START.
	 */
	if (port->state == PORT_RESET || port->state == PORT_RESET_WAIT_OFF)
		return;

	/*
	 * If we keep receiving hard reset requests, executing the hard reset
	 * must have failed. Revert to error recovery if that happens.
	 */
	tcpm_set_state(dev,
		       port->hard_reset_count < PD_N_HARD_RESET_COUNT ?
				HARD_RESET_START : ERROR_RECOVERY,
		       0);
}

static void tcpm_init(struct udevice *dev)
{
	const struct dm_tcpm_ops *drvops = dev_get_driver_ops(dev);
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	enum typec_cc_status cc1, cc2;

	drvops->init(dev);

	tcpm_reset_port(dev);

	/*
	 * XXX
	 * Should possibly wait for VBUS to settle if it was enabled locally
	 * since tcpm_reset_port() will disable VBUS.
	 */
	port->vbus_present = drvops->get_vbus(dev);
	if (port->vbus_present)
		port->vbus_never_low = true;

	/*
	 * 1. When vbus_present is true, voltage on VBUS is already at VSAFE5V.
	 * So implicitly vbus_vsafe0v = false.
	 *
	 * 2. When vbus_present is false and TCPC does NOT support querying
	 * vsafe0v status, then, it's best to assume vbus is at VSAFE0V i.e.
	 * vbus_vsafe0v is true.
	 *
	 * 3. When vbus_present is false and TCPC does support querying vsafe0v,
	 * then, query tcpc for vsafe0v status.
	 */
	if (port->vbus_present)
		port->vbus_vsafe0v = false;
	else
		port->vbus_vsafe0v = true;

	tcpm_set_state(dev, tcpm_default_state(port), 0);

	if (drvops->get_cc(dev, &cc1, &cc2) == 0)
		_tcpm_cc_change(dev, cc1, cc2);
}

static int tcpm_fw_get_caps(struct udevice *dev)
{
	const struct dm_tcpm_ops *drvops = dev_get_driver_ops(dev);
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	ofnode node;
	const char *cap_str;
	int ret;
	u32 mw;

	ret = drvops->get_connector_node(dev, &node);
	if (ret)
		return ret;

	cap_str = ofnode_read_string(node, "power-role");
	if (!cap_str)
		return -EINVAL;

	if (!strcmp("dual", cap_str))
		port->typec_type = TYPEC_PORT_DRP;
	else if (!strcmp("source", cap_str))
		port->typec_type = TYPEC_PORT_SRC;
	else if (!strcmp("sink", cap_str))
		port->typec_type = TYPEC_PORT_SNK;
	else
		return -EINVAL;

	port->port_type = port->typec_type;

	if (port->port_type == TYPEC_PORT_SNK)
		goto sink;

	/* Get source pdos */
	ret = ofnode_read_size(node, "source-pdos") / sizeof(u32);
	if (ret <= 0)
		return -EINVAL;

	port->nr_src_pdo = min(ret, PDO_MAX_OBJECTS);
	ret = ofnode_read_u32_array(node, "source-pdos",
				    port->src_pdo, port->nr_src_pdo);
	if (ret || tcpm_validate_caps(dev, port->src_pdo, port->nr_src_pdo))
		return -EINVAL;

	if (port->port_type == TYPEC_PORT_SRC)
		return 0;

	/* Get the preferred power role for DRP */
	cap_str = ofnode_read_string(node, "try-power-role");
	if (!cap_str)
		return -EINVAL;

	if (!strcmp("sink", cap_str))
		port->typec_prefer_role = TYPEC_SINK;
	else if (!strcmp("source", cap_str))
		port->typec_prefer_role = TYPEC_SOURCE;
	else
		return -EINVAL;

	if (port->typec_prefer_role < 0)
		return -EINVAL;
sink:
	/* Get sink pdos */
	ret = ofnode_read_size(node, "sink-pdos") / sizeof(u32);
	if (ret <= 0)
		return -EINVAL;

	port->nr_snk_pdo = min(ret, PDO_MAX_OBJECTS);
	ret = ofnode_read_u32_array(node, "sink-pdos",
				    port->snk_pdo, port->nr_snk_pdo);
	if (ret || tcpm_validate_caps(dev, port->snk_pdo, port->nr_snk_pdo))
		return -EINVAL;

	if (ofnode_read_u32_array(node, "op-sink-microwatt", &mw, 1))
		return -EINVAL;
	port->operating_snk_mw = mw / 1000;

	port->self_powered = ofnode_read_bool(node, "self-powered");

	return 0;
}

static int tcpm_port_init(struct udevice *dev)
{
	struct tcpm_port *port = dev_get_uclass_plat(dev);
	int err;

	err = tcpm_fw_get_caps(dev);
	if (err < 0) {
		dev_err(dev, "TCPM: please check the dts config: %d\n", err);
		return err;
	}

	port->try_role = port->typec_prefer_role;
	port->port_type = port->typec_type;

	tcpm_init(dev);

	dev_info(dev, "TCPM: init finished\n");

	return 0;
}

static void tcpm_poll_event(struct udevice *dev)
{
	const struct dm_tcpm_ops *drvops = dev_get_driver_ops(dev);
	struct tcpm_port *port = dev_get_uclass_plat(dev);

	if (!drvops->get_vbus(dev))
		return;

	while (port->poll_event_cnt < TCPM_POLL_EVENT_TIME_OUT) {
		if (!port->wait_dr_swap_message &&
		    (port->state == SNK_READY || port->state == SRC_READY))
			break;

		drvops->poll_event(dev);
		port->poll_event_cnt++;
		udelay(500);
		tcpm_check_and_run_delayed_work(dev);
	}

	if (port->state != SNK_READY && port->state != SRC_READY)
		dev_warn(dev, "TCPM: exit in state %s\n",
			 tcpm_states[port->state]);

	/*
	 * At this time, call the callback function of the respective pd chip
	 * to enter the low-power mode. In order to reduce the time spent on
	 * the PD chip driver as much as possible, the tcpm framework does not
	 * fully process the communication initiated by the device,so it should
	 * be noted that we can disable the internal oscillator, etc., but do
	 * not turn off the power of the transceiver module, otherwise the
	 * self-powered Type-C device will initiate a Message(eg: self-powered
	 * Type-C hub initiates a SINK capability request(PD_CTRL_GET_SINK_CAP))
	 * and the pd chip cannot reply to GoodCRC, causing the self-powered Type-C
	 * device to switch vbus to vSafe5v, or even turn off vbus.
	 */
	if (!drvops->enter_low_power_mode)
		return;

	if (drvops->enter_low_power_mode(dev, port->attached, port->pd_capable))
		dev_err(dev, "TCPM: failed to enter low power\n");
	else
		dev_info(dev, "TCPM: PD chip enter low power mode\n");
}

int tcpm_post_probe(struct udevice *dev)
{
	int ret = tcpm_port_init(dev);

	if (ret < 0) {
		dev_err(dev, "failed to tcpm port init\n");
		return ret;
	}

	tcpm_poll_event(dev);

	return 0;
}
