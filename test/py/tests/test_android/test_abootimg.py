# SPDX-License-Identifier:  GPL-2.0+
# Copyright (c) 2020
# Author: Sam Protsenko <joe.skb7@gmail.com>

# Test U-Boot's "abootimg" commands.

import os
import pytest
import utils

"""
These tests rely on disk image (boot.img), which is automatically created by
the test from the stored hex dump. This is done to avoid the dependency on the
most recent mkbootimg tool from AOSP/master. Here is the list of commands which
was used to generate the boot.img and obtain compressed hex dump from it:

    $ echo '/dts-v1/; / { model = "x1"; compatible = "y1,z1"; };' > test1.dts
    $ echo '/dts-v1/; / { model = "x2"; compatible = "y2,z2"; };' > test2.dts
    $ dtc test1.dts > dt1.dtb
    $ dtc test2.dts > dt2.dtb
    $ cat dt1.dtb dt2.dtb > dtb.img
    $ echo 'kernel payload' > kernel
    $ echo 'ramdisk payload' > ramdisk.img
    $ mkbootimg --kernel ./kernel --ramdisk ./ramdisk.img  \
                --cmdline "cmdline test" --dtb ./dtb.img   \
                --os_version R --os_patch_level 2019-06-05 \
                --header_version 2 --output boot.img
    $ gzip -9 boot.img
    $ xxd -p boot.img.gz > boot.img.gz.hex

Now one can obtain original boot.img from this hex dump like this:

    $ xxd -r -p boot.img.gz.hex boot.img.gz
    $ gunzip -9 boot.img.gz

For boot image header version 4, these tests rely on two images that are generated
using the same steps above :

1- boot.img :
    $ mkbootimg --kernel ./kernel --ramdisk ./ramdisk.img  \
                --cmdline "cmdline test" --dtb ./dtb.img   \
                --os_version R --os_patch_level 2019-06-05 \
                --header_version 4 --output ./boot.img

2- vendor_boot.img
    $ mkbootimg --kernel ./kernel --ramdisk ./ramdisk.img  \
                --cmdline "cmdline test" --dtb ./dtb.img   \
                --os_version R --os_patch_level 2019-06-05 \
                --pagesize 4096  --vendor_ramdisk ./ramdisk.img \
                --header_version 4 --vendor_boot ./vboot.img \

"""

# boot.img.gz hex dump
img_hex = """1f8b08084844af5d0203626f6f742e696d670073f47309f2f77451e46700
820606010106301084501f04181819041838181898803c3346060c909c9b
92939997aa50925a5cc2300a461c3078b2e1793c4b876fd92db97939fb6c
b7762ffff07d345446c1281805e8a0868d81e117a45e111c0d8dc101b253
8bf25273140a122b73f21353b8460364148c8251300a46c1281801a02831
3725b3387bb401300a46c1281805a360148c207081f7df5b20550bc41640
9c03c41a0c90f17fe85400986d82452b6c3680198a192a0ce17c3610ae34
d4a9820881a70f3873f35352731892f3730b124b32937252a96bb9119ae5
463a5546f82c1f05a360148c8251300a462e000085bf67f200200000"""

# bootable boot image v4 hex dump (with DTBs)
boot_img_hex = """1f8b0808e2dd60690203626f6f745f6e65772e696d6700edd8ab0d836018
40d14f900a0c8c5036e80a24180c24dde04f4034d04740b13d02d90920e7
8c70e5adbbe6d9b74d5544441987c72dfe64010000009cd5342e9f71beff
d2367fd3900b0200000017b4a4f7f05a2703000000002e6c0765d9bd6000
300000"""

