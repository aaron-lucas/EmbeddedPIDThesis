% Discrete Controller Parameters
% ----------------------------------------

fs = 50;
Ts = 1/fs;

% DC Motor Model
% ----------------------------------------

% Model Parameters (1st order)
Kdc = 2.305;    % DC gain
tau = 0.02722;     % Time constant
% Model Transfer Function (1st order)
TFn = Kdc;
TFd = [tau 1];


% % Model Parameters (2nd order)
% K = 0.01;   % EMF constant [V/rad/s] and motor torque constant [N.m/A]
% J = 0.01;   % Rotor moment of inertia [kg.m^2]
% b = 0.1;    % Motor viscous friction [N.m.s]
% L = 0.5;    % Coil inductance [H]
% R = 1;      % Electrical resistance [Ω]
% 
% % Model Transfer Function (2nd order)
% TFn = K;
% TFd = [J*L (J*R+b*L) (b*R+K^2)];



% PID Parameters
% ----------------------------------------

if (exist("C", "var"))
    % Update parameters from auto-tuning
    Kp = C.Kp;
    Ki = C.Ki;
    Kd = C.Kd;
    N =  1/C.Tf;
    N = 100;
    sw_b = C.b;
    sw_c = C.c;
else
    % Update parameters manually
    Kp = 0.0165;
    Ki = 1.6452;
    Kd = 0;
    N = 100;     % Derivative Filter Coefficient
    sw_b = 1;   % Setpoint weight b (proportional weight)
    sw_c = 1;   % Setpoint weight c (derivative weight)
end

output_limits = [-12 12]; % Voltage output limits