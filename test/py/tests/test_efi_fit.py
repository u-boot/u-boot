# SPDX-License-Identifier: GPL-2.0
# Copyright (c) 2019, Cristian Ciocaltea <cristian.ciocaltea@gmail.com>
#
# Work based on:
# - test_net.py
# Copyright (c) 2016, NVIDIA CORPORATION. All rights reserved.
# - test_fit.py
# Copyright (c) 2013, Google Inc.
#
# Test launching UEFI binaries from FIT images.

"""
Note: This test relies on boardenv_* containing configuration values to define
which network environment is available for testing. Without this, the parts
that rely on network will be automatically skipped.

For example:

# Boolean indicating whether the Ethernet device is attached to USB, and hence
# USB enumeration needs to be performed prior to network tests.
# This variable may be omitted if its value is False.
env__net_uses_usb = False

# Boolean indicating whether the Ethernet device is attached to PCI, and hence
# PCI enumeration needs to be performed prior to network tests.
# This variable may be omitted if its value is False.
env__net_uses_pci = True

# True if a DHCP server is attached to the network, and should be tested.
# If DHCP testing is not possible or desired, this variable may be omitted or
# set to False.
env__net_dhcp_server = True

# A list of environment variables that should be set in order to configure a
# static IP. If solely relying on DHCP, this variable may be omitted or set to
# an empty list.
env__net_static_env_vars = [
    ('ipaddr', '10.0.0.100'),
    ('netmask', '255.255.255.0'),
    ('serverip', '10.0.0.1'),
]

# Details regarding a file that may be read from a TFTP server. This variable
# may be omitted or set to None if TFTP testing is not possible or desired.
# Additionally, when the 'size' is not available, the file will be generated
# automatically in the TFTP root directory, as specified by the 'dn' field.
env__efi_fit_tftp_file = {
    'fn': 'test-efi-fit.img',   # File path relative to TFTP root
    'size': 3831,               # File size
    'crc32': '9fa3f79c',        # Checksum using CRC-32 algorithm, optional
    'addr': 0x40400000,         # Loading address, integer, optional
    'dn': 'tftp/root/dir',      # TFTP root directory path, optional
}
"""

import os.path
import pytest
import utils

# Define the parametrized ITS data to be used for FIT images generation.
ITS_DATA = '''
/dts-v1/;

/ {
    description = "EFI image with FDT blob";
    #address-cells = <1>;

    images {
        helloworld {
            description = "Test EFI";
            data = /incbin/("%(hello-bin)s");
            type = "%(kernel-type)s";
            arch = "%(sys-arch)s";
            os = "efi";
            compression = "%(efi-comp)s";
            load = <0x0>;
            entry = <0x0>;
        };
        dtbdump {
            description = "Test EFI fdtdump";
            data = /incbin/("%(dtbdump-bin)s");
            type = "%(kernel-type)s";
            arch = "%(sys-arch)s";
            os = "efi";
            compression = "%(efi-comp)s";
            load = <0x0>;
            entry = <0x0>;
        };
        initrddump {
            description = "Test EFI initrddump";
            data = /incbin/("%(initrddump-bin)s");
            type = "%(kernel-type)s";
            arch = "%(sys-arch)s";
            os = "efi";
            compression = "%(efi-comp)s";
            load = <0x0>;
            entry = <0x0>;
        };
        fdt {
            description = "Test FDT";
            data = /incbin/("%(fdt-bin)s");
            type = "flat_dt";
            arch = "%(sys-arch)s";
            compression = "%(fdt-comp)s";
        };
        initrd {
            description = "Initial RAM Disk";
            data = /incbin/("%(initrd-fs)s");
            type = "ramdisk";
            compression = "%(initrd-comp)s";
            os = "efi";
        };
    };

    configurations {
        default = "config-efi-fdt";

        config-efi {
            description = "EFI FIT w/o FDT";
            kernel = "helloworld";
        };

        config-efi-fdt {
            description = "EFI FIT w/ FDT";
            kernel = "dtbdump";
            fdt = "fdt";
        };

        config-efi-initrd {
            description = "EFI FIT w/ initrd";
            kernel = "initrddump";
            ramdisk = "initrd";
        };
    };
};
'''

# Define the parametrized FDT data to be used for DTB images generation.
FDT_DATA = '''
/dts-v1/;

/ {
    #address-cells = <1>;
    #size-cells = <1>;

    model = "%(sys-arch)s %(fdt_type)s EFI FIT FDT Boot Test";
    compatible = "%(sys-arch)s";

    reset@0 {
        compatible = "%(sys-arch)s,reset";
        reg = <0 4>;
    };
};
'''

