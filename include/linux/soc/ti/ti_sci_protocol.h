/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Texas Instruments System Control Interface Protocol
 * Based on include/linux/soc/ti/ti_sci_protocol.h from Linux.
 *
 * Copyright (C) 2018 Texas Instruments Incorporated - http://www.ti.com/
 *	Nishanth Menon
 *	Lokesh Vutla <lokeshvutla@ti.com>
 */

#ifndef __TISCI_PROTOCOL_H
#define __TISCI_PROTOCOL_H

/**
 * struct ti_sci_version_info - version information structure
 * @abi_major:	Major ABI version. Change here implies risk of backward
 *		compatibility break.
 * @abi_minor:	Minor ABI version. Change here implies new feature addition,
 *		or compatible change in ABI.
 * @firmware_revision:	Firmware revision (not usually used).
 * @firmware_description: Firmware description (not usually used).
 */
struct ti_sci_version_info {
	u8 abi_major;
	u8 abi_minor;
	u16 firmware_revision;
	char firmware_description[32];
};

struct ti_sci_handle;

/**
 * struct ti_sci_board_ops - Board config operations
 * @board_config: Command to set the board configuration
 *		  Returns 0 for successful exclusive request, else returns
 *		  corresponding error message.
 * @board_config_rm: Command to set the board resource management
 *		  configuration
 *		  Returns 0 for successful exclusive request, else returns
 *		  corresponding error message.
 * @board_config_security: Command to set the board security configuration
 *		  Returns 0 for successful exclusive request, else returns
 *		  corresponding error message.
 * @board_config_pm: Command to trigger and set the board power and clock
 *		  management related configuration
 *		  Returns 0 for successful exclusive request, else returns
 *		  corresponding error message.
 */
struct ti_sci_board_ops {
	int (*board_config)(const struct ti_sci_handle *handle,
			    u64 addr, u32 size);
	int (*board_config_rm)(const struct ti_sci_handle *handle,
			       u64 addr, u32 size);
	int (*board_config_security)(const struct ti_sci_handle *handle,
				     u64 addr, u32 size);
	int (*board_config_pm)(const struct ti_sci_handle *handle,
			       u64 addr, u32 size);
};

/**
 * struct ti_sci_dev_ops - Device control operations
 * @get_device: Command to request for device managed by TISCI
 *		Returns 0 for successful exclusive request, else returns
 *		corresponding error message.
 * @idle_device: Command to idle a device managed by TISCI
 *		Returns 0 for successful exclusive request, else returns
 *		corresponding error message.
 * @put_device:	Command to release a device managed by TISCI
 *		Returns 0 for successful release, else returns corresponding
 *		error message.
 * @is_valid:	Check if the device ID is a valid ID.
 *		Returns 0 if the ID is valid, else returns corresponding error.
 * @get_context_loss_count: Command to retrieve context loss counter - this
 *		increments every time the device looses context. Overflow
 *		is possible.
 *		- count: pointer to u32 which will retrieve counter
 *		Returns 0 for successful information request and count has
 *		proper data, else returns corresponding error message.
 * @is_idle:	Reports back about device idle state
 *		- req_state: Returns requested idle state
 *		Returns 0 for successful information request and req_state and
 *		current_state has proper data, else returns corresponding error
 *		message.
 * @is_stop:	Reports back about device stop state
 *		- req_state: Returns requested stop state
 *		- current_state: Returns current stop state
 *		Returns 0 for successful information request and req_state and
 *		current_state has proper data, else returns corresponding error
 *		message.
 * @is_on:	Reports back about device ON(or active) state
 *		- req_state: Returns requested ON state
 *		- current_state: Returns current ON state
 *		Returns 0 for successful information request and req_state and
 *		current_state has proper data, else returns corresponding error
 *		message.
 * @is_transitioning: Reports back if the device is in the middle of transition
 *		of state.
 *		-current_state: Returns 'true' if currently transitioning.
 * @set_device_resets: Command to configure resets for device managed by TISCI.
 *		-reset_state: Device specific reset bit field
 *		Returns 0 for successful request, else returns
 *		corresponding error message.
 * @get_device_resets: Command to read state of resets for device managed
 *		by TISCI.
 *		-reset_state: pointer to u32 which will retrieve resets
 *		Returns 0 for successful request, else returns
 *		corresponding error message.
 *
 * NOTE: for all these functions, the following parameters are generic in
 * nature:
 * -handle:	Pointer to TISCI handle as retrieved by *ti_sci_get_handle
 * -id:		Device Identifier
 *
 * Request for the device - NOTE: the client MUST maintain integrity of
 * usage count by balancing get_device with put_device. No refcounting is
 * managed by driver for that purpose.
 */
