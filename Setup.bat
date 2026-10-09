@echo off
rem Copyright (c) 2026 Alvaro Cabrero Barros. Licensed under the MIT License. See LICENSE in the repository root.
rem
rem One-step setup for HordeKiller. Run it once after cloning the repository (double-click it, or call
rem it from a terminal). It leaves the project ready to open:
rem
rem   1. Finds the Unreal Engine installation.
rem   2. Downloads the Git LFS files (Blueprints, levels, data assets).
rem   3. Generates the Visual Studio solution.
rem   4. Builds the editor target.
rem   5. Copies Epic's mannequins from the engine. They are Epic content, so they are not stored in
rem      the repository. Also recreates the animation Blueprint if it is missing.
rem
rem It is safe to run again: every step skips what is already done.
rem
rem Usage:  Setup.bat ["C:\Path\To\UE_5.8"]
rem   The engine folder is optional. Without it, the UE_ROOT environment variable is used if set, then
rem   the Epic Games Launcher's record of installed engines, then the default install folder.
rem   Set HK_NO_PAUSE=1 to skip the final "press any key" (for scripts).

setlocal EnableExtensions
cd /d "%~dp0"

set "ENGINE_VERSION=5.8"
set "PROJECT=%~dp0HordeKiller.uproject"
set "TARGET=HordeKillerEditor"

echo.
echo === HordeKiller setup ===

rem ---------------------------------------------------------------------------------------------
echo.
echo [1/5] Locating Unreal Engine %ENGINE_VERSION%...

set "UE=%~1"
if not defined UE if defined UE_ROOT set "UE=%UE_ROOT%"

rem The launcher lists the engines it installed in a JSON file; PowerShell reads it.
if not defined UE (
    for /f "usebackq delims=" %%A in (`powershell -NoProfile -ExecutionPolicy Bypass -Command "$f = Join-Path $env:ProgramData 'Epic\UnrealEngineLauncher\LauncherInstalled.dat'; if (Test-Path $f) { (Get-Content $f -Raw | ConvertFrom-Json).InstallationList | Where-Object { $_.AppName -eq 'UE_%ENGINE_VERSION%' } | Select-Object -First 1 -ExpandProperty InstallLocation }"`) do set "UE=%%A"
)
if not defined UE set "UE=%ProgramFiles%\Epic Games\UE_%ENGINE_VERSION%"

set "EDITOR_CMD=%UE%\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
set "UBT=%UE%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe"
set "BUILD_BAT=%UE%\Engine\Build\BatchFiles\Build.bat"

if not exist "%EDITOR_CMD%" (
    echo ERROR: Unreal Engine %ENGINE_VERSION% was not found at "%UE%".
    echo        Install it from the Epic Games Launcher, or pass its folder: Setup.bat "C:\Path\To\UE_%ENGINE_VERSION%"
    goto :fail
)
echo       Found at "%UE%"

rem ---------------------------------------------------------------------------------------------
echo.
echo [2/5] Downloading Git LFS files...

where git >nul 2>nul
if errorlevel 1 (
    echo ERROR: git was not found. Install Git for Windows and run this script again.
    goto :fail
)
git lfs version >nul 2>nul
if errorlevel 1 (
    echo ERROR: Git LFS was not found. Install it from https://git-lfs.com and run this script again.
    goto :fail
)

git lfs install --local
if errorlevel 1 goto :fail_step
git lfs pull
if errorlevel 1 (
    echo ERROR: "git lfs pull" failed. Check your connection and your access to the repository.
    goto :fail
)

rem ---------------------------------------------------------------------------------------------
echo.
echo [3/5] Generating the Visual Studio solution...

"%UBT%" -projectfiles -project="%PROJECT%" -game -rocket -progress
if errorlevel 1 (
    echo ERROR: Could not generate the project files. Check that Visual Studio 2022 is installed with
    echo        the "Game development with C++" workload.
    goto :fail
)

rem ---------------------------------------------------------------------------------------------
echo.
echo [4/5] Building %TARGET% (Win64, Development). This can take a few minutes...

call "%BUILD_BAT%" %TARGET% Win64 Development -Project="%PROJECT%" -WaitMutex
if errorlevel 1 (
    echo ERROR: The build failed. If the Unreal editor is open, close it and run this script again.
    goto :fail
)

rem ---------------------------------------------------------------------------------------------
echo.
echo [5/5] Installing the character models and the animation Blueprint...

rem These run the editor without a window, which is why the build has to come first.
if exist "%~dp0Content\Characters\Mannequins\Meshes\SKM_Manny_Simple.uasset" (
    echo       Mannequins already installed, skipped.
) else (
    "%EDITOR_CMD%" "%PROJECT%" -run=pythonscript -script="%~dp0Tools\install_mannequins.py" -unattended -nosplash -nullrhi >nul
    if not exist "%~dp0Content\Characters\Mannequins\Meshes\SKM_Manny_Simple.uasset" (
        echo ERROR: The mannequins could not be copied from the engine. See Saved\Logs\HordeKiller.log.
        goto :fail
    )
    echo       Mannequins installed.
)

if exist "%~dp0Content\Characters\Animation\ABP_HKHuman.uasset" (
    echo       ABP_HKHuman already generated, skipped.
) else (
    "%EDITOR_CMD%" "%PROJECT%" -run=pythonscript -script="%~dp0Tools\create_anim_blueprint.py" -unattended -nosplash -nullrhi >nul
    if not exist "%~dp0Content\Characters\Animation\ABP_HKHuman.uasset" (
        echo ERROR: ABP_HKHuman could not be generated. See Saved\Logs\HordeKiller.log.
        goto :fail
    )
    echo       ABP_HKHuman generated.
)

rem ---------------------------------------------------------------------------------------------
echo.
echo === Setup complete ===
echo Open HordeKiller.uproject to start the editor, or HordeKiller.sln to work on the code.
echo.
if not defined HK_NO_PAUSE pause
endlocal
exit /b 0

:fail_step
echo ERROR: The previous command failed.

:fail
echo.
echo === Setup did not finish ===
echo.
if not defined HK_NO_PAUSE pause
endlocal
exit /b 1
