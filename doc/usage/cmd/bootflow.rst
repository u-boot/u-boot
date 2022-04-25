.. SPDX-License-Identifier: GPL-2.0+:

bootflow command
================

Synopis
-------

::

    bootflow scan [-abel] [bootdev]
    bootflow list [-e]
    bootflow select [<num|name>]
    bootflow info [-d]
    bootflow boot


Description
-----------

The `bootflow` command is used to manage bootflows. It can scan bootdevs to
locate bootflows, list them and boot them.

See :doc:`../../develop/bootstd` for more information.


bootflow scan
~~~~~~~~~~~~~

Scans for available bootflows, optionally booting the first valid one it finds.
This operates in two modes:

- If no bootdev is selected (see `bootdev select`) it scans bootflows one
  by one, extracting all the bootdevs from each
- If a bootdev is selected, it just scans that one bootflow

Flags are:

-a
    Collect all bootflows, even those that cannot be loaded. Normally if a file
    is not where it is expected, then the bootflow fails and so is dropped
    during the scan. With this option you can see why each bootflow would be
    dropped.

-b
    Boot each valid bootflow as it is scanned. Typically only the first bootflow
    matters, since by then the system boots in the OS and U-Boot is no-longer
    running. `bootflow scan -b` is a quick way to boot the first available OS.
    A valid bootflow is one that made it all the way to the `loaded` state.

-e
    Used with -l to also show errors for each bootflow. The shows detailed error
    information for each bootflow that failed to make it to the `loaded` state.

-l
    List bootflows while scanning. This is helpful when you want to see what
    is happening during scanning. Use it with the `-b` flag to see which
    bootdev and bootflows are being tried.

The optional argument specifies a particular bootdev to scan. This can either be
the name of a bootdev or its sequence number (both shown with `bootdev list`).
Alternatively a convenience label can be used, like `mmc0`, which is the type of
device and an optional sequence number. Specifically, the label is the uclass of
the bootdev's parent followed by the sequence number of that parent. Sequence
numbers are typically set by aliases, so if you have 'mmc0' in your devicetree
alias section, then `mmc0` refers to the bootdev attached to that device.


bootflow list
~~~~~~~~~~~~~

Lists the previously scanned bootflows. You must use `bootflow scan` before this
to see anything.

If you scanned with -a and have bootflows with errors, -e can be used to show
those errors.

The list looks something like this:

===  ======  ======  ========  ====  ===============================   ================
Seq  Method  State   Uclass    Part  Name                              Filename
===  ======  ======  ========  ====  ===============================   ================
  0  distro  ready   mmc          2  mmc\@7e202000.bootdev.part_2      /boot/extlinux/extlinux.conf
  1  pxe     ready   ethernet     0  smsc95xx_eth.bootdev.0            rpi.pxe/extlinux/extlinux.conf
===  ======  ======  ========  ====  ===============================   ================

The fields are as follows:

Seq:
    Sequence number in the scan, used to reference the bootflow later

Method:
    The boot method (bootmeth) used to find the bootflow. Several methods are
    included in U-Boot.

State:
    Current state of the bootflow, indicating how far the bootdev got in
    obtaining a valid one. See :ref:`BootflowStates` for a list of states.

Uclass:
    Name of the media device's Uclass. This indicates the type of the parent
    device (e.g. MMC, Ethernet).

Part:
    Partition number being accesseed, numbered from 1. Normally a device will
    have a partition table with a small number of partitions. For devices
    without partition tables (e.g. network) this field is 0.

Name:
    Name of the bootflow. This is generated from the bootdev appended with
    the partition information

Filename:
    Name of the bootflow file. This indicates where the file is on the
    filesystem or network device.


bootflow select
~~~~~~~~~~~~~~~

Use this to select a particular bootflow. You can select it by the sequence
number or name, as shown in `bootflow list`.

Once a bootflow is selected, you can use `bootflow info` and `bootflow boot`.

If no bootflow name or number is provided, then any existing bootflow is
unselected.


bootflow info
~~~~~~~~~~~~~

