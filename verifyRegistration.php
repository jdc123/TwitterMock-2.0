<?php
//Verifies registration for a user
include("sessionTools.php");
include("connectionTools.php");
session_start();
$newUserName = strip_tags(trim($_POST["userName"]));
$newPwd = md5(strip_tags(trim($_POST["userPwd"])));
$reNewPwd = md5(strip_tags(trim($_POST["userPwd2"])));

//Check if any of the POST variables are empty or mismatched
if(strlen($newUserName)==0 or strlen($newPwd)==0 or strlen($reNewPwd) == 0)
{
  $_SESSION['notifyUser'] = "One or more input fields left blank.";
  header("Location: login.php");
  exit("One or more input fields left blank.");
}
//Checks if password forms are not matching
if($newPwd != $reNewPwd)
{
  $_SESSION['notifyUser'] = "Password fields not matching.";
  header("Location: login.php");
  exit("Password fields not matching.");
}
//server action starts here
$requestString =getIdentifier()."\t"."REGISTER"."\t".$newUserName."\t".$newPwd;
$ret = getFromServer($requestString);
if($ret[0])
{
  $_SESSION['notifyUser'] = "Registered successfully!";
  header("Location: login.php");
  exit("Registered successfully!");
} 
else{
  $_SESSION['notifyUser'] = "User already exists. Please try another username.";
  header("Location: login.php");
  exit("User already exists. Please try another username.");
}
?>