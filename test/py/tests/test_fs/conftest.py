# SPDX-License-Identifier:      GPL-2.0+
# Copyright (c) 2018, Linaro Limited
# Author: Takahiro Akashi <takahiro.akashi@linaro.org>

import os
import os.path
import pytest
import re
from subprocess import call, check_call, check_output, CalledProcessError
from fstest_defs import *
# pylint: disable=E0611
from tests import fs_helper

supported_fs_basic = ['fat16', 'fat32', 'exfat', 'ext4', 'fs_generic']
supported_fs_ext = ['fat12', 'fat16', 'fat32', 'exfat', 'fs_generic']
supported_fs_fat = ['fat12', 'fat16']
supported_fs_mkdir = ['fat12', 'fat16', 'fat32', 'exfat', 'fs_generic']
supported_fs_unlink = ['fat12', 'fat16', 'fat32', 'exfat', 'fs_generic']
supported_fs_symlink = ['ext4']
supported_fs_rename = ['fat12', 'fat16', 'fat32']

#
# Filesystem test specific setup
#
def pytest_addoption(parser):
    """Enable --fs-type option.

    See pytest_configure() about how it works.

    Args:
        parser: Pytest command-line parser.

    Returns:
        Nothing.
    """
    parser.addoption('--fs-type', action='append', default=None,
        help='Targeting Filesystem Types')

def pytest_configure(config):
    """Restrict a file system(s) to be tested.

    A file system explicitly named with --fs-type option is selected
    if it belongs to a default supported_fs_xxx list.
    Multiple options can be specified.

    Args:
        config: Pytest configuration.

    Returns:
        Nothing.
    """
    global supported_fs_basic
    global supported_fs_ext
    global supported_fs_fat
    global supported_fs_mkdir
    global supported_fs_unlink
    global supported_fs_symlink
    global supported_fs_rename

    def intersect(listA, listB):
        return  [x for x in listA if x in listB]

    supported_fs = config.getoption('fs_type')
    if supported_fs:
        print('*** FS TYPE modified: %s' % supported_fs)
        supported_fs_basic =  intersect(supported_fs, supported_fs_basic)
        supported_fs_ext =  intersect(supported_fs, supported_fs_ext)
        supported_fs_fat =  intersect(supported_fs, supported_fs_fat)
        supported_fs_mkdir =  intersect(supported_fs, supported_fs_mkdir)
        supported_fs_unlink =  intersect(supported_fs, supported_fs_unlink)
        supported_fs_symlink =  intersect(supported_fs, supported_fs_symlink)
        supported_fs_rename =  intersect(supported_fs, supported_fs_rename)

def pytest_generate_tests(metafunc):
    """Parametrize fixtures, fs_obj_xxx

    Each fixture will be parametrized with a corresponding support_fs_xxx
    list.

    Args:
        metafunc: Pytest test function.

    Returns:
        Nothing.
    """
    if 'fs_obj_basic' in metafunc.fixturenames:
        metafunc.parametrize('fs_obj_basic', supported_fs_basic,
            indirect=True, scope='module')
    if 'fs_obj_ext' in metafunc.fixturenames:
        metafunc.parametrize('fs_obj_ext', supported_fs_ext,
            indirect=True, scope='module')
    if 'fs_obj_fat' in metafunc.fixturenames:
        metafunc.parametrize('fs_obj_fat', supported_fs_fat,
            indirect=True, scope='module')
    if 'fs_obj_mkdir' in metafunc.fixturenames:
        metafunc.parametrize('fs_obj_mkdir', supported_fs_mkdir,
            indirect=True, scope='module')
    if 'fs_obj_unlink' in metafunc.fixturenames:
        metafunc.parametrize('fs_obj_unlink', supported_fs_unlink,
            indirect=True, scope='module')
    if 'fs_obj_symlink' in metafunc.fixturenames:
        metafunc.parametrize('fs_obj_symlink', supported_fs_symlink,
            indirect=True, scope='module')
    if 'fs_obj_rename' in metafunc.fixturenames:
        metafunc.parametrize('fs_obj_rename', supported_fs_rename,
            indirect=True, scope='module')

#
# Helper functions
#
def fstype_to_prefix(fs_type):
    """Convert a file system type to an U-Boot command prefix

    Args:
        fs_type: File system type.

    Return:
        A corresponding command prefix for file system type.
    """
    if fs_type == 'fs_generic' or fs_type == 'exfat':
        return ''
    elif re.match('fat', fs_type):
        return 'fat'
    else:
        return fs_type

