<?php
//Removes a user-followee pair from the file containing such pairs
include("sessionTools.php");
include("connectionTools.php");
validateUser();

$followee = $_POST['followee'];
$userName = $_SESSION['userName'];

$requestString = getIdentifier()."\t"."REMOVE_FOLLOWEE"."\t".$userName."\t".$followee;
$ret = getFromServer($requestString);
if($ret[0]){
	//relocate the user back to the home page
	header("Location: home.php");
	exit("User successfully unfollowed.");
}
else
{
	exit("Failed to remove followee.");
}
?>
