# Pakbuilder Script 1.0.0
#
# For selected folders in the element list, create links to those folders and build pak files from them. For the elements in the 
# "to copy" list, it copies the whole folder into the pakbuilder outputfolder. Run the script as an administrator to be able to 
# create the symbolic links in Windows.

######################
# Variables to adapt #
######################
# List of .\Art\[FOLDER] to pack
$ELEMENT_LIST = "ACO", "BUG", "Effects", "Interface", "PlotListEnhancements", "Shared", "Static", "Structures", "Terrain", "Units"
# Movies and music shall not be packed. GreatPeople and Leaderheads cause black tiles if packed. Therefore, from this list, the folders are copied
$ELEMENTS_TO_COPY_LIST = "Movies", "GreatPeople", "LeaderHeads"
# Name prefix
$RIVERSION="RI_361"
# In relation to .\Art, the directory, where the script is executed, e.g. in trunk parallel to .\mod
$MOD_RELATIVE_PATH_BASEDIR = "..\mod\Assets\Art"
# When symbolic links are created, this is the folder, where the links are created. The path is the relative path to the linked folders
$MOD_RELATIVE_PATH_ART = "..\..\$MOD_RELATIVE_PATH_BASEDIR"
# Pakbuilder path
$PAKBUILD_PATH = "..\installer\PakBuild\PakBuild.exe"
# Output folder path of the pakbuilder
$OUT_FOLDER = "PakBuild"

#############
# Constants #
#############
$BASE_FOLDER = Get-Location

echo "Base location $BASE_FOLDER"


#############
# Execution #
#############
if (-not (Test-Path $OUT_FOLDER)) {
    New-Item -ItemType Directory -Path $OUT_FOLDER
}

# For each folder in the art directory that should be used, create pak files
foreach ($element in $ELEMENT_LIST) {
	echo "Create directory and link for element $element"
	$directoryPath = "$($BASE_FOLDER)\Asset_$($element)"
	if (-not (Test-Path $directoryPath)) {
		New-Item -ItemType Directory -Path $directoryPath
	}
	
	$art_folder = "$($directoryPath)\Art"
	if (-not (Test-Path $art_folder)) {
		New-Item -ItemType Directory -Path $art_folder
	}
	
	cd $art_folder
	
	# Create link to folder
	cmd /c mklink /d $element $MOD_RELATIVE_PATH_ART\$element
	
	#echo "base $BASE_FOLDER"
	cd $BASE_FOLDER

	# Execute Pakbuild in a separate output folder to prevent that pakbuilder deletes the files of other builds
	$command = "$PAKBUILD_PATH /F /S=11 /I=.\Asset_$($element) /O=$($OUT_FOLDER)_$element /R=$($RIVERSION)_$element"
	Invoke-Expression $command
}

# obviously needed buffer time for the previous command to start writing files to disk as the process seems to detach from 
# the console script
sleep 200

foreach ($element in $ELEMENT_LIST) {
	#Copy to one folder
	Copy-Item -Path "$($OUT_FOLDER)_$($element)\*.*" -Destination ".\$OUT_FOLDER\" -Recurse -force
	
	#Delete temp output folders
	echo ".\$($OUT_FOLDER)_$($element)\"
	rm -r -fo ".\$($OUT_FOLDER)_$($element)\"
}	

# Copy movies and non packed items to art
foreach ($copyElement in $ELEMENTS_TO_COPY_LIST) {
	Copy-Item -Path "$MOD_RELATIVE_PATH_BASEDIR\$($copyElement)" -Destination "$OUT_FOLDER\Art\$copyElement" -Recurse -force
}
