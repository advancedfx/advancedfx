xcopy "resources\*" "%~1" /D /E /Y
xcopy "deps\release\Detours\LICENSE.md" "%~1\LICENSES\Detours\" /D /Y
xcopy "deps\release\rapidxml\license.txt" "%~1\LICENSES\rapidxml\" /D /Y
xcopy "deps/release/easywsclient\COPYING" "%~1\LICENSES\easywsclient\" /D /Y
xcopy "LICENSE" "%~1\LICENSES\advancedfx\" /D /Y
xcopy "shaders\build\*.acs" "%~1\resources\AfxHookSource\shaders\" /D /Y
py -3 "%~dp0\make_readme.py" "%~1"