<?php // php pages made with phpMyBuilder <http://kyber.dk/phpMyBuilder> ?>
<?php
	// (C) Copyright 2001
	// Murray Jensen <Murray.Jensen@csiro.au>
	// CSIRO Manufacturing Science and Technology, Preston Lab

	// doedit page (hymod_bddb / boards)

	require("defs.php");

	pg_head("$bddb_label - Board Registration Results");

	if (isset($_REQUEST['serno'])) {
		$serno=$_REQUEST['serno'];
		die("serial number must not be set ($serno) when Creating!");
	}

	$query="update boards set";

	list($y, $m, $d) = split("-", $date);
	if (!checkdate($m, $d, $y) || $y < 1999)
		die("date is invalid (input '$date', yyyy-mm-dd '$y-$m-$d')");
	$query.=" date='$date'";

	if ($batch != '') {
		if (strlen($batch) > 32)
			die("batch field too long (>32)");
		$query.=", batch='$batch'";
	}

	if (!in_array($type, $type_vals))
		die("Invalid type ($type) specified");
	$query.=", type='$type'";

	if (($rev = intval($rev)) <= 0 || $rev > 255)
		die("Revision number is invalid ($rev)");
	$query.=sprintf(", rev=%d", $rev);

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

	if ($cputyp == '') {
		if ($cpuspd != '')
			die("can't specify cpu speed if there is no cpu");
		if ($cpmspd != '')
			die("can't specify cpm speed if there is no cpu");
		if ($busspd != '')
			die("can't specify bus speed if there is no cpu");
	}
	else {
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

	if (($hschin = intval($hschin)) < 0 || $hschin > 4)
		die("Invalid number of hs input chans ($hschin)");
	if (($hschout = intval($hschout)) < 0 || $hschout > 4)
		die("Invalid number of hs output chans ($hschout)");
	if ($hstype == '') {
		if ($hschin != 0)
			die("number of high-speed input channels must be zero"
				. " if high-speed chip is not present");
		if ($hschout != 0)
			die("number of high-speed output channels must be zero"
				. " if high-speed chip is not present");
	}
	else
		$query.=", hstype='$hstype'";
	$query.=", hschin='$hschin'";
	$query.=", hschout='$hschout'";

	// echo "final query = '$query'<br>\n";

	$quant = intval($quant);
	if ($quant <= 0) $quant = 1;

	$sernos = array();
	if ($geneths)
		$ethaddrs = array();

	$sqlerr = '';

	while ($quant-- > 0) {

		mysql_query("insert into boards (serno) values (null)");
		if (mysql_errno()) {
			$sqlerr = mysql_error();
			break;
		}

		$serno = mysql_insert_id();
		if (!$serno) {
			$sqlerr = "couldn't allocate new serial number";
			break;
		}

		mysql_query($query . " where serno=$serno");
		if (mysql_errno()) {
			$sqlerr = mysql_error();
			break;
		}

		array_push($sernos, $serno);

		if ($geneths) {

			$ethaddr = gen_eth_addr($serno);

			mysql_query("update boards set ethaddr='$ethaddr'" .
			    " where serno=$serno");
			if (mysql_errno()) {
				$sqlerr = mysql_error();

				array_push($ethaddrs,
					"<font color=#ff0000><b>" .
					"db save fail" .
					"</b></font>");
				break;
			}

			array_push($ethaddrs, $ethaddr);
		}
	}

	$nsernos = count($sernos);

	if ($nsernos > 0) {

		write_eeprom_cfg_file();

		echo "<font size=+2>\n";
		echo "\t<p>\n";
		echo "\t\tThe following board serial numbers were"
			. " successfully allocated";
		if ($numerrs > 0)
			echo " (but with $numerrs cfg file error" .
				($numerrs > 1 ? "s" : "") . ")";
		echo ":\n";
		echo "\t</p>\n";

		echo "</font>\n";

		echo "<table align=center width=\"100%\">\n";
		echo "<tr>\n";
		echo "\t<th>Serial Number</th>\n";
		if ($numerrs > 0)
			echo "\t<th>Cfg File Errs</th>\n";
		if ($geneths)
			echo "\t<th>Ethernet Address</th>\n";
		echo "</tr>\n";

		for ($i = 0; $i < $nsernos; $i++) {

			$serno = sprintf("%010d", $sernos[$i]);

			echo "<tr>\n";

			echo "\t<td align=center><font size=+2>" .
				"<b>$serno</b></font></td>\n";

			if ($numerrs > 0) {
				if (($errstr = $cfgerrs[$i]) == '')
					$errstr = '&nbsp;';
				echo "\t<td align=center>" .
					"<font size=+2 color=#ff0000><b>" .
					$errstr .
					"</b></font></td>\n";
			}

			if ($geneths) {
				echo "\t<td align=center>" .
					"<font size=+2 color=#00ff00><b>" .
					$ethaddrs[$i] .
					"</b></font></td>\n";
			}

			echo "</tr>\n";
		}

		echo "</table>\n";
	}

	if ($sqlerr != '') {
		echo "\t<font size=+4>\n";
		echo "\t\t<p>\n";
		echo "\t\t\tThe following SQL error was encountered:\n";
		echo "\t\t</p>\n";
		echo "\t\t<center>\n";
		printf("\t\t\t<b>%s</b>\n", $sqlerr);
		echo "\t\t</center>\n";
		echo "\t</font>\n";
	}

?>
<p>
<table align=center width="100%">
<tr>
  <td align=center><a href="browse.php">Go to Browse</a></td>
  <td align=center><a href="index.php">Back to Start</a></td>
</tr>
</table>
<?php
	pg_foot();
?>
