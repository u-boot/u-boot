# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2017, Heinrich Schuchardt <xypron.glpk@gmx.de>

""" Test UEFI API implementation
"""

import pytest

@pytest.mark.buildconfigspec('cmd_bootefi_selftest')
def test_efi_selftest_base(ubman):
    """Run UEFI unit tests

    ubman -- U-Boot console

    This function executes all selftests that are not marked as on request.
    """
    ubman.run_command(cmd='setenv efi_selftest')
    ubman.run_command(cmd='bootefi selftest', wait_for_prompt=False)
    if ubman.p.expect(['Summary: 0 failures', 'Press any key']):
        raise Exception('Failures occurred during the EFI selftest')
    ubman.restart_uboot()

@pytest.mark.buildconfigspec('cmd_bootefi_selftest')
@pytest.mark.buildconfigspec('hush_parser')
@pytest.mark.buildconfigspec('of_control')
@pytest.mark.notbuildconfigspec('generate_acpi_table')
def test_efi_selftest_device_tree(ubman):
    """Test the device tree support in the UEFI sub-system

    ubman -- U-Boot console

    This test executes the UEFI unit test by calling 'bootefi selftest'.
    """
    ubman.run_command(cmd='setenv efi_selftest list')
    output = ubman.run_command('bootefi selftest')
    assert '\'device tree\'' in output
    ubman.run_command(cmd='setenv efi_selftest device tree')
    # Set serial# if it is not already set.
    ubman.run_command(cmd='setenv efi_test "${serial#}x"')
    ubman.run_command(cmd='test "${efi_test}" = x && setenv serial# 0')
    ubman.run_command(cmd='bootefi selftest ${fdtcontroladdr}', wait_for_prompt=False)
    if ubman.p.expect(['serial-number:', 'U-Boot']):
        raise Exception('serial-number missing in device tree')
    ubman.restart_uboot()

@pytest.mark.buildconfigspec('cmd_bootefi_selftest')
def test_efi_selftest_watchdog_reboot(ubman):
    """Test the watchdog timer

    ubman -- U-Boot console

    This function executes the 'watchdog reboot' unit test.
    """
    ubman.run_command(cmd='setenv efi_selftest list')
    output = ubman.run_command('bootefi selftest')
    assert '\'watchdog reboot\'' in output
    ubman.run_command(cmd='setenv efi_selftest watchdog reboot')
    ubman.run_command(cmd='bootefi selftest', wait_for_prompt=False)
    if ubman.p.expect(['resetting', 'U-Boot']):
        raise Exception('Reset failed in \'watchdog reboot\' test')
    ubman.run_command(cmd='', send_nl=False, wait_for_reboot=True)

@pytest.mark.buildconfigspec('cmd_bootefi_selftest')
def test_efi_selftest_text_input(ubman):
    """Test the EFI_SIMPLE_TEXT_INPUT_PROTOCOL

    ubman -- U-Boot console

    This function calls the text input EFI selftest.
    """
    ubman.run_command(cmd='setenv efi_selftest text input')
    ubman.run_command(cmd='bootefi selftest', wait_for_prompt=False)
    if ubman.p.expect([r'To terminate type \'x\'']):
        raise Exception('No prompt for \'text input\' test')
    ubman.drain_console()
    # EOT
    ubman.run_command(cmd=chr(4), wait_for_echo=False,
                               send_nl=False, wait_for_prompt=False)
    if ubman.p.expect([r'Unicode char 4 \(unknown\), scan code 0 \(Null\)']):
        raise Exception('EOT failed in \'text input\' test')
    ubman.drain_console()
    # BS
    ubman.run_command(cmd=chr(8), wait_for_echo=False,
                               send_nl=False, wait_for_prompt=False)
    if ubman.p.expect([r'Unicode char 8 \(BS\), scan code 0 \(Null\)']):
        raise Exception('BS failed in \'text input\' test')
    ubman.drain_console()
    # TAB
    ubman.run_command(cmd=chr(9), wait_for_echo=False,
                               send_nl=False, wait_for_prompt=False)
    if ubman.p.expect([r'Unicode char 9 \(TAB\), scan code 0 \(Null\)']):
        raise Exception('BS failed in \'text input\' test')
    ubman.drain_console()
    # a
    ubman.run_command(cmd='a', wait_for_echo=False, send_nl=False,
                               wait_for_prompt=False)
    if ubman.p.expect([r'Unicode char 97 \(\'a\'\), scan code 0 \(Null\)']):
        raise Exception('\'a\' failed in \'text input\' test')
    ubman.drain_console()
    # UP escape sequence
    ubman.run_command(cmd=chr(27) + '[A', wait_for_echo=False,
                               send_nl=False, wait_for_prompt=False)
    if ubman.p.expect([r'Unicode char 0 \(Null\), scan code 1 \(Up\)']):
        raise Exception('UP failed in \'text input\' test')
    ubman.drain_console()
    # Euro sign
    ubman.run_command(cmd=b'\xe2\x82\xac'.decode(), wait_for_echo=False,
                               send_nl=False, wait_for_prompt=False)
    if ubman.p.expect([r'Unicode char 8364 \(\'']):
        raise Exception('Euro sign failed in \'text input\' test')
    ubman.drain_console()
    ubman.run_command(cmd='x', wait_for_echo=False, send_nl=False,
                               wait_for_prompt=False)
    if ubman.p.expect(['Summary: 0 failures', 'Press any key']):
        raise Exception('Failures occurred during the EFI selftest')
    ubman.restart_uboot()

