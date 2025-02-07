clear all;
clc;
close all;
LOG_NAME  = 'log10';
LOG_FILE_TYPE = '.log';
LOG_FOLDER = '/home/farfar/Documents/servo/software/mcu/teensy-servo/logs/';
LOG_PATH = strcat(LOG_FOLDER, LOG_NAME, LOG_FILE_TYPE)
USE_OL = 1;
run("load_params.m")
%%
data = load(LOG_PATH);
t_i = 1;
r_i = 2;
e_i = 3;
y_i = 4;
u_i = 5;
cnt_i = 6;
t_log = data(:,t_i);
%%
if(t_log(1) == 0)
    t_log = t_log*1e-3;
else
    t_log = (t_log-t_log(1))*1e-6;
end
r_log = data(:,r_i);
e_log = data(:,e_i);
y_log = data(:,y_i);
u_log = data(:,u_i);
cnt_log = data(:,cnt_i);
%% Simulation settings
mdl = 'motor_block';
SIM_TIME = t_log(end);
USE_OL = 1;
u_sim_in = [t_log,u_log];
r_sim_in = [t_log,r_log];
simu = sim(mdl);
simu.t = (0:1e-3:SIM_TIME)';
%% Check reference tracking
% figure;
% stairs(t_log,r_log,'b'); hold on;
% stairs(t_log,y_log,'--r');
% legend('$\theta_{ref}$','$\theta$');
% xlabel('time (s)');
% ylabel('angle (rad)')
% title('Log ref and meas');
%% Plotting log data and sim data
figure;
ax1 = subplot(2,1,1); % Compare logged and simulated u
stairs(t_log,u_log,'b'); hold on;
stairs(simu.t,simu.u,'--r');
legend('$u(t)$-log','$u(t)$-sim');
xlabel('time (s)');
ylabel('voltage (V)');

ax2 = subplot(2,1,2); % Compare logged and simulated y
stairs(t_log, y_log, 'b'); hold on;
stairs(simu.t, simu.y, '--r');
stairs(t_log, r_log,'k');
xlabel('time (s)');
ylabel('angle (rad)')
legend('$y$-log','$y$-sim','$r$')
linkaxes([ax1 ax2], 'x');

xlim([4 8])
%%
figure;
ax1 = subplot(3,1,1);
stairs(simu.t,simu.u,'b');
legend('$u(t)$');
xlabel('time (s)');
ylabel('motor voltage (V)');
title('Simulated motor voltage input');
ax2 = subplot(3,1,2);
stairs(simu.t,simu.y,'g');
legend('$y(t)$');
xlabel('time (s)');
ylabel('rotor angle (rad)');
title('Simulated rotor angle');
ax3 = subplot(3,1,3);
plot(simu.t, simu.i, 'r');
xlabel('time (s)');
ylabel('winding current (A)');
title('Simulated winding current');
t = sgtitle('Motor model simulation');
legend('$i(t)$')
t.FontSize= 18;
linkaxes([ax1 ax2 ax3], 'x');
xlim([0 3])