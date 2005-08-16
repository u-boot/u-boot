<?php // php pages made with phpMyBuilder <http://kyber.dk/phpMyBuilder> ?>
<?php
	// (C) Copyright 2001
	// Murray Jensen <Murray.Jensen@csiro.au>
	// CSIRO Manufacturing Science and Technology, Preston Lab

	// list page (hymod_bddb / boards)

	require("defs.php");

	pg_head("$bddb_label - Browse Board Log");

	$serno=intval($serno);
	if ($serno == 0)
		die("serial number not specified or invalid!");

	function print_cell($str) {
		if ($str == '')
			$str = '&nbsp;';
		echo "\t<td>$str</td>\n";
	}
?>
<table align=center border=1 cellpadding=10>
<tr>
<th>serno / edit</th>
<th>ethaddr</th>
<th>date</th>
<th>batch</th>
<th>type</th>
<th>rev</th>
<th>location</th>
</tr>
<?php
	$r=mysql_query("select * from boards where serno=$serno");

	while($row=mysql_fetch_array($r)){
		foreach ($columns as $key) {
			if (!key_in_array($key, $row))
				$row[$key] = '';
		}

		echo "<tr>\n";
		print_cell("<a href=\"edit.php?serno=$row[serno]\">$row[serno]</a>");
		print_cell($row['ethaddr']);
		print_cell($row['date']);
		print_cell($row['batch']);
		print_cell($row['type']);
		print_cell($row['rev']);
		print_cell($row['location']);
		echo "</tr>\n";
	}

	mysql_free_result($r);
?>
</table>
<hr></hr>
<p></p>
<?php
	$limit=abs(isset($_REQUEST['limit'])?$_REQUEST['limit']:20);
	$offset=abs(isset($_REQUEST['offset'])?$_REQUEST['offset']:0);
	$lr=mysql_query("select count(*) as n from log where serno=$serno");
	$lrow=mysql_fetch_array($lr);
	if($lrow['n']>$limit){
		$preoffset=max(0,$offset-$limit);
		$postoffset=$offset+$limit;
		echo "<table width=\"100%\">\n<tr align=center>\n";
		printf("<td><%sa href=\"%s?submit=Log&serno=$serno&offset=%d\"><img border=0 alt=\"&lt;\" src=\"/icons/left.gif\"></a></td>\n", $offset>0?"":"no", $PHP_SELF, $preoffset);
		printf("<td><%sa href=\"%s?submit=Log&serno=$serno&offset=%d\"><img border=0 alt=\"&gt;\" src=\"/icons/right.gif\"></a></td>\n", $postoffset<$lrow['n']?"":"no", $PHP_SELF, $postoffset);
		echo "</tr>\n</table>\n";
	}
	mysql_free_result($lr);
?>
<table width="100%" border=1 cellpadding=10>
<tr valign=top>
<th>logno / edit</th>
<th>date</th>
<th>who</th>
<th width="70%">details</th>
</tr>
<?php
	$r=mysql_query("select * from log where serno=$serno order by logno limit $offset,$limit");

	while($row=mysql_fetch_array($r)){
		echo "<tr>\n";
		print_cell("<a href=\"edlog.php?serno=$row[serno]&logno=$row[logno]\">$row[logno]</a>");
		print_cell($row['date']);
		print_cell($row['who']);
		print_cell("<pre>" . urldecode($row['details']) . "</pre>");
		echo "</tr>\n";
	}

	mysql_free_result($r);
?>
</table>
<hr></hr>
<p></p>
<table width="100%">
<tr>
  <td align=center>
    <a href="newlog.php?serno=<?php echo "$serno"; ?>">Add to Log</a>
  </td>
  <td align=center>
    <a href="index.php">Back to Start</a>
  </td>
</tr>
</table>
<?php
	pg_foot();
?>
