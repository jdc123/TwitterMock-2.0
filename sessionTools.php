<?php
//Contains session tools

//starts the session
function startSession()
{
	session_save_path("/tmp");
	session_start();
}

//ends the session
function endSession($notify)
{
	session_destroy();
	if(isset($notify))
	{
		startSession();
		$_SESSION['notifyUser'] = $notify;
	}
	header("Location: login.php");
	exit("Logging out.");
}

//validates that user is logged in
function validateUser()
{
	startSession();
	if(!isset($_SESSION['userName']))
	{
		endSession();
	}
}
?>
