<?php
//Adds a followee to a file containing user-followee pairs
include("sessionTools.php");
include("connectionTools.php");

//check if user is logged in
validateUser();
$userName = $_SESSION['userName'];
$followee = $_POST['followee'];

//Open file containing user-followee pair
$requestString =getIdentifier()."\t"."ADD_FOLLOWEE"."\t".$userName."\t".$followee;
$ret = getFromServer($requestString);
if(!$ret[0]){
	exit("Failed to add followee.");
}
else{
//redirect the user to the home page
header("Location: home.php");
exit("Successfully added followee.");
}
?>
