@echo off
set proxy=%~dp0proxyServervs.exe
set path=%~dp0cfg.json
echo %path%
%proxy% -t -c %path%
pause