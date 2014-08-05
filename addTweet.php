<?php
//Adds a tweet to a file containing user-timestamp-message entries
include("sessionTools.php");
include("connectionTools.php");

//Check if user is logged in
validateUser();
$tweet = $_POST['tweet'];
$userName = $_SESSION['userName'];

//if tweet is not empty add a tweet entry to a file
if(strlen($tweet) > 0){
	$requestString = getIdentifier()."\t"."ADD_TWEET"."\t".$userName."\t".time()."\t".$tweet; //Nothing to return
	//exit( "request: ".$requestString." ".strlen($requestString)."\n");
	$ret = getFromServer($requestString);
	if(!$ret[0])
	{
		exit("Failed to tweet.");
	}
}
header("Location: home.php");
exit("Successfully tweeted.");

?>
