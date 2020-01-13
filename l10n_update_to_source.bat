echo "Project : installer\HlaeCore"
cd installer\HlaeCore
py -3.8 -m pip install polib 
py -3.8 ..\..\deps\wxl-po-tools\po2wxl.py -f -p -1 -l LangId ..\..\l10n\locales\de-DE\installer\HlaeCore\messages.po lang/de-DE.wxl
py -3.8 ..\..\deps\wxl-po-tools\po2wxl.py -f -p -1 -l LangId ..\..\l10n\locales\fi-FI\installer\HlaeCore\messages.po lang/fi-FI.wxl
py -3.8 ..\..\deps\wxl-po-tools\po2wxl.py -f -p -1 -l LangId ..\..\l10n\locales\pt-PT\installer\HlaeCore\messages.po lang/pt-PT.wxl
py -3.8 ..\..\deps\wxl-po-tools\po2wxl.py -f -p -1 -l LangId ..\..\l10n\locales\zh-CN\installer\HlaeCore\messages.po lang/zh-CN.wxl
cd ..\..

echo "Project : installer\setup"
cd installer\setup
py -3.8 -m pip install polib 
py -3.8 ..\..\deps\wxl-po-tools\po2wxl.py -f -p -1 -l LangId ..\..\l10n\locales\de-DE\installer\setup\messages.po lang/HyperlinkTheme_de-DE.wxl
py -3.8 ..\..\deps\wxl-po-tools\po2wxl.py -f -p -1 -l LangId ..\..\l10n\locales\fi-FI\installer\setup\messages.po lang/HyperlinkTheme_fi-FI.wxl
py -3.8 ..\..\deps\wxl-po-tools\po2wxl.py -f -p -1 -l LangId ..\..\l10n\locales\pt-PT\installer\setup\messages.po lang/HyperlinkTheme_pt-PT.wxl
py -3.8 ..\..\deps\wxl-po-tools\po2wxl.py -f -p -1 -l LangId ..\..\l10n\locales\zh-CN\installer\setup\messages.po lang/HyperlinkTheme_zh-CN.wxl
cd ..\..
