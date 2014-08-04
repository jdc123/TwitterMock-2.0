<?php
	include("sessionTools.php");
	//Start a session
	startSession();
	$msg = $_SESSION['notifyUser'];
	//End the session and redirect to the login page
	endSession($msg);
?>