def fstype_to_ubname(fs_type):
    """Convert a file system type to an U-Boot specific string

    A generated string can be used as part of file system related commands
    or a config name in u-boot. Currently fat16 and fat32 are handled
    specifically.

    Args:
        fs_type: File system type.

    Return:
        A corresponding string for file system type.
    """
    if re.match('fat', fs_type):
        return 'fat'
    else:
        return fs_type

def check_ubconfig(config, fs_type):
    """Check whether a file system is enabled in u-boot configuration.

    This function is assumed to be called in a fixture function so that
    the whole test cases will be skipped if a given file system is not
    enabled.

    Args:
        fs_type: File system type.

    Return:
        Nothing.
    """
    if fs_type == 'exfat' and not config.buildconfig.get('config_fs_%s' % fs_type, None):
        pytest.skip('.config feature "FS_%s" not enabled' % fs_type.upper())
    if fs_type != 'exfat' and not config.buildconfig.get('config_cmd_%s' % fs_type, None):
        pytest.skip('.config feature "CMD_%s" not enabled' % fs_type.upper())
    if fs_type == 'fs_generic' or fs_type == 'exfat':
        return
    if not config.buildconfig.get('config_%s_write' % fs_type, None):
        pytest.skip('.config feature "%s_WRITE" not enabled'
        % fs_type.upper())

# from test/py/conftest.py
def tool_is_in_path(tool):
    """Check whether a given command is available on host.

    Args:
        tool: Command name.

    Return:
        True if available, False if not.
    """
    for path in os.environ['PATH'].split(os.pathsep):
        fn = os.path.join(path, tool)
        if os.path.isfile(fn) and os.access(fn, os.X_OK):
            return True
    return False

#
# Fixture for basic fs test
#     derived from test/fs/fs-test.sh
#
@pytest.fixture()
def fs_obj_basic(request, u_boot_config):
    """Set up a file system to be used in basic fs test.

    Args:
        request: Pytest request object.
	u_boot_config: U-Boot configuration.

    Return:
        A fixture for basic fs test, i.e. a triplet of file system type,
        volume file name and  a list of MD5 hashes.
    """
    fs_type = request.param
    fs_cmd_prefix = fstype_to_prefix(fs_type)
    fs_cmd_write = 'save' if fs_type == 'fs_generic' or fs_type == 'exfat' else 'write'
    fs_img = ''

    fs_ubtype = fstype_to_ubname(fs_type)
    check_ubconfig(u_boot_config, fs_ubtype)

    scratch_dir = u_boot_config.persistent_data_dir + '/scratch'

    small_file = scratch_dir + '/' + SMALL_FILE
    big_file = scratch_dir + '/' + BIG_FILE

    try:
        check_call('mkdir -p %s' % scratch_dir, shell=True)
    except CalledProcessError as err:
        pytest.skip('Preparing mount folder failed for filesystem: ' + fs_type + '. {}'.format(err))
        call('rm -f %s' % fs_img, shell=True)
        return

    try:
        # Create a subdirectory.
        check_call('mkdir %s/SUBDIR' % scratch_dir, shell=True)

        # Create big file in this image.
        # Note that we work only on the start 1MB, couple MBs in the 2GB range
        # and the last 1 MB of the huge 2.5GB file.
        # So, just put random values only in those areas.
        check_call('dd if=/dev/urandom of=%s bs=1M count=1'
	    % big_file, shell=True)
        check_call('dd if=/dev/urandom of=%s bs=1M count=2 seek=2047'
            % big_file, shell=True)
        check_call('dd if=/dev/urandom of=%s bs=1M count=1 seek=2499'
            % big_file, shell=True)

        # Create a small file in this image.
        check_call('dd if=/dev/urandom of=%s bs=1M count=1'
	    % small_file, shell=True)

        # Delete the small file copies which possibly are written as part of a
        # previous test.
        # check_call('rm -f "%s.w"' % MB1, shell=True)
        # check_call('rm -f "%s.w2"' % MB1, shell=True)

        # Generate the md5sums of reads that we will test against small file
        out = check_output(
            'dd if=%s bs=1M skip=0 count=1 2> /dev/null | md5sum'
	    % small_file, shell=True).decode()
        md5val = [ out.split()[0] ]

        # Generate the md5sums of reads that we will test against big file
        # One from beginning of file.
        out = check_output(
            'dd if=%s bs=1M skip=0 count=1 2> /dev/null | md5sum'
	    % big_file, shell=True).decode()
        md5val.append(out.split()[0])

        # One from end of file.
        out = check_output(
            'dd if=%s bs=1M skip=2499 count=1 2> /dev/null | md5sum'
	    % big_file, shell=True).decode()
        md5val.append(out.split()[0])

        # One from the last 1MB chunk of 2GB
        out = check_output(
            'dd if=%s bs=1M skip=2047 count=1 2> /dev/null | md5sum'
	    % big_file, shell=True).decode()
        md5val.append(out.split()[0])

        # One from the start 1MB chunk from 2GB
        out = check_output(
            'dd if=%s bs=1M skip=2048 count=1 2> /dev/null | md5sum'
	    % big_file, shell=True).decode()
        md5val.append(out.split()[0])

        # One 1MB chunk crossing the 2GB boundary
        out = check_output(
            'dd if=%s bs=512K skip=4095 count=2 2> /dev/null | md5sum'
	    % big_file, shell=True).decode()
        md5val.append(out.split()[0])

        try:
            # 3GiB volume
            fs_img = fs_helper.mk_fs(u_boot_config, fs_type, 0xc0000000, '3GB', scratch_dir)
        except CalledProcessError as err:
            pytest.skip('Creating failed for filesystem: ' + fs_type + '. {}'.format(err))
            return

    except CalledProcessError as err:
        pytest.skip('Setup failed for filesystem: ' + fs_type + '. {}'.format(err))
        return
    else:
        yield [fs_ubtype, fs_cmd_prefix, fs_cmd_write, fs_img, md5val]
    finally:
        call('rm -rf %s' % scratch_dir, shell=True)
        call('rm -f %s' % fs_img, shell=True)

