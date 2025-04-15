# SPDX-License-Identifier: GPL-2.0+
# Copyright (c) 2018, Bootlin
# Author: Miquel Raynal <miquel.raynal@bootlin.com>

import os.path
import pytest
import utils
import re
import time

"""
Test the TPMv2.x related commands. You must have a working hardware setup in
order to do these tests.

Notes:
* These tests will prove the password mechanism. The TPM chip must be cleared of
any password.
* Commands like pcr_setauthpolicy and pcr_resetauthpolicy are not implemented
here because they would fail the tests in most cases (TPMs do not implement them
and return an error).


Note:
This test doesn't rely on boardenv_* configuration value but can change test
behavior.

* Setup env__tpm_device_test_skip to True if tests with TPM devices should be
skipped.

"""

updates = 0

def force_init(ubman, force=False):
    """When a test fails, U-Boot is reset. Because TPM stack must be initialized
    after each reboot, we must ensure these lines are always executed before
    trying any command or they will fail with no reason. Executing 'tpm init'
    twice will spawn an error used to detect that the TPM was not reset and no
    initialization code should be run.
    """
    skip_test = ubman.config.env.get('env__tpm_device_test_skip', False)
    if skip_test:
        pytest.skip('skip TPM device test')
    output = ubman.run_command('tpm2 autostart')
    if force or not 'Error' in output:
        ubman.run_command('echo --- start of init ---')
        ubman.run_command('tpm2 clear TPM2_RH_LOCKOUT')
        output = ubman.run_command('echo $?')
        if not output.endswith('0'):
            ubman.run_command('tpm2 clear TPM2_RH_PLATFORM')
        ubman.run_command('echo --- end of init ---')

def is_sandbox(ubman):
    # Array slice removes leading/trailing quotes.
    sys_arch = ubman.config.buildconfig.get('config_sys_arch', '"sandbox"')[1:-1]
    return sys_arch == 'sandbox'

@pytest.mark.buildconfigspec('cmd_tpm_v2')
def test_tpm2_init(ubman):
    """Init the software stack to use TPMv2 commands."""
    skip_test = ubman.config.env.get('env__tpm_device_test_skip', False)
    if skip_test:
        pytest.skip('skip TPM device test')
    ubman.run_command('tpm2 autostart')
    output = ubman.run_command('echo $?')
    assert output.endswith('0')

@pytest.mark.buildconfigspec('cmd_tpm_v2')
def test_tpm2_startup(ubman):
    """Execute a TPM2_Startup command.

    Initiate the TPM internal state machine.
    """
    skip_test = ubman.config.env.get('env__tpm_device_test_skip', False)
    if skip_test:
        pytest.skip('skip TPM device test')
    ubman.run_command('tpm2 startup TPM2_SU_CLEAR')
    output = ubman.run_command('echo $?')
    assert output.endswith('0')

def tpm2_sandbox_init(ubman):
    """Put sandbox back into a known state so we can run a test

    This allows all tests to run in parallel, since no test depends on another.
    """
    ubman.restart_uboot()
    ubman.run_command('tpm2 autostart')
    output = ubman.run_command('echo $?')
    assert output.endswith('0')

    skip_test = ubman.config.env.get('env__tpm_device_test_skip', False)
    if skip_test:
        pytest.skip('skip TPM device test')

@pytest.mark.buildconfigspec('cmd_tpm_v2')
def test_tpm2_sandbox_self_test_full(ubman):
    """Execute a TPM2_SelfTest (full) command.

    Ask the TPM to perform all self tests to also enable full capabilities.
    """
    if is_sandbox(ubman):
        ubman.restart_uboot()
        ubman.run_command('tpm2 autostart')
        output = ubman.run_command('echo $?')
        assert output.endswith('0')

        ubman.run_command('tpm2 startup TPM2_SU_CLEAR')
        output = ubman.run_command('echo $?')
        assert output.endswith('0')

    skip_test = ubman.config.env.get('env__tpm_device_test_skip', False)
    if skip_test:
        pytest.skip('skip TPM device test')
    ubman.run_command('tpm2 self_test full')
    output = ubman.run_command('echo $?')
    assert output.endswith('0')

