<?php // php pages made with phpMyBuilder <http://kyber.dk/phpMyBuilder> ?>
<?php
	// (C) Copyright 2001
	// Murray Jensen <Murray.Jensen@csiro.au>
	// CSIRO Manufacturing Science and Technology, Preston Lab

	require("defs.php");
	pg_head("$bddb_label - Unknown Submit Type");
?>
<center>
  <font size="+4">
    <b>
      The <?php echo "$bddb_label"; ?> form was submitted with an
      unknown SUBMIT type <?php echo "(value was '$submit')" ?>.
      <br></br>
      Perhaps you typed the URL in directly? Click here to go to the
      home page of the <a href="index.php"><?php echo "$bddb_label"; ?></a>.
    </b>
  </font>
</center>
<?php
	pg_foot();
?>
