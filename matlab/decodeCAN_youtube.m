clear all
close all
clc
addpath("fcn")
set(0, 'defaultlinelinewidth', 2)
set(0, ...
    'defaultAxesFontSize',24, ...
    'defaultTextFontSize',24, ...
    'defaultLegendFontSize',24)

%% log file
ffn = "../logs/YamahaR7_2026-08-09_12-58-17.log";
dbc_files = ["../dbc/can-imu-14bit.dbc", ...
    "../dbc/canmod-gps.dbc", ...
    "../dbc/OBD-v4.3.dbc", ...
    "../dbc/leanSensor_all.dbc", ...
    "../dbc/yamaha_r7.dbc"];

zones(1).name = "full throttle";
zones(1).time = datetime(["09-Aug-2026 13:06:16", "09-Aug-2026 13:06:25"], 'TimeZone','local');

zones(2).name = "roundabout";
zones(2).time = datetime(["09-Aug-2026 13:04:15", "09-Aug-2026 13:04:55"], 'TimeZone','local');

%% load can log
[msg_tt, db]  = loadCANLogAsMSGTT(ffn, dbc_files);
sg_tt = canSignalTimetable(msg_tt);

%% plot (1)
close all
h1 = plotIMU(sg_tt, zones(1).time, 25, figure('Name', 'IMU - 1'), 'IMU (full throttle)');
return
plotGNSS(sg_tt, zones(1).time, figure('Name', 'GNSS - 1'), 'GNSS (full throttle)');
plotEngine(sg_tt, zones(1).time, figure("Name",'Engine - 1'), 'Engine (full throttle)');

%% plot (2)
plotIMU(sg_tt, zones(2).time, 25, figure('Name', 'IMU - 2'), 'IMU (roundabout)');
plotGNSS(sg_tt, zones(2).time, figure('Name', 'GNSS - 2'), 'GNSS (roundabout)');
plotEngine(sg_tt, zones(2).time, figure("Name",'Engine - 2'), 'Engine (roundabout)');

%% time spreads
plotTimeSpread({sg_tt.Imu_1, sg_tt.GnssPosition}, figure('name', 'Jitter(INS)'), 'Jitter(INS)')
plotTimeSpread({sg_tt.Msg_20A, sg_tt.Msg_216}, figure('name', 'Jitter(Engine)'), 'Jitter(Engine)')

return
%% animation
outfile = 'imu.mp4';
animateTime(h1, outfile);

%% static
% 83.1097s to 126.3198s
% [8312 12633]
% 8312:12633
acc = sqrt(sg_tt.Imu_1.AccelerationY.^2 + sg_tt.Imu_1.AccelerationZ.^2);
acc = smooth(acc, 50);
acc(acc < 1) = 1;

lean_acc_abs = atand(sqrt(acc.^2 - 1));
lean_acc_abs = smooth(lean_acc_abs, 200);
% lean_acc_abs = lowpass(lean_acc_abs, 1, 100);

% lean
time = sg_tt.Imu_1.Time;
rollrate = sg_tt.Imu_1.AngularRateX;
rollrate = rollrate + .85;
roll_est = cumtrapz(seconds(time - time(1)), rollrate) + deg2rad(15);
% roll_est = highpass(roll_est, 0.0005, 100);
plot(time, roll_est)
hold on 
plot(time, lean_acc_abs)
hold off
return

%% try chatgpt code
ax = sg_tt.Imu_1.AccelerationX;
ay = sg_tt.Imu_1.AccelerationY;
az = sg_tt.Imu_1.AccelerationZ;
wx = sg_tt.Imu_1.AngularRateX;
wy = sg_tt.Imu_2.AngularRateY;
wz = sg_tt.Imu_2.AngularRateZ;
lean_est = estLean_chatgpt(ax, ay, az, wx, wy, wz)

plot(lean_est)
%% export
[f1, fn, ext] = fileparts(ffn);
processed_folder = fullfile(f1, fn);
if isfolder(processed_folder)
    rmdir(processed_folder, 's')
end
mkdir(processed_folder)
fns = string(fieldnames(sg_tt))';
tt = sg_tt.(fns(1));
for fn1 = fns(2:end)
    tt = synchronize(tt, sg_tt.(fn1))
end

fns = string(fieldnames(tt))';
tt = removevars(tt, fns(contains(fns, "Accuracy")));
tt = removevars(tt, fns(contains(fns, "Valid")));
fns = fns(1:end-3);
fns = fns(~contains(fns, "Valid"));
fns = fns(~contains(fns, "Accuracy"))
for fn1 = fns
    tt.(fn1) = fillmissing(tt.(fn1), 'linear');
    tt.(fn1) = fillmissing(tt.(fn1), 'next');