#
# Fixture for extended fs test
#
@pytest.fixture()
def fs_obj_ext(request, u_boot_config):
    """Set up a file system to be used in extended fs test.

    Args:
        request: Pytest request object.
	u_boot_config: U-Boot configuration.

    Return:
        A fixture for extended fs test, i.e. a triplet of file system type,
        volume file name and  a list of MD5 hashes.
    """
    fs_type = request.param
    fs_cmd_prefix = fstype_to_prefix(fs_type)
    fs_cmd_write = 'save' if fs_type == 'fs_generic' or fs_type == 'exfat' else 'write'
    fs_img = ''

    fs_ubtype = fstype_to_ubname(fs_type)
    check_ubconfig(u_boot_config, fs_ubtype)

    scratch_dir = u_boot_config.persistent_data_dir + '/scratch'

    min_file = scratch_dir + '/' + MIN_FILE
    tmp_file = scratch_dir + '/tmpfile'

    try:
        check_call('mkdir -p %s' % scratch_dir, shell=True)
    except CalledProcessError as err:
        pytest.skip('Preparing mount folder failed for filesystem: ' + fs_type + '. {}'.format(err))
        call('rm -f %s' % fs_img, shell=True)
        return

    try:
        # Create a test directory
        check_call('mkdir %s/dir1' % scratch_dir, shell=True)

        # Create a small file and calculate md5
        check_call('dd if=/dev/urandom of=%s bs=1K count=20'
            % min_file, shell=True)
        out = check_output(
            'dd if=%s bs=1K 2> /dev/null | md5sum'
            % min_file, shell=True).decode()
        md5val = [ out.split()[0] ]

        # Calculate md5sum of Test Case 4
        check_call('dd if=%s of=%s bs=1K count=20'
            % (min_file, tmp_file), shell=True)
        check_call('dd if=%s of=%s bs=1K seek=5 count=20'
            % (min_file, tmp_file), shell=True)
        out = check_output('dd if=%s bs=1K 2> /dev/null | md5sum'
            % tmp_file, shell=True).decode()
        md5val.append(out.split()[0])

        # Calculate md5sum of Test Case 5
        check_call('dd if=%s of=%s bs=1K count=20'
            % (min_file, tmp_file), shell=True)
        check_call('dd if=%s of=%s bs=1K seek=5 count=5'
            % (min_file, tmp_file), shell=True)
        out = check_output('dd if=%s bs=1K 2> /dev/null | md5sum'
            % tmp_file, shell=True).decode()
        md5val.append(out.split()[0])

        # Calculate md5sum of Test Case 7
        check_call('dd if=%s of=%s bs=1K count=20'
            % (min_file, tmp_file), shell=True)
        check_call('dd if=%s of=%s bs=1K seek=20 count=20'
            % (min_file, tmp_file), shell=True)
        out = check_output('dd if=%s bs=1K 2> /dev/null | md5sum'
            % tmp_file, shell=True).decode()
        md5val.append(out.split()[0])

        check_call('rm %s' % tmp_file, shell=True)

        try:
            # 128MiB volume
            fs_img = fs_helper.mk_fs(u_boot_config, fs_type, 0x8000000, '128MB', scratch_dir)
        except CalledProcessError as err:
            pytest.skip('Creating failed for filesystem: ' + fs_type + '. {}'.format(err))
            return

    except CalledProcessError:
        pytest.skip('Setup failed for filesystem: ' + fs_type)
        return
    else:
        yield [fs_ubtype, fs_cmd_prefix, fs_cmd_write, fs_img, md5val]
    finally:
        call('rm -rf %s' % scratch_dir, shell=True)
        call('rm -f %s' % fs_img, shell=True)

