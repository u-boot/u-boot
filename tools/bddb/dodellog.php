<?php // php pages made with phpMyBuilder <http://kyber.dk/phpMyBuilder> ?>
<?php
	// (C) Copyright 2001
	// Murray Jensen <Murray.Jensen@csiro.au>
	// CSIRO Manufacturing Science and Technology, Preston Lab

	// dodelete page (hymod_bddb / boards)

	require("defs.php");

	pg_head("$bddb_label - Delete Log Entry Results");

	if (!isset($_REQUEST['serno']))
		die("the board serial number was not specified");
	$serno=intval($_REQUEST['serno']);

	if (!isset($_REQUEST['logno']) || $_REQUEST['logno'] == 0)
		die("the log entry number not specified!");
	$logno=$_REQUEST['logno'];

	mysql_query("delete from log where serno=$serno and logno=$logno");

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
		echo "\t\t\tThe log entry with log number <b>$logno</b>\n";
		echo "\t\t\tand serial number <b>$serno</b> ";
		echo "was successfully deleted\n";
		echo "\t\t</p>\n";
		echo "\t</font>\n";
	}
?>
<p>
<table width="100%">
<tr>
  <td align=center>
    <a href="brlog.php?serno=<?php echo "$serno"; ?>">Back to Log</a>
  </td>
  <td align=center>
    <a href="index.php">Back to Start</a>
  </td>
</tr>
</table>
<?php
	pg_foot();
?>
