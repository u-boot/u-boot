# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2025 Linaro Limited
# Author: Raymond Mao <raymond.mao@linaro.org>
#
# Validate Firmware Handoff from TF-A and OP-TEE

import pytest
import re

def _norm_ws(s: str) -> str:
    """Normalize whitespace for robust comparisons."""
    return re.sub(r"\s+", " ", s).strip()

@pytest.mark.buildconfigspec("bloblist")
@pytest.mark.buildconfigspec("cmd_bloblist")
@pytest.mark.buildconfigspec("of_control")
@pytest.mark.buildconfigspec("cmd_fdt")
def test_fw_handoff_dt(ubman):
    """Validate FDT handoff via bloblist and $fdt_addr."""

    if (ubman.config.board_type != 'qemu_arm64' or
        ubman.config.board_identity != 'fw_handoff_tfa_optee'):
        pytest.skip("Test only for qemu_arm64 with ID fw_handoff_tfa_optee")

    bloblist = ubman.run_command("bloblist list")
    blob_fdt = re.search(r"^([0-9a-fA-F]+)\s+[0-9a-fA-F]+\s+1\s+Control FDT\s*$",
                         bloblist, re.MULTILINE)
    assert blob_fdt, "Control FDT entry not found in bloblist output"

    blob_fdt_addr = int(blob_fdt.group(1), 16)
    ubman.run_command(f"fdt addr {blob_fdt_addr:x}")

    reserved_a = ubman.run_command("fdt print /reserved-memory")
    firmware_a = ubman.run_command("fdt print /firmware")

    fdt_addr_out = ubman.run_command("echo $fdt_addr")
    fdt_addr_match = re.search(r"(?:0x)?([0-9a-fA-F]+)", fdt_addr_out)
    assert fdt_addr_match, "Could not parse $fdt_addr"

    fdt_addr = int(fdt_addr_match.group(1), 16)
    ubman.run_command(f"fdt addr {fdt_addr:x}")

    reserved_b = ubman.run_command("fdt print /reserved-memory")
    firmware_b = ubman.run_command("fdt print /firmware")

    # Normalize whitespace & compare
    assert _norm_ws(reserved_a) == _norm_ws(reserved_b), \
        "reserved-memory blocks differ between Control FDT and $fdt_addr FDT"
    assert _norm_ws(firmware_a) == _norm_ws(firmware_b), \
        "firmware blocks differ between Control FDT and $fdt_addr FDT"

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