# bootable vendor boot image v4 hex dump (with DTBs + bootconfig)
vboot_img_hex = """1f8b0808e2dd6069020376656e646f725f626f6f745f6e65772e696d6700
eddb316bc24014c0f14b2dd20a425c3b393a48c1d0c1a54be9aca588fbc5
0b3434e9c979d04628f811fd22426787d2bbc43a68e9d4a5f1ff83f7de25
5c787053f220d3d1fde3dd783c39174ee86255e68e4f0000e00405e2e835
e0e142886db9fae8f89c95dbaa7ac5890100f02f1899ab74f1dc9dcb22d3
52b538110000000000ea67ddfedcb8f2ee6228aa317ecf859fed7fcffefd
fae68747835d6dec42bc0df6d74d1fc5a0bfac6e89331797b9564926663a
9f4b9bc659f2b7cda383e6517f19fdd61c0000000080d3111e7c4f030000
000080fa912fcae854c55adbeb2769d4ab34c9ad4d16963f010000000000
a88d2fb468951800500000"""

# Expected response for "abootimg dtb_dump" command
dtb_dump_resp="""## DTB area contents (concat format):
 - DTB #0:
           (DTB)size = 125
          (DTB)model = x1
     (DTB)compatible = y1,z1
 - DTB #1:
           (DTB)size = 125
          (DTB)model = x2
     (DTB)compatible = y2,z2"""
# Address in RAM where to load the boot image ('abootimg' looks in $loadaddr)
loadaddr = 0x1000
# Address in RAM where to load the vendor boot image ('abootimg' looks in $vloadaddr)
vloadaddr= 0x10000
# Expected DTB #1 size
dtb1_size = 0x7d
# Expected DTB #2 size
dtb2_size = 0x7d
# Expected DTB #1 offset from the boot image start address
dtb1_offset = 0x1800 + dtb1_size
# Expected DTB offset from the vendor boot image start address
dtb2_offset = 0x2000 + dtb2_size
# Expected DTB aligned offset
dtba_offset = 0x4000
# DTB #1 start address in RAM
dtb1_addr = loadaddr + dtb1_offset
# DTB #2 start address in RAM
dtb2_addr = vloadaddr + dtb2_offset

class AbootimgTestDiskImage(object):
    """Disk image used by abootimg tests."""

    def __init__(self, ubman, image_name, hex_img):
        """Initialize a new AbootimgDiskImage object.

        Args:
            ubman: A U-Boot console.

        Returns:
            Nothing.
        """

        gz_hex = ubman.config.persistent_data_dir + '/' + image_name  + '.gz.hex'
        gz = ubman.config.persistent_data_dir + '/' + image_name + '.gz'

        filename = image_name
        persistent = ubman.config.persistent_data_dir + '/' + filename
        self.path = ubman.config.result_dir  + '/' + filename
        ubman.log.action('persistent is ' + persistent)
        with utils.persistent_file_helper(ubman.log, persistent):
            if os.path.exists(persistent):
                ubman.log.action('Disk image file ' + persistent +
                    ' already exists')
            else:
                ubman.log.action('Generating ' + persistent)

                f = open(gz_hex, "w")
                f.write(hex_img)
                f.close()
                cmd = ('xxd', '-r', '-p', gz_hex, gz)
                utils.run_and_log(ubman, cmd)
                cmd = ('gunzip', '-9', gz)
                utils.run_and_log(ubman, cmd)

        cmd = ('cp', persistent, self.path)
        utils.run_and_log(ubman, cmd)

gtdi1 = None
@pytest.fixture(scope='function')
def abootimg_disk_image(ubman):
    """pytest fixture to provide a AbootimgTestDiskImage object to tests.
    This is function-scoped because it uses ubman, which is also
    function-scoped. However, we don't need to actually do any function-scope
    work, so this simply returns the same object over and over each time."""

    global gtdi1
    if not gtdi1:
        gtdi1 = AbootimgTestDiskImage(ubman, 'boot.img', img_hex)
    return gtdi1

gtdi2 = None
@pytest.fixture(scope='function')
def abootimgv4_disk_image_vboot(ubman):
    """pytest fixture to provide a AbootimgTestDiskImage object to tests.
    This is function-scoped because it uses ubman, which is also
    function-scoped. However, we don't need to actually do any function-scope
    work, so this simply returns the same object over and over each time."""

    global gtdi2
    if not gtdi2:
        gtdi2 = AbootimgTestDiskImage(ubman, 'vendor_boot.img', vboot_img_hex)
    return gtdi2

