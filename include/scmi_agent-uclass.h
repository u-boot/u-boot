/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2019-2020 Linaro Limited.
 */
#ifndef _SCMI_AGENT_UCLASS_H
#define _SCMI_AGENT_UCLASS_H

struct udevice;
struct scmi_msg;
struct scmi_channel;

/**
 * struct scmi_transport_ops - The functions that a SCMI transport layer must implement.
 */
struct scmi_agent_ops {
	/*
	 * of_get_channel - Get SCMI channel from SCMI agent device tree node
	 *
	 * @dev:		SCMI protocol device using the transport
	 * @channel:		Output reference to SCMI channel upon success
	 * Return 0 upon success and a negative errno on failure
	 */
	int (*of_get_channel)(struct udevice *dev, struct scmi_channel **channel);

	/*
	 * process_msg - Request transport to get the SCMI message processed
	 *
	 * @dev:		SCMI protocol device using the transport
	 * @msg:		SCMI message to be transmitted
	 */
	int (*process_msg)(struct udevice *dev, struct scmi_channel *channel,
			   struct scmi_msg *msg);
};

#endif /* _SCMI_TRANSPORT_UCLASS_H */
