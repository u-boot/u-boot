<?php // php pages made with phpMyBuilder <http://kyber.dk/phpMyBuilder> ?>
<?php
	// (C) Copyright 2001
	// Murray Jensen <Murray.Jensen@cmst.csiro.au>
	// CSIRO Manufacturing Science and Technology, Preston Lab

	// list page (hymod_bddb / boards)

	require("defs.php");

	if (!isset($verbose))
		$verbose = 0;

	if (!isset($serno))
		$serno = 0;

	pg_head("$bddb_label - Browse database" . ($verbose?" (verbose)":""));
?>
<p></p>
<?php
	if ($serno == 0) {
		$limit=abs(isset($limit)?$limit:20);
		$offset=abs(isset($offset)?$offset:0);
		$lr=mysql_query("select count(*) as n from boards");
		$lrow=mysql_fetch_array($lr);
		if($lrow['n']>$limit){
			$preoffset=max(0,$offset-$limit);
			$postoffset=$offset+$limit;
			echo "<table width=\"100%\">\n<tr align=center>\n";
			printf("<td><%sa href=\"%s?offset=%d\"><img border=0 alt=\"&lt;\" src=\"/icons/left.gif\"></a></td>\n", $offset>0?"":"no", $PHP_SELF, $preoffset);
			printf("<td><%sa href=\"%s?offset=%d\"><img border=0 alt=\"&gt;\" src=\"/icons/right.gif\"></a></td>\n", $postoffset<$lrow['n']?"":"no", $PHP_SELF, $postoffset);
			echo "</tr>\n</table>\n";
		}
		mysql_free_result($lr);
	}
?>
<table align=center border=1 cellpadding=10>
<tr>
<th></th>
<th>serno / edit</th>
<th>ethaddr</th>
<th>date</th>
<th>batch</th>
<th>type</th>
<th>rev</th>
<th>location</th>
<?php
	if ($verbose) {
		echo "<th>comments</th>\n";
		echo "<th>sdram</th>\n";
		echo "<th>flash</th>\n";
		echo "<th>zbt</th>\n";
		echo "<th>xlxtyp</th>\n";
		echo "<th>xlxspd</th>\n";
		echo "<th>xlxtmp</th>\n";
		echo "<th>xlxgrd</th>\n";
		echo "<th>cputyp</th>\n";
		echo "<th>cpuspd</th>\n";
		echo "<th>cpmspd</th>\n";
		echo "<th>busspd</th>\n";
		echo "<th>hstype</th>\n";
		echo "<th>hschin</th>\n";
		echo "<th>hschout</th>\n";
	}
?>
</tr>
<?php
	if ($serno == 0)
		$r=mysql_query("select * from boards order by serno limit $offset,$limit");
	else
		$r=mysql_query("select * from boards where serno=$serno");

	function print_cell($str) {
		if ($str == '')
			$str = '&nbsp;';
		echo "\t<td>$str</td>\n";
	}

	while($row=mysql_fetch_array($r)){
		foreach ($columns as $key) {
			if (!key_in_array($key, $row))
				$row[$key] = '';
		}

		echo "<tr>\n";
		print_cell("<a href=\"brlog.php?serno=$row[serno]\">Log</a>");
		print_cell("<a href=\"edit.php?serno=$row[serno]\">$row[serno]</a>");
		print_cell($row['ethaddr']);
		print_cell($row['date']);
		print_cell($row['batch']);
		print_cell($row['type']);
		print_cell($row['rev']);
		print_cell($row['location']);
		if ($verbose) {
			print_cell("<pre>\n" . urldecode($row['comments']) .
				"\n\t</pre>");
			print_cell(gather_enum_multi_print("sdram", 4, $row));
			print_cell(gather_enum_multi_print("flash", 4, $row));
			print_cell(gather_enum_multi_print("zbt", 16, $row));
			print_cell(gather_enum_multi_print("xlxtyp", 4, $row));
			print_cell(gather_enum_multi_print("xlxspd", 4, $row));
			print_cell(gather_enum_multi_print("xlxtmp", 4, $row));
			print_cell(gather_enum_multi_print("xlxgrd", 4, $row));
			print_cell($row['cputyp']);
			print_cell($row['cpuspd']);
			print_cell($row['cpmspd']);
			print_cell($row['busspd']);
			print_cell($row['hstype']);
			print_cell($row['hschin']);
			print_cell($row['hschout']);
		}
		echo "</tr>\n";
	}
?>
</table>
<p></p>
<table width="100%">
<tr>
  <td align=center><?php
	if ($verbose)
		echo "<a href=\"browse.php?verbose=0\">Terse Listing</a>";
	else
		echo "<a href=\"browse.php?verbose=1\">Verbose Listing</a>";
  ?></td>
  <td align=center><a href="index.php">Back to Start</a></td>
</tr>
</table>
<?php
	pg_foot();
?>
