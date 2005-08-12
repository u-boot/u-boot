<?php // php pages made with phpMyBuilder <http://kyber.dk/phpMyBuilder> ?>
<?php
	// (C) Copyright 2001
	// Murray Jensen <Murray.Jensen@csiro.au>
	// CSIRO Manufacturing Science and Technology, Preston Lab

	// list page (hymod_bddb / boards)

	require("defs.php");

	$serno=isset($_REQUEST['serno'])?$_REQUEST['serno']:'';

	$verbose=isset($_REQUEST['verbose'])?intval($_REQUEST['verbose']):0;

	pg_head("$bddb_label - Browse database" . ($verbose?" (verbose)":""));
?>
<p></p>
<?php
	$limit=isset($_REQUEST['limit'])?abs(intval($_REQUEST['limit'])):20;
	$offset=isset($_REQUEST['offset'])?abs(intval($_REQUEST['offset'])):0;

	if ($serno == '') {

		$lr=mysql_query("select count(*) as n from boards");
		$lrow=mysql_fetch_array($lr);

		if($lrow['n']>$limit){
			$preoffset=max(0,$offset-$limit);
			$postoffset=$offset+$limit;
			echo "<table width=\"100%\">\n<tr>\n";
			printf("<td align=left><%sa href=\"%s?submit=Browse&offset=%d&verbose=%d\"><img border=0 alt=\"&lt;\" src=\"/icons/left.gif\"></a></td>\n", $offset>0?"":"no", $PHP_SELF, $preoffset, $verbose);
			printf("<td align=right><%sa href=\"%s?submit=Browse&offset=%d&verbose=%d\"><img border=0 alt=\"&gt;\" src=\"/icons/right.gif\"></a></td>\n", $postoffset<$lrow['n']?"":"no", $PHP_SELF, $postoffset, $offset);
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
	$query = "select * from boards";
	if ($serno != '') {
		$pre = " where ";
		foreach (preg_split("/[\s,]+/", $serno) as $s) {
			if (preg_match('/^[0-9]+$/',$s))
				$query .= $pre . "serno=" . $s;
			else if (preg_match('/^([0-9]+)-([0-9]+)$/',$s,$m)) {
				$m1 = intval($m[1]); $m2 = intval($m[2]);
				if ($m2 <= $m1)
					die("bad serial number range ($s)");
				$query .= $pre . "(serno>=$m[1] and serno<=$m[2])";
			}
			else
				die("illegal serial number ($s)");
			$pre = " or ";
		}
	}
	$query .= " order by serno";
	if ($serno == '')
		$query .= " limit $offset,$limit";

	$r = mysql_query($query);

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
	printf("<a href=\"%s?submit=Browse&offset=%d&verbose=%d%s\">%s Listing</a>\n", $PHP_SELF, $offset, $verbose?0:1, $serno!=''?"&serno=$serno":'', $verbose?"Terse":"Verbose");
  ?></td>
  <td align=center><a href="index.php">Back to Start</a></td>
</tr>
</table>
<?php
	pg_foot();
?>
