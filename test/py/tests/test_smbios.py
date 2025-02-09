# SPDX-License-Identifier: GPL-2.0-or-later

"""Test smbios command"""

import pytest

@pytest.mark.buildconfigspec('cmd_smbios')
@pytest.mark.notbuildconfigspec('qfw_smbios')
@pytest.mark.notbuildconfigspec('sandbox')
def test_cmd_smbios(ubman):
    """Run the smbios command"""
    output = ubman.run_command('smbios')
    assert 'DMI type 127,' in output

@pytest.mark.buildconfigspec('cmd_smbios')
@pytest.mark.buildconfigspec('qfw_smbios')
@pytest.mark.notbuildconfigspec('sandbox')
# TODO:
# QEMU v8.2.0 lacks SMBIOS support for RISC-V
# Once support is available in our Docker image we can remove the constraint.
@pytest.mark.notbuildconfigspec('riscv')
def test_cmd_smbios_qemu(ubman):
    """Run the smbios command on QEMU"""
    output = ubman.run_command('smbios')
    assert 'DMI type 1,' in output
    assert 'Manufacturer: QEMU' in output
    assert 'DMI type 127,' in output

@pytest.mark.buildconfigspec('cmd_smbios')
@pytest.mark.buildconfigspec('sandbox')
def test_cmd_smbios_sandbox(ubman):
    """Run the smbios command on the sandbox"""
    output = ubman.run_command('smbios')
    assert 'DMI type 0,' in output
    assert 'Vendor: U-Boot' in output
    assert 'DMI type 1,' in output
    assert 'Manufacturer: sandbox' in output
    assert 'DMI type 2,' in output
    assert 'DMI type 3,' in output
    assert 'DMI type 4,' in output
    assert 'DMI type 127,' in output

@pytest.mark.buildconfigspec('cmd_smbios')
@pytest.mark.buildconfigspec('sysinfo_smbios')
@pytest.mark.buildconfigspec('generate_smbios_table_verbose')
def test_cmd_smbios_sysinfo_verbose(ubman):
    """Run the smbios command"""
    output = ubman.run_command('smbios')
    assert 'DMI type 0,' in output
    assert 'Vendor: U-Boot' in output
    assert 'DMI type 1,' in output
    assert 'Manufacturer: linux' in output
    assert 'DMI type 2,' in output
    assert 'DMI type 3,' in output
    assert 'DMI type 7,' in output
    assert 'DMI type 4,' in output
    assert 'DMI type 127,' in output
