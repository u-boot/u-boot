<?php // php pages made with phpMyBuilder <http://kyber.dk/phpMyBuilder> ?>
<?php
	// (C) Copyright 2001
	// Murray Jensen <Murray.Jensen@csiro.au>
	// CSIRO Manufacturing Science and Technology, Preston Lab

	$serno=isset($_REQUEST['serno'])?$_REQUEST['serno']:'';

	$submit=isset($_REQUEST['submit'])?$_REQUEST['submit']:"[NOT SET]";

	switch ($submit) {

	case "New":
		require("new.php");
		break;

	case "Edit":
		require("edit.php");
		break;

	case "Browse":
		require("browse.php");
		break;

	case "Log":
		require("brlog.php");
		break;

	default:
		require("badsubmit.php");
		break;
	}
?>
