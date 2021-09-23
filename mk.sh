@echo off
cls
cd build
echo building the sw....
ninja -j 16
if errorlevel 1 (
   echo MAKE failed with return code %errorlevel%
   cd ..
   exit /b %errorlevel%
)

@REM : make run
echo loading the software ....
start /WAIT "loading the firmware into the  Teensy board" "%TEENSY_TOOLS%\teensy_reboot.exe"
if errorlevel 1 (
   echo REBOOT failed with return code %errorlevel%
   cd ..
   exit /b %errorlevel%
)


cd ..
cd lib-vs2019-x64\release
@REM cls
echo 'make' was OK; starting USB console ..

PING localhost -n 6 >NUL

echo starting....
tusb.exe %1
if errorlevel 1 (
   echo USB console failed with return code %errorlevel%
)
cd ..
cd ..
