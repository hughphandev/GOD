@echo off
set CFlags=-DDEBUG -DSLOW=1 -W4 -MT -wd4100 -wd4189 -wd4201 -wd4505 -wd4838 -wd4324 -nologo -Oi -Od -fp:fast -GR- -Gm- -Z7 -EHa 

set LDLibs= gdi32.lib msvcrt.lib winmm.lib User32.lib D3D11.lib D3DCompiler.lib dxgi.lib

set LDFlags=-incremental:no /NODEFAULTLIB:libcmt  

set Optimize=/0i /02 /fp:fast


set ProjectDir=%CD%
set TestDir=%ProjectDir%\tests
set BuildDir=%ProjectDir%\build
set BuildAssetDir=%BuildDir%\asset

set SourceDir=%ProjectDir%\src
set AssetDir=%ProjectDir%\asset
set ShaderDir=%SourceDir%\shader
set VendorInclude=%SourceDir%\vendor\include
set VendorLibs=%SourceDir%\vendor\lib 

set ObjDir=%BuildDir%\obj

set DATETIME=%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~1,1%%time:~3,2%%time:~6,2%

SET GameFile=%SourceDir%\game.cpp
SET PlatformFiles=%SourceDir%\win32_platform.cpp
SET AssetPackerFile=%SourceDir%\hz_asset_builder.cpp

mkdir %ObjDir% 2> NUL

REM Clean up	
   del %BuildDir%\*.pdb

REM Compile Shader
	fxc /Od /Zi /T vs_5_0 /E:vs_main /Fo %BuildDir%\default_vs.fxo %ShaderDir%\default_vs.hlsl
	fxc /Od /Zi /T ps_5_0 /E:ps_main /Fo %BuildDir%\default_ps.fxo %ShaderDir%\default_ps.hlsl

REM Asset Packer code
	cl %CFlags% /I%VendorInclude% /Fo:%ObjDir% /Fd:%ObjDir% %AssetPackerFile% /link %LDFlags% %LDLibs% assimp-vc143-mt.lib /LIBPATH:%VendorLibs% /OUT:%BuildDir%\AssetPacker.exe 


@REM REM Game code
	cl -DLIBRARY_EXPORTS %CFlags% /I%VendorInclude% /Fo:%ObjDir% /Fd:%ObjDir% %GameFile% /link /DLL %LDFlags% %LDLibs% /LIBPATH:%VendorLibs% /OUT:%BuildDir%\game_temp.dll /PDB:%BuildDir%\game_%DATETIME%.pdb

REM Platform code
	cl %CFlags% /I%VendorInclude% /Fo:%ObjDir% /Fd:%ObjDir% %PlatformFiles% /link %LDFlags% %LDLibs% /LIBPATH:%VendorLibs% /OUT:%BuildDir%\GOD.exe 

REM Test code
	@REM cl %CFlags% /I%ProjectDir% /I%IncludeDir% /Fo:%ObjDir% /Fd:%ObjDir% %TestDir%\test.cpp /link %LDFlags% %LDLibs% /OUT:%BuildDir%\test.exe

REM Build Assets
	echo Build Assets

	for %%f in (%AssetDir%\*.glb) do (
		%BuildDir%\AssetPacker.exe %BuildAssetDir%\%%~nf.hza %%f
	)

REM Reload Dll
	echo Reload Dll
	xcopy /Q %BuildDir%\game_temp.dll %BuildDir%\game.dll /Y
	del %BuildDir%\game_temp.dll
