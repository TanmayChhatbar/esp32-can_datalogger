clear all
close all
clc

%% load
logfile = "../logs/run2.log";
t = getRxFromLogs(logfile);
t_obd = t(hex2dec(t.ID)>=0x7E8, :);

%% decode OBD
% decode rpm and vehicle speed
t_rpm = t_obd(hex2dec(t_obd.D3) == 0x0C, :);
t_vx = t_obd(hex2dec(t_obd.D3) == 0x0D, :);
tt_rpm = timetable(t_rpm.("Time Stamp"), (hex2dec(t_rpm.D4)*256+hex2dec(t_rpm.D5))/4, 'VariableNames', {'EngineRPM'});
tt_vx = timetable(t_vx.("Time Stamp"), (hex2dec(t_vx.D4)), 'VariableNames', {'VehicleSpeed'});

% common timestamps
t_common = tt_rpm.Time;
tt_vx_common = retime(tt_vx, t_common, 'linear');
tt = synchronize(tt_rpm, tt_vx_common);

% estimate effective gear ratio
tt.gear_ratio_est = tt.VehicleSpeed ./ tt.EngineRPM;

%% plot raw
clf
s(1) = subplot(211);
yyaxis right
plot(tt.Time, tt.EngineRPM)
yyaxis left
plot(tt.Time, tt.VehicleSpeed)

s(2) = subplot(212);
plot(tt.Time, tt.gear_ratio_est)

linkaxes(s, 'x')

%% functions
function t = getRxFromLogs(logfile)
ds = readlines(logfile);
ds(ds == "") = [];
ds_rx = ds(contains(ds, 'Rx'));
ds_rx = split(ds_rx, ',');
ds_rx = ds_rx(:, ~all(ds_rx==""));

tmp.time = datetime(str2double(ds_rx(:, 1))/1e6, 'ConvertFrom', 'posixtime', 'TimeZone', 'local');
tmp.id = (ds_rx(:, 2));
tmp.extended = ds_rx(:, 3) == "true";
tmp.dir = ds_rx(:, 4);
tmp.bus = str2double(ds_rx(:, 5));
tmp.len = str2double(ds_rx(:, 6));
for i = 1:8
    tmp.("d"+string(i)) = ds_rx(:, 6+i);
end

t = table(tmp.time, tmp.id, tmp.extended, tmp.dir, tmp.bus, tmp.len, ...
    tmp.d1, ...
    tmp.d2, ...
    tmp.d3, ...
    tmp.d4, ...
    tmp.d5, ...
    tmp.d6, ...
    tmp.d7, ...
    tmp.d8, ...
    'VariableNames',split(ds(1, :), ',')');
end