clear all
close all
clc
addpath("fcn")

%% log file
ffn = "../logs/YamahaR7_2026-07-07_20-58-20.log";
dbc_files = ["../dbc/can-imu-14bit.dbc", "../dbc/canmod-gps.dbc", "../dbc/OBD-v4.3.dbc", "../dbc/leanSensor_all.dbc"];

%% load can log
msg_tt = loadCANLogAsMSGTT(ffn, dbc_files);
sg_tt = canSignalTimetable(msg_tt);

%% plot
if isfield(sg_tt, 'GnssImu')
    figure('Name', 'IMU')
    subplot(211)
    plot(sg_tt.GnssImu.Time, sg_tt.GnssImu.AccelerationX, 'r')
    hold on
    plot(sg_tt.GnssImu.Time, sg_tt.GnssImu.AccelerationY, 'g')
    plot(sg_tt.GnssImu.Time, sg_tt.GnssImu.AccelerationZ, 'b')
    hold off
    xlabel("Time [s]")
    ylabel("Acceleration [m/s^2]")
    grid on
    
    subplot(212)
    plot(sg_tt.GnssImu.Time, sg_tt.GnssImu.AngularRateX, 'r')
    hold on
    plot(sg_tt.GnssImu.Time, sg_tt.GnssImu.AngularRateY, 'g')
    plot(sg_tt.GnssImu.Time, sg_tt.GnssImu.AngularRateZ, 'b')
    hold off
    xlabel("Time [s]")
    ylabel("Angular rate [deg/s]")
    grid on
    
    fontsize(20, 'points')
end
if isfield(sg_tt, 'GnssPosition')
    figure('Name', 'GPS')
    geoplot(sg_tt.GnssPosition.Latitude, sg_tt.GnssPosition.Longitude)
end
