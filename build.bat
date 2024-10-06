@echo off
set CFlags=-DDEBUG -DSLOW=1 -W4 -MT -wd4100 -wd4189 -wd4201 -wd4505 -wd4838 -wd4458 -nologo -Oi -Od -fp:fast -GR- -Gm- -Z7 -EHa 

set LDLibs= gdi32.lib msvcrt.lib winmm.lib User32.lib D3D11.lib D3DCompiler.lib dxgi.lib

set LDFlags=-incremental:no /NODEFAULTLIB:libcmt  

set Optimize=/0i /02 /fp:fast


set ProjectDir=%CD%
set TestDir=%ProjectDir%\tests
set BuildDir=%ProjectDir%\build
set SourceDir=%ProjectDir%\src
set VendorInclude=%ProjectDir%\vendor\include
set VendorLibs=%ProjectDir%\vendor\lib 

set ObjDir=%BuildDir%\obj

set DATETIME=%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~1,1%%time:~3,2%%time:~6,2%

SET GameFile=%SourceDir%\game.cpp
SET PlatformFiles=%SourceDir%\win32_platform.cpp

mkdir %ObjDir% 2> NUL

REM Clean up	
   del %BuildDir%\*.pdb

REM Game code
	@REM cl %CFlags% /I%ProjectDir% /I%IncludeDir% -Fo:%ObjDir% -Fd:%BuildDir% %ProjectDir%\src\game.cpp /link %LDFlags% /DLL /EXPORT:GameUpdateAndRender /EXPORT:GameOutputSound /OUT:%BuildDir%\game.dll /PDB:%BuildDir%\game_%DATETIME%.pdb
	@REM cl -DLIBRARY_EXPORTS %CFlags% /I%SourceInclude% /I%VendorInclude% /Fo:%ObjDir% /Fd:%ObjDir% %GameFile% /link /DLL %LDFlags% %LDLibs% /LIBPATH:%VendorLibs% /OUT:%BuildDir%\game_temp.dll /PDB:%BuildDir%\game_%DATETIME%.pdb

REM Platform code
	cl %CFlags% /I%VendorInclude% /Fo:%ObjDir% /Fd:%ObjDir% %PlatformFiles% /link %LDFlags% %LDLibs% /LIBPATH:%VendorLibs% /OUT:%BuildDir%\GOD.exe 

REM Test code
	@REM cl %CFlags% /I%ProjectDir% /I%IncludeDir% /Fo:%ObjDir% /Fd:%ObjDir% %TestDir%\test.cpp /link %LDFlags% %LDLibs% /OUT:%BuildDir%\test.exe

REM Copy Assets
	xcopy /S /Q %ProjectDir%\assets\ %BuildDir%\assets\ /Y
	xcopy /Q %BuildDir%\game_temp.dll %BuildDir%\game.dll /Y
	del %BuildDir%\game_temp.dll
