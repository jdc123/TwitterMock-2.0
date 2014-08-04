<?php
//
function getPorts($fileName)
{
	$contents = file_get_contents($fileName);
	$ports = array_filter(explode(";", $contents), "notWhiteSpace"); //gets rid of whitespace
	return $ports;
}

//Retrieves responses from server in form of an array
function getResponse($fp)
{
	$arr = array();
	while( ($ret = fgets($fp)) !== false)
	{
		 $str = trim($ret);
		 if(strlen($str) > 0){
			array_push($arr, $str);
		 }
	}
	return $arr;
}

//Creates request to begin elections
function getPrimaryRequest($oldPrimary)
{
	$primaryReq = getIdentifier()."\t"."GET_PRIMARY"."\t".$oldPrimary;
	return $primaryReq;
}

//Send request to server socket
function sendRequest($fp, $requestString)
{
	fwrite($fp, $requestString);
}

//Remove white space
function notWhiteSpace($str)
{
	return !(ctype_space($str));
}
//Main connection func that returns a response in form of an array after
//connection is established
function getFromServer($requestString)
{
	//Connection information
	$protIP = "tcp://127.0.0.1";
	$portsFile = "ports.txt";
	$timeout = 2; //in seconds
	
	//Deserialize ports
	$ports = getPorts($portsFile);
	
	//Obtain index to last port (where primary should be running)
	$last_ind = (int)(count($ports)-1);
	
	//Election related (in case the primary goes down)
	$callElections = false;
	$oldPrimary = (int)$ports[(int)$last_ind];
	
	//Try Connecting
	$fp = fsockopen($protIP, (int)$ports[$last_ind],$errno, $errstr, $timeout);
	while($fp == false)
	{
		//The time has come to electe a new leader!
		$callElections = true;
		//Mark old primary
		$oldPrimary = (int)$ports[$last_ind];
		$last_ind--;
		if($last_ind < 0)
		{
			exit("Ran out of destination servers! Oh no :(");
		}
		//Try reconnecting
		$fp = fsockopen($protIP, (int)$ports[$last_ind],$errno, $errstr, $timeout);
		echo "Trying to connect to ".$ports[$last_ind]."\r\n";	
	}	
	if($callElections)
	{
		//Generate a Primary Request String
		$primaryReq = getPrimaryRequest($oldPrimary);
		
		//Send the request to the back-end
		sendRequest($fp, $primaryReq);
		
		//Get election response
		$r = getResponse($fp);
		
		//Remove faulty ports
		$contents = "";
		for($i = 0; $i <= $last_ind; $i++)
		{
			$contents = $contents.$ports[$i].";";
		}
		file_put_contents($portsFile, $contents);
		
		//Close connection
		fclose($fp); 
	}
	//Reopen connection
	$fp = fsockopen($protIP, (int)$ports[$last_ind],$errno, $errstr, $timeout);
	
	//send original request to the server
	sendRequest($fp, $requestString);
	
	//obtain response
	$responseArr = getResponse($fp);
	
	//close connection
	fclose($fp);
	
	return $responseArr;
}
//generates a hash
function getIdentifier()
{
	return md5(rand(0, 999999999999999));
}



?>
