<?php // php pages made with phpMyBuilder <http://kyber.dk/phpMyBuilder> ?>
<?php
	// (C) Copyright 2001
	// Murray Jensen <Murray.Jensen@csiro.au>
	// CSIRO Manufacturing Science and Technology, Preston Lab

	require("defs.php");
	pg_head("$bddb_label");
?>
<font size="+4">
  <form action=execute.php method=POST>
    <table width="100%" cellspacing=10 cellpadding=10>
      <tr>
	<td align=center>
	  <input type=submit name=submit value="New"></input>
	</td>
	<td align=center>
	  <input type=submit name=submit value="Edit"></input>
	</td>
	<td align=center>
	  <input type=submit name=submit value="Browse"></input>
	</td>
	<td align=center>
	  <input type=submit name=submit value="Log"></input>
	</td>
      </tr>
      <tr>
	<td align=center colspan=4>
	  <b>Serial Number:</b>
	  <input type=text name=serno size=10 maxsize=10 value=""></input>
	</td>
      </tr>
    </table>
  </form>
</font>
<?php
	pg_foot();
?>
