/*
 * Copyright (C) 2013, Intel Corporation
 * Copyright (C) 2015 Google, Inc
 *
 * SPDX-License-Identifier:	Intel
 */

#ifndef _FSP_AZALIA_H_
#define _FSP_AZALIA_H_

struct __packed pch_azalia_verb_table_header {
	uint32_t vendor_device_id;
	uint16_t sub_system_id;
	uint8_t revision_id;		/* 0xff applies to all steppings */
	uint8_t front_panel_support;
	uint16_t number_of_rear_jacks;
	uint16_t number_of_front_jacks;
};

struct __packed pch_azalia_verb_table {
	struct pch_azalia_verb_table_header verb_table_header;
	const uint32_t *verb_table_data;
};

struct __packed pch_azalia_config {
	uint8_t pme_enable:1;
	uint8_t docking_supported:1;
	uint8_t docking_attached:1;
	uint8_t hdmi_codec_enable:1;
	uint8_t azalia_v_ci_enable:1;
	uint8_t rsvdbits:3;
	/* number of verb tables provided by platform */
	uint8_t azalia_verb_table_num;
	const struct pch_azalia_verb_table *azalia_verb_table;
	/* delay timer after azalia reset */
	uint16_t reset_wait_timer_us;
};

#endif