end
% tt
% writetimetable(tt, fullfile(processed_folder, fn) + ".csv")
tt = timetable2table(tt);
tt.Time = seconds(tt.Time - tt.Time(1))
writetable(tt, fullfile(processed_folder, fn) + ".csv")

%% lean

%% functions
function h = plotIMU(sg_tt, xl, sf, f, ttl)
if isfield(sg_tt, 'Imu_1')
    h = figure(f);
    s(1) = subplot(211);
    if ~isempty(sf)
        ax = smooth(sg_tt.Imu_1.AccelerationX, sf);
        ay = smooth(sg_tt.Imu_1.AccelerationY, sf);
        az = smooth(sg_tt.Imu_1.AccelerationZ, sf);

        wx = smooth(sg_tt.Imu_1.AngularRateX, sf);
        wy = smooth(sg_tt.Imu_2.AngularRateY, sf);
        wz = smooth(sg_tt.Imu_2.AngularRateZ, sf);
    else
        ax = sg_tt.Imu_1.AccelerationX;
        ay = sg_tt.Imu_1.AccelerationY;
        az = sg_tt.Imu_1.AccelerationZ;

        wx = sg_tt.Imu_1.AngularRateX;
        wy = sg_tt.Imu_2.AngularRateY;
        wz = sg_tt.Imu_2.AngularRateZ;
    end
    plot(sg_tt.Imu_1.Time, ax, 'r', 'DisplayName', 'x')
    hold on
    plot(sg_tt.Imu_1.Time, ay, 'g', 'DisplayName', 'y')
    plot(sg_tt.Imu_1.Time, az, 'b', 'DisplayName', 'z')
    legend('location', 'northeast')
    hold off
    xlabel("Time [s]")
    ylabel("Acceleration [g]")
    grid on
    
    s(2) = subplot(212);
    plot(sg_tt.Imu_1.Time, wx, 'r', 'DisplayName', 'x')
    hold on
    plot(sg_tt.Imu_2.Time, wy, 'g', 'DisplayName', 'y')
    plot(sg_tt.Imu_2.Time, wz, 'b', 'DisplayName', 'z')
    hold off
    xlabel("Time [s]")
    ylabel("Angular rate [deg/s]")
    grid on
    
    sgtitle(ttl)
    linkaxes(s, 'x')
    xlim(xl)
    exportgraphics(gcf, "figs/"+ttl+".png", 'Resolution',300)
end
end

function h = plotGNSS(sg_tt, xl, f, ttl)
if isfield(sg_tt, 'GnssPosition')
    if length(xl) == 2
        idx = sg_tt.GnssPosition.Time > xl(1) & sg_tt.GnssPosition.Time < xl(2);
    else
        idx = ~isnan(sg_tt.GnssPosition.Latitude);
    end
    figure(f)
    geoplot(sg_tt.GnssPosition.Latitude(idx), ...
        sg_tt.GnssPosition.Longitude(idx))
    geobasemap('streets')

    sgtitle(ttl)
    exportgraphics(gcf, "figs/"+ttl+".png", 'Resolution',300)
end
end

function h = plotEngine(sg_tt, xl, f, ttl)
figure(f)
s(1) = subplot(211);
stairs(sg_tt.Msg_216.Time, sg_tt.Msg_216.Throttle_Position, 'marker', '.')
grid on
xlabel("Time [s]")
ylabel("TPS [%]")
s(2) = subplot(212);
stairs(sg_tt.Msg_20A.Time, sg_tt.Msg_20A.Engine_Speed, 'marker', '.')
grid on
xlabel("Time [s]")
ylabel("Engine Speed [rpm]")
linkaxes(s, 'x')
xlim(xl)
sgtitle(ttl)

exportgraphics(gcf, "figs/"+ttl+".png", 'Resolution',300)
end

function plotTimeSpread(sg, f, ttl)
figure(f)
% xm = (1:length(sg.Time)-1) / (length(sg.Time)-1) * 100;
% plot(xm', sort(seconds(diff(sg.Time))))
% xlabel("\%")
% ylabel("Time [s]")
for i = 1:length(sg)
    subplot(length(sg), 1, i)
    hf = seconds(diff(sg{i}.Time));
    hf = sort(hf);
    hf = hf(1:round(end*0.99));
    histogram(hf, numbins=81)
    grid on
    xlabel("dt [s]")
    ylabel(sg{i}.Properties.VariableNames{1}, 'Interpreter','none')
end
sgtitle(ttl)
exportgraphics(gcf, "figs/"+ttl+".png", 'Resolution',300)
end
