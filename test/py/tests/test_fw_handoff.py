# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2025 Linaro Limited
# Author: Raymond Mao <raymond.mao@linaro.org>
#
# Validate Firmware Handoff from TF-A and OP-TEE

import pytest
import re

@pytest.mark.buildconfigspec("bloblist")
@pytest.mark.buildconfigspec("cmd_bloblist")
def test_fw_handoff_dt(ubman):
    """Validate handoff of the FDT via bloblist to $fdt_addr."""

    if (ubman.config.board_type != 'qemu_arm64' or
        ubman.config.board_identity != 'fw_handoff_tfa_optee'):
        pytest.skip("This test is only for qemu_arm64 with ID fw_handoff_tfa_optee")

    # Get the address from $fdt_addr
    fdt_addr_out = ubman.run_command("echo $fdt_addr")
    fdt_addr_match = re.search(r"(?:0x)?([0-9a-fA-F]+)", fdt_addr_out)
    assert fdt_addr_match, "Could not parse fdt_addr"
    fdt_addr = int(fdt_addr_match.group(1), 16)

    ubman.run_command(f"fdt addr {fdt_addr:x}")
    fdt_output = ubman.run_command("fdt print")

    # 'reserved-memory' and 'firmware' nodes are appended runtime by OP-TEE,
    # thus the existence indicates that the Firmware Handoff works properly.
    expected_keywords = [
        "reserved-memory",
        "optee_core",
        "optee_shm",
        "firmware",
        "optee"
    ]

    for keyword in expected_keywords:
        assert keyword in fdt_output, f"Missing expected FDT node: {keyword}"

@pytest.mark.buildconfigspec("bloblist")
@pytest.mark.buildconfigspec("cmd_bloblist")
@pytest.mark.buildconfigspec("cmd_memory")
def test_fw_handoff_eventlog(ubman):
    """Validate expected events exist in the TPM event log blob."""

    if (ubman.config.board_type != 'qemu_arm64' or
        ubman.config.board_identity != 'fw_handoff_tfa_optee'):
        pytest.skip("This test is only for qemu_arm64 with ID fw_handoff_tfa_optee")

    # Get the address and size of eventlog from the bloblist
    bloblist_output = ubman.run_command("bloblist list")
    evt_addr = None
    evt_size = None
    for line in bloblist_output.splitlines():
        if "TPM event log" in line:
            parts = line.strip().split()
            evt_addr = int(parts[0], 16)
            evt_size = int(parts[1], 16)
            break

    assert evt_addr is not None and evt_size is not None, \
        "TPM event log not found in bloblist"

    # Read byte from memory and extract printable ASCII from each line
    md_output = ubman.run_command(f"md.b {evt_addr:x} {evt_size}")
    ascii_log = ""
    for line in md_output.splitlines():
        match = re.search(r'([0-9a-f]+:.*?)((?:\s[0-9a-f]{2}){1,16})\s+(.*)', line)
        if match:
            ascii_part = match.group(3).strip()
            ascii_log += ascii_part

    # The events created by TF-A are expected
    expected_keywords = [
        "SECURE_RT_EL3",
        "SECURE_RT_EL1_OPTEE",
        "SECURE_RT_EL1_OPTEE_EXTRA1"
    ]

    for keyword in expected_keywords:
        assert keyword in ascii_log, f"Missing expected event: {keyword}"
