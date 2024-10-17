/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright 2015-2017 Google, Inc
 * Copyright 2024 Collabora
 */

#ifndef __LINUX_USB_TCPM_H
#define __LINUX_USB_TCPM_H

#include <dm/of.h>
#include <linux/bitops.h>
#include "pd.h"

enum typec_orientation {
	TYPEC_ORIENTATION_NONE,
	TYPEC_ORIENTATION_NORMAL,
	TYPEC_ORIENTATION_REVERSE,
};

enum typec_cc_status {
	TYPEC_CC_OPEN,
	TYPEC_CC_RA,
	TYPEC_CC_RD,
	TYPEC_CC_RP_DEF,
	TYPEC_CC_RP_1_5,
	TYPEC_CC_RP_3_0,
};

enum typec_cc_polarity {
	TYPEC_POLARITY_CC1,
	TYPEC_POLARITY_CC2,
};

enum tcpm_transmit_status {
	TCPC_TX_SUCCESS = 0,
	TCPC_TX_DISCARDED = 1,
	TCPC_TX_FAILED = 2,
};

enum tcpm_transmit_type {
	TCPC_TX_SOP = 0,
	TCPC_TX_SOP_PRIME = 1,
	TCPC_TX_SOP_PRIME_PRIME = 2,
	TCPC_TX_SOP_DEBUG_PRIME = 3,
	TCPC_TX_SOP_DEBUG_PRIME_PRIME = 4,
	TCPC_TX_HARD_RESET = 5,
	TCPC_TX_CABLE_RESET = 6,
	TCPC_TX_BIST_MODE_2 = 7
};

struct dm_tcpm_ops {
	int (*get_connector_node)(struct udevice *dev, ofnode *connector_node);
	int (*init)(struct udevice *dev);
	int (*get_vbus)(struct udevice *dev);
	int (*set_cc)(struct udevice *dev, enum typec_cc_status cc);
	int (*get_cc)(struct udevice *dev, enum typec_cc_status *cc1,
		      enum typec_cc_status *cc2);
	int (*set_polarity)(struct udevice *dev,
			    enum typec_cc_polarity polarity);
	int (*set_vconn)(struct udevice *dev, bool on);
	int (*set_vbus)(struct udevice *dev, bool on, bool charge);
	int (*set_pd_rx)(struct udevice *dev, bool on);
	int (*set_roles)(struct udevice *dev, bool attached,
			 enum typec_role role, enum typec_data_role data);
	int (*start_toggling)(struct udevice *dev,
			      enum typec_port_type port_type,
			      enum typec_cc_status cc);
	int (*pd_transmit)(struct udevice *dev, enum tcpm_transmit_type type,
			   const struct pd_message *msg, unsigned int negotiated_rev);
	void (*poll_event)(struct udevice *dev);
	int (*enter_low_power_mode)(struct udevice *dev, bool attached, bool pd_capable);
};

/* API for drivers */
void tcpm_vbus_change(struct udevice *dev);
void tcpm_cc_change(struct udevice *dev);
void tcpm_pd_receive(struct udevice *dev, const struct pd_message *msg);
void tcpm_pd_transmit_complete(struct udevice *dev,
			       enum tcpm_transmit_status status);
void tcpm_pd_hard_reset(struct udevice *dev);

/* API for boards */
extern const char * const typec_pd_rev_name[];
extern const char * const typec_orientation_name[];
extern const char * const typec_role_name[];
extern const char * const typec_data_role_name[];
extern const char * const typec_cc_status_name[];

int tcpm_get(int index, struct udevice **devp);
int tcpm_get_pd_rev(struct udevice *dev);
int tcpm_get_current(struct udevice *dev);
int tcpm_get_voltage(struct udevice *dev);
enum typec_orientation tcpm_get_orientation(struct udevice *dev);
enum typec_role tcpm_get_pwr_role(struct udevice *dev);
enum typec_data_role tcpm_get_data_role(struct udevice *dev);
bool tcpm_is_connected(struct udevice *dev);
const char *tcpm_get_state(struct udevice *dev);

#endif /* __LINUX_USB_TCPM_H */
