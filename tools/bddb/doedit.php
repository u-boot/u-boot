<?php // php pages made with phpMyBuilder <http://kyber.dk/phpMyBuilder> ?>
<?php
	// (C) Copyright 2001
	// Murray Jensen <Murray.Jensen@cmst.csiro.au>
	// CSIRO Manufacturing Science and Technology, Preston Lab

	// doedit page (hymod_bddb / boards)

	require("defs.php");

	pg_head("$bddb_label - Edit Board Results");

	if ($serno == 0)
		die("the board serial number was not specified");

	$query="update boards set";

	if (isset($ethaddr)) {
		if (!eth_addr_is_valid($ethaddr))
			die("ethaddr is invalid ('$ethaddr')");
		$query.=" ethaddr='$ethaddr',";
	}

	if (isset($date)) {
		list($y, $m, $d) = split("-", $date);
		if (!checkdate($m, $d, $y) || $y < 1999)
			die("date is invalid (input '$date', " .
				"yyyy-mm-dd '$y-$m-$d')");
		$query.=" date='$date'";
	}

	if (isset($batch)) {
		if (strlen($batch) > 32)
			die("batch field too long (>32)");
		$query.=", batch='$batch'";
	}

	if (isset($type)) {
		if (!in_array($type, $type_vals))
			die("Invalid type ($type) specified");
		$query.=", type='$type'";
	}

	if (isset($rev)) {
		if (($rev = intval($rev)) <= 0 || $rev > 255)
			die("Revision number is invalid ($rev)");
		$query.=sprintf(", rev=%d", $rev);
	}

	if (isset($location)) {
		if (strlen($location) > 64)
			die("location field too long (>64)");
		$query.=", location='$location'";
	}

	if (isset($comments))
		$query.=", comments='" . rawurlencode($comments) . "'";

	$query.=gather_enum_multi_query("sdram", 4);

	$query.=gather_enum_multi_query("flash", 4);

	$query.=gather_enum_multi_query("zbt", 16);

	$query.=gather_enum_multi_query("xlxtyp", 4);
	$nxlx = count_enum_multi("xlxtyp", 4);

	$query.=gather_enum_multi_query("xlxspd", 4);
	if (count_enum_multi("xlxspd", 4) != $nxlx)
		die("number of xilinx speeds not same as number of types");

	$query.=gather_enum_multi_query("xlxtmp", 4);
	if (count_enum_multi("xlxtmp", 4) != $nxlx)
		die("number of xilinx temps. not same as number of types");

	$query.=gather_enum_multi_query("xlxgrd", 4);
	if (count_enum_multi("xlxgrd", 4) != $nxlx)
		die("number of xilinx grades not same as number of types");

	if (isset($cputyp)) {
		$query.=", cputyp='$cputyp'";
		if ($cpuspd == '')
			die("must specify cpu speed if cpu type is defined");
		$query.=", cpuspd='$cpuspd'";
		if ($cpmspd == '')
			die("must specify cpm speed if cpu type is defined");
		$query.=", cpmspd='$cpmspd'";
		if ($busspd == '')
			die("must specify bus speed if cpu type is defined");
		$query.=", busspd='$busspd'";
	}
	else {
		if (isset($cpuspd))
			die("can't specify cpu speed if there is no cpu");
		if (isset($cpmspd))
			die("can't specify cpm speed if there is no cpu");
		if (isset($busspd))
			die("can't specify bus speed if there is no cpu");
	}

	if (isset($hschin)) {
		if (($hschin = intval($hschin)) < 0 || $hschin > 4)
			die("Invalid number of hs input chans ($hschin)");
	}
	else
		$hschin = 0;
	if (isset($hschout)) {
		if (($hschout = intval($hschout)) < 0 || $hschout > 4)
			die("Invalid number of hs output chans ($hschout)");
	}
	else
		$hschout = 0;
	if (isset($hstype))
		$query.=", hstype='$hstype'";
	else {
		if ($hschin != 0)
			die("number of high-speed input channels must be zero"
				. " if high-speed chip is not present");
		if ($hschout != 0)
			die("number of high-speed output channels must be zero"
				. " if high-speed chip is not present");
	}
	$query.=", hschin='$hschin'";
	$query.=", hschout='$hschout'";

	$query.=" where serno=$serno";

	mysql_query($query);
	if(mysql_errno()) {
		$errstr = mysql_error();
		echo "\t<font size=+4>\n";
		echo "\t\t<p>\n";
		echo "\t\t\tThe following error was encountered:\n";
		echo "\t\t</p>\n";
		echo "\t\t<center>\n";
		printf("\t\t\t<b>%s</b>\n", $errstr);
		echo "\t\t</center>\n";
		echo "\t</font>\n";
	}
	else {
		$sernos = array($serno);
		$nsernos = 1;

		write_eeprom_cfg_file();

		echo "\t<font size=+2>\n";
		echo "\t\t<p>\n";
		echo "\t\t\tThe board with serial number <b>$serno</b> was"
			. " successfully updated";
		if ($numerrs > 0) {
			$errstr = $cfgerrs[0];
			echo "<br>\n\t\t\t";
			echo "(but the cfg file update failed: $errstr)";
		}
		echo "\n";
		echo "\t\t</p>\n";
		echo "\t</font>\n";
	}

?>
<p>
<table align=center width="100%">
<tr>
  <td align=center><a href="browse.php">Back to Browse</a></td>
  <td align=center><a href="index.php">Back to Start</a></td>
</tr>
</table>
<?php
	pg_foot();
?>
