<?php // php pages made with phpMyBuilder <http://kyber.dk/phpMyBuilder> ?>
<?php
	// (C) Copyright 2001
	// Murray Jensen <Murray.Jensen@csiro.au>
	// CSIRO Manufacturing Science and Technology, Preston Lab

	// edit page (hymod_bddb / boards)

	require("defs.php");

	pg_head("$bddb_label - New Board Registration");
?>
<form action=donew.php method=POST>
<p></p>
<?php
	$serno=intval($serno);
	// if a serial number was supplied, fetch the record
	// and use its contents as defaults
	if ($serno != 0) {
		$r=mysql_query("select * from boards where serno=$serno");
		$row=mysql_fetch_array($r);
		if(!$row)die("no record of serial number '$serno' in database");
	}
	else
		$row = array();

	begin_table(5);

	// date date
	print_field("date", array('date' => date("Y-m-d")));

	// batch char(32)
	print_field("batch", $row, 32);

	// type enum('IO','CLP','DSP','INPUT','ALT-INPUT','DISPLAY')
	print_enum("type", $row, $type_vals, 0);

	// rev tinyint(3) unsigned zerofill
	print_field("rev", $row, 3, 'rev_filter');

	// sdram[0-3] enum('32M','64M','128M','256M')
	print_enum_multi("sdram", $row, $sdram_vals, 4, array(2));

	// flash[0-3] enum('4M','8M','16M','32M','64M')
	print_enum_multi("flash", $row, $flash_vals, 4, array(2));

	// zbt[0-f] enum('512K','1M','2M','4M')
	print_enum_multi("zbt", $row, $zbt_vals, 16, array(2, 2));

	// xlxtyp[0-3] enum('XCV300E','XCV400E','XCV600E')
	print_enum_multi("xlxtyp", $row, $xlxtyp_vals, 4, array(1), 1);

	// xlxspd[0-3] enum('6','7','8')
	print_enum_multi("xlxspd", $row, $xlxspd_vals, 4, array(1), 1);

	// xlxtmp[0-3] enum('COM','IND')
	print_enum_multi("xlxtmp", $row, $xlxtmp_vals, 4, array(1), 1);

	// xlxgrd[0-3] enum('NORMAL','ENGSAMP')
	print_enum_multi("xlxgrd", $row, $xlxgrd_vals, 4, array(1), 1);

	// cputyp enum('MPC8260(HIP3)','MPC8260A(HIP4)','MPC8280(HIP7)')
	print_enum("cputyp", $row, $cputyp_vals, 1);

	// cpuspd enum('33MHZ','66MHZ','100MHZ','133MHZ','166MHZ','200MHZ','233MHZ','266MHZ')
	print_enum_select("cpuspd", $row, $clk_vals, 4);

	// cpmspd enum('33MHZ','66MHZ','100MHZ','133MHZ','166MHZ','200MHZ','233MHZ','266MHZ')
	print_enum_select("cpmspd", $row, $clk_vals, 4);

	// busspd enum('33MHZ','66MHZ','100MHZ','133MHZ','166MHZ','200MHZ','233MHZ','266MHZ')
	print_enum_select("busspd", $row, $clk_vals, 2);

	// hstype enum('AMCC-S2064A')
	print_enum("hstype", $row, $hstype_vals, 1);

	// hschin enum('0','1','2','3','4')
	print_enum("hschin", $row, $hschin_vals, 4);

	// hschout enum('0','1','2','3','4')
	print_enum("hschout", $row, $hschout_vals, 4);

	end_table();
?>
<p></p>
<table width="100%">
<tr>
  <td align=center colspan=3>
    Allocate
    <input type=text name=quant size=2 maxlength=2 value=" 1">
    board serial number(s)
  </td>
</tr>
<tr>
  <td align=center colspan=3>
    <input type=checkbox name=geneths checked>
    Generate Ethernet Address(es)
  </td>
</tr>
<tr>
  <td colspan=3>
    &nbsp;
  </td>
</tr>
<tr>
  <td align=center>
    <input type=submit value="Register Board">
  </td>
  <td>
    &nbsp;
  </td>
  <td align=center>
    <input type=reset value="Reset Form Contents">
  </td>
</tr>
</table>
</form>
<?php
	pg_foot();
?>
