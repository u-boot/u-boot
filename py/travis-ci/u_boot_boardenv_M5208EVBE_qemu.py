import os
import travis_tftp

env__net_uses_pci = False
env__net_dhcp_server = True
env__net_tftp_readable_file = travis_tftp.file2env('u-boot')
