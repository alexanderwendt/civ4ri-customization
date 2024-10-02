# Generate Civ4 RI mod folder from code

######################
# Variables to adapt #
######################
# List of .\Art\[FOLDER] to pack
#$ELEMENT_LIST = "ACO", "BUG", "Effects", "Interface", "PlotListEnhancements", "Shared", "Static", "Structures", "Terrain", "Units"
# Movies and music shall not be packed. GreatPeople and Leaderheads cause black tiles if packed. Therefore, from this list, the folders are copied
#$ELEMENTS_TO_COPY_LIST = "Movies", "GreatPeople", "LeaderHeads"
# Name prefix
$RIVERSION="RI_361_r5430"
# In relation to .\Art, the directory, where the script is executed, e.g. in trunk parallel to .\mod
$MOD_RELATIVE_PATH_BASEDIR = "..\mod"
# When symbolic links are created, this is the folder, where the links are created. The path is the relative path to the linked folders
#$MOD_RELATIVE_PATH_ART = "..\..\$MOD_RELATIVE_PATH_BASEDIR"
# Output folder path of the pakbuilder
$OUT_FOLDER = ".\realism_invictus_${RIVERSION}"
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