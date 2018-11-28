@echo off
set proxy=%~dp0proxyServer.exe
set path=%~dp0cfg.json
echo %path%
%proxy% -c %path%
pause