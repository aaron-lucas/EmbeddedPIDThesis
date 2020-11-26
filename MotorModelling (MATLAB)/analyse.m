clc; clear; close all;

%% Load Data
data = readtable("output.csv");
time = data.Time / 1000000;    % Convert us to s
speed = data.Speed;

%% Define Experimental Parameters

% Motor Parameters
% -----------------
W_max   = 26;               % Maximum theoretical speed (rpm)

% Timing Parameters
% ------------------
T_bump    = 1;              % Time of bump signal (s)
T_len     = time(end);      % Length of signal (s)

% Measurement Parameters
% -----------------------
R_before  = [0.2 0.8];      % Proportion of signal before bump to use for averaging
R_after   = [0.2 0.8];      % Proportion of signal after bump to use for averaging
R_all     = [0.2 0.8];      % Proportion of whole signal to use for fit analysis
W_error   = 0.06;           % Relative error allowed when searching for time constant

% Control Signal Parameters
% --------------------------
T_zero    = 1.5 / 1000;     % Neutral PWM period (s)
T_max     = 2.0 / 1000;     % Max speed PWM period (s)
T_before  = 1.794 / 1000;   % PWM period before bump (s)
T_after   = 2.018 / 1000;   % PWM period after bump (s)

V_in      = 12;             % Voltage input to motor driver (V)

%% Calculate Experimental Constants

T_thres    = [ R_before(1) * T_bump
               R_before(2) * T_bump
               T_bump + R_after(1) * (T_len - T_bump)
               T_bump + R_after(2) * (T_len - T_bump)
               R_all(1) * T_len
               R_all(2) * T_len ];

% Saturate PWM period if necessary
if T_after > T_max
    T_after = T_max;
end

% Calculate equivalent control signal voltages
T_range   = T_max - T_zero;
V_before  = (T_before - T_zero) / T_range * V_in;
V_after   = (T_after - T_zero) / T_range * V_in;
V_bump    = V_after - V_before;

% Calculate expected motor speeds
W_before_ex  = (T_before - T_zero) / T_range * W_max;
W_after_ex   = (T_after - T_zero) / T_range * W_max;
W_bump_ex    = W_after_ex - W_before_ex;

%% Process

% Lowpass filter noise with moving average filter
N_filter = 3;
taps = ones(1, N_filter) / N_filter;
speed_lpf = conv(speed, taps, 'same');

% Extract speeds before and after the bump, ignoring effects at start and
% end
speed_before  = speed_lpf(time > T_thres(1) & time < T_thres(2));
speed_after   = speed_lpf(time > T_thres(3) & time < T_thres(4));

% Calculate speed change caused by bump
W_before = mean(speed_before);
W_after = mean(speed_after);
W_bump = W_after - W_before;

% Search for time constant (time taken for speed to reach 63% of final
% value)
W_tau = W_before + W_bump * 0.63;
taus = time(speed_lpf > W_tau * (1 - W_error) & speed_lpf < W_tau * (1 + W_error));
tau = interp1(speed_lpf(time == taus(1) | time == taus(end)), [taus(1), taus(end)], W_tau) - T_bump;

% Calculate DC gain of motor (ratio of speed change to voltage change)
V_bump_meas = (W_after - W_before) / W_max * V_in;

K_dc = W_bump / V_bump;

%% Verification

% Generate 1st order transfer function based off data measurements
s = tf('s');
TF_sim = K_dc / (1 + tau * s)

% Simulate step response
fs_sim = 1000;
t_sim = 0:(1/fs_sim):(T_len);

step_input = V_bump * heaviside(t_sim - 1);
speed_sim = lsim(TF_sim, step_input', t_sim)';

% Remove DC offset of speed data
speed_no_offset = speed_lpf - W_before;

% Calculate fit value for simulated transfer function
speed_interp = interp1(time, speed_no_offset, t_sim);
speed_act_clip = speed_interp(t_sim > T_thres(5) & t_sim < T_thres(6));
speed_sim_clip = speed_sim(t_sim > T_thres(5) & t_sim < T_thres(6));
fit = (1 - norm(speed_act_clip - speed_sim_clip, 2) / norm(speed_act_clip - mean(speed_act_clip), 2)) * 100

% Plot filtered measurements alongside simulated measurements for visual 
% verification
time_vals = t_sim > 0.6 & t_sim < 1.6;
figure;
hold on;
plot(t_sim(time_vals)-0.6, speed_interp(time_vals)+W_before, 'LineWidth', 1.5);
plot(t_sim(time_vals)-0.6, speed_sim(time_vals)+W_before, 'LineWidth', 1.5);
set(gca, 'FontSize', 14);
title('Bump Test Results for DC Motor Modelling', 'FontSize', 16);
xlabel('Time (s)', 'FontSize', 14);
ylabel('Speed (rpm)', 'FontSize',14);
legend('Physical Motor', 'Simulated Model', 'Location', 'northwest');
hold off;

