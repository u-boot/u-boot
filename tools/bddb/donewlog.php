<?php // php pages made with phpMyBuilder <http://kyber.dk/phpMyBuilder> ?>
<?php
	// (C) Copyright 2001
	// Murray Jensen <Murray.Jensen@csiro.au>
	// CSIRO Manufacturing Science and Technology, Preston Lab

	// doedit page (hymod_bddb / boards)

	require("defs.php");

	pg_head("$bddb_label - Add Log Entry Results");

	if (!isset($_REQUEST['serno']) || $_REQUEST['serno'] == '')
		die("serial number not specified!");
	$serno=intval($_REQUEST['serno']);

	if (isset($_REQUEST['logno'])) {
		$logno=$_REQUEST['logno'];
		die("log number must not be set ($logno) when Creating!");
	}

	$query="update log set serno=$serno";

	list($y, $m, $d) = split("-", $date);
	if (!checkdate($m, $d, $y) || $y < 1999)
		die("date is invalid (input '$date', yyyy-mm-dd '$y-$m-$d')");
	$query.=", date='$date'";

	if (isset($_REQUEST['who'])) {
		$who=$_REQUEST['who'];
		$query.=", who='" . $who . "'";
	}

	if (isset($_REQUEST['details'])) {
		$details=$_REQUEST['details'];
		$query.=", details='" . rawurlencode($details) . "'";
	}

	// echo "final query = '$query'<br>\n";

	$sqlerr = '';

	mysql_query("insert into log (logno) values (null)");
	if (mysql_errno())
		$sqlerr = mysql_error();
	else {
		$logno = mysql_insert_id();
		if (!$logno)
			$sqlerr = "couldn't allocate new serial number";
		else {
			mysql_query($query . " where logno=$logno");
			if (mysql_errno())
				$sqlerr = mysql_error();
		}
	}

	if ($sqlerr == '') {
		echo "<font size=+2>\n";
		echo "\t<p>\n";
		echo "\t\tA log entry with log number '$logno' was " .
			"added to the board with serial number '$serno'\n";
		echo "\t</p>\n";
		echo "</font>\n";
	}
	else {
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
<p></p>
<table width="100%">
<tr>
  <td align=center><a href="brlog.php?serno=<?php echo "$serno"; ?>">Go to Browse</a></td>
  <td align=center><a href="index.php">Back to Start</a></td>
</tr>
</table>
<?php
	pg_foot();
?>
