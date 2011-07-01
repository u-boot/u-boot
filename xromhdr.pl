#!/usr/bin/perl

use File::Basename;

print "Xilinx Boot ROM header generator 1.0\n";
if ($#ARGV != 1) {
    print "Example usage:\n";
    print "$0 u-boot.bin NOR\n";
    print "$0 u-boot.bin QSPI\n";
    exit (1);
}

$hdr_type = "NONE";
if ($ARGV[1] eq "NOR") {
    print "NOR header selected.\n";
    $hdr_type = "nor";
} elsif ($ARGV[1] eq "QSPI") {
    print "QSPI header selected.\n";
    $hdr_type = "qspi";
} else {
    print "ABORT: Must select NOR or QSPI header.\n";
    exit (1);
}

$input_bin = $ARGV[0];
$work_dir  = dirname($input_bin);
$work_file = basename($input_bin);
print "Wrap file: $input_bin\n";
$output_bin = "$work_dir/$hdr_type-$work_file";
print "Output file: $output_bin\n";

open(OUTFILE, ">$output_bin");

# RESET VECTOR
$a = pack('V', hex("0x00000000"));
print OUTFILE $a;
# UNDEFINED VECTOR
$a = pack('V', hex("0x00000000"));
print OUTFILE $a;
# SW VECTOR
$a = pack('V', hex("0x00000000"));
print OUTFILE $a;
# PREFETCH VECTOR
$a = pack('V', hex("0x00000000"));
print OUTFILE $a;
# DATA ABORT VECTOR
$a = pack('V', hex("0x00000000"));
print OUTFILE $a;
# unused VECTOR
$a = pack('V', hex("0x00000000"));
print OUTFILE $a;
# IRQ
$a = pack('V', hex("0x00000000"));
print OUTFILE $a;
# FIQ 
$a = pack('V', hex("0x00000000"));
print OUTFILE $a;

$HDR_20 = 0xAA995566;
$HDR_24 = 0x584C4E58;
$HDR_28 = 0x00000000;
$HDR_2C = 0x544F4F42;
$HDR_30 = 0x000008A0;
$HDR_34 = 0x00000000;
$HDR_38 = 0x00000000;

$HDR_3C = 0xDEADBEEF;
if ($hdr_type eq "nor") {
 $HDR_3C = 0xE20008A0;
} 
if ($hdr_type eq "qspi") {
 $HDR_3C = 0xFC0008A0;
}

$HDR_40 = 0x00000000;
$HDR_44 = 0x00000001;
$HDR_CSUM = (~($HDR_20+$HDR_24+$HDR_28+$HDR_2C+$HDR_30+$HDR_34+$HDR_38+$HDR_3C+$HDR_40+$HDR_44));
$HDR_CSUM = $HDR_CSUM & 0xFFFFFFFF;

print OUTFILE pack('V', $HDR_20);
print OUTFILE pack('V', $HDR_24);
print OUTFILE pack('V', $HDR_28);
print OUTFILE pack('V', $HDR_2C);
print OUTFILE pack('V', $HDR_30);
print OUTFILE pack('V', $HDR_34);
print OUTFILE pack('V', $HDR_38);
print OUTFILE pack('V', $HDR_3C);
print OUTFILE pack('V', $HDR_40);
print OUTFILE pack('V', $HDR_44);
print OUTFILE pack('V', $HDR_CSUM);

for ($i = 0; $i < 21; $i++) {
    print OUTFILE pack('V', 0);
}
# A0 
# 256 reg init pairs
for ($i = 0; $i < 256; $i++) {
    print OUTFILE pack('V', 0xFFFFFFFF);
    print OUTFILE pack('V', 0);
}

close($OUTFILE);

`cat $input_bin >> $output_bin`;

