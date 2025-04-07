# SPDX-License-Identifier:      GPL-2.0+

""" Unit test for cat command
"""

import pytest

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_cat')
def test_cat(ubman, cat_data):
    """ Unit test for cat

    Args:
        ubman -- U-Boot console
        cat_data -- Path to the disk image used for testing.
    """
    response = ubman.run_command_list([
        f'host bind 0 {cat_data}',
        'cat host 0 hello'])
    assert 'hello world' in response