@pytest.mark.buildconfigspec('cmd_bootefi_selftest')
def test_efi_selftest_text_input_ex(ubman):
    """Test the EFI_SIMPLE_TEXT_INPUT_EX_PROTOCOL

    ubman -- U-Boot console

    This function calls the extended text input EFI selftest.
    """
    ubman.run_command(cmd='setenv efi_selftest extended text input')
    ubman.run_command(cmd='bootefi selftest', wait_for_prompt=False)
    if ubman.p.expect([r'To terminate type \'CTRL\+x\'']):
        raise Exception('No prompt for \'text input\' test')
    ubman.drain_console()
    # EOT
    ubman.run_command(cmd=chr(4), wait_for_echo=False,
                               send_nl=False, wait_for_prompt=False)
    if ubman.p.expect([r'Unicode char 100 \(\'d\'\), scan code 0 \(CTRL\+Null\)']):
        raise Exception('EOT failed in \'text input\' test')
    ubman.drain_console()
    # BS
    ubman.run_command(cmd=chr(8), wait_for_echo=False,
                               send_nl=False, wait_for_prompt=False)
    if ubman.p.expect([r'Unicode char 8 \(BS\), scan code 0 \(\+Null\)']):
        raise Exception('BS failed in \'text input\' test')
    ubman.drain_console()
    # TAB
    ubman.run_command(cmd=chr(9), wait_for_echo=False,
                               send_nl=False, wait_for_prompt=False)
    if ubman.p.expect([r'Unicode char 9 \(TAB\), scan code 0 \(\+Null\)']):
        raise Exception('TAB failed in \'text input\' test')
    ubman.drain_console()
    # a
    ubman.run_command(cmd='a', wait_for_echo=False, send_nl=False,
                               wait_for_prompt=False)
    if ubman.p.expect([r'Unicode char 97 \(\'a\'\), scan code 0 \(Null\)']):
        raise Exception('\'a\' failed in \'text input\' test')
    ubman.drain_console()
    # UP escape sequence
    ubman.run_command(cmd=chr(27) + '[A', wait_for_echo=False,
                               send_nl=False, wait_for_prompt=False)
    if ubman.p.expect([r'Unicode char 0 \(Null\), scan code 1 \(\+Up\)']):
        raise Exception('UP failed in \'text input\' test')
    ubman.drain_console()
    # Euro sign
    ubman.run_command(cmd=b'\xe2\x82\xac'.decode(), wait_for_echo=False,
                               send_nl=False, wait_for_prompt=False)
    if ubman.p.expect([r'Unicode char 8364 \(\'']):
        raise Exception('Euro sign failed in \'text input\' test')
    ubman.drain_console()
    # SHIFT+ALT+FN 5
    ubman.run_command(cmd=b'\x1b\x5b\x31\x35\x3b\x34\x7e'.decode(),
                               wait_for_echo=False, send_nl=False,
                               wait_for_prompt=False)
    if ubman.p.expect([r'Unicode char 0 \(Null\), scan code 15 \(SHIFT\+ALT\+FN 5\)']):
        raise Exception('SHIFT+ALT+FN 5 failed in \'text input\' test')
    ubman.drain_console()
    ubman.run_command(cmd=chr(24), wait_for_echo=False, send_nl=False,
                               wait_for_prompt=False)
    if ubman.p.expect(['Summary: 0 failures', 'Press any key']):
        raise Exception('Failures occurred during the EFI selftest')
    ubman.restart_uboot()

@pytest.mark.buildconfigspec('cmd_bootefi_selftest')
@pytest.mark.buildconfigspec('efi_tcg2_protocol')
@pytest.mark.notbuildconfigspec('sandbox')
def test_efi_selftest_tcg2(ubman):
    """Test the EFI_TCG2 PROTOCOL

    ubman -- U-Boot console

    This function executes the 'tcg2' unit test.
    """
    ubman.restart_uboot()
    ubman.run_command(cmd='setenv efi_selftest list')
    output = ubman.run_command('bootefi selftest')
    assert '\'tcg2\'' in output
    ubman.run_command(cmd='setenv efi_selftest tcg2')
    ubman.run_command(cmd='bootefi selftest', wait_for_prompt=False)
    if ubman.p.expect(['Summary: 0 failures', 'Press any key']):
        raise Exception('Failures occurred during the EFI selftest')
    ubman.restart_uboot()