#
# Fixture for mkdir test
#
@pytest.fixture()
def fs_obj_mkdir(request, u_boot_config):
    """Set up a file system to be used in mkdir test.

    Args:
        request: Pytest request object.
	u_boot_config: U-Boot configuration.

    Return:
        A fixture for mkdir test, i.e. a duplet of file system type and
        volume file name.
    """
    fs_type = request.param
    fs_cmd_prefix = fstype_to_prefix(fs_type)
    fs_img = ''

    fs_ubtype = fstype_to_ubname(fs_type)
    check_ubconfig(u_boot_config, fs_ubtype)

    try:
        # 128MiB volume
        fs_img = fs_helper.mk_fs(u_boot_config, fs_type, 0x8000000, '128MB', None)
    except:
        pytest.skip('Setup failed for filesystem: ' + fs_type)
        return
    else:
        yield [fs_ubtype, fs_cmd_prefix, fs_img]
    call('rm -f %s' % fs_img, shell=True)

#
# Fixture for unlink test
#
@pytest.fixture()
def fs_obj_unlink(request, u_boot_config):
    """Set up a file system to be used in unlink test.

    Args:
        request: Pytest request object.
	u_boot_config: U-Boot configuration.

    Return:
        A fixture for unlink test, i.e. a duplet of file system type and
        volume file name.
    """
    fs_type = request.param
    fs_cmd_prefix = fstype_to_prefix(fs_type)
    fs_img = ''

    fs_ubtype = fstype_to_ubname(fs_type)
    check_ubconfig(u_boot_config, fs_ubtype)

    scratch_dir = u_boot_config.persistent_data_dir + '/scratch'

    try:
        check_call('mkdir -p %s' % scratch_dir, shell=True)
    except CalledProcessError as err:
        pytest.skip('Preparing mount folder failed for filesystem: ' + fs_type + '. {}'.format(err))
        call('rm -f %s' % fs_img, shell=True)
        return

    try:
        # Test Case 1 & 3
        check_call('mkdir %s/dir1' % scratch_dir, shell=True)
        check_call('dd if=/dev/urandom of=%s/dir1/file1 bs=1K count=1'
                                    % scratch_dir, shell=True)
        check_call('dd if=/dev/urandom of=%s/dir1/file2 bs=1K count=1'
                                    % scratch_dir, shell=True)

        # Test Case 2
        check_call('mkdir %s/dir2' % scratch_dir, shell=True)
        for i in range(0, 20):
            check_call('mkdir %s/dir2/0123456789abcdef%02x'
                                    % (scratch_dir, i), shell=True)

        # Test Case 4
        check_call('mkdir %s/dir4' % scratch_dir, shell=True)

        # Test Case 5, 6 & 7
        check_call('mkdir %s/dir5' % scratch_dir, shell=True)
        check_call('dd if=/dev/urandom of=%s/dir5/file1 bs=1K count=1'
                                    % scratch_dir, shell=True)

        try:
            # 128MiB volume
            fs_img = fs_helper.mk_fs(u_boot_config, fs_type, 0x8000000, '128MB', scratch_dir)
        except CalledProcessError as err:
            pytest.skip('Creating failed for filesystem: ' + fs_type + '. {}'.format(err))
            return

    except CalledProcessError:
        pytest.skip('Setup failed for filesystem: ' + fs_type)
        return
    else:
        yield [fs_ubtype, fs_cmd_prefix, fs_img]
    finally:
        call('rm -rf %s' % scratch_dir, shell=True)
        call('rm -f %s' % fs_img, shell=True)

