#!/bin/awk
BEGIN { print "unsigned char bootscript[] = { \n"}
{ for (i = 2; i <= NF ; i++ ) printf "0x"$i","
  print ""
}
END { print "\n};\n" }