struct ti_sci_dev_ops {
	int (*get_device)(const struct ti_sci_handle *handle, u32 id);
	int (*idle_device)(const struct ti_sci_handle *handle, u32 id);
	int (*put_device)(const struct ti_sci_handle *handle, u32 id);
	int (*is_valid)(const struct ti_sci_handle *handle, u32 id);
	int (*get_context_loss_count)(const struct ti_sci_handle *handle,
				      u32 id, u32 *count);
	int (*is_idle)(const struct ti_sci_handle *handle, u32 id,
		       bool *requested_state);
	int (*is_stop)(const struct ti_sci_handle *handle, u32 id,
		       bool *req_state, bool *current_state);
	int (*is_on)(const struct ti_sci_handle *handle, u32 id,
		     bool *req_state, bool *current_state);
	int (*is_transitioning)(const struct ti_sci_handle *handle, u32 id,
				bool *current_state);
	int (*set_device_resets)(const struct ti_sci_handle *handle, u32 id,
				 u32 reset_state);
	int (*get_device_resets)(const struct ti_sci_handle *handle, u32 id,
				 u32 *reset_state);
};

/**
 * struct ti_sci_clk_ops - Clock control operations
 * @get_clock:	Request for activation of clock and manage by processor
 *		- needs_ssc: 'true' if Spread Spectrum clock is desired.
 *		- can_change_freq: 'true' if frequency change is desired.
 *		- enable_input_term: 'true' if input termination is desired.
 * @idle_clock:	Request for Idling a clock managed by processor
 * @put_clock:	Release the clock to be auto managed by TISCI
 * @is_auto:	Is the clock being auto managed
 *		- req_state: state indicating if the clock is auto managed
 * @is_on:	Is the clock ON
 *		- req_state: if the clock is requested to be forced ON
 *		- current_state: if the clock is currently ON
 * @is_off:	Is the clock OFF
 *		- req_state: if the clock is requested to be forced OFF
 *		- current_state: if the clock is currently Gated
 * @set_parent:	Set the clock source of a specific device clock
 *		- parent_id: Parent clock identifier to set.
 * @get_parent:	Get the current clock source of a specific device clock
 *		- parent_id: Parent clock identifier which is the parent.
 * @get_num_parents: Get the number of parents of the current clock source
 *		- num_parents: returns the number of parent clocks.
 * @get_best_match_freq: Find a best matching frequency for a frequency
 *		range.
 *		- match_freq: Best matching frequency in Hz.
 * @set_freq:	Set the Clock frequency
 * @get_freq:	Get the Clock frequency
 *		- current_freq: Frequency in Hz that the clock is at.
 *
 * NOTE: for all these functions, the following parameters are generic in
 * nature:
 * -handle:	Pointer to TISCI handle as retrieved by *ti_sci_get_handle
 * -did:	Device identifier this request is for
 * -cid:	Clock identifier for the device for this request.
 *		Each device has it's own set of clock inputs. This indexes
 *		which clock input to modify.
 * -min_freq:	The minimum allowable frequency in Hz. This is the minimum
 *		allowable programmed frequency and does not account for clock
 *		tolerances and jitter.
 * -target_freq: The target clock frequency in Hz. A frequency will be
 *		processed as close to this target frequency as possible.
 * -max_freq:	The maximum allowable frequency in Hz. This is the maximum
 *		allowable programmed frequency and does not account for clock
 *		tolerances and jitter.
 *
 * Request for the clock - NOTE: the client MUST maintain integrity of
 * usage count by balancing get_clock with put_clock. No refcounting is
 * managed by driver for that purpose.
 */
struct ti_sci_clk_ops {
	int (*get_clock)(const struct ti_sci_handle *handle, u32 did, u8 cid,
			 bool needs_ssc, bool can_change_freq,
			 bool enable_input_term);
	int (*idle_clock)(const struct ti_sci_handle *handle, u32 did, u8 cid);
	int (*put_clock)(const struct ti_sci_handle *handle, u32 did, u8 cid);
	int (*is_auto)(const struct ti_sci_handle *handle, u32 did, u8 cid,
		       bool *req_state);
	int (*is_on)(const struct ti_sci_handle *handle, u32 did, u8 cid,
		     bool *req_state, bool *current_state);
	int (*is_off)(const struct ti_sci_handle *handle, u32 did, u8 cid,
		      bool *req_state, bool *current_state);
	int (*set_parent)(const struct ti_sci_handle *handle, u32 did, u8 cid,
			  u8 parent_id);
	int (*get_parent)(const struct ti_sci_handle *handle, u32 did, u8 cid,
			  u8 *parent_id);
	int (*get_num_parents)(const struct ti_sci_handle *handle, u32 did,
			       u8 cid, u8 *num_parents);
	int (*get_best_match_freq)(const struct ti_sci_handle *handle, u32 did,
				   u8 cid, u64 min_freq, u64 target_freq,
				   u64 max_freq, u64 *match_freq);
	int (*set_freq)(const struct ti_sci_handle *handle, u32 did, u8 cid,
			u64 min_freq, u64 target_freq, u64 max_freq);
	int (*get_freq)(const struct ti_sci_handle *handle, u32 did, u8 cid,
			u64 *current_freq);
};

