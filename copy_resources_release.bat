xcopy "resources\*" "%~1" /D /E /Y
xcopy "deps\release\Detours\LICENSE.md" "%~1\LICENSES\Detours\" /D /Y
xcopy "deps\release\rapidxml\license.txt" "%~1\LICENSES\rapidxml\" /D /Y
xcopy "deps\release\easywsclient\COPYING" "%~1\LICENSES\easywsclient\" /D /Y
xcopy "AfxHookSource2Rs\THIRDPARTY.yml" "%~1\LICENSES\advancedfx\AfxHookSource2Rs\" /D /Y
xcopy "" "%~1\LICENSES\advancedfx\" /D /Y
xcopy "shaders\build\*.acs" "%~1\resources\shaders\" /D /Y
py -3 "%~dp0\make_readme.py" "%~1"
