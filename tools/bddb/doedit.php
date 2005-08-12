<?php // php pages made with phpMyBuilder <http://kyber.dk/phpMyBuilder> ?>
<?php
	// (C) Copyright 2001
	// Murray Jensen <Murray.Jensen@csiro.au>
	// CSIRO Manufacturing Science and Technology, Preston Lab

	// doedit page (hymod_bddb / boards)

	require("defs.php");

	pg_head("$bddb_label - Edit Board Results");

	if (!isset($_REQUEST['serno']) || $_REQUEST['serno'] == '')
		die("the board serial number was not specified");
	$serno=intval($_REQUEST['serno']);

	$query="update boards set";

	if (isset($_REQUEST['ethaddr'])) {
		$ethaddr=$_REQUEST['ethaddr'];
		if (!eth_addr_is_valid($ethaddr))
			die("ethaddr is invalid ('$ethaddr')");
		$query.=" ethaddr='$ethaddr',";
	}

	if (isset($_REQUEST['date'])) {
		$date=$_REQUEST['date'];
		list($y, $m, $d) = split("-", $date);
		if (!checkdate($m, $d, $y) || $y < 1999)
			die("date is invalid (input '$date', " .
				"yyyy-mm-dd '$y-$m-$d')");
		$query.=" date='$date'";
	}

	if (isset($_REQUEST['batch'])) {
		$batch=$_REQUEST['batch'];
		if (strlen($batch) > 32)
			die("batch field too long (>32)");
		$query.=", batch='$batch'";
	}

	if (isset($_REQUEST['type'])) {
		$type=$_REQUEST['type'];
		if (!in_array($type, $type_vals))
			die("Invalid type ($type) specified");
		$query.=", type='$type'";
	}

	if (isset($_REQUEST['rev'])) {
		$rev=$_REQUEST['rev'];
		if (($rev = intval($rev)) <= 0 || $rev > 255)
			die("Revision number is invalid ($rev)");
		$query.=sprintf(", rev=%d", $rev);
	}

	if (isset($_REQUEST['location'])) {
		$location=$_REQUEST['location'];
		if (strlen($location) > 64)
			die("location field too long (>64)");
		$query.=", location='$location'";
	}

	if (isset($_REQUEST['comments']))
		$comments=$_REQUEST['comments'];
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

	if (isset($_REQUEST['cputyp'])) {
		$cputyp=$_REQUEST['cputyp'];
		$query.=", cputyp='$cputyp'";
		if (!isset($_REQUEST['cpuspd']) || $_REQUEST['cpuspd'] == '')
			die("must specify cpu speed if cpu type is defined");
		$cpuspd=$_REQUEST['cpuspd'];
		$query.=", cpuspd='$cpuspd'";
		if (!isset($_REQUEST['cpmspd']) || $_REQUEST['cpmspd'] == '')
			die("must specify cpm speed if cpu type is defined");
		$cpmspd=$_REQUEST['cpmspd'];
		$query.=", cpmspd='$cpmspd'";
		if (!isset($_REQUEST['busspd']) || $_REQUEST['busspd'] == '')
			die("must specify bus speed if cpu type is defined");
		$busspd=$_REQUEST['busspd'];
		$query.=", busspd='$busspd'";
	}
	else {
		if (isset($_REQUEST['cpuspd']))
			die("can't specify cpu speed if there is no cpu");
		if (isset($_REQUEST['cpmspd']))
			die("can't specify cpm speed if there is no cpu");
		if (isset($_REQUEST['busspd']))
			die("can't specify bus speed if there is no cpu");
	}

	if (isset($_REQUEST['hschin'])) {
		$hschin=$_REQUEST['hschin'];
		if (($hschin = intval($hschin)) < 0 || $hschin > 4)
			die("Invalid number of hs input chans ($hschin)");
	}
	else
		$hschin = 0;
	if (isset($_REQUEST['hschout'])) {
		$hschout=$_REQUEST['hschout'];
		if (($hschout = intval($hschout)) < 0 || $hschout > 4)
			die("Invalid number of hs output chans ($hschout)");
	}
	else
		$hschout = 0;
	if (isset($_REQUEST['hstype'])) {
		$hstype=$_REQUEST['hstype'];
		$query.=", hstype='$hstype'";
	}
	else {
		if ($_REQUEST['hschin'] != 0)
			die("number of high-speed input channels must be zero"
				. " if high-speed chip is not present");
		if ($_REQUEST['hschout'] != 0)
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
