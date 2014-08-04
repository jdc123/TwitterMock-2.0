<?php
//Presents login/registration forms to the user
session_start();
//Checks for any messages that the user should see
if(isset($_SESSION['notifyUser']))
{	
	echo $_SESSION['notifyUser'];
	unset($_SESSION['notifyUser']);
}

?>

<!DOCTYPE html>
<html>
<head>
	<meta charset="UTF-8">
</head>
<body>
	<center>
						<h1>Welcome To Twitter </h1>
						<strong>Log In</strong><br>
		<!-- Log In Form -->
			<form name="login" action="verifyLogin.php" method="post">
				Username: <input type="text" name="userName" maxlength="32"><br>
				Password: <input type="password" name="userPwd" maxlength="32"><br>
				<input type="submit" value="Log In"> <br>
			</form>
								OR <br>
					<strong>Register</strong><br>
		<!-- Registration Form -->
			<form name="register" action="verifyRegistration.php" method="post">
				Username: <input type="text" name="userName" maxlength="32"> <br>
				Password: <input type="password" name="userPwd" maxlength="32"><br>
				Re-Enter Password: <input type="password" name="userPwd2" maxlength="32"><br>
				<input type="submit" value="Register"><br>
	</center>
</body>
</html>
