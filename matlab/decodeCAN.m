clear all
close all
clc
addpath("fcn")

%% log file
ffn = "../logs/YamahaR7_2026-07-26_16-53-49.log";
dbc_files = ["../dbc/can-imu-14bit.dbc", ...
    "../dbc/canmod-gps.dbc", ...
    "../dbc/OBD-v4.3.dbc", ...
    "../dbc/leanSensor_all.dbc", ...
    "../dbc/yamaha_r7.dbc"];

%% load can log
[msg_tt, db]  = loadCANLogAsMSGTT(ffn, dbc_files);
sg_tt = canSignalTimetable(msg_tt);
return
%% plot
if isfield(sg_tt, 'Imu_1')
    figure('Name', 'IMU')
    s(1) = subplot(211);
    plot(sg_tt.Imu_1.Time, sg_tt.Imu_1.AccelerationX, 'r')
    hold on
    plot(sg_tt.Imu_1.Time, sg_tt.Imu_1.AccelerationY, 'g')
    plot(sg_tt.Imu_1.Time, sg_tt.Imu_1.AccelerationZ, 'b')
    hold off
    xlabel("Time [s]")
    ylabel("Acceleration [m/s^2]")
    grid on
    
    s(2) = subplot(212);
    plot(sg_tt.Imu_1.Time, sg_tt.Imu_1.AngularRateX, 'r')
    hold on
    plot(sg_tt.Imu_2.Time, sg_tt.Imu_2.AngularRateY, 'g')
    plot(sg_tt.Imu_2.Time, sg_tt.Imu_2.AngularRateZ, 'b')
    hold off
    xlabel("Time [s]")
    ylabel("Angular rate [deg/s]")
    grid on
    
    fontsize(20, 'points')
    linkaxes(s, 'x')
end
if isfield(sg_tt, 'GnssPosition')
    figure('Name', 'GPS')
    geoplot(sg_tt.GnssPosition.Latitude, sg_tt.GnssPosition.Longitude)
end

%% TPS, engine RPM
figure('Name', 'tps')
s(1) = subplot(211);
plot(sg_tt.Msg_216.Time, sg_tt.Msg_216.Throttle_Position)
grid on
xlabel("Time [s]")
ylabel("TPS [%]")
s(2) = subplot(212);
plot(sg_tt.Msg_20A.Time, sg_tt.Msg_20A.Engine_Speed)
grid on
xlabel("Time [s]")
ylabel("Engine Speed [rpm]")
linkaxes(s, 'x')