#
# Fixture for symlink fs test
#
@pytest.fixture()
def fs_obj_symlink(request, u_boot_config):
    """Set up a file system to be used in symlink fs test.

    Args:
        request: Pytest request object.
        u_boot_config: U-Boot configuration.

    Return:
        A fixture for basic fs test, i.e. a triplet of file system type,
        volume file name and  a list of MD5 hashes.
    """
    fs_type = request.param
    fs_img = ''

    fs_ubtype = fstype_to_ubname(fs_type)
    check_ubconfig(u_boot_config, fs_ubtype)

    scratch_dir = u_boot_config.persistent_data_dir + '/scratch'

    small_file = scratch_dir + '/' + SMALL_FILE
    medium_file = scratch_dir + '/' + MEDIUM_FILE

    try:
        check_call('mkdir -p %s' % scratch_dir, shell=True)
    except CalledProcessError as err:
        pytest.skip('Preparing mount folder failed for filesystem: ' + fs_type + '. {}'.format(err))
        call('rm -f %s' % fs_img, shell=True)
        return

    try:
        # Create a subdirectory.
        check_call('mkdir %s/SUBDIR' % scratch_dir, shell=True)

        # Create a small file in this image.
        check_call('dd if=/dev/urandom of=%s bs=1M count=1'
                   % small_file, shell=True)

        # Create a medium file in this image.
        check_call('dd if=/dev/urandom of=%s bs=10M count=1'
                   % medium_file, shell=True)

        # Generate the md5sums of reads that we will test against small file
        out = check_output(
            'dd if=%s bs=1M skip=0 count=1 2> /dev/null | md5sum'
            % small_file, shell=True).decode()
        md5val = [out.split()[0]]
        out = check_output(
            'dd if=%s bs=10M skip=0 count=1 2> /dev/null | md5sum'
            % medium_file, shell=True).decode()
        md5val.extend([out.split()[0]])

        try:
            # 1GiB volume
            fs_img = fs_helper.mk_fs(u_boot_config, fs_type, 0x40000000, '1GB', scratch_dir)
        except CalledProcessError as err:
            pytest.skip('Creating failed for filesystem: ' + fs_type + '. {}'.format(err))
            return

    except CalledProcessError:
        pytest.skip('Setup failed for filesystem: ' + fs_type)
        return
    else:
        yield [fs_ubtype, fs_img, md5val]
    finally:
        call('rm -rf %s' % scratch_dir, shell=True)
        call('rm -f %s' % fs_img, shell=True)