gtdi3 = None
@pytest.fixture(scope='function')
def abootimgv4_disk_image_boot(ubman):
    """pytest fixture to provide a AbootimgTestDiskImage object to tests.
    This is function-scoped because it uses ubman, which is also
    function-scoped. However, we don't need to actually do any function-scope
    work, so this simply returns the same object over and over each time."""

    global gtdi3
    if not gtdi3:
        gtdi3 = AbootimgTestDiskImage(ubman, 'bootv4.img', boot_img_hex)
    return gtdi3

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('android_boot_image')
@pytest.mark.buildconfigspec('cmd_abootimg')
@pytest.mark.buildconfigspec('cmd_fdt')
@pytest.mark.requiredtool('xxd')
@pytest.mark.requiredtool('gunzip')
def test_abootimg(abootimg_disk_image, ubman):
    """Test the 'abootimg' command."""

    ubman.log.action('Loading disk image to RAM...')
    ubman.run_command('setenv loadaddr 0x%x' % (loadaddr))
    ubman.run_command('host load hostfs - 0x%x %s' % (loadaddr,
        abootimg_disk_image.path))

    ubman.log.action('Testing \'abootimg get ver\'...')
    response = ubman.run_command('abootimg get ver')
    assert response == "2"
    ubman.run_command('abootimg get ver v')
    response = ubman.run_command('env print v')
    assert response == 'v=2'

    ubman.log.action('Testing \'abootimg get recovery_dtbo\'...')
    response = ubman.run_command('abootimg get recovery_dtbo a')
    assert response == 'Error: recovery_dtbo_size is 0'

    ubman.log.action('Testing \'abootimg dump dtb\'...')
    response = ubman.run_command('abootimg dump dtb').replace('\r', '')
    assert response == dtb_dump_resp

    ubman.log.action('Testing \'abootimg get dtb_load_addr\'...')
    ubman.run_command('abootimg get dtb_load_addr a')
    response = ubman.run_command('env print a')
    assert response == 'a=11f00000'

    ubman.log.action('Testing \'abootimg get dtb --index\'...')
    ubman.run_command('abootimg get dtb --index=1 dtb1_start dtb1_size')
    response = ubman.run_command('env print dtb1_start')
    correct_str = "dtb1_start=%x" % (dtb1_addr)
    assert response == correct_str
    response = ubman.run_command('env print dtb1_size')
    correct_str = "dtb1_size=%x" % (dtb1_size)
    assert response == correct_str
    ubman.run_command('setenv dtbaaddr 0x%x' % (dtba_offset))
    ubman.run_command('cp.b $dtb1_start $dtbaaddr $dtb1_size')
    ubman.run_command('fdt addr $dtbaaddr')
    ubman.run_command('fdt get value v / model')
    response = ubman.run_command('env print v')
    assert response == 'v=x2'

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('android_boot_image')
@pytest.mark.buildconfigspec('cmd_abootimg')
@pytest.mark.buildconfigspec('cmd_fdt')
@pytest.mark.requiredtool('xxd')
@pytest.mark.requiredtool('gunzip')
def test_abootimgv4(abootimgv4_disk_image_vboot, abootimgv4_disk_image_boot, ubman):
    """Test the 'abootimg' command with boot image header v4."""

    ubman.log.action('Loading disk image to RAM...')
    ubman.run_command('setenv loadaddr 0x%x' % (loadaddr))
    ubman.run_command('setenv vloadaddr 0x%x' % (vloadaddr))
    ubman.run_command('host load hostfs - 0x%x %s' % (vloadaddr,
	abootimgv4_disk_image_vboot.path))
    ubman.run_command('host load hostfs - 0x%x %s' % (loadaddr,
        abootimgv4_disk_image_boot.path))
    ubman.run_command('abootimg addr 0x%x 0x%x' % (loadaddr, vloadaddr))
    ubman.log.action('Testing \'abootimg get ver\'...')
    response = ubman.run_command('abootimg get ver')
    assert response == "4"
    ubman.run_command('abootimg get ver v')
    response = ubman.run_command('env print v')
    assert response == 'v=4'

    ubman.log.action('Testing \'abootimg get recovery_dtbo\'...')
    response = ubman.run_command('abootimg get recovery_dtbo a')
    assert response == 'Error: header version must be >= 1 and <= 2 to get dtbo'

    ubman.log.action('Testing \'abootimg get dtb_load_addr\'...')
    ubman.run_command('abootimg get dtb_load_addr a')
    response = ubman.run_command('env print a')
    assert response == 'a=11f00000'

    ubman.log.action('Testing \'abootimg get dtb --index\'...')
    ubman.run_command('abootimg get dtb --index=1 dtb2_start dtb2_size')
    response = ubman.run_command('env print dtb2_start')
    correct_str = "dtb2_start=%x" % (dtb2_addr)
    assert response == correct_str
    response = ubman.run_command('env print dtb2_size')
    correct_str = "dtb2_size=%x" % (dtb2_size)
    assert response == correct_str

    ubman.run_command('setenv dtbaaddr 0x%x' % (dtba_offset))
    ubman.run_command('cp.b $dtb2_start $dtbaaddr $dtb2_size')
    ubman.run_command('fdt addr $dtbaaddr')
    ubman.run_command('fdt get value v / model')
    response = ubman.run_command('env print v')
    assert response == 'v=x2'