@pytest.mark.buildconfigspec('cmd_tpm_v2')
def test_tpm2_continue_self_test(ubman):
    """Execute a TPM2_SelfTest (continued) command.

    Ask the TPM to finish its self tests (alternative to the full test) in order
    to enter a fully operational state.
    """

    skip_test = ubman.config.env.get('env__tpm_device_test_skip', False)
    if skip_test:
        pytest.skip('skip TPM device test')
    if is_sandbox(ubman):
        tpm2_sandbox_init(ubman)
    ubman.run_command('tpm2 self_test continue')
    output = ubman.run_command('echo $?')
    assert output.endswith('0')

@pytest.mark.buildconfigspec('cmd_tpm_v2')
def test_tpm2_clear(ubman):
    """Execute a TPM2_Clear command.

    Ask the TPM to reset entirely its internal state (including internal
    configuration, passwords, counters and DAM parameters). This is half of the
    TAKE_OWNERSHIP command from TPMv1.

    Use the LOCKOUT hierarchy for this. The LOCKOUT/PLATFORM hierarchies must
    not have a password set, otherwise this test will fail. ENDORSEMENT and
    PLATFORM hierarchies are also available.
    """
    if is_sandbox(ubman):
        tpm2_sandbox_init(ubman)

    skip_test = ubman.config.env.get('env__tpm_device_test_skip', False)
    if skip_test:
        pytest.skip('skip TPM device test')
    ubman.run_command('tpm2 clear TPM2_RH_LOCKOUT')
    output = ubman.run_command('echo $?')
    assert output.endswith('0')

    ubman.run_command('tpm2 clear TPM2_RH_PLATFORM')
    output = ubman.run_command('echo $?')
    assert output.endswith('0')

@pytest.mark.buildconfigspec('cmd_tpm_v2')
def test_tpm2_change_auth(ubman):
    """Execute a TPM2_HierarchyChangeAuth command.

    Ask the TPM to change the owner, ie. set a new password: 'unicorn'

    Use the LOCKOUT hierarchy for this. ENDORSEMENT and PLATFORM hierarchies are
    also available.
    """
    if is_sandbox(ubman):
        tpm2_sandbox_init(ubman)
    force_init(ubman)

    ubman.run_command('tpm2 change_auth TPM2_RH_LOCKOUT unicorn')
    output = ubman.run_command('echo $?')
    assert output.endswith('0')

    ubman.run_command('tpm2 clear TPM2_RH_LOCKOUT unicorn')
    output = ubman.run_command('echo $?')
    ubman.run_command('tpm2 clear TPM2_RH_PLATFORM')
    assert output.endswith('0')

@pytest.mark.buildconfigspec('sandbox')
@pytest.mark.buildconfigspec('cmd_tpm_v2')
def test_tpm2_get_capability(ubman):
    """Execute a TPM_GetCapability command.

    Display one capability. In our test case, let's display the default DAM
    lockout counter that should be 0 since the CLEAR:
    - TPM_CAP_TPM_PROPERTIES = 0x6
    - TPM_PT_LOCKOUT_COUNTER (1st parameter) = PTR_VAR + 14

    There is no expected default values because it would depend on the chip
    used. We can still save them in order to check they have changed later.
    """
    if is_sandbox(ubman):
        tpm2_sandbox_init(ubman)

    force_init(ubman)
    ram = utils.find_ram_base(ubman)

    read_cap = ubman.run_command('tpm2 get_capability 0x6 0x20e 0x200 1') #0x%x 1' % ram)
    output = ubman.run_command('echo $?')
    assert output.endswith('0')
    assert 'Property 0x0000020e: 0x00000000' in read_cap

