@echo off
set proxy=.\live555ProxyServer.exe
set source1=rtsp://admin:admin123@192.168.1.17:554/Streaming/Channels/331
set source2=rtsp://admin:admin123@192.168.1.16:554/h264/ch33/main/av_stream
set user=admin
set password=admin123
set userR=admin
set passwordR=admin123
set ip1=172.20.47.132
set ip2=192.168.1.132
set port=9000
rem %proxy%  -p %port%  -u %user% %password% -ip %ip% %source1%
%proxy% -v -p %port% -ip %ip1% %source2%