# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2018, NVIDIA CORPORATION. All rights reserved.

# Test U-Boot's "mmc read" command. The test reads data from the eMMC or SD
# card, and validates the no errors occurred, and that the expected data was
# read if the test configuration contains a CRC of the expected data.

import pytest
import u_boot_utils

"""
This test relies on boardenv_* to containing configuration values to define
which MMC devices should be tested. For example:

env__mmc_rd_configs = (
    {
        "fixture_id": "emmc-boot0",
        "is_emmc": True,
        "devid": 0,
        "partid": 1,
        "sector": 0x10,
        "count": 1,
    },
    {
        "fixture_id": "emmc-boot1",
        "is_emmc": True,
        "devid": 0,
        "partid": 2,
        "sector": 0x10,
        "count": 1,
    },
    {
        "fixture_id": "emmc-data",
        "is_emmc": True,
        "devid": 0,
        "partid": 0,
        "sector": 0x10,
        "count": 0x1000,
    },
    {
        "fixture_id": "sd-mbr",
        "is_emmc": False,
        "devid": 1,
        "partid": None,
        "sector": 0,
        "count": 1,
        "crc32": "8f6ecf0d",
    },
    {
        "fixture_id": "sd-large",
        "is_emmc": False,
        "devid": 1,
        "partid": None,
        "sector": 0x10,
        "count": 0x1000,
    },
)
"""

@pytest.mark.buildconfigspec('cmd_mmc')
def test_mmc_rd(u_boot_console, env__mmc_rd_config):
    """Test the "mmc read" command.

    Args:
        u_boot_console: A U-Boot console connection.
        env__mmc_rd_config: The single MMC configuration on which
            to run the test. See the file-level comment above for details
            of the format.

    Returns:
        Nothing.
    """

    is_emmc = env__mmc_rd_config['is_emmc']
    devid = env__mmc_rd_config['devid']
    partid = env__mmc_rd_config.get('partid', 0)
    sector = env__mmc_rd_config.get('sector', 0)
    count_sectors = env__mmc_rd_config.get('count', 1)
    expected_crc32 = env__mmc_rd_config.get('crc32', None)

    count_bytes = count_sectors * 512
    bcfg = u_boot_console.config.buildconfig
    has_cmd_memory = bcfg.get('config_cmd_memory', 'n') == 'y'
    has_cmd_crc32 = bcfg.get('config_cmd_crc32', 'n') == 'y'
    ram_base = u_boot_utils.find_ram_base(u_boot_console)
    addr = '0x%08x' % ram_base

    # Select MMC device
    cmd = 'mmc dev %d' % devid
    if is_emmc:
        cmd += ' %d' % partid
    response = u_boot_console.run_command(cmd)
    assert 'no card present' not in response
    if is_emmc:
        partid_response = "(part %d)" % partid
    else:
        partid_response = ""
    good_response = 'mmc%d%s is current device' % (devid, partid_response)
    assert good_response in response

    # Clear target RAM
    if expected_crc32:
        if has_cmd_memory and has_cmd_crc32:
            cmd = 'mw.b %s 0 0x%x' % (addr, count_bytes)
            u_boot_console.run_command(cmd)

            cmd = 'crc32 %s 0x%x' % (addr, count_bytes)
            response = u_boot_console.run_command(cmd)
            assert expected_crc32 not in response
        else:
            u_boot_console.log.warning(
                'CONFIG_CMD_MEMORY or CONFIG_CMD_CRC32 != y: Skipping RAM clear')

    # Read data
    cmd = 'mmc read %s %x %x' % (addr, sector, count_sectors)
    response = u_boot_console.run_command(cmd)
    good_response = 'MMC read: dev # %d, block # %d, count %d ... %d blocks read: OK' % (
        devid, sector, count_sectors, count_sectors)
    assert good_response in response

    # Check target RAM
    if expected_crc32:
        if has_cmd_crc32:
            cmd = 'crc32 %s 0x%x' % (addr, count_bytes)
            response = u_boot_console.run_command(cmd)
            assert expected_crc32 in response
        else:
            u_boot_console.log.warning('CONFIG_CMD_CRC32 != y: Skipping check')
