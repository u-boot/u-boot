<?php // php pages made with phpMyBuilder <http://kyber.dk/phpMyBuilder> ?>
<?php
	// (C) Copyright 2001
	// Murray Jensen <Murray.Jensen@csiro.au>
	// CSIRO Manufacturing Science and Technology, Preston Lab

	// doedit page (hymod_bddb / boards)

	require("defs.php");

	pg_head("$bddb_label - Edit Log Entry Results");

	if (!isset($_REQUEST['serno']) || $_REQUEST['serno'] == '')
		die("the board serial number was not specified");
	$serno=intval($_REQUEST['serno']);

	if (!isset($_REQUEST['logno']) || $_REQUEST['logno'] == '')
		die("log number not specified!");
	$logno=intval($_REQUEST['logno']);

	$query="update log set";

	if (isset($_REQUEST['date'])) {
		$date=$_REQUEST['date'];
		list($y, $m, $d) = split("-", $date);
		if (!checkdate($m, $d, $y) || $y < 1999)
			die("date is invalid (input '$date', " .
				"yyyy-mm-dd '$y-$m-$d')");
		$query.=" date='$date'";
	}

	if (isset($_REQUEST['who'])) {
		$who=$_REQUEST['who'];
		$query.=", who='" . $who . "'";
	}

	if (isset($_REQUEST['details'])) {
		$details=$_REQUEST['details'];
		$query.=", details='" . rawurlencode($details) . "'";
	}

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
