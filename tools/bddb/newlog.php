<?php // php pages made with phpMyBuilder <http://kyber.dk/phpMyBuilder> ?>
<?php
	// (C) Copyright 2001
	// Murray Jensen <Murray.Jensen@csiro.au>
	// CSIRO Manufacturing Science and Technology, Preston Lab

	// edit page (hymod_bddb / boards)

	require("defs.php");

	pg_head("$bddb_label - New Log Entry");

	if (!isset($_REQUEST['serno']) || $_REQUEST['serno'] == '')
		die("serial number not specified or invalid!");
	$serno=intval($_REQUEST['serno']);

	if (isset($_REQUEST['logno'])) {
		$logno=$_REQUEST['logno'];
		die("log number must not be specified when adding! ($logno)");
	}
?>
<form action=donewlog.php method=POST>
<p></p>
<?php
	echo "<input type=hidden name=serno value=$serno>\n";

	begin_table(3);

	// date date
	print_field("date", array('date' => date("Y-m-d")));

	// who char(20)
	print_field("who", array());

	// details text
	print_field_multiline("details", array(), 60, 10, 'text_filter');

	end_table();
?>
<p></p>
<table width="100%">
<tr>
  <td align=center>
    <input type=submit value="Add Log Entry">
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