This shows information on the current bootflow, with the format looking like
this:

=========  ===============================
Name       mmc\@7e202000.bootdev.part_2
Device     mmc\@7e202000.bootdev
Block dev  mmc\@7e202000.blk
Type       distro
Method:    syslinux
State      ready
Partition  2
Subdir     (none)
Filename   /extlinux/extlinux.conf
Buffer     3db7ad48
Size       232 (562 bytes)
Error      0
=========  ===============================

Most of the information is the same as `bootflow list` above. The new fields
are:

Device
    Name of the bootdev

Block dev
    Name of the block device, if any. Network devices don't have a block device.

Subdir
    Subdirectory used for retrieving files. For network bootdevs this is the
    directory of the 'bootfile' parameter passed from DHCP. All file retrievals
    when booting are relative to this.

Buffer
    Buffer containing the bootflow file. You can use the :doc:`md` to look at
    it, or dump it with `bootflow info -d`.

Size
    Size of the bootflow file

Error
    Error number returned from scanning for the bootflow. This is 0 if the
    bootflow is in the 'loaded' state, or a negative error value on error. You
    can look up Linux error codes to find the meaning of the number.

Use the `-d` flag to dump out the contents of the bootfile file.


bootflow boot
~~~~~~~~~~~~~

This boots the current bootflow.


Example
-------

Here is an example of scanning for bootflows, then listing them::

    U-Boot> bootflow scan -l
    Scanning for bootflows in all bootdevs
    Seq  Type         State   Uclass    Part  Name                      Filename
    ---  -----------  ------  --------  ----  ------------------------  ----------------
    Scanning bootdev 'mmc@7e202000.bootdev':
      0  distro       ready   mmc          2  mmc@7e202000.bootdev.p    /extlinux/extlinux.conf
    Scanning bootdev 'sdhci@7e300000.bootdev':
    Card did not respond to voltage select! : -110
    Scanning bootdev 'smsc95xx_eth.bootdev':
    Waiting for Ethernet connection... done.
    BOOTP broadcast 1
    DHCP client bound to address 192.168.4.30 (4 ms)
    Using smsc95xx_eth device
    TFTP from server 192.168.4.1; our IP address is 192.168.4.30
    Filename 'rpi.pxe/'.
    Load address: 0x200000
    Loading: *
    TFTP error: 'Is a directory' (0)
    Starting again

    missing environment variable: pxeuuid
    Retrieving file: rpi.pxe/pxelinux.cfg/01-b8-27-eb-a6-61-e1
    Waiting for Ethernet connection... done.
    Using smsc95xx_eth device
    TFTP from server 192.168.4.1; our IP address is 192.168.4.30
    Filename 'rpi.pxe/pxelinux.cfg/01-b8-27-eb-a6-61-e1'.
    Load address: 0x2500000
    Loading: ##################################################  566 Bytes
    	 45.9 KiB/s
    done
    Bytes transferred = 566 (236 hex)
      1  distro       ready   ethernet     0  smsc95xx_eth.bootdev.0 rpi.pxe/extlinux/extlinux.conf
    No more bootdevs
    ---  -----------  ------  --------  ----  ------------------------  ----------------
    (2 bootflows, 2 valid)
    U-Boot> bootflow l
    Showing all bootflows
    Seq  Type         State   Uclass    Part  Name                      Filename
    ---  -----------  ------  --------  ----  ------------------------  ----------------
      0  distro       ready   mmc          2  mmc@7e202000.bootdev.p    /extlinux/extlinux.conf
      1  pxe          ready   ethernet     0  smsc95xx_eth.bootdev.0     rpi.pxe/extlinux/extlinux.conf
    ---  -----------  ------  --------  ----  ------------------------  ----------------
    (2 bootflows, 2 valid)


