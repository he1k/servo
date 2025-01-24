clear all;
clc;
close all;
logs_path = '/home/farfar/Documents/servo/software/matlab/logs/';
log_name = 'log5.bin';
log_path = strcat(logs_path,log_name)
fp = fopen(log_path);
if (fp == -1)
    disp('ERROR OPENING LOG FILE');
    return;
end
data = uint8(fread(fp,inf,'uint8'));
class(data)
size(data)
%%
idx = find(data == 3,1,'last')
% memcpy(&l->start, &line[START_OFFSET], sizeof(l->start));
% memcpy(&l->t,     &line[T_OFFSET],     sizeof(l->t));
% memcpy(&l->u,     &line[U_OFFSET],     sizeof(l->u));
% memcpy(&l->y,     &line[Y_OFFSET],     sizeof(l->y));
% memcpy(&l->r,     &line[R_OFFSET],     sizeof(l->r));
% memcpy(&l->e,     &line[E_OFFSET],     sizeof(l->e));
% memcpy(&l->ctrl,  &line[CTRL_OFFSET],  sizeof(l->ctrl));
% memcpy(&l->state, &line[STATE_OFFSET], sizeof(l->state));
% memcpy(&l->end,   &line[END_OFFSET],   sizeof(l->end));
line_len = 24;
n = numel(data(1:idx))/line_len;
dat.start = zeros(1,n);
dat.t = zeros(1,n);
dat.u = zeros(1,n);
dat.y = zeros(1,n);
dat.r = zeros(1,n);
dat.e = zeros(1,n);
dat.ctrl = zeros(1,n);
dat.state = zeros(1,n);
dat.end = zeros(1,n);

for i = 1:n
    dat.start(i) = data((i-1)*line_len+1);
    dat.t(i)     = typecast(data((i-1)*line_len+2:(i-1)*line_len+5),'uint32');
    dat.u(i)     = typecast(data((i-1)*line_len+6:(i-1)*line_len+9),'single');
    dat.y(i)     = typecast(data((i-1)*line_len+10:(i-1)*line_len+13),'single');
    dat.r(i)     = typecast(data((i-1)*line_len+14:(i-1)*line_len+17),'single');
    dat.e(i)     = typecast(data((i-1)*line_len+18:(i-1)*line_len+21),'single');
    dat.ctrl(i)  = data((i-1)*line_len+22);
    dat.state(i) = data((i-1)*line_len+23);
    dat.end(i)   = data((i-1)*line_len+24);
end
dat.head = dat.ctrl;
dat.tail = dat.state;
t = (dat.t-dat.t(1))*1e-6;
%%
figure;
subplot(3,1,1);
plot(t,dat.u,'b');
legend('$u(t)$');
xlabel('time (s)');
ylabel('voltage (V)');
grid on;
subplot(3,1,2);
plot(t,dat.y,'g');
legend('$y(t)$');
xlabel('time (s)');
ylabel('angle (rad)');
grid on;
subplot(3,1,3);
plot(t,dat.y,'g'); hold on;
plot(t, dat.r, '--r');
legend('$y(t)$','$r(t)$');
xlabel('time (s)');
ylabel('angle (rad)');
grid on;
