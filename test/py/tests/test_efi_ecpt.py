# SPDX-License-Identifier: GPL-2.0-or-later
""" Unit test for the EFI Conformance Profiles Table (ECPT)
"""

import pytest


@pytest.mark.buildconfigspec('cmd_efidebug')
@pytest.mark.buildconfigspec('efi_ecpt')
def test_efi_ecpt(ubman) -> None:
    """ Unit test for the ECPT
    This test assumes nothing about the ECPT contents, it just checks that the
    ECPT table is there and that the efidebug ecpt command does not fail.

    Args:
        ubman -- U-Boot console
    """
    response = ubman.run_command('efidebug tables')
    assert ('36122546-f7e7-4c8f-bd9b-eb8525b50c0b  '
            'EFI Conformance Profiles Table') in response

    response = ubman.run_command('efidebug ecpt')
    assert 'Unknown command' not in response
    assert 'Configure UEFI environment' not in response
    assert 'Usage:' not in response
    assert 'table missing' not in response


@pytest.mark.buildconfigspec('cmd_efidebug')
@pytest.mark.buildconfigspec('efi_ecpt')
@pytest.mark.buildconfigspec('efi_ebbr_2_1_conformance')
def test_efi_ecpt_ebbr_2_1(ubman) -> None:
    """ Unit test for the ECPT, with EBBR 2.1 profile
    This test uses the efidebug ecpt command to dump the ECPT and check that
    the EBBR 2.1 conformance profile is there.

    Args:
        ubman -- U-Boot console
    """
    response = ubman.run_command('efidebug ecpt')
    assert ('cce33c35-74ac-4087-bce7-8b29b02eeb27  '
            'EFI EBBR 2.1 Conformance Profile') in response
