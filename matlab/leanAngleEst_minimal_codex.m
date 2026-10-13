clear
close all
clc

% https://github.com/kmatzen/telemetrik

filename = "GX010043";
gopro_rootdir = "T:\Media\GoPro";
file = findFile2(filename, gopro_rootdir);
clear filename gopro_rootdir

%% CORI to lean angle
% CORI is quaternion orientation in the GoPro frame.  Use the frame
% correction found in gopro_dir_cal3 before converting to Euler angles.
cori = getGoProTelemetry(file, "CORI");
cori = array2table(cori, 'VariableNames', ["Timestamp", "W", "X", "Y", "Z"]);

q = goProFixFrameCORI([cori.W, cori.X, cori.Y, cori.Z]);
eul = quat2eul(q, 'ZYX');

% In the corrected frame, ZYX roll is the camera/bike lean angle.
lean = rad2deg(eul(:, 3));
lean = unwrap(deg2rad(lean));
lean = rad2deg(lean);

result = table(cori.Timestamp, lean, ...
    'VariableNames', ["Timestamp", "LeanAngle"]);
outfile = fullfile(file.folder, strrep(file.name, '.MP4', '_lean_codex.csv'));
writetable(result, outfile)

%% plot
figure(1); clf
plot(result.Timestamp / 1e3, result.LeanAngle)
xlabel('time (s)')
ylabel('lean angle (deg)')
grid on
fontsize(24, 'points')
