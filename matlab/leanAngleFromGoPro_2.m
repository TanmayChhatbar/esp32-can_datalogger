clear
close all
clc

filename = "GX010043";
gopro_rootdir = "T:\Media\GoPro";
file = findFile2(filename, gopro_rootdir);
clear filename gopro_rootdir

%% cori to RPY
% make sure telemetry exists
cori = getGoProTelemetry(file, "CORI");
cori = array2table(cori, 'VariableNames', ["Timestamp", "W", "X", "Y", "Z"]);
tmp = goProFixFrameCORI([cori.W, cori.X, cori.Y, cori.Z]);
cori.W = tmp(:, 1);
cori.X = tmp(:, 2);
cori.Y = tmp(:, 3);
cori.Z = tmp(:, 4);

% cori to RPY
eul = quat2eul([cori.W, cori.X, cori.Y, cori.Z], 'ZYX');
rpy = array2table([cori.Timestamp, rad2deg(eul)]);
rpy.Properties.VariableNames = ["Timestamp", "Yaw", "Pitch", "Roll"];
outfile = fullfile(file.folder, strrep(file.name, '.MP4', '_rpy.csv'));
writetable(rpy, outfile)
% delete(telemetry_file)

% plot
if 1
    figure(1); clf
    plot(rpy.Timestamp, rpy.Yaw, 'displayname', 'yaw')
    hold on
    plot(rpy.Timestamp, rpy.Pitch, 'displayname', 'pitch')
    plot(rpy.Timestamp, rpy.Roll, 'displayname', 'roll')
    hold off
    legend
    fontsize(24, 'points')
end

%% angular rates
% make sure telemetry exists
rates = getGoProTelemetry(file, "GYRO");
for i = 2:width(rates)
    rates(:, i) = smooth(rates(:, i), 50);
end
rates = array2table(rates);
rates.Properties.VariableNames = ["Timestamp", "Roll rate", "Pitch rate", "Yaw rate"];
outfile = fullfile(file.folder, strrep(file.name, '.MP4', '_gyro.csv'));
writetable(rates, outfile)
% delete(telemetry_file)

% plot
if 1
    figure(1); clf
    plot(rates.Timestamp / 1e3, rates.("Yaw rate"), 'displayname', 'yaw rate')
    hold on
    plot(rates.Timestamp / 1e3, rates.("Pitch rate"), 'displayname', 'pitch rate')
    plot(rates.Timestamp / 1e3, rates.("Roll rate"), 'displayname', 'roll rate')
    hold off
    legend
    fontsize(24, 'points')
end

%% rpy from rates

%% acc
% make sure telemetry exists
type = "ACCL";
telemetry_file = fullfile(file.folder, strrep(file.name, '.MP4', "."+type));
if ~isfile(telemetry_file)
    cur_dir = cd;
    cd(file.folder)
    fprintf("Telemetry not found (" + type + ")\n\tRunning ""telemetrik " + ...
        file.name + """ in """ + strrep(file.folder, '\', '\\') + """\n")
    status = system("telemetrik --streams " + type + " " + file.name);
    if status ~= 0
        error("Could not run telemetrik (https://github.com/kmatzen/telemetrik)")
    end
    cd(cur_dir)
end

% load telemetry
accl = readlines(telemetry_file);
accl = split(accl(accl ~= ""), ',');
accl = str2double(accl);
for i = 2:width(accl)
    accl(:, i) = smooth(accl(:, i), 50);
end
accl = array2table(accl);
accl = accl(:, [])
accl.Properties.VariableNames = ["Timestamp", "X", "Y", "Z"];
outfile = fullfile(file.folder, strrep(file.name, '.MP4', '_accl.csv'));
writetable(accl, outfile)
% delete(telemetry_file)

% plot
if 1
    figure(1); clf
    plot(accl.Timestamp / 1e3, accl.("X"), 'displayname', 'X')
    hold on
    plot(accl.Timestamp / 1e3, accl.("Y"), 'displayname', 'Y')
    plot(accl.Timestamp / 1e3, accl.("Z"), 'displayname', 'Z')
    hold off
    legend
    fontsize(24, 'points')
end