The second one is then selected by name (we could instead use `bootflow sel 0`),
displayed and booted::

    U-Boot> bootflow info
    No bootflow selected
    U-Boot> bootflow sel mmc@7e202000.bootdev.part_2
    U-Boot> bootflow info
    Name:      mmc@7e202000.bootdev.part_2
    Device:    mmc@7e202000.bootdev
    Block dev: mmc@7e202000.blk
    Sequence:  1
    Method:    distro
    State:     ready
    Partition: 2
    Subdir:    (none)
    Filename:  extlinux/extlinux.conf
    Buffer:    3db7ae88
    Size:      232 (562 bytes)
    Error:     0
    U-Boot> bootflow boot
    ** Booting bootflow 'smsc95xx_eth.bootdev.0'
    Ignoring unknown command: ui
    Ignoring malformed menu command:  autoboot
    Ignoring malformed menu command:  hidden
    Ignoring unknown command: totaltimeout
    1:	Fedora-Workstation-armhfp-31-1.9 (5.3.7-301.fc31.armv7hl)
    Retrieving file: rpi.pxe/initramfs-5.3.7-301.fc31.armv7hl.img
    get 2700000 rpi.pxe/initramfs-5.3.7-301.fc31.armv7hl.img
    Waiting for Ethernet connection... done.
    Using smsc95xx_eth device
    TFTP from server 192.168.4.1; our IP address is 192.168.4.30
    Filename 'rpi.pxe/initramfs-5.3.7-301.fc31.armv7hl.img'.
    Load address: 0x2700000
    Loading: ###################################T ###############  57.7 MiB
    	 1.9 MiB/s
    done
    Bytes transferred = 60498594 (39b22a2 hex)
    Retrieving file: rpi.pxe//vmlinuz-5.3.7-301.fc31.armv7hl
    get 80000 rpi.pxe//vmlinuz-5.3.7-301.fc31.armv7hl
    Waiting for Ethernet connection... done.
    Using smsc95xx_eth device
    TFTP from server 192.168.4.1; our IP address is 192.168.4.30
    Filename 'rpi.pxe//vmlinuz-5.3.7-301.fc31.armv7hl'.
    Load address: 0x80000
    Loading: ##################################################  7.2 MiB
    	 2.3 MiB/s
    done
    Bytes transferred = 7508480 (729200 hex)
    append: ro root=UUID=9732b35b-4cd5-458b-9b91-80f7047e0b8a rhgb quiet LANG=en_US.UTF-8 cma=192MB cma=256MB
    Retrieving file: rpi.pxe//dtb-5.3.7-301.fc31.armv7hl/bcm2837-rpi-3-b.dtb
    get 2600000 rpi.pxe//dtb-5.3.7-301.fc31.armv7hl/bcm2837-rpi-3-b.dtb
    Waiting for Ethernet connection... done.
    Using smsc95xx_eth device
    TFTP from server 192.168.4.1; our IP address is 192.168.4.30
    Filename 'rpi.pxe//dtb-5.3.7-301.fc31.armv7hl/bcm2837-rpi-3-b.dtb'.
    Load address: 0x2600000
    Loading: ##################################################  13.8 KiB
    	 764.6 KiB/s
    done
    Bytes transferred = 14102 (3716 hex)
    Kernel image @ 0x080000 [ 0x000000 - 0x729200 ]
    ## Flattened Device Tree blob at 02600000
       Booting using the fdt blob at 0x2600000
       Using Device Tree in place at 02600000, end 02606715

    Starting kernel ...

    [  OK  ] Started Show Plymouth Boot Screen.
    [  OK  ] Started Forward Password R…s to Plymouth Directory Watch.
    [  OK  ] Reached target Local Encrypted Volumes.
    [  OK  ] Reached target Paths.
    ....


