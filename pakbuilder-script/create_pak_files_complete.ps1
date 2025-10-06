# Pakbuilder Script 1.0.0
#
# For selected folders in the element list, create links to those folders and build pak files from them. For the elements in the 
# "to copy" list, it copies the whole folder into the pakbuilder outputfolder. Run the script as an administrator to be able to 
# create the symbolic links in Windows.

######################
# Variables to adapt #
######################
# Name prefix
$RIVERSION="RI_372_r5521"


##########################
# Variables not to adapt #
##########################
# List of .\Art\[FOLDER] to pack
$ELEMENT_LIST = "ACO", "BUG", "Effects", "Interface", "PlotListEnhancements", "Shared", "Static", "Structures", "Terrain", "Units"
# Movies and music shall not be packed. GreatPeople and Leaderheads cause black tiles if packed. Therefore, from this list, the folders are copied
$ELEMENTS_TO_COPY_LIST = "Movies", "GreatPeople", "LeaderHeads"
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

echo "Create mod folder for civ ri"

# Generate Civ4 RI mod folder from code
######################
# Variables to adapt #
######################
# In relation to .\Art, the directory, where the script is executed, e.g. in trunk parallel to .\mod
$MOD_RELATIVE_PATH_BASEDIR = "..\mod"
# When symbolic links are created, this is the folder, where the links are created. The path is the relative path to the linked folders
#$MOD_RELATIVE_PATH_ART = "..\..\$MOD_RELATIVE_PATH_BASEDIR"
# Output folder path of the pakbuilder
$OUT_FOLDER = ".\Realism Invictus_${RIVERSION}"
$PAKBUILD_DIRECTORY = "./PakBuild"

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
$directories = Get-ChildItem -Path $MOD_RELATIVE_PATH_BASEDIR -Directory | Where-Object { $_.Name -notin @("Assets", "GameCore") }

foreach ($directory in $directories) {
	echo "dir: ${directory}"
	Copy-Item -Path $directory.FullName -Destination $OUT_FOLDER -Recurse -Force
}	

# Copy realism.ini
echo "Copy realism.ini"
Copy-Item -Path $MOD_RELATIVE_PATH_BASEDIR\*.* -Destination $OUT_FOLDER

# Copy all except Art from asserts
echo "Copy all folders from asset except Art"
$MOD_RELATIVE_PATH_ASSETS = "${MOD_RELATIVE_PATH_BASEDIR}\Assets"
$OUT_FOLDER_ASSETS = "${OUT_FOLDER}\Assets"
if (-not (Test-Path $OUT_FOLDER_ASSETS)) {
    New-Item -ItemType Directory -Path $OUT_FOLDER_ASSETS
}

$directoriesAssets = Get-ChildItem -Path $MOD_RELATIVE_PATH_ASSETS -Directory | Where-Object { $_.Name -notin @("Art") }
foreach ($directory in $directoriesAssets) {
	echo "dir: ${directory} to ${OUT_FOLDER_ASSETS}"
	Copy-Item -Path $directory -Destination $OUT_FOLDER_ASSETS -Recurse -Force
}	

echo "Copy all files from asset"
Copy-Item -Path $MOD_RELATIVE_PATH_ASSETS\*.* -Destination "${OUT_FOLDER}\Assets" -Force

echo "Copy directories from repacked files"
$directoryPakbuild = Get-ChildItem -Path $PAKBUILD_DIRECTORY -Directory
foreach ($directory in $directoryPakbuild) {
	echo "dir: ${directory} to ${OUT_FOLDER_ASSETS}"
	Copy-Item -Path $directory -Destination $OUT_FOLDER_ASSETS -Recurse -Force
}

echo "Copy files from repacked files"
Copy-Item -Path $PAKBUILD_DIRECTORY\*.* -Destination "${OUT_FOLDER}\Assets" -Force

# FILE MODIFICATIONS #

Write-Host "Start replacement of Realism.thm."
# Define the file path
$filePath = "${OUT_FOLDER}\Resource\Realism.thm"

# Read the file content
$content = Get-Content -Path $filePath -Raw

# Replace the target string
$updatedContent = $content -replace 'Mods/Realism/Resource', 'Mods/Realism Invictus/Resource'

#Write-Host $updatedContent

# Write the updated content back to the file
Set-Content -Path $filePath -Value $updatedContent

Write-Host "Start replacement of CIV4ArtDefines_Misc.xml."

# Define the file path
$filePath = "${OUT_FOLDER}\Assets\XML\Art\CIV4ArtDefines_Misc.xml"

# Read the file content
$content = Get-Content -Path $filePath -Raw

# Replace the target string
$updatedContent = $content -replace 'Mods/Realism/Resource', 'Mods/Realism Invictus/Resource'

#Write-Host $updatedContent

# Write the updated content back to the file
Set-Content -Path $filePath -Value $updatedContent

Write-Host "Replacement of CIV4ArtDefines_Misc.xml complete."


# FILE ADDITIONS #

# Define the path and filename
$newFilePath = "${OUT_FOLDER}\realism_version_${RIVERSION}.txt"

# Define the content
$content = "${RIVERSION}"

# Create the file and write the content
Set-Content -Path $newFilePath -Value $content

Write-Host "File created at $newFilePath with specified content."