@pytest.mark.buildconfigspec('bootm_efi')
@pytest.mark.buildconfigspec('BOOTEFI_HELLO_COMPILE')
@pytest.mark.buildconfigspec('EFI_LOAD_FILE2_INITRD')
@pytest.mark.buildconfigspec('fit')
@pytest.mark.notbuildconfigspec('generate_acpi_table')
@pytest.mark.requiredtool('dtc')
def test_efi_fit_launch(ubman):
    """Test handling of UEFI binaries inside FIT images.

    The tests are trying to launch U-Boot's helloworld.efi embedded into
    FIT images, in uncompressed or gzip compressed format.

    Additionally, a sample FDT blob is created and embedded into the above
    mentioned FIT images, in uncompressed or gzip compressed format.

    For more details, see launch_efi().

    The following test cases are currently defined and enabled:
     - Launch uncompressed FIT EFI & internal FDT
     - Launch uncompressed FIT EFI & FIT FDT
     - Launch uncompressed FIT EFI & internal FDT & FIT initrd
     - Launch compressed FIT EFI & internal FDT
     - Launch compressed FIT EFI & FIT FDT
     - Launch compressed FIT EFI & internal FDT & FIT initrd
    """

    def net_pre_commands():
        """Execute any commands required to enable network hardware.

        These commands are provided by the boardenv_* file; see the comment
        at the beginning of this file.
        """

        init_usb = ubman.config.env.get('env__net_uses_usb', False)
        if init_usb:
            ubman.run_command('usb start')

        init_pci = ubman.config.env.get('env__net_uses_pci', False)
        if init_pci:
            ubman.run_command('pci enum')

    def net_dhcp():
        """Execute the dhcp command.

        The boardenv_* file may be used to enable/disable DHCP; see the
        comment at the beginning of this file.
        """

        has_dhcp = ubman.config.buildconfig.get('config_cmd_dhcp', 'n') == 'y'
        if not has_dhcp:
            ubman.log.warning('CONFIG_CMD_DHCP != y: Skipping DHCP network setup')
            return False

        test_dhcp = ubman.config.env.get('env__net_dhcp_server', False)
        if not test_dhcp:
            ubman.log.info('No DHCP server available')
            return False

        ubman.run_command('setenv autoload no')
        output = ubman.run_command('dhcp')
        assert 'DHCP client bound to address ' in output
        return True

    def net_setup_static():
        """Set up a static IP configuration.

        The configuration is provided by the boardenv_* file; see the comment at
        the beginning of this file.
        """

        has_dhcp = ubman.config.buildconfig.get('config_cmd_dhcp', 'n') == 'y'
        if not has_dhcp:
            ubman.log.warning('CONFIG_NET != y: Skipping static network setup')
            return False

        env_vars = ubman.config.env.get('env__net_static_env_vars', None)
        if not env_vars:
            ubman.log.info('No static network configuration is defined')
            return False

        for (var, val) in env_vars:
            ubman.run_command('setenv %s %s' % (var, val))
        return True

    def make_fpath(file_name):
        """Compute the path of a given (temporary) file.

        Args:
            file_name -- The name of a file within U-Boot build dir.
        Return:
            The computed file path.
        """

        return os.path.join(ubman.config.build_dir, file_name)

    def make_efi(fname, efi_file, comp):
        """Create an UEFI binary.

        This simply copies lib/efi_loader/helloworld.efi into U-Boot
        build dir and, optionally, compresses the file using gzip.

        Args:
            fname -- The target file name within U-Boot build dir.
            efi_file -- The source .efi application
            comp -- Flag to enable gzip compression.
        Return:
            The path of the created file.
        """

        bin_path = make_fpath(fname)
        utils.run_and_log(ubman,
                          ['cp', make_fpath(f'lib/efi_loader/{efi_file}'),
                           bin_path])
        if comp:
            utils.run_and_log(ubman, ['gzip', '-f', bin_path])
            bin_path += '.gz'
        return bin_path

    def make_dtb(fdt_type, comp):
        """Create a sample DTB file.

        Creates a DTS file and compiles it to a DTB.

        Args:
            fdt_type -- The type of the FDT, i.e. internal, user.
            comp -- Flag to enable gzip compression.
        Return:
            The path of the created file.
        """

        # Generate resources referenced by FDT.
        fdt_params = {
            'sys-arch': sys_arch,
            'fdt_type': fdt_type,
        }

        # Generate a test FDT file.
        dts = make_fpath('test-efi-fit-%s.dts' % fdt_type)
        with open(dts, 'w', encoding='ascii') as file:
            file.write(FDT_DATA % fdt_params)

        # Build the test FDT.
        dtb = make_fpath('test-efi-fit-%s.dtb' % fdt_type)
        utils.run_and_log(ubman,
                          ['dtc', '-I', 'dts', '-O', 'dtb', '-o', dtb, dts])
        if comp:
            utils.run_and_log(ubman, ['gzip', '-f', dtb])
            dtb += '.gz'
        return dtb

    def make_initrd(comp):
        """Create a sample initrd.

        Creates an initrd.

        Args:
            comp -- Flag to enable gzip compression.
        Return:
            The path of the created file.
        """

        # Generate a test initrd file.
        initrd = make_fpath('test-efi-initrd')
        with open(initrd, 'w', encoding='ascii') as file:
            file.write('test-efi-initrd')

        if comp:
            utils.run_and_log(ubman, ['gzip', '-f', initrd])
            initrd += '.gz'
        return initrd

    def make_fit(comp):
        """Create a sample FIT image.

        Runs 'mkimage' to create a FIT image within U-Boot build dir.
        Args:
            comp -- Enable gzip compression for the EFI binary and FDT blob.
        Return:
            The path of the created file.
        """

        # Generate resources referenced by ITS.
        hello_bin = os.path.basename(make_efi('test-efi-helloworld.efi', 'helloworld.efi', comp))
        dtbdump_bin = os.path.basename(make_efi('test-efi-dtbdump.efi', 'dtbdump.efi', comp))
        initrddump_bin = os.path.basename(make_efi('test-efi-initrddump.efi', 'initrddump.efi', comp))
        fdt_bin = os.path.basename(make_dtb('user', comp))
        initrd_fs = make_initrd(comp)
        initrd_fs = os.path.basename(initrd_fs)
        compression = 'gzip' if comp else 'none'
        kernel_type = 'kernel' if comp else 'kernel_noload'

        its_params = {
            'sys-arch': sys_arch,
            'hello-bin': hello_bin,
            'dtbdump-bin': dtbdump_bin,
            'initrddump-bin': initrddump_bin,
            'kernel-type': kernel_type,
            'efi-comp': compression,
            'fdt-bin': fdt_bin,
            'fdt-comp': compression,
            'initrd-fs': initrd_fs,
            'initrd-comp': compression,
        }

        # Generate a test ITS file.
        its_path = make_fpath('test-efi-fit.its')
        with open(its_path, 'w', encoding='ascii') as file:
            file.write(ITS_DATA % its_params)

        # Build the test ITS.
        fit_path = make_fpath('test-efi-fit.fit')
        utils.run_and_log(
            ubman, [make_fpath('tools/mkimage'), '-f', its_path, fit_path])
        return fit_path

    def load_fit_from_host(fit):
        """Load the FIT image using the 'host load' command and return its address.

        Args:
            fit -- Dictionary describing the FIT image to load, see
                   env__efi_fit_test_file in the comment at the beginning of
                   this file.
        Return:
            The address where the file has been loaded.
        """

        addr = fit.get('addr', None)
        if not addr:
            addr = utils.find_ram_base(ubman)

        output = ubman.run_command(
            'host load hostfs - %x %s/%s' % (addr, fit['dn'], fit['fn']))
        expected_text = ' bytes read'
        size = fit.get('size', None)
        if size:
            expected_text = '%d' % size + expected_text
        assert expected_text in output

        return addr

    def load_fit_from_tftp(fit):
        """Load the FIT image using the tftpboot command and return its address.

        The file is downloaded from the TFTP server, its size and optionally its
        CRC32 are validated.

        Args:
            fit -- Dictionary describing the FIT image to load, see env__efi_fit_tftp_file
                   in the comment at the beginning of this file.
        Return:
            The address where the file has been loaded.
        """

        addr = fit.get('addr', None)
        if not addr:
            addr = utils.find_ram_base(ubman)

        file_name = fit['fn']
        output = ubman.run_command('tftpboot %x %s' % (addr, file_name))
        expected_text = 'Bytes transferred = '
        size = fit.get('size', None)
        if size:
            expected_text += '%d' % size
        assert expected_text in output

        expected_crc = fit.get('crc32', None)
        if not expected_crc:
            return addr

        if ubman.config.buildconfig.get('config_cmd_crc32', 'n') != 'y':
            return addr

        output = ubman.run_command('crc32 $fileaddr $filesize')
        assert expected_crc in output

        return addr

    def launch_efi(enable_fdt, enable_initrd, enable_comp):
        """Launch U-Boot's helloworld.efi binary from a FIT image.

        An external image file can be downloaded from TFTP, when related
        details are provided by the boardenv_* file; see the comment at the
        beginning of this file.

        If the size of the TFTP file is not provided within env__efi_fit_tftp_file,
        the test image is generated automatically and placed in the TFTP root
        directory specified via the 'dn' field.

        When running the tests on Sandbox, the image file is loaded directly
        from the host filesystem.

        Once the load address is available on U-Boot console, the 'bootm'
        command is executed for either 'config-efi', 'config-efi-fdt' or
        'config-efi-initrd' FIT configuration, depending on the value of the
        'enable_fdt' and 'enable_initrd' function arguments.

        Eventually the 'Hello, world' message is expected in the U-Boot console.

        Args:
            enable_fdt -- Flag to enable using the FDT blob inside FIT image.
            enable_initrd -- Flag to enable using an initrd inside FIT image.
            enable_comp -- Flag to enable GZIP compression on EFI and FDT
                           generated content.
        """

        with ubman.log.section('FDT=%s;INITRD=%s;COMP=%s' % (enable_fdt, enable_initrd, enable_comp)):
            if is_sandbox:
                fit = {
                    'dn': ubman.config.build_dir,
                }
            else:
                # Init networking.
                net_pre_commands()
                net_set_up = net_dhcp()
                net_set_up = net_setup_static() or net_set_up
                if not net_set_up:
                    pytest.skip('Network not initialized')

                fit = ubman.config.env.get('env__efi_fit_tftp_file', None)
                if not fit:
                    pytest.skip('No env__efi_fit_tftp_file binary specified in environment')

            size = fit.get('size', None)
            if not size:
                if not fit.get('dn', None):
                    pytest.skip('Neither "size", nor "dn" info provided in env__efi_fit_tftp_file')

                # Create test FIT image.
                fit_path = make_fit(enable_comp)
                fit['fn'] = os.path.basename(fit_path)
                fit['size'] = os.path.getsize(fit_path)

                # Copy image to TFTP root directory.
                if fit['dn'] != ubman.config.build_dir:
                    utils.run_and_log(ubman,
                                      ['mv', '-f', fit_path, '%s/' % fit['dn']])

            # Load FIT image.
            addr = load_fit_from_host(fit) if is_sandbox else load_fit_from_tftp(fit)

            # Select boot configuration.
            fit_config = 'config-efi'
            fit_config = fit_config + '-fdt' if enable_fdt else fit_config
            fit_config = fit_config + '-initrd' if enable_initrd else fit_config

            # Try booting.
            ubman.run_command('setenv bootargs nocolor')
            output = ubman.run_command('bootm %x#%s' % (addr, fit_config))
            assert '## Application failed' not in output
            if enable_fdt:
                assert 'Booting using the fdt blob' in output
                assert 'DTB Dump' in output
            if enable_initrd:
                assert 'Loading ramdisk' in output
                assert 'INITRD Dump' in output
            if enable_fdt:
                response = ubman.run_command(cmd = 'dump', wait_for_echo=False)
                assert 'EFI FIT FDT Boot Test' in response
            if enable_initrd:
                response = ubman.run_command('load', wait_for_echo=False)
                assert f"crc32: 0x0c77b025" in response
            if not enable_fdt and not enable_initrd:
                    assert 'Hello, world' in output
            ubman.restart_uboot()

    # Array slice removes leading/trailing quotes.
    sys_arch = ubman.config.buildconfig.get('config_sys_arch', '"sandbox"')[1:-1]
    if sys_arch == 'arm':
        arm64 = ubman.config.buildconfig.get('config_arm64')
        if arm64:
            sys_arch = 'arm64'

    is_sandbox = sys_arch == 'sandbox'

    if is_sandbox:
        old_dtb = ubman.config.dtb

    try:
        if is_sandbox:
            # Use our own device tree file, will be restored afterwards.
            control_dtb = make_dtb('internal', False)
            ubman.config.dtb = control_dtb

        # Run tests
        # - fdt OFF, initrd OFF, gzip OFF
        launch_efi(False, False, False)
        # - fdt ON, initrd OFF, gzip OFF
        launch_efi(True, False, False)
        # - fdt OFF, initrd ON, gzip OFF
        launch_efi(False, True, False)

        if is_sandbox:
            # - fdt OFF, initrd OFF, gzip ON
            launch_efi(False, False, True)
            # - fdt ON, initrd OFF, gzip ON
            launch_efi(True, False, True)
            # - fdt OFF, initrd ON, gzip ON
            launch_efi(False, True, True)

    finally:
        if is_sandbox:
            # Go back to the original U-Boot with the correct dtb.
            ubman.config.dtb = old_dtb
            ubman.restart_uboot()
