<?php
//Contains tools that load/display users
function loadOthers($userName)
{
	//Loads followees
	$requestString = getIdentifier()."\t"."LIST_OTHERS"."\t".$userName;
	$others = getFromServer($requestString);
	//for each user entry in $pairsArray, if $userName is equivalent to
	//first field in entry then add the second field (followee) to an array
	//containing all the followees for that userName
	return $others;
}
function listOtherUsers($others)
{
	//Lists users that or not $userName's followees
	foreach($others as $oneUser)
	{ 	
			?>
		  	<html>
		  		<option value = "<?php echo $oneUser; ?>" selected>
		  			<?php echo $oneUser ?>
		  		</option>
		  	</html>
			<?
	}
}
function showOtherUsers($others)
{
	if(count($others)<=0)
	{
		echo "<i>None</i>";
	}
	else
	{
		?>
		<html>
		<form name = "followee" action="addFollowee.php" method = "post">
			<select name="followee" selected>
				<?php listOtherUsers($others); ?>
			</select>
			<input type="submit" value="Follow">
		</form>
		</html>
		<?php
	} 
}

function loadFollowees($userName)
{
	//Loads followees
	$requestString = getIdentifier()."\t"."LIST_FOLLOWEES"."\t".$userName;
	$followees = getFromServer($requestString);
	//for each user entry in $pairsArray, if $userName is equivalent to
	//first field in entry then add the second field (followee) to an array
	//containing all the followees for that userName
	return $followees;

}
function showFollowees($followees)
{
	//display all followees in a table
	if(count($followees) > 0){
		echo "<table border = '1'>";		   
		foreach($followees as $followee)
		{
	        ?>
			<html>
				<tr>
					<td>
						<?php
							echo $followee;
						?>
					</td>
					<td>
						<!-- Show followee button -->
						<form name = "followees" action="removeFollowee.php" method = "post">
							<input type="hidden" name = "followee" value="<?php echo $followee; ?>">
							<input type="submit" value="Unfollow">
						</form>
					</td>
				</tr>
			</html>
			<?php
		}
		echo "</table>";
	}
	else { echo "<i>Nobody</i>";}
}

?>
