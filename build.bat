@echo off
set CFlags=-DDEBUG -DSLOW=1 -D_CRT_SECURE_NO_WARNINGS -W4 -MT -wd4100 -wd4189 -wd4201 -wd4505 -wd4838 -wd4324 -nologo -Oi -Od -fp:fast -GR- -Gm- -Z7 -EHa

set LDLibs=gdi32.lib msvcrt.lib winmm.lib User32.lib D3D11.lib D3DCompiler.lib dxgi.lib vulkan-1.lib

set LDFlags=-incremental:no /NODEFAULTLIB:libcmt

set Optimize=/0i /02 /fp:fast


set ProjectDir=%CD%
set TestDir=%ProjectDir%\tests
set BuildDir=%ProjectDir%\build
set BuildAssetDir=%BuildDir%\asset
set BuildShaderDir=%BuildDir%\shader

if not exist %BuildAssetDir% mkdir %BuildAssetDir%
if not exist %BuildShaderDir% mkdir %BuildShaderDir%

set SourceDir=%ProjectDir%\src
set AssetDir=%ProjectDir%\asset
set ShaderDir=%SourceDir%\shader

set VendorInclude=/I%SourceDir%\vendor\include /I%VULKAN_SDK%\Include
set VendorLibs=/LIBPATH:%SourceDir%\vendor\lib /LIBPATH:%VULKAN_SDK%\Lib

set ObjDir=%BuildDir%\obj

set DATETIME=%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~1,1%%time:~3,2%%time:~6,2%

SET GameFile=%SourceDir%\game.cpp
SET PlatformFiles=%SourceDir%\win32_platform.cpp
SET AssetPackerFile=%SourceDir%\hz_asset_builder.cpp

mkdir %ObjDir% 2> NUL

REM Clean up	
   del %BuildDir%\*.pdb

REM Compile Shader
	@REM fxc /Od /Zi /T vs_5_0 /Fo %BuildDir%\shaders\default_vs.fxo %ShaderDir%\default_vs.hlsl
	@REM fxc /Od /Zi /T ps_5_0 /Fo %BuildDir%\shaders\default_ps.fxo %ShaderDir%\default_ps.hlsl
	@REM fxc /Od /Zi /T vs_5_0 /Fo %BuildDir%\shaders\voxel_vs.fxo %ShaderDir%\voxel_vs.hlsl
	@REM fxc /Od /Zi /T ps_5_0 /Fo %BuildDir%\shaders\voxel_ps.fxo %ShaderDir%\voxel_ps.hlsl
	@REM fxc /Od /Zi /T ps_5_0 /Fo %BuildDir%\shaders\voxel_cs.fxo %ShaderDir%\voxel_cs.hlsl
	@REM dxc /Od /Zi /T -spirv /T cs_5_0 /Fo %BuildDir%\shader\compute.spv %ShaderDir%\compute.hlsl
	glslc -g %ShaderDir%\compute.comp -o %BuildDir%\shader\compute.spv

REM Asset Packer code
	cl %CFlags% %VendorInclude% /Fo:%ObjDir% /Fd:%ObjDir% %AssetPackerFile% /link %LDFlags% %LDLibs% assimp-vc143-mt.lib %VendorLibs% /OUT:%BuildDir%\AssetPacker.exe 


@REM REM Game code
	cl -DLIBRARY_EXPORTS %CFlags% %VendorInclude% /Fo:%ObjDir% /Fd:%ObjDir% %GameFile% /link /DLL %LDFlags% %LDLibs% %VendorLibs% /OUT:%BuildDir%\game_temp.dll /PDB:%BuildDir%\game_%DATETIME%.pdb

REM Platform code
	cl %CFlags% %VendorInclude% /Fo:%ObjDir% /Fd:%ObjDir% %PlatformFiles% /link %LDFlags% %LDLibs% %VendorLibs% /OUT:%BuildDir%\GOD.exe 

REM Test code
	@REM cl %CFlags% /I%ProjectDir% /I%IncludeDir% /Fo:%ObjDir% /Fd:%ObjDir% %TestDir%\test.cpp /link %LDFlags% %LDLibs% /OUT:%BuildDir%\test.exe

REM Build Assets
	echo Build Assets
	for /R %SourceDir%\vendor\lib %%f in (*.dll) do (
		xcopy /Q %%f %BuildDir%\%%~nf.dll /Y
	)


	for /R %AssetDir% %%f in (*) do (
		%BuildDir%\AssetPacker.exe %%f %BuildAssetDir%\%%~nf.hza 
	)

REM Reload Dll
	echo Reload Dll
	xcopy /Q %BuildDir%\game_temp.dll %BuildDir%\game.dll /Y
	del %BuildDir%\game_temp.dll
