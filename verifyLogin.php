<?php
//load relevant tools
include("sessionTools.php");
include("connectionTools.php");
startSession();
//Assign POST variables to local variables
$userName = strip_tags(trim($_POST["userName"]));
$userPwd = md5(strip_tags(trim($_POST["userPwd"])));
 
//Check if userName and userPwd passed in from POST are blank
if(strlen($userName) == 0 or strlen($userPwd) == 0) 
{
  $_SESSION['notifyUser'] = "Username or password left blank.";
  header("Location: login.php");
  exit("Username and/or password field left blank.");
}
//Server start
$requestString =getIdentifier()."\t"."LOGIN"."\t".$userName."\t".$userPwd;
$ret = getFromServer($requestString);
//echo $ret;
if($ret[0])
  {
    $_SESSION['userName'] = $userName;
    header("Location: home.php");
    exit("Success: Relocating to the login page.");
  }
else
{
  $_SESSION['notifyUser'] = "User not found in the database.";
  //relocates user back to the home page
  header("Location: login.php");
  exit("User not found in the database.");
}

?>