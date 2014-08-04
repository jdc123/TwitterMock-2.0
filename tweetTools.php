<?php
//Contains tweet tools that load/display tweets
function loadTweets($userName)
{
	$tweets = array();
	$requestString = getIdentifier()."\t"."LIST_TWEETS"."\t".$userName;
	$pairsArray = getFromServer($requestString);
	//for the passed-in userName add the tweets him/her and his followees to the tweets array
	for($i=0; $i < count($pairsArray);$i++)
	{
	  $tweetInfo = explode("\t", $pairsArray[$i]) or exit("Unable to write to explode!");
	  array_push($tweets, $tweetInfo);
	}
	return $tweets;
}

function displayTweets($tweets)
{
	if(count($tweets)==0)
	{
		echo "<i> No tweets </i>";
	}
	else
	{
		for( $i = count($tweets) -1; $i >=0; $i--){	
		?>
			<textarea rows="4" cols="50"><?php echo $tweets[$i][2].  " by " . $tweets[$i][0] . " on " . date("D M j G:i:s Y T",$tweets[$i][1]); ?>
			</textarea>
			<br>
		<?php
		}
	}
}
?>
