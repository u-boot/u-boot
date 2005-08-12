<?php // php pages made with phpMyBuilder <http://kyber.dk/phpMyBuilder> ?>
<?php
	// (C) Copyright 2001
	// Murray Jensen <Murray.Jensen@csiro.au>
	// CSIRO Manufacturing Science and Technology, Preston Lab

	// edit page (hymod_bddb / boards)

	require("defs.php");

	pg_head("$bddb_label - Edit Board Log Entry");

	if (!isset($_REQUEST['serno']) || $_REQUEST['serno'] == '')
		die("serial number not specified!");
	$serno=intval($_REQUEST['serno']);

	if (!isset($_REQUEST['logno']) || $_REQUEST['logno'] == '')
		die("log number not specified!");
	$logno=intval($_REQUEST['logno']);

	$pserno = sprintf("%010d", $serno);
	$plogno = sprintf("%010d", $logno);

	echo "<center><b><font size=+2>";
	echo "Board Serial Number: $pserno, Log Number: $plogno";
	echo "</font></b></center>\n";

?>
<p>
<form action=doedlog.php method=POST>
<?php
	echo "<input type=hidden name=serno value=$serno>\n";
	echo "<input type=hidden name=logno value=$logno>\n";

	$r=mysql_query("select * from log where serno=$serno and logno=$logno");
	$row=mysql_fetch_array($r);
	if(!$row)
		die("no record of log entry with serial number '$serno' " .
			"and log number '$logno' in database");

	begin_table(3);

	// date date
	print_field("date", $row);

	// who char(20)
	print_field("who", $row);

	// details text
	print_field_multiline("details", $row, 60, 10, 'text_filter');

	end_table();

	echo "<p>\n";
	echo "<center><b>";
	echo "<font color=#ff0000>WARNING: NO UNDO ON DELETE!</font>";
	echo "<br></br>\n";
	echo "<tt>[ <a href=\"dodellog.php?serno=$serno&logno=$logno\">delete</a> ]</tt>";
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
