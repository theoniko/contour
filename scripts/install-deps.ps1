#! /usr/bin/env pwsh

# Let's assume for now, that this script is only invoked from within Windows
# But in the future, I'd like it to support all the others, too.

class ThirdParty
{
    [ValidateNotNullOrEmpty()] [string] $Folder
    [ValidateNotNullOrEmpty()] [string] $Archive
    [ValidateNotNullOrEmpty()] [string] $URI
}

$ThirdParties =
@(
    [ThirdParty]@{
        Folder="libunicode-b1112e06b532b93e08b752b1c65200d69555b1ed";
        Archive="libunicode-b1112e06b532b93e08b752b1c65200d69555b1ed.zip";
        URI="https://github.com/contour-terminal/libunicode/archive/b1112e06b532b93e08b752b1c65200d69555b1ed.zip"
    };
    [ThirdParty]@{
        Folder="termbench-pro-cd571e3cebb7c00de9168126b28852f32fb204ed";
        Archive="termbench-pro-cd571e3cebb7c00de9168126b28852f32fb204ed.zip";
        URI="https://github.com/contour-terminal/termbench-pro/archive/cd571e3cebb7c00de9168126b28852f32fb204ed.zip"
    };
    [ThirdParty]@{
        Folder="GSL-3.1.0";
        Archive="gsl-3.1.0.zip";
        URI="https://github.com/microsoft/GSL/archive/refs/tags/v3.1.0.zip"
    };
    [ThirdParty]@{
        Folder="Catch2-2.13.7";
        Archive="Catch2-2.13.7.zip";
        URI="https://github.com/catchorg/Catch2/archive/refs/tags/v2.13.7.zip"
    }
)

function Fetch-And-Add
{
    param (
        [Parameter(Mandatory)] [string] $Target,
        [Parameter(Mandatory)] [string] $Folder,
        [Parameter(Mandatory)] [string] $Archive,
        [Parameter(Mandatory)] [string] $URI,
        [Parameter(Mandatory)] [string] $CMakeListsFile
    )

    $DistfilesDir = "${Target}/distfiles"
    if (! [System.IO.Directory]::Exists($DistfilesDir))
    {
        New-Item -ItemType Directory -Force -Path $DistfilesDir
    }

    $ArchivePath = "${DistfilesDir}/${Archive}"
    if (! [System.IO.File]::Exists($ArchivePath))
    {
        Write-Host "Downloading $Archive to $ArchivePath"
        Invoke-WebRequest -Uri $URI -OutFile $ArchivePath
    }
    else
    {
        Write-Host "Already there: $ArchivePath"
    }

    if (! [System.IO.Directory]::Exists("$Target/sources/$Folder"))
    {
        Write-Host "Populating ${Folder}"
        Expand-Archive $ArchivePath -DestinationPath "${Target}/sources/"
    }
    else
    {
        Write-Host "Already there ${Folder}"
    }

    Add-Content $CMakeListsFile "add_subdirectory(${Folder} EXCLUDE_FROM_ALL)"
}

function Run
{
    $ProjectRoot = "${PSScriptRoot}/.."
    $ThirsPartiesDir = "${ProjectRoot}/_deps"
    $DistfilesDir = "${ThirsPartiesDir}/distfiles"
    $SourcesDir = "${ThirsPartiesDir}/sources"
    $CMakeListsFile = "${SourcesDir}/CMakeLists.txt"

    if (! [System.IO.Directory]::Exists($DistfilesDir))
    {
        New-Item -ItemType Directory -Force -Path $DistfilesDir
    }

    if (! [System.IO.Directory]::Exists($SourcesDir))
    {
        New-Item -ItemType Directory -Force -Path $SourcesDir
    }

    if ([System.IO.File]::Exists($CMakeListsFile))
    {
        Clear-Content $CMakeListsFile
    }

    foreach($TP in $ThirdParties)
    {
        Fetch-And-Add -Folder $TP.Folder -Archive $TP.Archive -URI $TP.URI -Target $ThirsPartiesDir -CMakeListsFile $CMakeListsFile
    }
}

Run