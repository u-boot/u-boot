# SPDX-License-Identifier:      GPL-2.0+

""" Unit test for cat command
"""

import pytest
from subprocess import call, check_call, CalledProcessError
from tests import fs_helper

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_cat')
def test_cat(ubman):
    """ Unit test for cat

    Args:
        ubman -- U-Boot console
    """
    try:
        scratch_dir = ubman.config.persistent_data_dir + '/scratch'

        check_call('mkdir -p %s' % scratch_dir, shell=True)

        with open(scratch_dir + '/hello', 'w', encoding = 'ascii') as file:
            file.write('hello world\n')

        cat_data = fs_helper.mk_fs(ubman.config, 'vfat', 0x100000,
                                   'test_cat', scratch_dir)
        response = ubman.run_command_list([ f'host bind 0 {cat_data}',
                                                    'cat host 0 hello'])
        assert 'hello world' in response
    except CalledProcessError as err:
        pytest.skip('Preparing test_cat image failed')
        call('rm -f %s' % cat_data, shell=True)
        return
    finally:
        call('rm -rf %s' % scratch_dir, shell=True)
        call('rm -f %s' % cat_data, shell=True)
