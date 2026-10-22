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

3- vbmeta.img
    $ ./avbtool.py add_hash_footer \
                   --image boot.img \
                   --partition_name boot \
                   --partition_size 4194304

    $ ./avbtool.py make_vbmeta_image \
                   --key test/data/testkey_rsa4096.pem \
                   --algorithm SHA256_RSA4096 \
                   --include_descriptors_from_image boot.img \
                   --padding_size 4096 --flags 1 \
                   --output vbmeta.img
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

vbmeta_img_hex = """
1f8b0808f36fcf6a020376626d6574612e696d6700edd369301bea1a07f0
7062dfb758521a55b4b650722807572d41ab2d5a6a4b90a225b64aad69c5
d67063b968a9725b91632bb12fb11faa45c4762c8928557b7a6c557b0fee
ccd9e69e993b77eecc9db91feef87df9bfcfbcf3cefbe5f99b385cd10400
006c803fb0ffe5d7e4f86500fc19e4cff9c775efaf01e4fe2d8701ff5aef
3f9d7ff9d323d413131888866869e8686802fef7025c122d02b16f9c05b1
3fd89904c6054cb993631550b20e3d36f0fc97a5fab3e3ab1813c7059894
3024ec1e6a4c412f9d71de37ebb3132b8f25858b29e0a98d19820724a8ad
11ef74e93b156edc57630f18b6788530b0de2f661a5f755bda6b28cc5367
f41f67dc7c008db14b0f961539b149a94adc44317ab0f55f257d50addbb2
6ca967eb2f0240832e33ad34f0165457652a752b30e1a9ea60bb64458785
4d17bf22cc1fc07b777210d3f3e139b0377020d02b59ac2d84ccb5f3ac4a
eeb60e1322f9445a259a6b3c79f2a563e7e16a9c54886ac6323659f8ef85
82e36fd1b7728f1d645495422df053249da0d2c58fe44b24a250cb8f7306
faa26fb8231abdb6ce657cf416e183cb4a111de2a8363c1d8a61b5673f86
01ef83ddc1059b3a370c8a23a5560bd9433d664dde352db09412cab2272f
5a145b363c347439a13b1ecc535e95f53ea79c401b48efa277065eb21467
6f104b19d15d8f4790cce128dd673132db7edc29bb63b0899e25ab1778ce
f14622bd6d9ff0e486336968f432b3c107e9e0fa75044c19e8474936cfec
c85b0a1cad94ac579b390a995d312280a6d3b0dc2224da514ef5965f2749
4733a4324d40ec6bb8b8c7ec2d0bbad1a8ffdbf3743231b1125be6ffe07d
bbf14e2e7c3c7cba09be7b0486e22b708b40cc39d88176eb190bcbb9905c
09cf1c8dbd282817c483f5b7f7b1cc6147f935fb65c5b7d42ad70b5cb49a
667f3d51fd70d38a5dcb017c8df917df58c27485c47fb846bf1781f27b3d
42ee795c827dfb6fdf007feb11e4bfdd61cfc0404cc5d6f032473f5cc44e
f49025bc09950a607ad7114cf1da446413f27b3fc1c875231c6ba6c5d629
179c169fd52add2237fc48f17ee770a9b15ca6ade3094e1c001006dc9e04
9219c0caf9d178f4638124c3aee451d1db2ccfbecf42081ae841991956cd
cbfa693d4815d0c2b921c6657fede06ed3854870fcf34486cff70a07c557
df5b0bede94b55dc2a9138ae2297c3980664e5d6ca9551a8aece84ad2ada
59999ec559e947c1510f35ebaa136a6d5ebf7e03d291a12cf71c1ef08495
6567fe6456310469ddc4894c29f0c3839bfc27a2dce1789043666397ca8b
aa6b6cfc9a8c4cfce65a00a15dd365b7a8721a873547a4a56722cc8e29b1
5ceb11d40ead10cf3bf303ddad5d22b244eb8e63e547c45d1809417def08
35daec737be2bb5fe95dbc36405923229b731364c6792a9e5e3718ea77ed
8bc0edd18aac4b7c19a35a2207bbec281519f511196f30c8a4ad6dcfe607
eb45652b64f49d7965f5c53e6f413b3d90de57d04ff6c64b47ac0284a213
23c2fce49b6b4a0500e7d42e5ba2c441d4b66b4cae141cc650e760918560
1809522a81a6ccc4ba2d1c024739f89af56baccca1f01e0113e36c9c8d71
677d4b10b258b7d2f29ca8b9c68e103e632e9eed4054485ce2fc6052ce7a
53b7645090c4a7f677e8b39eb5dbea20fc570739f3b28a4b0b88f4357b6f
44db1a7949f4b2405cdf8a1f5abd70bddbd9bb22399b2e80b6ead9e9ced0
22f0ac6b334b751baca4f605ddcd7bb5d337208ec2ede1ea714a13548f03
e86c0787a69db3e81940b11b0dabf1f6198164b069aa8198064c09d4eae6
940ee4359aeec1cce4bb6283cb525ed79988accaa5fc2c1e9b713dce97d7
0f867063b9d292672e8f58bb2051a5776779fbf928ced8377c4bedc704a1
473aa9ecde18363435b41cebc30d3596295273af3d61a65044f901d1b49b
44d7bbc439e425afbebcf6095e31fc772ae6707eea95d8c324afe5a4bab4
3db92585c26127dd880cfaa0e1d13c916f089d90eff5c5f59d350a19a0af
af42f06d80b6ecd75cf56c954957f49ebf4f75b7e2dbfd6bbb51a87d5764
0950f2e1e72fdf7882136aaa3670120d63d73f04974fa900c7eae49bbd1c
1c570ee30b1e890956d25fdc5121c5c4a7f0e4fb76b64cee3c296f7be8c1
b3946c1856b3a6095900c866af74f400c0f2d5a065131f8d55bdd645701d
a78256f019ab8150ebd06f53484904ebc86d7caf8490eee55d7f9cf4866b
89f720181421ff81aea5d92d3fade3f2a38caa91dbde272a476a63bf9546
fefc76b1a5b4701eb078fb02a637100551b538af242d3ba2a6e5ef7cb180
b3a8c684c6ab9cc593550ef70abf67fb7c8251a49e3fb43a87d98a9dcf9e
7c3cb6cf17646a581d958d9be8a74bb7a2f88e6c128f7dca29b49b8df76a
5d26e4147f467e9752ad8dadb562330ebe2d68dbeca174f869b17ee5f5d0
40d6ab894ea47d3a996306b26ccb7479e6b3b6bd7ec52b488c2f48fc4cdf
ac3415b9231b7415cd8959add708fb88219272f2135e7e2e673c069c3a75
ead4a953a7feeffd03523602d400100000
"""

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
