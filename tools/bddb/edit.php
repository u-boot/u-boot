<?php // php pages made with phpMyBuilder <http://kyber.dk/phpMyBuilder> ?>
<?php
	// (C) Copyright 2001
	// Murray Jensen <Murray.Jensen@csiro.au>
	// CSIRO Manufacturing Science and Technology, Preston Lab

	// edit page (hymod_bddb / boards)

	require("defs.php");

	pg_head("$bddb_label - Edit Board Registration");

	if ($serno == 0)
		die("serial number not specified or invalid!");

	$pserno = sprintf("%010d", $serno);

	echo "<center><b><font size=+2>";
	echo "Board Serial Number: $pserno";
	echo "</font></b></center>\n";

?>
<p>
<form action=doedit.php method=POST>
<?php
	echo "<input type=hidden name=serno value=$serno>\n";

	$r=mysql_query("select * from boards where serno=$serno");
	$row=mysql_fetch_array($r);
	if(!$row) die("no record of serial number '$serno' in database");

	begin_table(5);

	// ethaddr char(17)
	print_field("ethaddr", $row, 17);

	// date date
	print_field("date", $row);

	// batch char(32)
	print_field("batch", $row, 32);

	// type enum('IO','CLP','DSP','INPUT','ALT-INPUT','DISPLAY')
	print_enum("type", $row, $type_vals);

	// rev tinyint(3) unsigned zerofill
	print_field("rev", $row, 3, 'rev_filter');

	// location char(64)
	print_field("location", $row, 64);

	// comments text
	print_field_multiline("comments", $row, 60, 10, 'text_filter');

	// sdram[0-3] enum('32M','64M','128M','256M')
	print_enum_multi("sdram", $row, $sdram_vals, 4, array());

	// flash[0-3] enum('4M','8M','16M','32M','64M')
	print_enum_multi("flash", $row, $flash_vals, 4, array());

	// zbt[0-f] enum('512K','1M','2M','4M')
	print_enum_multi("zbt", $row, $zbt_vals, 16, array());

	// xlxtyp[0-3] enum('XCV300E','XCV400E','XCV600E')
	print_enum_multi("xlxtyp", $row, $xlxtyp_vals, 4, array(), 1);

	// xlxspd[0-3] enum('6','7','8')
	print_enum_multi("xlxspd", $row, $xlxspd_vals, 4, array(), 1);

	// xlxtmp[0-3] enum('COM','IND')
	print_enum_multi("xlxtmp", $row, $xlxtmp_vals, 4, array(), 1);

	// xlxgrd[0-3] enum('NORMAL','ENGSAMP')
	print_enum_multi("xlxgrd", $row, $xlxgrd_vals, 4, array(), 1);

	// cputyp enum('MPC8260(HIP3)','MPC8260A(HIP4)','MPC8280(HIP7)')
	print_enum("cputyp", $row, $cputyp_vals);

	// cpuspd enum('33MHZ','66MHZ','100MHZ','133MHZ','166MHZ','200MHZ','233MHZ','266MHZ')
	print_enum_select("cpuspd", $row, $clk_vals);

	// cpmspd enum('33MHZ','66MHZ','100MHZ','133MHZ','166MHZ','200MHZ','233MHZ','266MHZ')
	print_enum_select("cpmspd", $row, $clk_vals);

	// busspd enum('33MHZ','66MHZ','100MHZ','133MHZ','166MHZ','200MHZ','233MHZ','266MHZ')
	print_enum_select("busspd", $row, $clk_vals);

	// hstype enum('AMCC-S2064A')
	print_enum("hstype", $row, $hstype_vals);

	// hschin enum('0','1','2','3','4')
	print_enum("hschin", $row, $hschin_vals);

	// hschout enum('0','1','2','3','4')
	print_enum("hschout", $row, $hschout_vals);

	end_table();

	echo "<p>\n";
	echo "<center><b>";
	echo "<font color=#ff0000>WARNING: NO UNDO ON DELETE!</font>";
	echo "<br></br>\n";
	echo "<tt>[ <a href=\"dodelete.php?serno=$serno\">delete</a> ]</tt>";
	echo "</b></center>\n";
	echo "</p>\n";
?>
<p>
<table align=center width="100%">
<tr>
  <td align=center>
    <input type=submit value=Edit>
  </td>
  <td>
    &nbsp;
  </td>
  <td align=center>
    <input type=reset value=Reset>
  </td>
  <td>
    &nbsp;
  </td>
  <td align=center>
    <a href="index.php">Back to Start</a>
  </td>
</tr>
</table>
</p>
</form>
<?php
	pg_foot();
?>
