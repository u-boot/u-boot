# SPDX-License-Identifier:      GPL-2.0+
""" Unit test for UEFI menu-driven configuration
"""

import pytest
import shutil
import pytest
import time
from subprocess import call, check_call, CalledProcessError
from tests import fs_helper

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('cmd_eficonfig')
@pytest.mark.buildconfigspec('cmd_bootefi_bootmgr')
def test_efi_eficonfig(ubman):

    def prepare_image(u_boot_config):
        """Set up a file system to be used in UEFI "eficonfig" command
           tests. This creates a disk image with the following files:
                 initrd-1.img
                 initrd-2.img
                 initrddump.efi

        Args:
            u_boot_config -- U-Boot configuration.

        Return:
            A path to disk image to be used for testing

        """
        try:
            image_path, mnt_point = fs_helper.setup_image(u_boot_config, 0,
                                                          0xc,
                                                          basename='test_eficonfig')

            with open(mnt_point + '/initrd-1.img', 'w', encoding = 'ascii') as file:
                file.write("initrd 1")

            with open(mnt_point + '/initrd-2.img', 'w', encoding = 'ascii') as file:
                file.write("initrd 2")

            shutil.copyfile(u_boot_config.build_dir + '/lib/efi_loader/initrddump.efi',
                            mnt_point + '/initrddump.efi')

            fsfile = fs_helper.mk_fs(ubman.config, 'vfat', 0x100000,
                                     'test_eficonfig', mnt_point)
            check_call(f'dd if={fsfile} of={image_path} bs=1M seek=1', shell=True)

            yield image_path
        except CalledProcessError as err:
            pytest.skip('Preparing test_eficonfig image failed')
            call('rm -f %s' % image_path, shell=True)
        finally:
            call('rm -rf %s' % mnt_point, shell=True)
            call('rm -f %s' % image_path, shell=True)

    def send_user_input_and_wait(user_str, expect_str):
        time.sleep(0.1) # TODO: does not work correctly without sleep
        ubman.run_command(cmd=user_str, wait_for_prompt=False,
                                   wait_for_echo=True, send_nl=False)
        ubman.run_command(cmd='\x0d', wait_for_prompt=False,
                                   wait_for_echo=False, send_nl=False)
        if expect_str is not None:
            for i in expect_str:
                ubman.p.expect([i])

    def press_up_down_enter_and_wait(up_count, down_count, enter, expect_str):
        # press UP key
        for i in range(up_count):
            ubman.run_command(cmd='\x1b\x5b\x41', wait_for_prompt=False,
                                       wait_for_echo=False, send_nl=False)
        # press DOWN key
        for i in range(down_count):
            ubman.run_command(cmd='\x1b\x5b\x42', wait_for_prompt=False,
                                       wait_for_echo=False, send_nl=False)
        # press ENTER if requested
        if enter:
            ubman.run_command(cmd='\x0d', wait_for_prompt=False,
                                       wait_for_echo=False, send_nl=False)
        # wait expected output
        if expect_str is not None:
            for i in expect_str:
                ubman.p.expect([i])

    def press_escape_key(wait_prompt):
        ubman.run_command(cmd='\x1b', wait_for_prompt=wait_prompt, wait_for_echo=False, send_nl=False)

    def press_enter_key(wait_prompt):
        ubman.run_command(cmd='\x0d', wait_for_prompt=wait_prompt,
                                   wait_for_echo=False, send_nl=False)

    def check_current_is_maintenance_menu():
        for i in ('UEFI Maintenance Menu', 'Add Boot Option', 'Edit Boot Option',
                  'Change Boot Order', 'Delete Boot Option', 'Quit'):
            ubman.p.expect([i])

    """ Unit test for "eficonfig" command
    The menu-driven interface is used to set up UEFI load options.
    The bootefi bootmgr loads initrddump.efi as a payload.
    The crc32 of the loaded initrd.img is checked

    Args:
        ubman -- U-Boot console
    """
    # This test passes for unknown reasons in the bowels of U-Boot. It needs to
    # be replaced with a unit test.
    return

    # Restart the system to clean the previous state
    ubman.restart_uboot()

    efi_eficonfig_data = prepare_image(ubman.config)
    with ubman.temporary_timeout(500):
        #
        # Test Case 1: Check the menu is displayed
        #
        ubman.run_command('eficonfig', wait_for_prompt=False)
        for i in ('UEFI Maintenance Menu', 'Add Boot Option', 'Edit Boot Option',
                  'Change Boot Order', 'Delete Boot Option', 'Quit'):
            ubman.p.expect([i])
        # Select "Add Boot Option"
        press_enter_key(False)
        for i in ('Add Boot Option', 'Description:', 'File', 'Initrd File', 'Optional Data',
                  'Save', 'Quit'):
            ubman.p.expect([i])
        press_escape_key(False)
        check_current_is_maintenance_menu()
        # return to U-Boot console
        press_escape_key(True)

        #
        # Test Case 2: check auto generated media device entry
        #

        # bind the test disk image for succeeding tests
        ubman.run_command(cmd = f'host bind 0 {efi_eficonfig_data}')

        ubman.run_command('eficonfig', wait_for_prompt=False)

        # Change the Boot Order
        press_up_down_enter_and_wait(0, 2, True, 'Quit')
        for i in ('host 0:1', 'Save', 'Quit'):
            ubman.p.expect([i])
        # disable auto generated boot option for succeeding test
        ubman.run_command(cmd=' ', wait_for_prompt=False,
                                       wait_for_echo=False, send_nl=False)
        # Save the BootOrder
        press_up_down_enter_and_wait(0, 1, True, None)
        check_current_is_maintenance_menu()

        #
        # Test Case 3: Add first Boot Option and load it
        #

        # Select 'Add Boot Option'
        press_up_down_enter_and_wait(0, 0, True, 'Quit')

        # Press the enter key to select 'Description:' entry, then enter Description
        press_up_down_enter_and_wait(0, 0, True, 'Enter description:')
        # Send Description user input, press ENTER key to complete
        send_user_input_and_wait('test 1', 'Quit')

        # Set EFI image(initrddump.efi)
        press_up_down_enter_and_wait(0, 1, True, 'Quit')
        press_up_down_enter_and_wait(0, 0, True, 'host 0:1')
        # Select 'host 0:1'
        press_up_down_enter_and_wait(0, 0, True, 'Quit')
        # Press down key to select "initrddump.efi" entry followed by the enter key
        press_up_down_enter_and_wait(0, 2, True, 'Quit')

        # Set Initrd file(initrd-1.img)
        press_up_down_enter_and_wait(0, 2, True, 'Quit')
        press_up_down_enter_and_wait(0, 0, True, 'host 0:1')
        # Select 'host 0:1'
        press_up_down_enter_and_wait(0, 0, True, 'Quit')
        # Press down key to select "initrd-1.img" entry followed by the enter key
        press_up_down_enter_and_wait(0, 0, True, 'Quit')

        # Set optional_data
        press_up_down_enter_and_wait(0, 3, True, 'Optional Data:')
        # Send Description user input, press ENTER key to complete
        send_user_input_and_wait('nocolor', None)
        for i in ('Description: test 1', 'File: host 0:1/initrddump.efi',
                  'Initrd File: host 0:1/initrd-1.img', 'Optional Data: nocolor', 'Save', 'Quit'):
            ubman.p.expect([i])

        # Save the Boot Option
        press_up_down_enter_and_wait(0, 4, True, None)
        check_current_is_maintenance_menu()

        # Check the newly added Boot Option is handled correctly
        # Return to U-Boot console
        press_escape_key(True)
        ubman.run_command(cmd = 'bootefi bootmgr')
        response = ubman.run_command(cmd = 'load', wait_for_echo=False)
        assert 'crc32: 0x181464af' in response
        ubman.run_command(cmd = 'exit', wait_for_echo=False)

        #
        # Test Case 4: Add second Boot Option and load it
        #
        ubman.run_command('eficonfig', wait_for_prompt=False)

        # Select 'Add Boot Option'
        press_up_down_enter_and_wait(0, 0, True, 'Quit')

        # Press the enter key to select 'Description:' entry, then enter Description
        press_up_down_enter_and_wait(0, 0, True, 'Enter description:')
        # Send Description user input, press ENTER key to complete
        send_user_input_and_wait('test 2', 'Quit')

        # Set EFI image(initrddump.efi)
        press_up_down_enter_and_wait(0, 1, True, 'Quit')
        press_up_down_enter_and_wait(0, 0, True, 'host 0:1')
        # Select 'host 0:1'
        press_up_down_enter_and_wait(0, 0, True, 'Quit')
        # Press down key to select "initrddump.efi" entry followed by the enter key
        press_up_down_enter_and_wait(0, 2, True, 'Quit')

        # Set Initrd file(initrd-2.img)
        press_up_down_enter_and_wait(0, 2, True, 'Quit')
        press_up_down_enter_and_wait(0, 0, True, 'host 0:1')
        # Select 'host 0:1'
        press_up_down_enter_and_wait(0, 0, True, 'Quit')
        # Press down key to select "initrd-2.img" entry followed by the enter key
        press_up_down_enter_and_wait(0, 1, True, 'Quit')

        # Set optional_data
        press_up_down_enter_and_wait(0, 3, True, 'Optional Data:')
        # Send Description user input, press ENTER key to complete
        send_user_input_and_wait('nocolor', None)
        for i in ('Description: test 2', 'File: host 0:1/initrddump.efi',
                  'Initrd File: host 0:1/initrd-2.img', 'Optional Data: nocolor', 'Save', 'Quit'):
            ubman.p.expect([i])

        # Save the Boot Option
        press_up_down_enter_and_wait(0, 4, True, 'Quit')

        # Change the Boot Order
        press_up_down_enter_and_wait(0, 2, True, 'Quit')
        press_up_down_enter_and_wait(0, 1, False, 'Quit')
        # move 'test 1' to the second entry
        ubman.run_command(cmd='+', wait_for_prompt=False,
                                       wait_for_echo=False, send_nl=False)
        for i in ('test 2', 'test 1', 'host 0:1', 'Save', 'Quit'):
            ubman.p.expect([i])
        # Save the BootOrder
        press_up_down_enter_and_wait(0, 3, True, None)
        check_current_is_maintenance_menu()

        # Check the newly added Boot Option is handled correctly
        # Return to U-Boot console
        press_escape_key(True)
        ubman.run_command(cmd = 'bootefi bootmgr')
        response = ubman.run_command(cmd = 'load', wait_for_echo=False)
        assert 'crc32: 0x811d3515' in response
        ubman.run_command(cmd = 'exit', wait_for_echo=False)

        #
        # Test Case 5: Change BootOrder and load it
        #
        ubman.run_command('eficonfig', wait_for_prompt=False)

        # Change the Boot Order
        press_up_down_enter_and_wait(0, 2, True, None)
        # Check the current BootOrder
        for i in ('test 2', 'test 1', 'host 0:1', 'Save', 'Quit'):
            ubman.p.expect([i])
        # move 'test 2' to the second entry
        ubman.run_command(cmd='-', wait_for_prompt=False,
                                       wait_for_echo=False, send_nl=False)
        for i in ('test 1', 'test 2', 'host 0:1', 'Save', 'Quit'):
            ubman.p.expect([i])
        # Save the BootOrder
        press_up_down_enter_and_wait(0, 2, True, None)
        check_current_is_maintenance_menu()

        # Return to U-Boot console
        press_escape_key(True)
        ubman.run_command(cmd = 'bootefi bootmgr')
        response = ubman.run_command(cmd = 'load', wait_for_echo=False)
        assert 'crc32: 0x181464af' in response
        ubman.run_command(cmd = 'exit', wait_for_echo=False)

        #
        # Test Case 6: Delete Boot Option(label:test 2)
        #
        ubman.run_command('eficonfig', wait_for_prompt=False)

        # Select 'Delete Boot Option'
        press_up_down_enter_and_wait(0, 3, True, None)
        # Check the current BootOrder
        for i in ('test 1', 'test 2', 'Quit'):
            ubman.p.expect([i])

        # Delete 'test 2'
        press_up_down_enter_and_wait(0, 1, True, None)
        for i in ('test 1', 'Quit'):
            ubman.p.expect([i])
        press_escape_key(False)
        check_current_is_maintenance_menu()
        # Return to U-Boot console
        press_escape_key(True)

        #
        # Test Case 7: Edit Boot Option
        #
        ubman.run_command('eficonfig', wait_for_prompt=False)
        # Select 'Edit Boot Option'
        press_up_down_enter_and_wait(0, 1, True, None)
        # Check the current BootOrder
        for i in ('test 1', 'Quit'):
            ubman.p.expect([i])
        press_up_down_enter_and_wait(0, 0, True, None)
        for i in ('Description: test 1', 'File: host 0:1/initrddump.efi',
                  'Initrd File: host 0:1/initrd-1.img', 'Optional Data: nocolor', 'Save', 'Quit'):
            ubman.p.expect([i])

        # Press the enter key to select 'Description:' entry, then enter Description
        press_up_down_enter_and_wait(0, 0, True, 'Enter description:')
        # Send Description user input, press ENTER key to complete
        send_user_input_and_wait('test 3', 'Quit')

        # Set EFI image(initrddump.efi)
        press_up_down_enter_and_wait(0, 1, True, 'Quit')
        press_up_down_enter_and_wait(0, 0, True, 'host 0:1')
        # Select 'host 0:1'
        press_up_down_enter_and_wait(0, 0, True, 'Quit')
        # Press down key to select "initrddump.efi" entry followed by the enter key
        press_up_down_enter_and_wait(0, 2, True, 'Quit')

        # Set Initrd file(initrd-2.img)
        press_up_down_enter_and_wait(0, 2, True, 'Quit')
        press_up_down_enter_and_wait(0, 0, True, 'host 0:1')
        # Select 'host 0:1'
        press_up_down_enter_and_wait(0, 0, True, 'Quit')
        # Press down key to select "initrd-1.img" entry followed by the enter key
        press_up_down_enter_and_wait(0, 1, True, 'Quit')

        # Set optional_data
        press_up_down_enter_and_wait(0, 3, True, 'Optional Data:')
        # Send Description user input, press ENTER key to complete
        send_user_input_and_wait('', None)
        for i in ('Description: test 3', 'File: host 0:1/initrddump.efi',
                  'Initrd File: host 0:1/initrd-2.img', 'Optional Data:', 'Save', 'Quit'):
            ubman.p.expect([i])

        # Save the Boot Option
        press_up_down_enter_and_wait(0, 4, True, 'Quit')
        press_escape_key(False)
        check_current_is_maintenance_menu()

        # Check the updated Boot Option is handled correctly
        # Return to U-Boot console
        press_escape_key(True)
        ubman.run_command(cmd = 'bootefi bootmgr')
        response = ubman.run_command(cmd = 'load', wait_for_echo=False)
        assert 'crc32: 0x811d3515' in response
        ubman.run_command(cmd = 'exit', wait_for_echo=False)

        #
        # Test Case 8: Delete Boot Option(label:test 3)
        #
        ubman.run_command('eficonfig', wait_for_prompt=False)

        # Select 'Delete Boot Option'
        press_up_down_enter_and_wait(0, 3, True, None)
        # Check the current BootOrder
        for i in ('test 3', 'Quit'):
            ubman.p.expect([i])

        # Delete 'test 3'
        press_up_down_enter_and_wait(0, 0, True, 'Quit')
        press_escape_key(False)
        check_current_is_maintenance_menu()
        # Return to U-Boot console
        press_escape_key(True)

        # remove the host device
        ubman.run_command(cmd = f'host bind -r 0')

        #
        # Test Case 9: No block device found
        #
        ubman.run_command('eficonfig', wait_for_prompt=False)

        # Select 'Add Boot Option'
        press_up_down_enter_and_wait(0, 0, True, 'Quit')

        # Set EFI image
        press_up_down_enter_and_wait(0, 1, True, 'Quit')
        press_up_down_enter_and_wait(0, 0, True, 'No block device found!')
        press_escape_key(False)
        press_escape_key(False)
        check_current_is_maintenance_menu()
        # Return to U-Boot console
        press_escape_key(True)
