@echo off
setlocal enabledelayedexpansion
set "driverNonPackedPath=C:\Users\Lidoma\Desktop\plNIC\Driver\plNICDriver\plNICDriver\bin\Release\net6.0\win-x64\plNICDriver.exe"
set "driverPacketPath=C:\Users\Lidoma\Desktop\plNIC\Driver\plNICDriver\plNICDriver\bin\Release\net6.0\win-x64\publish\win-x64\plNICDriver.exe"
set "programName=plNICDriver"
for /l %%x in (11, 1, 12) do (
	set "name=%programName%_%%x"
	echo !name!
	set "port=COM%%x"
	echo !port!

	REM start %name% %driverPacketPath% !port! %%x		
	REM call :pause 500
	
	start %name% %driverNonPackedPath% !port!	%%x
	timeout 1
)

pause

taskkill /IM %programName%.exe /T /F	



