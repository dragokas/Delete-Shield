@echo off
cd /d "%~dp0"

:: CONSTANTS
set compressor=%ProgramFiles%\7-Zip\7z.exe
set prj=DeleteShield
set release_dir=%~dp0x64\Release

rd /s /q "%release_dir%"

call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
msbuild %prj%.slnx /p:Configuration=Release /p:Platform=x64

if not exist "%release_dir%\%prj%.exe" (
  echo Failed to build the solution!
  pause>NUL & exit /b
)

cd /d "%release_dir%"

:: Remove logfile
2>nul del "%prj%.pdb"

:: Prepare release archive
if exist "%prj%.zip" del "%prj%.zip"
"%compressor%" a -tzip -mx=9 -r "%prj%.zip" .\*
move "%prj%.zip" "%~dp0"

pause