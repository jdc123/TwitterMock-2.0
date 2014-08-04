<?php
//load relevant tools
include("sessionTools.php");
include("connectionTools.php");
startSession();
validateUser();

//Delete user account
$userName = $_SESSION['userName'];
$requestString =getIdentifier()."\t"."DELETE_ACCOUNT"."\t".$userName;
$ret = getFromServer($requestString);
if($ret[0])
{
  $_SESSION['notifyUser'] = "Account successfully removed.";
}
else
{
  $_SESSION['notifyUser'] = "Failed to remove account from system.";
}
header("Location: logout.php");
exit("Relocating to the logout page for cleaning up");

?>
