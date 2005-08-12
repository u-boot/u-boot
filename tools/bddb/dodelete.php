<?php // php pages made with phpMyBuilder <http://kyber.dk/phpMyBuilder> ?>
<?php
	// (C) Copyright 2001
	// Murray Jensen <Murray.Jensen@csiro.au>
	// CSIRO Manufacturing Science and Technology, Preston Lab

	// dodelete page (hymod_bddb / boards)

	require("defs.php");

	pg_head("$bddb_label - Delete Board Results");

	if (!isset($_REQUEST['serno']))
		die("the board serial number was not specified");
	$serno=intval($_REQUEST['serno']);

	mysql_query("delete from boards where serno=$serno");

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
		echo "\t\t\tThe board with serial number <b>$serno</b> was"
			. " successfully deleted\n";
		mysql_query("delete from log where serno=$serno");
		if (mysql_errno()) {
			$errstr = mysql_error();
			echo "\t\t\t<font size=+4>\n";
			echo "\t\t\t\t<p>\n";
			echo "\t\t\t\t\tBut the following error occurred " .
				"when deleting the log entries:\n";
			echo "\t\t\t\t</p>\n";
			echo "\t\t\t\t<center>\n";
			printf("\t\t\t\t\t<b>%s</b>\n", $errstr);
			echo "\t\t\t\t</center>\n";
			echo "\t\t\t</font>\n";
		}
		echo "\t\t</p>\n";
		echo "\t</font>\n";
	}
?>
<p>
<table width="100%">
<tr>
  <td align=center>
    <a href="browse.php">Back to Browse</a>
  </td>
  <td align=center>
    <a href="index.php">Back to Start</a>
  </td>
</tr>
</table>
<?php
	pg_foot();
?>
