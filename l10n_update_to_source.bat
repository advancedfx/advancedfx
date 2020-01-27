echo Requires GNU gettext for Windows to be installed.


echo "Project: hlae"

mkdir build\Release\bin\locales
cd build\Release\bin\locales

for %%l in (de fi hu ja nl pt-PT ru zh-CN) do (
	mkdir %%l\hlae
	cd %%l\hlae
	msgfmt --output-file=messages.mo ..\..\..\..\..\..\l10n\locales\%%l\hlae\messages.po
	cd ..\..
)

cd ..\..\..\..


echo "Project : installer\HlaeCore"

cd installer\HlaeCore
py -3.8 -m pip install polib 
py -3.8 ..\..\deps\wxl-po-tools\po2wxl.py -f -p -1 -l LangId ..\..\l10n\locales\de\installer\HlaeCore\messages.po lang/de-DE.wxl
py -3.8 ..\..\deps\wxl-po-tools\po2wxl.py -f -p -1 -l LangId ..\..\l10n\locales\fi\installer\HlaeCore\messages.po lang/fi-FI.wxl
py -3.8 ..\..\deps\wxl-po-tools\po2wxl.py -f -p -1 -l LangId ..\..\l10n\locales\hu\installer\HlaeCore\messages.po lang/hu-HU.wxl
py -3.8 ..\..\deps\wxl-po-tools\po2wxl.py -f -p -1 -l LangId ..\..\l10n\locales\ja\installer\HlaeCore\messages.po lang/ja-JP.wxl
py -3.8 ..\..\deps\wxl-po-tools\po2wxl.py -f -p -1 -l LangId ..\..\l10n\locales\nl\installer\HlaeCore\messages.po lang/nl-NL.wxl
py -3.8 ..\..\deps\wxl-po-tools\po2wxl.py -f -p -1 -l LangId ..\..\l10n\locales\pt-PT\installer\HlaeCore\messages.po lang/pt-PT.wxl
py -3.8 ..\..\deps\wxl-po-tools\po2wxl.py -f -p -1 -l LangId ..\..\l10n\locales\ru\installer\HlaeCore\messages.po lang/ru-RU.wxl
py -3.8 ..\..\deps\wxl-po-tools\po2wxl.py -f -p -1 -l LangId ..\..\l10n\locales\zh-CN\installer\HlaeCore\messages.po lang/zh-CN.wxl
cd ..\..


echo "Project : installer\setup"

cd installer\setup
py -3.8 -m pip install polib 
py -3.8 ..\..\deps\wxl-po-tools\po2wxl.py -f -p -1 -l LangId ..\..\l10n\locales\de\installer\setup\messages.po lang/HyperlinkTheme_de-DE.wxl
py -3.8 ..\..\deps\wxl-po-tools\po2wxl.py -f -p -1 -l LangId ..\..\l10n\locales\fi\installer\setup\messages.po lang/HyperlinkTheme_fi-FI.wxl
py -3.8 ..\..\deps\wxl-po-tools\po2wxl.py -f -p -1 -l LangId ..\..\l10n\locales\hu\installer\setup\messages.po lang/HyperlinkTheme_hu-HU.wxl
py -3.8 ..\..\deps\wxl-po-tools\po2wxl.py -f -p -1 -l LangId ..\..\l10n\locales\ja\installer\setup\messages.po lang/HyperlinkTheme_ja-JP.wxl
py -3.8 ..\..\deps\wxl-po-tools\po2wxl.py -f -p -1 -l LangId ..\..\l10n\locales\nl\installer\setup\messages.po lang/HyperlinkTheme_nl-NL.wxl
py -3.8 ..\..\deps\wxl-po-tools\po2wxl.py -f -p -1 -l LangId ..\..\l10n\locales\pt-PT\installer\setup\messages.po lang/HyperlinkTheme_pt-PT.wxl
py -3.8 ..\..\deps\wxl-po-tools\po2wxl.py -f -p -1 -l LangId ..\..\l10n\locales\ru\installer\setup\messages.po lang/HyperlinkTheme_ru-RU.wxl
py -3.8 ..\..\deps\wxl-po-tools\po2wxl.py -f -p -1 -l LangId ..\..\l10n\locales\zh-CN\installer\setup\messages.po lang/HyperlinkTheme_zh-CN.wxl
cd ..\..
