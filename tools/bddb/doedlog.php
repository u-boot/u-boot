<?php // php pages made with phpMyBuilder <http://kyber.dk/phpMyBuilder> ?>
<?php
	// (C) Copyright 2001
	// Murray Jensen <Murray.Jensen@cmst.csiro.au>
	// CSIRO Manufacturing Science and Technology, Preston Lab

	// doedit page (hymod_bddb / boards)

	require("defs.php");

	pg_head("$bddb_label - Edit Log Entry Results");

	if ($serno == 0)
		die("the board serial number was not specified");

	if (!isset($logno) || $logno == 0)
		die("log number not specified!");

	$query="update log set";

	if (isset($date)) {
		list($y, $m, $d) = split("-", $date);
		if (!checkdate($m, $d, $y) || $y < 1999)
			die("date is invalid (input '$date', " .
				"yyyy-mm-dd '$y-$m-$d')");
		$query.=" date='$date'";
	}

	if (isset($who))
		$query.=", who='" . $who . "'";

	if (isset($details))
		$query.=", details='" . rawurlencode($details) . "'";

	$query.=" where serno=$serno and logno=$logno";

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
		echo "\t<font size=+2>\n";
		echo "\t\t<p>\n";
		echo "\t\t\tThe log entry with log number <b>$logno</b> and\n";
		echo "\t\t\tserial number <b>$serno</b> ";
		echo "was successfully updated\n";
		echo "\t\t</p>\n";
		echo "\t</font>\n";
	}

?>
<p>
<table align=center width="100%">
<tr>
  <td align=center><a href="brlog.php?serno=<?php echo "$serno"; ?>">Back to Log</a></td>
  <td align=center><a href="index.php">Back to Start</a></td>
</tr>
</table>
<?php
	pg_foot();
?>
