clc;
%% Resolution
Gv = 400e-3;% CS amplifier gain V/A
VCC = 5;   % Supply voltage / VDD 
ADC_bits = 12;
volt_res = VCC/(2^ADC_bits);
curr_res = volt_res/Gv;
disp(['Current resolution = ', num2str(curr_res*1e3),'mA'])
%% Current consumption
% INA253A3 - 2.6 mA (quiescent current at 5V)
%

%% Cost
sum = 0;
INA253A3IPWR = 3.40;
sum = sum + INA253A3IPWR;
AD7685CRMZ = 26.42;
sum = sum + AD7685CRMZ;
ADR435BRMZ = 13.02;
%sum = sum + ADR435BRMZ;
ADP7104ARDZ_9_R7 = 4.58;
%sum = sum + ADP7104ARDZ_9_R7;
ADP7104ARDZ_5_R7 = 6.06;
%sum = sum + ADP7104ARDZ_5_R7;
LT6658AHMSE_5_PBF = 10.59;
sum = sum + LT6658AHMSE_5_PBF;
sum