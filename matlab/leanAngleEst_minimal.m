clear
close all
clc

% https://github.com/kmatzen/telemetrik

filename = "GX010043";
gopro_rootdir = "T:\Media\GoPro";
file = findFile2(filename, gopro_rootdir);
clear filename gopro_rootdir

%% imu and gps data
rates = getGoProTelemetry(file, "GYRO");
rates(:, 2:end) = goProFixFrameIMU(rates(:, 2:end));
eul = zeros(height(rates), 3);
fs = round(height(rates) / range(rates(:, 1)) * 1e3)
for i = 2:height(rates)
    eul_rate = gyroToEulerRates(rates(i-1, 2:end)', eul(i-1, :));
    % eul_rate = rates(i-1, 4:-1:2)';
    eul(i, :) = eul(i-1, :) + eul_rate' ./ fs;
end

% for i = 1:width(eul)
%     eul(:, i) = smooth(eul(:, i), 50);
% end

% for i = 1:width(eul)
%     eul(:, i) = highpass(eul(:, i), 0.0001, 200);
% end

eul = rad2deg(eul);


clf
plot(eul(:, 1))
hold on
plot(eul(:, 2))
plot(eul(:, 3))
legend('yaw', 'pitch', 'roll')
%% 
eul = zeros(height(rates), 3);
for i = 2:height(rates)
    gyro = rates(i-1, 2:end)';
    eul(i, :) = eul;
end