#
# Fixture for rename test
#
@pytest.fixture()
def fs_obj_rename(request, u_boot_config):
    """Set up a file system to be used in rename tests.

    Args:
        request: Pytest request object.
        u_boot_config: U-Boot configuration.

    Return:
        A fixture for rename tests, i.e. a triplet of file system type,
        volume file name, and dictionary of test identifier and md5val.
    """
    def new_rand_file(path):
        check_call('dd if=/dev/urandom of=%s bs=1K count=1' % path, shell=True)

    def file_hash(path):
        out = check_output(
            'dd if=%s bs=1K skip=0 count=1 2> /dev/null | md5sum' % path,
            shell=True
        )
        return out.decode().split()[0]

    fs_type = request.param
    fs_img = ''

    fs_ubtype = fstype_to_ubname(fs_type)
    check_ubconfig(u_boot_config, fs_ubtype)

    mount_dir = u_boot_config.persistent_data_dir + '/scratch'

    try:
        check_call('mkdir -p %s' % mount_dir, shell=True)
    except CalledProcessError as err:
        pytest.skip('Preparing mount folder failed for filesystem: ' + fs_type + '. {}'.format(err))
        call('rm -f %s' % fs_img, shell=True)
        return

    try:
        md5val = {}
        # Test Case 1
        check_call('mkdir %s/test1' % mount_dir, shell=True)
        new_rand_file('%s/test1/file1' % mount_dir)
        md5val['test1'] = file_hash('%s/test1/file1' % mount_dir)

        # Test Case 2
        check_call('mkdir %s/test2' % mount_dir, shell=True)
        new_rand_file('%s/test2/file1' % mount_dir)
        new_rand_file('%s/test2/file_exist' % mount_dir)
        md5val['test2'] = file_hash('%s/test2/file1' % mount_dir)

        # Test Case 3
        check_call('mkdir -p %s/test3/dir1' % mount_dir, shell=True)
        new_rand_file('%s/test3/dir1/file1' % mount_dir)
        md5val['test3'] = file_hash('%s/test3/dir1/file1' % mount_dir)

        # Test Case 4
        check_call('mkdir -p %s/test4/dir1' % mount_dir, shell=True)
        check_call('mkdir -p %s/test4/dir2/dir1' % mount_dir, shell=True)
        new_rand_file('%s/test4/dir1/file1' % mount_dir)
        md5val['test4'] = file_hash('%s/test4/dir1/file1' % mount_dir)

        # Test Case 5
        check_call('mkdir -p %s/test5/dir1' % mount_dir, shell=True)
        new_rand_file('%s/test5/file2' % mount_dir)
        md5val['test5'] = file_hash('%s/test5/file2' % mount_dir)

        # Test Case 6
        check_call('mkdir -p %s/test6/dir2/existing' % mount_dir, shell=True)
        new_rand_file('%s/test6/existing' % mount_dir)
        md5val['test6'] = file_hash('%s/test6/existing' % mount_dir)

        # Test Case 7
        check_call('mkdir -p %s/test7/dir1' % mount_dir, shell=True)
        check_call('mkdir -p %s/test7/dir2/dir1' % mount_dir, shell=True)
        new_rand_file('%s/test7/dir2/dir1/file1' % mount_dir)
        md5val['test7'] = file_hash('%s/test7/dir2/dir1/file1' % mount_dir)

        # Test Case 8
        check_call('mkdir -p %s/test8/dir1' % mount_dir, shell=True)
        new_rand_file('%s/test8/dir1/file1' % mount_dir)
        md5val['test8'] = file_hash('%s/test8/dir1/file1' % mount_dir)

        # Test Case 9
        check_call('mkdir -p %s/test9/dir1/nested/inner' % mount_dir, shell=True)
        new_rand_file('%s/test9/dir1/nested/inner/file1' % mount_dir)

        # Test Case 10
        check_call('mkdir -p %s/test10' % mount_dir, shell=True)
        new_rand_file('%s/test10/file1' % mount_dir)
        md5val['test10'] = file_hash('%s/test10/file1' % mount_dir)

        # Test Case 11
        check_call('mkdir -p %s/test11/dir1' % mount_dir, shell=True)
        new_rand_file('%s/test11/dir1/file1' % mount_dir)
        md5val['test11'] = file_hash('%s/test11/dir1/file1' % mount_dir)

        try:
            # 128MiB volume
            fs_img = fs_helper.mk_fs(u_boot_config, fs_type, 0x8000000, '128MB', mount_dir)
        except CalledProcessError as err:
            pytest.skip('Creating failed for filesystem: ' + fs_type + '. {}'.format(err))
            return

    except CalledProcessError:
        pytest.skip('Setup failed for filesystem: ' + fs_type)
        return
    else:
        yield [fs_ubtype, fs_img, md5val]
    finally:
        call('rm -rf %s' % mount_dir, shell=True)
        call('rm -f %s' % fs_img, shell=True)

#
# Fixture for fat test
#
@pytest.fixture()
def fs_obj_fat(request, u_boot_config):
    """Set up a file system to be used in fat test.

    Args:
        request: Pytest request object.
        u_boot_config: U-Boot configuration.

    Return:
        A fixture for fat test, i.e. a duplet of file system type and
        volume file name.
    """

    # the maximum size of a FAT12 filesystem resulting in 4084 clusters
    MAX_FAT12_SIZE = 261695 * 1024

    # the minimum size of a FAT16 filesystem that can be created with
    # mkfs.vfat resulting in 4087 clusters
    MIN_FAT16_SIZE = 8208 * 1024

    fs_type = request.param
    fs_img = ''

    fs_ubtype = fstype_to_ubname(fs_type)
    check_ubconfig(u_boot_config, fs_ubtype)

    fs_size = MAX_FAT12_SIZE if fs_type == 'fat12' else MIN_FAT16_SIZE

    try:
        # the volume size depends on the filesystem
        fs_img = fs_helper.mk_fs(u_boot_config, fs_type, fs_size, f'{fs_size}', None, 1024)
    except:
        pytest.skip('Setup failed for filesystem: ' + fs_type)
        return
    else:
        yield [fs_ubtype, fs_img]
    call('rm -f %s' % fs_img, shell=True)