/**
 * struct ti_sci_core_ops - SoC Core Operations
 * @reboot_device: Reboot the SoC
 *		Returns 0 for successful request(ideally should never return),
 *		else returns corresponding error value.
 */
struct ti_sci_core_ops {
	int (*reboot_device)(const struct ti_sci_handle *handle);
};

/**
 * struct ti_sci_proc_ops - Processor specific operations.
 *
 * @proc_request: Request for controlling a physical processor.
 *		The requesting host should be in the processor access list.
 * @proc_release: Relinquish a physical processor control
 * @proc_handover: Handover a physical processor control to another host
 *		   in the permitted list.
 * @set_proc_boot_cfg: Base configuration of the processor
 * @set_proc_boot_ctrl: Setup limited control flags in specific cases.
 * @proc_auth_boot_image:
 * @get_proc_boot_status: Get the state of physical processor
 *
 * NOTE: for all these functions, the following parameters are generic in
 * nature:
 * -handle:	Pointer to TISCI handle as retrieved by *ti_sci_get_handle
 * -pid:	Processor ID
 *
 */
struct ti_sci_proc_ops {
	int (*proc_request)(const struct ti_sci_handle *handle, u8 pid);
	int (*proc_release)(const struct ti_sci_handle *handle, u8 pid);
	int (*proc_handover)(const struct ti_sci_handle *handle, u8 pid,
			     u8 hid);
	int (*set_proc_boot_cfg)(const struct ti_sci_handle *handle, u8 pid,
				 u64 bv, u32 cfg_set, u32 cfg_clr);
	int (*set_proc_boot_ctrl)(const struct ti_sci_handle *handle, u8 pid,
				  u32 ctrl_set, u32 ctrl_clr);
	int (*proc_auth_boot_image)(const struct ti_sci_handle *handle, u8 pid,
				    u64 caddr);
	int (*get_proc_boot_status)(const struct ti_sci_handle *handle, u8 pid,
				    u64 *bv, u32 *cfg_flags, u32 *ctrl_flags,
				    u32 *sts_flags);
};

/**
 * struct ti_sci_ops - Function support for TI SCI
 * @board_ops:	Miscellaneous operations
 * @dev_ops:	Device specific operations
 * @clk_ops:	Clock specific operations
 * @core_ops:	Core specific operations
 * @proc_ops:	Processor specific operations
 */
struct ti_sci_ops {
	struct ti_sci_board_ops board_ops;
	struct ti_sci_dev_ops dev_ops;
	struct ti_sci_clk_ops clk_ops;
	struct ti_sci_core_ops core_ops;
	struct ti_sci_proc_ops proc_ops;
};

/**
 * struct ti_sci_handle - Handle returned to TI SCI clients for usage.
 * @ops:	operations that are made available to TI SCI clients
 * @version:	structure containing version information
 */
struct ti_sci_handle {
	struct ti_sci_ops ops;
	struct ti_sci_version_info version;
};

#if IS_ENABLED(CONFIG_TI_SCI_PROTOCOL)

const struct ti_sci_handle *ti_sci_get_handle_from_sysfw(struct udevice *dev);
const struct ti_sci_handle *ti_sci_get_handle(struct udevice *dev);
const struct ti_sci_handle *ti_sci_get_by_phandle(struct udevice *dev,
						  const char *property);

#else	/* CONFIG_TI_SCI_PROTOCOL */

static inline
const struct ti_sci_handle *ti_sci_get_handle_from_sysfw(struct udevice *dev)
{
	return ERR_PTR(-EINVAL);
}

static inline const struct ti_sci_handle *ti_sci_get_handle(struct udevice *dev)
{
	return ERR_PTR(-EINVAL);
}

static inline
const struct ti_sci_handle *ti_sci_get_by_phandle(struct udevice *dev,
						  const char *property)
{
	return ERR_PTR(-EINVAL);
}
#endif	/* CONFIG_TI_SCI_PROTOCOL */

#endif	/* __TISCI_PROTOCOL_H */
