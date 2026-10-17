@IF ["%~1"]==[""] (
	@echo Usage: %0 ^<HLAE build bin path to create locales in^>
	@EXIT /B 1
)

@echo Requires GNU gettext for Windows to be installed.

@echo "Project: hlae"

mkdir "%~1\locales"
cd "%~1\locales"

for %%l in (de fi fr-FR hu it ja nl pl pt-BR pt-PT ru zh-CN) do (
	mkdir "%%l\hlae"
	cd "%%l\hlae"
	msgfmt --output-file=messages.mo "%~dp0deps\release\l10n\locales\%%l\hlae\messages.po"
	cd ..\..
)

cd "%~dp0"
