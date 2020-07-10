If you want to use the new FFMPEG feature in HLAE, then you have two options:


A) Install FFMPEG in this folder:

you need to download FFMPEG from here:
https://ffmpeg.org/download.html#build-windows

and place the contents of the root folder in that download in this folder,
so you would end up with e.g. "./bin/ffmpeg.exe".


B) Reference FFMPEG:

Create ffmpeg.ini file in this folder with the following contents:
; BEGIN FILE CONTENTS
[Ffmpeg]
Path=C:\Users\Dominik\Desktop\ffmpeg\bin\ffmpeg.exe
; END FILE CONTENTS