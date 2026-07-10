clear all
close all
clc
addpath("fcn")

%% log file
dbc_files = ["../dbc/can-imu-14bit.dbc", "../dbc/canmod-gps.dbc", "../dbc/OBD-v4.3.dbc", "../dbc/leanSensor_all.dbc"];
dbs = canDatabase(dbc_files(1));
for dbc_file = dbc_files(2:end)
    dbs(end+1) = canDatabase(dbc_file);
end

%% init live log table
varnames = ["Time",    "ID",    "Extended", "Name", "Data", "Length", "Signals", "Error",   "Remote"];
vartypes = ["datetime","uint32","logical",  "cell", "cell", "uint8",  "cell",    "logical", "logical"];
tt = table( ...
    'Size', [0 length(varnames)], ...
    'VariableTypes', vartypes, ...
    'VariableNames', varnames);
% tt.Time.TimeZone = "local";
tt = table2timetable(tt);

%% log data
ports = serialportlist;
s = serialport(ports(1), 921600, 'Timeout',2);
h_track = struct([]);
hl = nan;
while 1 % TODO end condition
    sd = s.readline();
    if isempty(sd)
        % timeout
        break;
    end

    if sum(char(sd) == ',') == 14
        % construct row from read line
        sd = sd.strip();
        nr = rowFromString(sd); % new row
        if nr{2} == 255
            break
        end
    
        %% construct as CAN message
        nm = canMessage(nr{2}, nr{3}, nr{6});
        nm.Timestamp = posixtime(nr{1});
        nm.Data = nr{5}{1};
        nm = canMessageTimetable(nm, dbs);
        nm.Time = datetime(seconds(nm.Time), 'ConvertFrom', 'posixtime');
        sg = canSignalTimetable(nm);

        % %% plot callback
        % TODO
        % fns = string(fields(sg))';
        % fns = fns(~contains(fns, ["Properties", "Time", "Variables"]));
        
        % for fn = fns
        %     if fn ~= "roll_deg"
        %         continue
        %     end
        %     % [h, hl] = figureList(fn, h_track);
        %     % figure(h)
        %     if isa(hl, 'matlab.graphics.chart.primitive.Line')
        %         % hl.XData = [hl.XData sg.Time];
        %         % hl.YData = [hl.YData sg.(fn)];
        %         addpoints(hl, sg.Time, sg.(fn));
        %     else
        %         hl = animatedline(sg.Time, sg.(fn));
        %         % hl.addpoints();
        %     end
        %     drawnow
        % end

        %% add to table
        tt = [tt; nm];

        % break
    else
        fprintf(sd)
        if contains(sd, "Logging stopped")
            break
        end
    end
end
clear s

% save log
% log_name = 
% save(log_name, 'tt')
tt
return

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

%% functions
function row = rowFromString(sd)
sd = sd.split(',');
time = datetime(str2num(sd(1))/1e6, 'ConvertFrom', 'posixtime', 'TimeZone','local');
id = uint32(hex2dec(sd(2)));
extended = sd(3) == "true"; % extended
name = {char([])}; % name
data = {uint8(hex2dec(sd(7:7+7))')};
remote = sd(4) == "Rx";
length = uint8(str2num(sd(6)));
signals = {struct([])};
error = false;

row = {time id extended name data length signals error remote};
end
