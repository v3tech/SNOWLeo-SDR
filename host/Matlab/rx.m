clc;
clear;
close all;
warning off;
cyc=1;
%% open control port
ctrl_link = udp('192.168.1.10', 5006);
fopen(ctrl_link);
%% open data port
data_link = tcpip('192.168.1.10', 5004);
set(data_link,'InputBufferSize',256*1024);
set(data_link,'OutputBufferSize',16*1024);
fopen(data_link);

%% send rx freq
freq_hex=dec2hex(1090e6,8);
rx_freq=[0 0 hex2dec('18') hex2dec('f0') hex2dec(freq_hex(7:8)) hex2dec(freq_hex(5:6)) hex2dec(freq_hex(3:4)) hex2dec(freq_hex(1:2))];
fwrite(ctrl_link,rx_freq,'uint8');

%% send rx vga
rx_vga=[0 hex2dec('d0') hex2dec('20') hex2dec('f0') 0 0 0 hex2dec('03')];
fwrite(ctrl_link,rx_vga,'uint8');

%% send rx dc set cmd
% fwrite(ctrl_link,[170 115 hex2dec('23') hex2dec('f0') 0 0 0 0],'uint8');
% fwrite(ctrl_link,[0 0 hex2dec('23') hex2dec('f0') 0 0 0 0],'uint8');

%% send handshake cmd
size_hex=dec2hex(8*1024,8);
handshake=[2 1 hex2dec('16') hex2dec('f0') hex2dec(size_hex(7:8)) hex2dec(size_hex(5:6)) hex2dec(size_hex(3:4)) hex2dec(size_hex(1:2))];
fwrite(ctrl_link,handshake,'uint8');
pause(0.5);

while (1)
    %% read 256*1024 bytes data from zing
    data = fread(data_link,8*1024,'uint8');
     
    %% receive
    figure(1);clf;
    datah=data(2:2:end);
    datal=data(1:2:end);
    datah_hex=dec2hex(datah,2);
    datal_hex=dec2hex(datal,2);
    data_hex(:,1:2)=datah_hex;
    data_hex(:,3:4)=datal_hex;
    dataun=hex2dec(data_hex);
    datain=dataun-(dataun>32767)*65536;
    a1=datain(2:2:end);
    a2=datain(1:2:end);
    [uV sV] = memory;
    mem=round(uV.MemUsedMATLAB/2^20);
    subplot(221);
    plot(a1,'b');
    hold on
    plot(a2,'r');
    subplot(222);
    plot(a1,a2);
    title(['ÐÅºÅ²¨ÐÎ cyc=',num2str(cyc),';mem=',num2str(mem),'MB']);
%     pause;
    cyc=cyc+1;
    %clearvars -except cyc;
    %clear maplemex
end

%% close all link
fclose(data_link);
delete(data_link);
clear data_link;

fclose(ctrl_link);
delete(ctrl_link);
clear ctrl_link;