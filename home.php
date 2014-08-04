<?php
//Home page for the user
include("sessionTools.php");
include("userTools.php");
include("tweetTools.php");
include("connectionTools.php");
//Checks if the user is logged in
validateUser();
$userName = $_SESSION['userName'];
$followees = loadFollowees($userName);
$others = loadOthers($userName);
$tweets = loadTweets($userName);
?>

<!DOCTYPE html>
<html>
<head>
	<meta charset="UTF-8">
	<!-- Basic CSS -->
	<style>
	body
	{
		background-color: #d0e4fe;
	}
	#header
	{
		height:20%;
 		width:100%;
		background-color:#FFA500;
		text-align:center;
	}
	#side
	{
		background-color:#FFD700;
		height:200%;
		width:20%;
		float:left;

	}
	#mid
	{
		background-color:#EEEEEE;
		height:100%;
		width:80%;
		float:left;
		text-align:center;
	}
	</style>
</head>
<body>	
		<div id="header">
								<h1>Twitter</h1>
			Hi, <strong> <?php echo $userName; ?> </strong>
			<form name="logout" action="logout.php">
				<input type="submit" value = "Log Out">
			</form>
			<!--
			Deleting your account will remove you from the system.
			All of your tweets will be deleted and your followers will no longer see them.
			-->
			<form name="deleteAccount" action="deleteAccount.php">
				<input type="submit" value = "Delete Account">
			</form>
		</div>
		<div id="side">
			<center>
			<br>
				Following:
				<?php showFollowees($followees); ?>
			<br>
				All Other Users:
				<?php showOtherUsers($others); ?>
			<br>
		</center>
		</div>
		<div id="mid">
			Tweets:
			<br>
				<form action="addTweet.php" method = "post">
				<textarea name = "tweet" maxlength = "120" rows="4" cols="50"></textarea>
				<br>
				<input type="submit" value="Tweet">
				</form>
			<br>
				<?php displayTweets($tweets); ?>
		</div>

</body>
</html>