Here we scan for bootflows and boot the first one found::

    U-Boot> bootflow scan -bl
    Scanning for bootflows in all bootdevs
    Seq  Method       State   Uclass    Part  Name                    Filename
    ---  -----------  ------  --------  ----  ----------------------  ----------------
    Scanning bootdev 'mmc@7e202000.bootdev':
      0  distro       ready   mmc          2  mmc@7e202000.bootdev.p  /extlinux/extlinux.conf
    ** Booting bootflow 'mmc@7e202000.bootdev.part_2'
    Ignoring unknown command: ui
    Ignoring malformed menu command:  autoboot
    Ignoring malformed menu command:  hidden
    Ignoring unknown command: totaltimeout
    1:	Fedora-KDE-armhfp-31-1.9 (5.3.7-301.fc31.armv7hl)
    Retrieving file: /initramfs-5.3.7-301.fc31.armv7hl.img
    getfile 2700000 /initramfs-5.3.7-301.fc31.armv7hl.img
    Retrieving file: /vmlinuz-5.3.7-301.fc31.armv7hl
    getfile 80000 /vmlinuz-5.3.7-301.fc31.armv7hl
    append: ro root=UUID=b8781f09-e2dd-4cb8-979b-7df5eeaaabea rhgb LANG=en_US.UTF-8 cma=192MB console=tty0 console=ttyS1,115200
    Retrieving file: /dtb-5.3.7-301.fc31.armv7hl/bcm2837-rpi-3-b.dtb
    getfile 2600000 /dtb-5.3.7-301.fc31.armv7hl/bcm2837-rpi-3-b.dtb
    Kernel image @ 0x080000 [ 0x000000 - 0x729200 ]
    ## Flattened Device Tree blob at 02600000
       Booting using the fdt blob at 0x2600000
       Using Device Tree in place at 02600000, end 02606715

    Starting kernel ...

    [    0.000000] Booting Linux on physical CPU 0x0


Here is am example using the -e flag to see all errors::

    U-Boot> bootflow scan -a
    Card did not respond to voltage select! : -110
    Waiting for Ethernet connection... done.
    BOOTP broadcast 1
    DHCP client bound to address 192.168.4.30 (4 ms)
    Using smsc95xx_eth device
    TFTP from server 192.168.4.1; our IP address is 192.168.4.30
    Filename 'rpi.pxe/'.
    Load address: 0x200000
    Loading: *
    TFTP error: 'Is a directory' (0)
    Starting again

    missing environment variable: pxeuuid
    Retrieving file: rpi.pxe/pxelinux.cfg/01-b8-27-eb-a6-61-e1
    Waiting for Ethernet connection... done.
    Using smsc95xx_eth device
    TFTP from server 192.168.4.1; our IP address is 192.168.4.30
    Filename 'rpi.pxe/pxelinux.cfg/01-b8-27-eb-a6-61-e1'.
    Load address: 0x2500000
    Loading: ##################################################  566 Bytes
    	 49.8 KiB/s
    done
    Bytes transferred = 566 (236 hex)
    U-Boot> bootflow l -e
    Showing all bootflows
    Seq  Type         State   Uclass    Part  Name                   Filename
    ---  -----------  ------  --------  ----  ---------------------  ----------------
      0  distro       fs      mmc          1  mmc@7e202000.bootdev.p /extlinux/extlinux.conf
         ** File not found, err=-2
      1  distro       ready   mmc          2  mmc@7e202000.bootdev.p /extlinux/extlinux.conf
      2  distro       fs      mmc          3  mmc@7e202000.bootdev.p /extlinux/extlinux.conf
         ** File not found, err=-1
      3  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
      4  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
      5  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
      6  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
      7  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
      8  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
      9  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
      a  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
      b  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
      c  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
      d  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
      e  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
      f  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
     10  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
     11  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
     12  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
     13  distro       media   mmc          0  mmc@7e202000.bootdev.p <NULL>
         ** No partition found, err=-2
     14  distro       ready   ethernet     0  smsc95xx_eth.bootdev.0 rpi.pxe/extlinux/extlinux.conf
    ---  -----------  ------  --------  ----  ---------------------  ----------------
    (21 bootflows, 2 valid)
    U-Boot>


Return value
------------

On success `bootflow boot` normally boots into the Operating System and does not
return to U-Boot. If something about the U-Boot processing fails, then the
return value $? is 1. If the boot succeeds but for some reason the Operating
System returns, then $? is 0, indicating success.

For other subcommands, the return value $? is always 0 (true).


.. BootflowStates_:
