@IF ["%~1"] == [""] goto :help
@IF ["%~2"] == [""] goto :help
@IF ["%~3"] == [""] goto :help

setlocal EnableDelayedExpansion

cd "%~dp0"

@echo ==================================================
@echo Installing WiX Toolset v5.0.2 ...
@echo ==================================================
dotnet tool install --global wix --version 5.0.2
@IF !errorlevel! NEQ 0 EXIT /B 1

@echo ==================================================
@echo Installing WiX Toolset v5.0.2 dependencies ...
@echo ==================================================
wix extension add -g WixToolset.Bal.wixext/5.0.2
@IF !errorlevel! NEQ 0 EXIT /B 1
wix extension add -g WixToolset.Netfx.wixext/5.0.2
@IF !errorlevel! NEQ 0 EXIT /B 1
wix extension add -g WixToolset.UI.wixext/5.0.2
@IF !errorlevel! NEQ 0 EXIT /B 1
wix extension add -g WixToolset.Util.wixext/5.0.2
@IF !errorlevel! NEQ 0 EXIT /B 1

@echo ==================================================
@echo Building HlaeFfmpegExtension ...
@echo ==================================================
mkdir "%~1\HlaeFfmpegExtension"
cd "%~1\HlaeFfmpegExtension"
cd "%~dp0HlaeFfmpegExtension"
"%~3" -t:build -restore "%~dp0HlaeFfmpegExtension\HlaeFfmpegExtension.csproj" "-property:Configuration=Release" "-property:Platform=x86" "-property:OutputPath=%~1\HlaeFfmpegExtension"
@IF !errorlevel! NEQ 0 EXIT /B 1

@echo ==================================================
@echo Building HlaeCoreExtension ...
@echo ==================================================
mkdir "%~1\HlaeCoreExtension"
cd "%~1\HlaeCoreExtension"
cd "%~dp0HlaeCoreExtension"
"%~3" -t:build -restore "%~dp0HlaeCoreExtension\HlaeCoreExtension.csproj" "-property:Configuration=Release" "-property:Platform=x86" "-property:OutputPath=%~1\HlaeCoreExtension"
@IF !errorlevel! NEQ 0 EXIT /B 1

@echo ==================================================
@echo Building HlaeFfmpeg ...
@echo ==================================================
@FOR %%l IN (en-US de-DE fi-FI hu-HU it-IT ja-JP nl-NL pl-PL pt-BR pt-PT ru-RU tr-TR zh-CN) DO (
    mkdir "%~1\HlaeFfmpeg\%%l"
    mkdir "%~1\HlaeFfmpeg\%%l\tmp"
    cd "%~1\HlaeFfmpeg\%%l"
    wix build -arch x86 -defaultcompressionlevel high -culture "%%l" -intermediateFolder "%~1\HlaeFfmpeg\%%l\tmp" -o "%~1\HlaeFfmpeg\%%l\HlaeFfmpeg.msi" -ext WixToolset.UI.wixext/5.0.2 -d "var.HlaeFfmpegExtension.TargetDir=%~1\HlaeFfmpegExtension" "%~dp0HlaeFfmpeg\Package.wxs" "%~dp0HlaeFfmpeg\MyWixUI_InstallDir.wxs" "%~dp0HlaeFfmpeg\FfmpegDlg.wxs" "%~dp0HlaeFfmpeg\lang\%%l.wxl" "%~dp0shared\Dependency\lang\%%l.wxl"
    @IF !errorlevel! NEQ 0 EXIT /B 1
    wix msi validate "%~1\HlaeFfmpeg\%%l\HlaeFfmpeg.msi" -sice "ICE03"
    @IF !errorlevel! NEQ 0 EXIT /B 1
)
cd "%~1\HlaeFfmpeg"
@FOR %%l IN (de-DE fi-FI hu-HU it-IT ja-JP nl-NL pl-PL pt-BR pt-PT ru-RU tr-TR zh-CN) DO (
    wix msi transform -t language -intermediateFolder "%~1\HlaeFfmpeg\%%l\tmp" "%~1\HlaeFfmpeg\en-US\HlaeFfmpeg.msi" "%~1\HlaeFfmpeg\%%l\HlaeFfmpeg.msi" -o "%~1\HlaeFfmpeg\%%l\HlaeFfmpeg.mst"
    @IF !errorlevel! NEQ 0 EXIT /B 1
)

@echo ==================================================
@echo Building HlaeCore ...
@echo ==================================================
@FOR %%l IN (en-US de-DE fi-FI hu-HU it-IT ja-JP nl-NL pl-PL pt-BR pt-PT ru-RU tr-TR zh-CN) DO (
    mkdir "%~1\HlaeCore\%%l"
    mkdir "%~1\HlaeCore\%%l\tmp"
    cd "%~1\HlaeCore\%%l"
    wix build -arch x86 -defaultcompressionlevel high -culture "%%l" -intermediateFolder "%~1\HlaeCore\%%l\tmp" -o "%~1\HlaeCore\%%l\HlaeCore.msi" -ext WixToolset.UI.wixext/5.0.2 -d "var.HlaeCoreExtension.TargetDir=%~1\HlaeCoreExtension"  -d "var.Configuration=Release" "%~dp0HlaeCore\Package.wxs" "%~dp0HlaeCore\MyWixUI_Mondo.wxs" "%~dp0HlaeCore\MyBrowseDlg.wxs" "%~dp0HlaeCore\MyCustomizeDlg.wxs" "%~dp0HlaeCore\lang\%%l.wxl" "%~dp0shared\Dependency\lang\%%l.wxl"
    @IF !errorlevel! NEQ 0 EXIT /B 1
    wix msi validate "%~1\HlaeCore\%%l\HlaeCore.msi"
    @IF !errorlevel! NEQ 0 EXIT /B 1
)
cd "%~1\HlaeCore"
@FOR %%l IN (de-DE fi-FI hu-HU it-IT ja-JP nl-NL pl-PL pt-BR pt-PT ru-RU tr-TR zh-CN) DO (
    wix msi transform -t language -intermediateFolder "%~1\HlaeCore\%%l\tmp" "%~1\HlaeCore\en-US\HlaeCore.msi" "%~1\HlaeCore\%%l\HlaeCore.msi" -o "%~1\HlaeCore\%%l\HlaeCore.mst"
    @IF !errorlevel! NEQ 0 EXIT /B 1
)

@echo ==================================================
@echo Building DeleteHlaeAppData ...
@echo ==================================================
mkdir "%~1\DeleteHlaeAppData"
cd "%~1\DeleteHlaeAppData"
cd "%~dp0DeleteHlaeAppData"
"%~3" -t:build -restore "%~dp0DeleteHlaeAppData\DeleteHlaeAppData.csproj" "-property:Configuration=Release" "-property:Platform=x86" "-property:OutputPath=%~1\DeleteHlaeAppData"
@IF !errorlevel! NEQ 0 EXIT /B 1

@echo ==================================================
@echo Building UninstallHlaeWixV3 ...
@echo ==================================================
mkdir "%~1\UninstallHlaeWixV3"
cd "%~1\UninstallHlaeWixV3"
cd "%~dp0UninstallHlaeWixV3"
"%~3" -t:build -restore "%~dp0UninstallHlaeWixV3\UninstallHlaeWixV3.csproj" "-property:Configuration=Release" "-property:Platform=x86" "-property:OutputPath=%~1\UninstallHlaeWixV3"
@IF !errorlevel! NEQ 0 EXIT /B 1

@echo ==================================================
@echo Building Setup ...
@echo ==================================================
@IF !errorlevel! NEQ 0 EXIT /B 1
mkdir "%~1\setup\%%l"
mkdir "%~1\setup\%%l\tmp"
cd "%~dp0setup"
wix build -arch x86 -culture en-US -culture de-DE -culture fi-FI -culture hu-HU -culture it-IT -culture ja-JP -culture nl-NL -culture pl-PL -culture pt-BR -culture pt-PT -culture ru-RU -culture zh-CN -ext "%USERPROFILE%\.wix\extensions\WixToolset.Bal.wixext\5.0.2\wixext5\WixToolset.BootstrapperApplications.wixext.dll" -ext "WixToolset.Netfx.wixext/5.0.2" -ext "WixToolset.Util.wixext/5.0.2" -d "var.InstallerBuildDir=%~1" -intermediateFolder "%1\setup\tmp" -outputtype bundle -o "%2\HLAE_Setup.exe" "Bundle.wxs"
@IF !errorlevel! NEQ 0 EXIT /B 1

cd "%~dp0"
@EXIT /B 0


:help
@echo Usage: %0 ^<build folder^> ^<install folder^> ^<msbuild.exe path^>
@EXIT /B 1