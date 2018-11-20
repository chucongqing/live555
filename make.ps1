#PowerShell (New-Object System.Net.WebClient).DownloadFile('http://www.live555.com/liveMedia/public/live555-latest.tar.gz','live555-latest.tar.gz');
#"%PROGRAMFILES%\7-Zip\7z.exe" x -aoa live555-latest.tar.gz
#"%PROGRAMFILES%\7-Zip\7z.exe" x -aoa live555-latest.tar

powershell -Command "(gc .\win32config) -replace '!include    <ntwin32.mak>', '#!include    <ntwin32.mak>' | Out-File live\win32config"
powershell -Command "(gc .\win32config) -replace 'c:\\Program Files\\DevStudio\\Vc', 'C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Tools\MSVC\14.16.27023' | Out-File .\win32config"
powershell -Command "(gc .\win32config) -replace '\(TOOLS32\)\\bin\\cl', '(TOOLS32)\bin\HostX86\x64\cl' | Out-File .\win32config"
powershell -Command "(gc .\win32config) -replace 'LINK =			\$\(link\) -out:', 'LINK = link ws2_32.lib /out:' | Out-File .\win32config"
powershell -Command "(gc .\win32config) -replace 'LIBRARY_LINK =		lib -out:', 'LIBRARY_LINK = lib /out:' | Out-File .\win32config"
powershell -Command "(gc .\win32config) -replace 'msvcirt.lib', 'msvcrt.lib' | Out-File .\win32config"

call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\vsdevcmd" -arch=x64

#cd live

call genWindowsMakefiles

cd liveMedia
del *.obj *.lib
nmake /B -f liveMedia.mak
cd ..\groupsock
del *.obj *.lib
nmake /B -f groupsock.mak
cd ..\UsageEnvironment
del *.obj *.lib
nmake /B -f UsageEnvironment.mak
cd ..\BasicUsageEnvironment
del *.obj *.lib
nmake /B -f BasicUsageEnvironment.mak
cd ..\testProgs
del *.obj *.lib
nmake /B -f testProgs.mak
cd ..\mediaServer
del *.obj *.lib
nmake /B -f mediaServer.mak
cd ..

pause