@pytest.mark.boardspec('sandbox')
@pytest.mark.buildconfigspec('android_boot_image')
@pytest.mark.buildconfigspec('cmd_abootimg')
@pytest.mark.requiredtool('xxd')
@pytest.mark.requiredtool('gunzip')
def test_abootimg_bootconfig(abootimgv4_disk_image_vboot,
                              abootimgv4_disk_image_boot,
                              ubman):
    """Test bootconfig handling with boot image v4.

    Verifies that androidboot.* parameters from bootargs are appended to the
    bootconfig section in vendor_boot image in memory, and that non-androidboot
    parameters remain in bootargs.
    """

    # Setup addresses
    ram_base = utils.find_ram_base(ubman)
    ramdisk_addr_r = ram_base + 0x4000000
    ubman.run_command('setenv ramdisk_addr_r 0x%x' % ramdisk_addr_r)
    ubman.run_command('setenv loadaddr 0x%x' % loadaddr)
    ubman.run_command('setenv vloadaddr 0x%x' % vloadaddr)

    # Set bootargs with androidboot.* parameters
    ubman.run_command('setenv bootargs "androidboot.serialno=ABC123 androidboot.mode=recovery console=ttyS0"')

    # Load images
    ubman.run_command('host load hostfs - 0x%x %s' % (vloadaddr,
        abootimgv4_disk_image_vboot.path))
    ubman.run_command('host load hostfs - 0x%x %s' % (loadaddr,
        abootimgv4_disk_image_boot.path))
    ubman.run_command('abootimg addr 0x%x 0x%x' % (loadaddr, vloadaddr))

    # Extract ramdisk (triggers bootconfig append)
    ubman.run_command('abootimg get ramdisk ramdisk_addr ramdisk_size')

    # Get ramdisk address
    response = ubman.run_command('env print ramdisk_addr')
    ramdisk_start = int(response.split('=')[1], 16)

    # Verify androidboot.* parameters were removed from bootargs
    response = ubman.run_command('env print bootargs')
    assert 'androidboot.' not in response
    assert 'console=ttyS0' in response

    # Get ramdisk size and verify BOOTCONFIG magic at the end
    response = ubman.run_command('env print ramdisk_size')
    ramdisk_size = int(response.split('=')[1], 16)

    # Dump the end of the ramdisk where BOOTCONFIG trailer should be
    # The trailer is at the end, so dump the last 48 bytes
    response = ubman.run_command('md.b 0x%x 48' % (ramdisk_start + ramdisk_size - 48))

    # Verify BOOTCONFIG magic is present
    assert 'BOOTCONFIG' in response
