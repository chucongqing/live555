set opt1=%1

IF "%opt1%"=="1" (
    cd ../liveMedia
    del *.obj *.lib
    nmake /B -f liveMedia.mak
) ELSE (
    echo fuck
)

cd /d %~dp0
del *.obj
nmake /B -f proxyServer.mak
