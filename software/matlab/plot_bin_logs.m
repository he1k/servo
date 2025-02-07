%% Plot and compare binary logs
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Init
clear all;
clc;
close all;
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% Load binary log data
logs_path = '/home/farfar/Documents/servo/software/matlab/logs/';
log_name = 'log6.bin';
log_path = strcat(logs_path,log_name)
fp = fopen(log_path);
if (fp == -1)
    disp('ERROR OPENING LOG FILE');
    return;
end
data = uint8(fread(fp,inf,'uint8'));
%% Parse binary data
% Here finding number of samples
idx = find(data == 3,1,'last'); % This will be the same as data length bc
                                % file is truncated
line_len = 24; % length of each entry in bytes
n = fix(numel(data(1:idx))/line_len);
log.start = zeros(1,n);
log.t_us = zeros(1,n);
log.u = zeros(1,n);
log.y = zeros(1,n);
log.r = zeros(1,n);
log.e = zeros(1,n);
log.ctrl = zeros(1,n);
log.state = zeros(1,n);
log.end = zeros(1,n);

for i = 1:n
    log.start(i) = data((i-1)*line_len+1);
    log.t_us(i)  = typecast(data((i-1)*line_len+2:(i-1)*line_len+5),  'uint32');
    log.u(i)     = typecast(data((i-1)*line_len+6:(i-1)*line_len+9),  'single');
    log.y(i)     = typecast(data((i-1)*line_len+10:(i-1)*line_len+13),'single');
    log.r(i)     = typecast(data((i-1)*line_len+14:(i-1)*line_len+17),'single');
    log.e(i)     = typecast(data((i-1)*line_len+18:(i-1)*line_len+21),'single');
    log.ctrl(i)  = data((i-1)*line_len+22);
    log.state(i) = data((i-1)*line_len+23);
    log.end(i)   = data((i-1)*line_len+24);
end
log.t = (log.t_us-log.t_us(1))*1e-6; % time in seconds without offset
%% Plot log data
figure('Name','Log data');
ax1 = subplot(3,1,1);
stairs(log.t,log.u,'b');
legend('$u(t)$');
xlabel('time (s)');
ylabel('voltage (V)');
grid on;

ax2= subplot(3,1,2);
stairs(log.t,log.y,'g');
legend('$y(t)$');
xlabel('time (s)');
ylabel('angle (rad)');
grid on;

ax3 = subplot(3,1,3);
stairs(log.t,log.y,'g'); hold on;
stairs(log.t, log.r, '--r');
legend('$y(t)$','$r(t)$');
xlabel('time (s)');
ylabel('angle (rad)');
grid on;

linkaxes([ax1 ax2 ax3], 'x')
sgtitle('Log data')
set(gcf, 'Position',  [100, 100, 1600, 800]);
%% Simulate model with log data
mdl = 'motor_block.slx';
run('load_params.m')
SIM_TIME = log.t(end);
USE_OL = 0;
u_sim_in = [log.t',log.u'];
r_sim_in = [log.t',log.r'];
simu = sim(mdl);
simu.t = (0:1e-3:SIM_TIME)';
%% Plot and compare
figure('Name','Sim and log data');
ax1 = subplot(3,1,1);
stairs(log.t,log.u,'b'); hold on;
stairs(simu.t,simu.u,'--r'); hold on;
legend('$u(t)$-log','$u(t)$-sim');
xlabel('time (s)');
ylabel('voltage (V)');
grid on;

ax2= subplot(3,1,2);
stairs(log.t,log.y,'g'); hold on;
stairs(simu.t,simu.y,'--b'); hold on;
legend('$y(t)$-log','$y(t)$-sim');
xlabel('time (s)');
ylabel('angle (rad)');
grid on;

ax3 = subplot(3,1,3);
stairs(log.t,log.y,'g'); hold on;
stairs(simu.t,simu.y,'--b'); hold on;
stairs(log.t, log.r, '--r');
legend('$y(t)$-log','$y(t)$-sim','$r(t)$');
xlabel('time (s)');
ylabel('angle (rad)');
grid on;

linkaxes([ax1 ax2 ax3], 'x')
sgtitle('Sim and log data')
set(gcf, 'Position',  [100, 100, 1600, 800]);