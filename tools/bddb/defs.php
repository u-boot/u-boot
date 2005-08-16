<?php // php pages made with phpMyBuilder <http://kyber.dk/phpMyBuilder> ?>
<?php
	// (C) Copyright 2001
	// Murray Jensen <Murray.Jensen@csiro.au>
	// CSIRO Manufacturing Science and Technology, Preston Lab

	// contains mysql user id and password - keep secret
	require("config.php");

	if (isset($_REQUEST['logout'])) {
		Header("status: 401 Unauthorized");
		Header("HTTP/1.0 401 Unauthorized");
		Header("WWW-authenticate: basic realm=\"$bddb_label\"");

		echo "<html><head><title>" .
			"Access to '$bddb_label' Denied" .
			"</title></head>\n";
		echo "<body bgcolor=#ffffff><br></br><br></br><center><h1>" .
			"You must be an Authorised User " .
			"to access the '$bddb_label'" .
			"</h1>\n</center></body></html>\n";
		exit;
	}

	// contents of the various enumerated types - if first item is
	// empty ('') then the enum is allowed to be null (ie "not null"
	// is not set on the column)

	// all column names in the database table
	$columns = array(
		'serno','ethaddr','date','batch',
		'type','rev','location','comments',
		'sdram0','sdram1','sdram2','sdram3',
		'flash0','flash1','flash2','flash3',
		'zbt0','zbt1','zbt2','zbt3','zbt4','zbt5','zbt6','zbt7',
		'zbt8','zbt9','zbta','zbtb','zbtc','zbtd','zbte','zbtf',
		'xlxtyp0','xlxtyp1','xlxtyp2','xlxtyp3',
		'xlxspd0','xlxspd1','xlxspd2','xlxspd3',
		'xlxtmp0','xlxtmp1','xlxtmp2','xlxtmp3',
		'xlxgrd0','xlxgrd1','xlxgrd2','xlxgrd3',
		'cputyp','cpuspd','cpmspd','busspd',
		'hstype','hschin','hschout'
	);

	// board type
	$type_vals = array('IO','CLP','DSP','INPUT','ALT-INPUT','DISPLAY');

	// Xilinx fpga types
	$xlxtyp_vals = array('','XCV300E','XCV400E','XCV600E','XC2V2000','XC2V3000','XC2V4000','XC2V6000','XC2VP2','XC2VP4','XC2VP7','XC2VP20','XC2VP30','XC2VP50','XC4VFX20','XC4VFX40','XC4VFX60','XC4VFX100','XC4VFX140');

	// Xilinx fpga speeds
	$xlxspd_vals = array('','6','7','8','4','5','9','10','11','12');

	// Xilinx fpga temperatures (commercial or industrial)
	$xlxtmp_vals = array('','COM','IND');

	// Xilinx fpga grades (normal or engineering sample)
	$xlxgrd_vals = array('','NORMAL','ENGSAMP');

	// CPU types
	$cputyp_vals = array('','MPC8260(HIP3)','MPC8260A(HIP4)','MPC8280(HIP7)','MPC8560');

	// CPU/BUS/CPM clock speeds 
	$clk_vals = array('','33MHZ','66MHZ','100MHZ','133MHZ','166MHZ','200MHZ','233MHZ','266MHZ','300MHZ','333MHZ','366MHZ','400MHZ','433MHZ','466MHZ','500MHZ','533MHZ','566MHZ','600MHZ','633MHZ','666MHZ','700MHZ','733MHZ','766MHZ','800MHZ','833MHZ','866MHZ','900MHZ','933MHZ','966MHZ','1000MHZ','1033MHZ','1066MHZ','1100MHZ','1133MHZ','1166MHZ','1200MHZ','1233MHZ','1266MHZ','1300MHZ','1333MHZ');

	// sdram sizes (nbits array is for eeprom config file)
	$sdram_vals = array('','32M','64M','128M','256M','512M','1G','2G','4G');
	$sdram_nbits = array(0,25,26,27,28,29,30,31,32);

	// flash sizes (nbits array is for eeprom config file)
	$flash_vals = array('','4M','8M','16M','32M','64M','128M','256M','512M','1G');
	$flash_nbits = array(0,22,23,24,25,26,27,28,29,30);

	// zbt ram sizes (nbits array is for write into eeprom config file)
	$zbt_vals = array('','512K','1M','2M','4M','8M','16M');
	$zbt_nbits = array(0,19,20,21,22,23,24);

	// high-speed serial attributes
	$hstype_vals = array('','AMCC-S2064A','Xilinx-Rockets');
	$hschin_vals = array('0','1','2','3','4','5','6','7','8','9','10','11','12','13','14','15','16');
	$hschout_vals = array('0','1','2','3','4','5','6','7','8','9','10','11','12','13','14','15','16');

	// value filters - used when outputting html
	function rev_filter($num) {
		if ($num == 0)
			return "001";
		else
			return sprintf("%03d", $num);
	}

	function text_filter($str) {
		return urldecode($str);
	}

	mt_srand(time() | getmypid());

	// set up MySQL connection
	mysql_connect("", $mysql_user, $mysql_pw) || die("cannot connect");
	mysql_select_db($mysql_db) || die("cannot select db");

	// page header
	function pg_head($title)
	{
		echo "<html>\n<head>\n";
		echo "<link rel=stylesheet href=\"bddb.css\" type=\"text/css\" title=\"style sheet\"></link>\n";
		echo "<title>$title</title>\n";
		echo "</head>\n";
		echo "<body>\n";
		echo "<center><h1>$title</h1></center>\n";
		echo "<hr></hr>\n";
	}

	// page footer
	function pg_foot()
	{
		echo "<hr></hr>\n";
		echo "<table width=\"100%\"><tr><td align=left>\n<address>" .
			"If you have any problems, email " .
			"<a href=\"mailto:Murray.Jensen@csiro.au\">" .
			"Murray Jensen" .
			"</a></address>\n" .
			"</td><td align=right>\n" .
			"<a href=\"index.php?logout=true\">logout</a>\n" .
			"</td></tr></table>\n";
		echo "<p><small><i>Made with " .
		    "<a href=\"http://kyber.dk/phpMyBuilder/\">" .
		    "Kyber phpMyBuilder</a></i></small></p>\n";
		echo "</body>\n";
		echo "</html>\n";
	}

	// some support functions

	if (!function_exists('array_search')) {

		function array_search($needle, $haystack, $strict = false) {

			if (is_array($haystack) && count($haystack)) {

				$ntype = gettype($needle);

				foreach ($haystack as $key => $value) {

					if ($value == $needle && (!$strict ||
					    gettype($value) == $ntype))
						return $key;
				}
			}

			return false;
		}
	}

	if (!function_exists('in_array')) {

		function in_array($needle, $haystack, $strict = false) {

			if (is_array($haystack) && count($haystack)) {

				$ntype = gettype($needle);

				foreach ($haystack as $key => $value) {

					if ($value == $needle && (!$strict ||
					    gettype($value) == $ntype))
						return true;
				}
			}

			return false;
		}
	}

	function key_in_array($key, $array) {
		return in_array($key, array_keys($array), true);
	}

	function enum_to_index($name, $vals) {
		$index = array_search($GLOBALS[$name], $vals);
		if ($vals[0] != '')
		        $index++;
		return $index;
	}

	// fetch a value from an array - return empty string is not present
	function get_key_value($key, $array) {
		if (key_in_array($key, $array))
			return $array[$key];
		else
			return '';
	}

	function fprintf() {
		$n = func_num_args();
		if ($n < 2)
			return FALSE;
		$a = func_get_args();
		$fp = array_shift($a);
		$x = "\$s = sprintf";
		$sep = '(';
		foreach ($a as $z) {
			$x .= "$sep'$z'";
			$sep = ',';
		}
		$x .= ');';
		eval($x);
		$l = strlen($s);
		$r = fwrite($fp, $s, $l);
		if ($r != $l)
			return FALSE;
		else
			return TRUE;
	}

	// functions to display (print) a database table and its columns

	function begin_table($ncols) {
		global $table_ncols;
		$table_ncols = $ncols;
		echo "<table align=center width=\"100%\""
			. " border=1 cellpadding=4 cols=$table_ncols>\n";
	}

	function begin_field($name, $span = 0) {
		global $table_ncols;
		echo "<tr valign=top>\n";
		echo "\t<th align=center>$name</th>\n";
		if ($span <= 0)
			$span = $table_ncols - 1;
		if ($span > 1)
			echo "\t<td colspan=$span>\n";
		else
			echo "\t<td>\n";
	}

	function cont_field($span = 1) {
		echo "\t</td>\n";
		if ($span > 1)
			echo "\t<td colspan=$span>\n";
		else
			echo "\t<td>\n";
	}

	function end_field() {
		echo "\t</td>\n";
		echo "</tr>\n";
	}

	function end_table() {
		echo "</table>\n";
	}

	function print_field($name, $array, $size = 0, $filt='') {

		begin_field($name);

		if (key_in_array($name, $array))
			$value = $array[$name];
		else
			$value = '';

		if ($filt != '')
			$value = $filt($value);

		echo "\t\t<input name=$name value=\"$value\"";
		if ($size > 0)
			echo " size=$size maxlength=$size";
		echo "></input>\n";

		end_field();
	}

	function print_field_multiline($name, $array, $cols, $rows, $filt='') {

		begin_field($name);

		if (key_in_array($name, $array))
			$value = $array[$name];
		else
			$value = '';

		if ($filt != '')
			$value = $filt($value);

		echo "\t\t<textarea name=$name " .
			"cols=$cols rows=$rows wrap=off>\n";
		echo "$value";
		echo "</textarea>\n";

		end_field();
	}

	// print a mysql ENUM as an html RADIO INPUT
	function print_enum($name, $array, $vals, $def = -1) {

		begin_field($name);

		if (key_in_array($name, $array))
			$chk = array_search($array[$name], $vals, FALSE);
		else
			$chk = $def;

		$nval = count($vals);

		for ($i = 0; $i < $nval; $i++) {

			$val = $vals[$i];
			if ($val == '')
				$pval = "none";
			else
				$pval = "$val";

			printf("\t\t<input type=radio name=$name"
				. " value=\"$val\"%s>$pval</input>\n",
				$i == $chk ? " checked" : "");
		}

		end_field();
	}

	// print a mysql ENUM as an html SELECT INPUT
	function print_enum_select($name, $array, $vals, $def = -1) {

		begin_field($name);

		echo "\t\t<select name=$name>\n";

		if (key_in_array($name, $array))
			$chk = array_search($array[$name], $vals, FALSE);
		else
			$chk = $def;

		$nval = count($vals);

		for ($i = 0; $i < $nval; $i++) {

			$val = $vals[$i];
			if ($val == '')
				$pval = "none";
			else
				$pval = "$val";

			printf("\t\t\t<option " .
				"value=\"%s\"%s>%s</option>\n",
				$val, $i == $chk ? " selected" : "", $pval);
		}

		echo "\t\t</select>\n";

		end_field();
	}

	// print a group of mysql ENUMs (e.g. name0,name1,...) as an html SELECT
	function print_enum_multi($base, $array, $vals, $cnt, $defs, $grp = 0) {

		global $table_ncols;

		if ($grp <= 0)
			$grp = $cnt;
		$ncell = $cnt / $grp;
		$span = ($table_ncols - 1) / $ncell;

		begin_field($base, $span);

		$nval = count($vals);

		for ($i = 0; $i < $cnt; $i++) {

			if ($i > 0 && ($i % $grp) == 0)
				cont_field($span);

			$name = sprintf("%s%x", $base, $i);

			echo "\t\t<select name=$name>\n";

			if (key_in_array($name, $array))
				$ai = array_search($array[$name], $vals, FALSE);
			else {
				if (key_in_array($i, $defs))
					$ai = $defs[$i];
				else
					$ai = 0;
			}

			for ($j = 0; $j < $nval; $j++) {

				$val = $vals[$j];
				if ($val == '')
					$pval = "&nbsp;";
				else
					$pval = "$val";

				printf("\t\t\t<option " .
					"value=\"%s\"%s>%s</option>\n",
					$val,
					$j == $ai ? " selected" : "",
					$pval);
			}

			echo "\t\t</select>\n";
		}

		end_field();
	}

	// functions to handle the form input

	// fetch all the parts of an "enum_multi" into a string suitable
	// for a MySQL query
	function gather_enum_multi_query($base, $cnt) {

		$retval = '';

		for ($i = 0; $i < $cnt; $i++) {

			$name = sprintf("%s%x", $base, $i);

			if (isset($_REQUEST[$name])) {
				$retval .= sprintf(", %s='%s'",
					$name, $_REQUEST[$name]);
			}
		}

		return $retval;
	}

	// fetch all the parts of an "enum_multi" into a string suitable
	// for a display e.g. in an html table cell
	function gather_enum_multi_print($base, $cnt, $array) {

		$retval = '';

		for ($i = 0; $i < $cnt; $i++) {

			$name = sprintf("%s%x", $base, $i);

			if ($array[$name] != '') {
				if ($retval != '')
					$retval .= ',';
				$retval .= $array[$name];
			}
		}

		return $retval;
	}

	// fetch all the parts of an "enum_multi" into a string suitable
	// for writing to the eeprom data file
	function gather_enum_multi_write($base, $cnt, $vals, $xfrm = array()) {

		$retval = '';

		for ($i = 0; $i < $cnt; $i++) {

			$name = sprintf("%s%x", $base, $i);

			if ($GLOBALS[$name] != '') {
				if ($retval != '')
					$retval .= ',';
				$index = enum_to_index($name, $vals);
				if ($xfrm != array())
					$retval .= $xfrm[$index];
				else
					$retval .= $index;
			}
		}

		return $retval;
	}

	// count how many parts of an "enum_multi" are actually set
	function count_enum_multi($base, $cnt) {

		$retval = 0;

		for ($i = 0; $i < $cnt; $i++) {

			$name = sprintf("%s%x", $base, $i);

			if (isset($_REQUEST[$name]))
				$retval++;
		}

		return $retval;
	}

	// ethernet address functions

	// generate a (possibly not unique) random vendor ethernet address
	// (setting bit 6 in the ethernet address - motorola wise i.e. bit 0
	// is the most significant bit - means it is not an assigned ethernet
	// address - it is a "locally administered" address). Also, make sure
	// it is NOT a multicast ethernet address (by setting bit 7 to 0).
	// e.g. the first byte of all ethernet addresses generated here will
	// have 2 in the bottom two bits (incidentally, these are the first
	// two bits transmitted on the wire, since the octets in ethernet
	// addresses are transmitted LSB first).

	function gen_eth_addr($serno) {

		$ethaddr_hgh = (mt_rand(0, 65535) & 0xfeff) | 0x0200;
		$ethaddr_mid = mt_rand(0, 65535);
		$ethaddr_low = mt_rand(0, 65535);

		return sprintf("%02lx:%02lx:%02lx:%02lx:%02lx:%02lx",
			$ethaddr_hgh >> 8, $ethaddr_hgh & 0xff,
			$ethaddr_mid >> 8, $ethaddr_mid & 0xff,
			$ethaddr_low >> 8, $ethaddr_low & 0xff);
	}

	// check that an ethernet address is valid
	function eth_addr_is_valid($ethaddr) {

		$ethbytes = split(':', $ethaddr);

		if (count($ethbytes) != 6)
			return FALSE;

		for ($i = 0; $i < 6; $i++) {
			$ethbyte = $ethbytes[$i];
			if (!ereg('^[0-9a-f][0-9a-f]$', $ethbyte))
				return FALSE;
		}

		return TRUE;
	}

	// write a simple eeprom configuration file
	function write_eeprom_cfg_file() {

		global $sernos, $nsernos, $bddb_cfgdir, $numerrs, $cfgerrs;
		global $date, $batch, $type_vals, $rev;
		global $sdram_vals, $sdram_nbits;
		global $flash_vals, $flash_nbits;
		global $zbt_vals, $zbt_nbits;
		global $xlxtyp_vals, $xlxspd_vals, $xlxtmp_vals, $xlxgrd_vals;
		global $cputyp, $cputyp_vals, $clk_vals;
		global $hstype, $hstype_vals, $hschin, $hschout;

		$numerrs = 0;
		$cfgerrs = array();

		for ($i = 0; $i < $nsernos; $i++) {

			$serno = sprintf("%010d", $sernos[$i]);

			$wfp = @fopen($bddb_cfgdir . "/$serno.cfg", "w");
			if (!$wfp) {
				$cfgerrs[$i] = 'file create fail';
				$numerrs++;
				continue;
			}
			set_file_buffer($wfp, 0);

			if (!fprintf($wfp, "serno=%d\n", $sernos[$i])) {
				$cfgerrs[$i] = 'cfg wr fail (serno)';
				fclose($wfp);
				$numerrs++;
				continue;
			}

			if (!fprintf($wfp, "date=%s\n", $date)) {
				$cfgerrs[$i] = 'cfg wr fail (date)';
				fclose($wfp);
				$numerrs++;
				continue;
			}

			if ($batch != '') {
				if (!fprintf($wfp, "batch=%s\n", $batch)) {
					$cfgerrs[$i] = 'cfg wr fail (batch)';
					fclose($wfp);
					$numerrs++;
					continue;
				}
			}

			$typei = enum_to_index("type", $type_vals);
			if (!fprintf($wfp, "type=%d\n", $typei)) {
				$cfgerrs[$i] = 'cfg wr fail (type)';
				fclose($wfp);
				$numerrs++;
				continue;
			}

			if (!fprintf($wfp, "rev=%d\n", $rev)) {
				$cfgerrs[$i] = 'cfg wr fail (rev)';
				fclose($wfp);
				$numerrs++;
				continue;
			}

			$s = gather_enum_multi_write("sdram", 4,
				$sdram_vals, $sdram_nbits);
			if ($s != '') {
				$b = fprintf($wfp, "sdram=%s\n", $s);
				if (!$b) {
					$cfgerrs[$i] = 'cfg wr fail (sdram)';
					fclose($wfp);
					$numerrs++;
					continue;
				}
			}

			$s = gather_enum_multi_write("flash", 4,
				$flash_vals, $flash_nbits);
			if ($s != '') {
				$b = fprintf($wfp, "flash=%s\n", $s);
				if (!$b) {
					$cfgerrs[$i] = 'cfg wr fail (flash)';
					fclose($wfp);
					$numerrs++;
					continue;
				}
			}

			$s = gather_enum_multi_write("zbt", 16,
				$zbt_vals, $zbt_nbits);
			if ($s != '') {
				$b = fprintf($wfp, "zbt=%s\n", $s);
				if (!$b) {
					$cfgerrs[$i] = 'cfg wr fail (zbt)';
					fclose($wfp);
					$numerrs++;
					continue;
				}
			}

			$s = gather_enum_multi_write("xlxtyp", 4, $xlxtyp_vals);
			if ($s != '') {
				$b = fprintf($wfp, "xlxtyp=%s\n", $s);
				if (!$b) {
					$cfgerrs[$i] = 'cfg wr fail (xlxtyp)';
					fclose($wfp);
					$numerrs++;
					continue;
				}
			}

			$s = gather_enum_multi_write("xlxspd", 4, $xlxspd_vals);
			if ($s != '') {
				$b = fprintf($wfp, "xlxspd=%s\n", $s);
				if (!$b) {
					$cfgerrs[$i] = 'cfg wr fail (xlxspd)';
					fclose($wfp);
					$numerrs++;
					continue;
				}
			}

			$s = gather_enum_multi_write("xlxtmp", 4, $xlxtmp_vals);
			if ($s != '') {
				$b = fprintf($wfp, "xlxtmp=%s\n", $s);
				if (!$b) {
					$cfgerrs[$i] = 'cfg wr fail (xlxtmp)';
					fclose($wfp);
					$numerrs++;
					continue;
				}
			}

			$s = gather_enum_multi_write("xlxgrd", 4, $xlxgrd_vals);
			if ($s != '') {
				$b = fprintf($wfp, "xlxgrd=%s\n", $s);
				if (!$b) {
					$cfgerrs[$i] = 'cfg wr fail (xlxgrd)';
					fclose($wfp);
					$numerrs++;
					continue;
				}
			}

			if ($cputyp != '') {
				$cputypi = enum_to_index("cputyp",$cputyp_vals);
				$cpuspdi = enum_to_index("cpuspd", $clk_vals);
				$busspdi = enum_to_index("busspd", $clk_vals);
				$cpmspdi = enum_to_index("cpmspd", $clk_vals);
				$b = fprintf($wfp, "cputyp=%d\ncpuspd=%d\n" .
					"busspd=%d\ncpmspd=%d\n",
					$cputypi, $cpuspdi, $busspdi, $cpmspdi);
				if (!$b) {
					$cfgerrs[$i] = 'cfg wr fail (cputyp)';
					fclose($wfp);
					$numerrs++;
					continue;
				}
			}

			if ($hstype != '') {
				$hstypei = enum_to_index("hstype",$hstype_vals);
				$b = fprintf($wfp, "hstype=%d\n" .
					"hschin=%s\nhschout=%s\n",
					$hstypei, $hschin, $hschout);
				if (!$b) {
					$cfgerrs[$i] = 'cfg wr fail (hstype)';
					fclose($wfp);
					$numerrs++;
					continue;
				}
			}

			if (!fclose($wfp)) {
				$cfgerrs[$i] = 'file cls fail';
				$numerrs++;
			}
		}

		return $numerrs;
	}
?>
