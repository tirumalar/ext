<?php

$sessionDir = trim($argv[1],' \t\\/');
$targetDir = "parsed_".$sessionDir;

echo "$targetDir\n";
// TODO: distribute by camera IDs;

if (!is_dir($sessionDir))
{
	//printf("Changing working directory to: %s\n", $sessionDir);
	//chdir($sessionDir);
		printf("Cannot find session directory: %s\n", $sessionDir);
	exit(1);
}

if (is_dir($targetDir))
{
	printf("Target directory already exists! (%s)\n", $targetDir);
	exit(1);
}
else
{
	mkdir($targetDir);
}

$infoFile = $sessionDir."/Info.txt";
if (!is_file($infoFile))
{
	printf("Cannot find session log: %s\n", $infoFile);
	exit(1);
}

$sessionActive = false;

$sessionCount = 0;
$totalInputImagesCount = 0;
$totalFaceImagesCount = 0;

$totalFailures = 0;
$totalMatches = 0;

$inputImagesOutOfSessionCount = 0;

$lastFaceImageFrameNum = 255;

$handle = fopen($infoFile, "r");
if ($handle) 
{
	while (($line = fgets($handle)) !== false) //&& $sessionCount < 5
	{
		$line = trim($line);
		$lineParts = explode(" ", $line);
		//printf("line: %s\n", $line);
		//2018 06 20 20:05:43 1529539543:556454673 Switching_to_iris_mode
		//2018 06 20 20:05:43 1529539543:617153332 Saved-InputImage-FrNum140-CamID130-DebugSessions/Session/InputImage_2018_06_20_20-05-43_1529539543_617153332_140_130.pgm
		//2018 06 20 20:05:43 1529539543:618939130 Saved-InputImage-FrNum140-CamID129-DebugSessions/Session/InputImage_2018_06_20_20-05-43_1529539543_618939130_140_129.pgm
		//2018 06 20 20:05:43 1529539543:631411661 Saved-FaceImage-FrNum140-CamID4-DebugSessions/Session/FaceImage_2018_06_20_20-05-43_1529539543_631319828_140_4.pgm
		//2018 06 20 20:05:57 1529539557:669096177 Switched_to_face_mode
		if ($lineParts[5] === 'Switching') // Should be: Switching_to_iris_mode. Switching for now because was merged incorrectly
		{
			$curSession = str_replace(':', '_', implode("_", array_slice($lineParts, 0, 4)));
			if (mkdir($targetDir."/".$curSession))
			{
				$sessionActive = true;
				$sessionCount++;
				printf("session start %s\n", $curSession);
				mkdir($targetDir."/".$curSession."/FaceImages");
				mkdir($targetDir."/".$curSession."/IrisImages");
			}
		}
		else if ($lineParts[5] === 'Switched_to_face_mode')
		{
			if ($sessionActive === true)
			{
				$sessionActive = false;
				printf("Session end %s\n", $curSession);			
			}
		}
		else if (strncmp($lineParts[5], 'Saved', 5) === 0)
		{
			$imageInfoParts = explode("-", $lineParts[5]);
			
			// Saved-FaceImage-FrNum225
			$frNum = substr($imageInfoParts[2], 5);
			
			if ($imageInfoParts[1] === 'FaceImage')
			{
				$imageCategory = "FaceImages";
				$totalFaceImagesCount++;
				$lastFaceImageFrameNum = $frNum;
			}
			else if ($imageInfoParts[1] === 'InputImage')
			{
				$imageCategory = "IrisImages";
				$totalInputImagesCount++;
			}
			
			// DebugSessions/Session/InputImage_2018_06_20_20-14-22_1529540062_107915395_83_1.pgm
			// TODO: remove dashes from timestamp
			$imageFileFullPathParts = explode("/", $lineParts[5]);
			$imageFile = end($imageFileFullPathParts);
			
			if ($sessionActive || ($frNum - $lastFaceImageFrameNum) < 5 || ($frNum - $lastFaceImageFrameNum) > 250 )
			{
				$imageFileDestination = $targetDir."/".$curSession."/".$imageCategory."/".$imageFile;
				printf("moving image %s to %s\n", $imageFile, $imageFileDestination);
				// TODO: change to rename
				copy($sessionDir."/".$imageFile, $imageFileDestination);
			}
			else
			{
				printf("image out of session: %s\n", $imageFile);
				$inputImagesOutOfSessionCount++;
			}
		}
		else if (strncmp($lineParts[5], 'Match_s', 7) === 0)
		{
			echo "Match success!\n";
			$totalMatches++;
			//$matchEventDest = ($sessionActive) ? $sessionDir : $targetDir;
			file_put_contents($targetDir."/".$curSession."/Matched.txt", $line."\n", FILE_APPEND); // associate with last session

		}
		else if (strncmp($lineParts[5], 'Match_f', 7) === 0)
		{
			echo "Match fail!\n";
			$totalFailures++;
			//$matchEventDest = ($sessionActive) ? $sessionDir : $targetDir;
			file_put_contents($targetDir."/".$curSession."/MatchFailure.txt", $line."\n", FILE_APPEND);
		}
		
		//readline(">...");
        //readline_add_history($line);
	}
}
else
{
	printf("Cannot find file %s\n", $infoFile);
}

printf("Total Match Successes Count: %d\n", $totalMatches);
printf("Total Match Failures Count: %d\n", $totalFailures);
printf("Total Face Images Count: %d\n", $totalFaceImagesCount);
printf("Total Input Images Count: %d\n", $totalInputImagesCount);
printf("Input Images Out Of Session Count: %d\n", $inputImagesOutOfSessionCount);
printf("Session Count: %d\n", $sessionCount);
readline(">...");
//readline_add_history($line);
?>