@pytest.mark.buildconfigspec('cmd_tpm_v2')
def test_tpm2_dam_parameters(ubman):
    """Execute a TPM2_DictionaryAttackParameters command.

    Change Dictionary Attack Mitigation (DAM) parameters. Ask the TPM to change:
    - Max number of failed authentication before lockout: 3
    - Time before the failure counter is automatically decremented: 10 sec
    - Time after a lockout failure before it can be attempted again: 0 sec

    For an unknown reason, the DAM parameters must be changed before changing
    the authentication, otherwise the lockout will be engaged after the first
    failed authentication attempt.
    """
    if is_sandbox(ubman):
        tpm2_sandbox_init(ubman)
    force_init(ubman)
    ram = utils.find_ram_base(ubman)

    # Set the DAM parameters to known values
    ubman.run_command('tpm2 dam_parameters 3 10 0')
    output = ubman.run_command('echo $?')
    assert output.endswith('0')

    # Check the values have been saved
    read_cap = ubman.run_command('tpm2 get_capability 0x6 0x20f 0x%x 3' % ram)
    output = ubman.run_command('echo $?')
    assert output.endswith('0')
    assert 'Property 0x0000020f: 0x00000003' in read_cap
    assert 'Property 0x00000210: 0x0000000a' in read_cap
    assert 'Property 0x00000211: 0x00000000' in read_cap

@pytest.mark.buildconfigspec('cmd_tpm_v2')
def test_tpm2_pcr_read(ubman):
    """Execute a TPM2_PCR_Read command.

    Perform a PCR read of the 10th PCR. Must be zero.
    """
    if is_sandbox(ubman):
        tpm2_sandbox_init(ubman)

    force_init(ubman)
    ram = utils.find_ram_base(ubman)

    read_pcr = ubman.run_command('tpm2 pcr_read 10 0x%x' % ram)
    output = ubman.run_command('echo $?')
    assert output.endswith('0')

    # Save the number of PCR updates
    str = re.findall(r'\d+ known updates', read_pcr)[0]
    global updates
    updates = int(re.findall(r'\d+', str)[0])

    # Check the output value
    assert 'PCR #10 sha256 32 byte content' in read_pcr
    assert '00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00' in read_pcr

@pytest.mark.buildconfigspec('cmd_tpm_v2')
def test_tpm2_pcr_extend(ubman):
    """Execute a TPM2_PCR_Extend command.

    Perform a PCR extension with a known hash in memory (zeroed since the board
    must have been rebooted).

    No authentication mechanism is used here, not protecting against packet
    replay, yet.
    """
    if is_sandbox(ubman):
        tpm2_sandbox_init(ubman)
    force_init(ubman)
    ram = utils.find_ram_base(ubman)

    read_pcr = ubman.run_command('tpm2 pcr_read 10 0x%x' % (ram + 0x20))
    output = ubman.run_command('echo $?')
    assert output.endswith('0')
    str = re.findall(r'\d+ known updates', read_pcr)[0]
    updates = int(re.findall(r'\d+', str)[0])

    ubman.run_command('tpm2 pcr_extend 10 0x%x' % ram)
    output = ubman.run_command('echo $?')
    assert output.endswith('0')

    # Read the value back into a different place so we can still use 'ram' as
    # our zero bytes
    read_pcr = ubman.run_command('tpm2 pcr_read 10 0x%x' % (ram + 0x20))
    output = ubman.run_command('echo $?')
    assert output.endswith('0')
    assert 'f5 a5 fd 42 d1 6a 20 30 27 98 ef 6e d3 09 97 9b' in read_pcr
    assert '43 00 3d 23 20 d9 f0 e8 ea 98 31 a9 27 59 fb 4b' in read_pcr

    str = re.findall(r'\d+ known updates', read_pcr)[0]
    new_updates = int(re.findall(r'\d+', str)[0])
    assert (updates + 1) == new_updates

    ubman.run_command('tpm2 pcr_extend 10 0x%x' % ram)
    output = ubman.run_command('echo $?')
    assert output.endswith('0')

    read_pcr = ubman.run_command('tpm2 pcr_read 10 0x%x' % (ram + 0x20))
    output = ubman.run_command('echo $?')
    assert output.endswith('0')
    assert '7a 05 01 f5 95 7b df 9c b3 a8 ff 49 66 f0 22 65' in read_pcr
    assert 'f9 68 65 8b 7a 9c 62 64 2c ba 11 65 e8 66 42 f5' in read_pcr

    str = re.findall(r'\d+ known updates', read_pcr)[0]
    new_updates = int(re.findall(r'\d+', str)[0])
    assert (updates + 2) == new_updates

@pytest.mark.buildconfigspec('cmd_tpm_v2')
def test_tpm2_cleanup(ubman):
    """Ensure the TPM is cleared from password or test related configuration."""

    force_init(ubman, True)
