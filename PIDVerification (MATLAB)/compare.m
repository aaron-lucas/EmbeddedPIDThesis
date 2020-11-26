clc; clear; close all;
%% Load simulation data
load simulationDataFast;
simTime = out.ScopeData.time;
simData = out.ScopeData.signals(3).values;
refSignal = out.ScopeData.signals(2).values;

%% Load and filter experimental data

mcuData = readtable('mcuDataFast.csv').Var1;
fpgaData = readtable('fpgaDataFast.csv').Var1;

N_filter = 5;
taps = ones(1, N_filter) / N_filter;

mcuData = conv(mcuData, taps, 'same');
fpgaData = conv(fpgaData, taps, 'same');

fs = 60;
expTime = (0:(length(mcuData)-1))/fs;

%% Plot all datasets
startTime = 3;
endTime = 9;
simTimeSelection = simTime > startTime & simTime < endTime;
expTimeSelection = expTime > startTime & expTime < endTime;

simTime = simTime(simTimeSelection) - startTime;
expTime = expTime(expTimeSelection) - startTime;

mcuData     = mcuData(expTimeSelection);
fpgaData    = fpgaData(expTimeSelection);
simData     = simData(simTimeSelection);
refSignal   = refSignal(simTimeSelection);

% MCU Comparison
figure(1);
grid on;
hold on;

plot(expTime, mcuData, '-', 'Color', '#EC9A29', 'LineWidth', 4);
plot(simTime, simData, ':', 'Color', '#282A3E', 'LineWidth', 2.8);
plot(simTime, refSignal, '-.', 'Color', '#707F99', 'LineWidth', 1.6);
 
hold off;
ylim([8 24]);

title({'Comparison of Microcontroller PID Implementation Step Response', 'with Simulated Model'}, 'FontSize', 18);
xlabel('Time (s)', 'FontSize', 15);
ylabel('Motor Speed (rpm)', 'FontSize', 15);
set(gca, 'FontSize', 13);

legend('Experimental Step Response (MCU)', 'Simulated Step Response', 'Reference Signal');

% FPGA Comparison
figure(2);
grid on;
hold on;

plot(expTime, fpgaData, '-', 'Color', '#C02D0C', 'LineWidth', 4);
plot(simTime, simData, ':', 'Color', '#282A3E', 'LineWidth', 2.8);
plot(simTime, refSignal, '-.', 'Color', '#707F99', 'LineWidth', 1.6);
 
hold off;
ylim([8 24]);

title({'Comparison of FPGA PID Implementation Step Response', 'with Simulated Model'}, 'FontSize', 18);
xlabel('Time (s)', 'FontSize', 15);
ylabel('Motor Speed (rpm)', 'FontSize', 15);
set(gca, 'FontSize', 13);

legend('Experimental Step Response (FPGA)', 'Simulated Step Response', 'Reference Signal');

mcuFit = fit(mcuData, simData)
fpgaFit = fit(fpgaData, simData)

%% Functions
function f = fit(measured, simulated)
    f = (1 - norm(measured - simulated, 2) / norm(measured - mean(measured), 2)) * 100;
end
