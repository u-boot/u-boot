# SPDX-License-Identifier:      GPL-2.0+

""" Unit test for semihosting
"""

import pytest

@pytest.mark.buildconfigspec('semihosting')
def test_semihosting_hostfs(ubman, semihosting_data):
    """ Unit test for semihosting

    Args:
        ubman -- U-Boot console
        semihosting_data -- Path to the disk image used for testing.
    """
    response = ubman.run_command(
        f'load hostfs - $loadaddr {semihosting_data}')
    assert '11 bytes read' in response

    response = ubman.run_command(
        'crc32 $loadaddr $filesize')
    assert '==> 60cfccfc' in response

    ubman.run_command(
        f'save hostfs - $loadaddr {semihosting_data} 11 11')

    response = ubman.run_command(
        f'load hostfs - $loadaddr {semihosting_data} 4 13')
    assert '4 bytes read' in response

    response = ubman.run_command(
        'crc32 $loadaddr $filesize')
    assert '==> e29063ea' in response
