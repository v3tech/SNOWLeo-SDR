clc;
clear all;
close all;
%% open control port
ctrl_link = udp('192.168.1.10', 5006);
fopen(ctrl_link);
%% open data port
data_link = tcpip('192.168.1.10', 5005);
set(data_link,'InputBufferSize',16*1024);
set(data_link,'OutputBufferSize',64*1024);
fopen(data_link);

%% send tx sample rate set cmd
samp_rate=dec2hex(10e6,8);
tx_samp=[0 0 hex2dec('24') hex2dec('f0') hex2dec(samp_rate(7:8)) hex2dec(samp_rate(5:6)) hex2dec(samp_rate(3:4)) hex2dec(samp_rate(1:2))];
fwrite(ctrl_link,tx_samp,'uint8');
%% send tx freq set cmd
freq_hex=dec2hex(1090e6,8);
tx_freq=[0 0 hex2dec('17') hex2dec('f0') hex2dec(freq_hex(7:8)) hex2dec(freq_hex(5:6)) hex2dec(freq_hex(3:4)) hex2dec(freq_hex(1:2))];
fwrite(ctrl_link,tx_freq,'uint8');
%% send tx vga set cmd
tx_vga=[hex2dec('0') hex2dec('00') hex2dec('19') hex2dec('f0') 0 0 hex2dec('00') hex2dec('0b')];
fwrite(ctrl_link,tx_vga,'uint8');

%% send tx dc set cmd
% fwrite(ctrl_link,[170 115 hex2dec('21') hex2dec('f0') 0 0 0 0],'uint8');
% fwrite(ctrl_link,[0 0 hex2dec('21') hex2dec('f0') 0 0 0 0],'uint8');
%% send handshake cmd
handshake=[2 0 hex2dec('16') hex2dec('f0') 0 0 0 0];
fwrite(ctrl_link ,handshake, 'uint8');
pause(0.5);

%% main
fid1=fopen('F:\matlab_work\tone_16bit.dat','r');
txdata=fread(fid1,'int16');
txd1=(txdata<0)*65536+txdata;
txd2=dec2hex(txd1,4);
txd3=txd2(:,1:2);
txd4=txd2(:,3:4);
txd5=hex2dec(txd3);
txd6=hex2dec(txd4);
txd7=zeros(length(txd6)*2,1);
txd7(1:2:end)=txd6;
txd7(2:2:end)=txd5;
fclose('all');

%% Write data to the zing and read from the host.
fwrite(data_link,txd7,'uint8');

%% close all link
fclose(data_link);
delete(data_link);
clear data_link;
fclose(ctrl_link);
delete(ctrl_link);
clear ctrl_link;
