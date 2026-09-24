@echo off
rem Configure, build, test and package with the VS 2022 x64 toolset.
setlocal
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >nul || exit /b 1
if "%VCPKG_ROOT%"=="" set "VCPKG_ROOT=%USERPROFILE%\vcpkg"
cd /d "%~dp0"
set "LRB_VERSION=0.2.0"
cmake --preset vs2022-release || exit /b 1
cmake --build --preset vs2022-release || exit /b 1
ctest --test-dir "G:/skyrim-build/LockpickRevealBar/build/release-msvc-%VCToolsVersion%" --output-on-failure || exit /b 1
cmake --build --preset vs2022-release --target package_mod || exit /b 1
set "ZIP=G:\skyrim-build\LockpickRevealBar\LockpickRevealBar-%LRB_VERSION%.zip"
if exist "%ZIP%" del /q "%ZIP%"
powershell -NoProfile -Command "Compress-Archive -Path 'G:\skyrim-build\LockpickRevealBar\package\SKSE' -DestinationPath '%ZIP%'" || exit /b 1
echo Package: G:\skyrim-build\LockpickRevealBar\package
echo MO2 zip: %ZIP%
