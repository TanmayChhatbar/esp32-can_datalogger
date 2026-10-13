clear
close all
clc

% https://github.com/kmatzen/telemetrik

filename = "GX010043";
gopro_rootdir = "T:\Media\GoPro";
file = findFile2(filename, gopro_rootdir);
clear filename gopro_rootdir

%% imu and gps data
acc = getGoProTelemetry(file, "ACCL");
rates = getGoProTelemetry(file, "GYRO");
cori = getGoProTelemetry(file, "CORI");
acc(:, 2:end) = goProFixFrameIMU(acc(:, 2:end));
rates(:, 2:end) = goProFixFrameIMU(rates(:, 2:end));
cori(:, 2:end) = goProFixFrameCORI(cori(:, 2:end));
for i = 2:width(rates)
    rates(:, i) = smooth(rates(:, i), 20);
end
for i = 2:width(acc)
    acc(:, i) = smooth(acc(:, i), 20);
end
imu = array2table([acc rates(:, 2:end)]);
imu.Properties.VariableNames = ["Timestamp", "acc_x", "acc_y", "acc_z", "p", "q", "r"];
clear rates acc

gps = getGoProTelemetry(file, "GPS5");
gps = array2table(gps, 'VariableNames', ["Timestamp", "lat", "lon", "alt", "speed_2d", "speed_3d", "fix", "dop"]);
geoplot(gps.lat, gps.lon)

% correct timestamps
fs_imu = height(imu) / (range(imu.Timestamp) / 1e3);
fs_gps = height(gps) / (range(gps.Timestamp) / 1e3);
fs_imu = round(fs_imu);
fs_gps = round(fs_gps);

imu.Timestamp = imu.Timestamp - (1:height(imu))'/fs_imu*1e3;
gps.Timestamp = gps.Timestamp - (1:height(gps))'/fs_gps*1e3;

%% sensor fusion using MATLAB functions
filt = insfilterAsync;
filt.ReferenceLocation = [gps.lat(1), gps.lon(1), gps.alt(1)];
initstate = zeros(22, 1);
initstate(1:4) = cori(end, [5 2 3 4]);
initstate(5:22) = 0;
filt.State = [initstate(1:4);0;0;0;initstate(5:10);0;0;0;initstate(11:end)];

Rvel  = 0.464;
Racc  = 800;
Rgyro = 1e-4;
Rpos  = 34;

accel = table2array(imu(:, 2:4));
gyro = table2array(imu(:, 5:7));
lla = table2array(gps(:, 2:4));
gpsvel3d = table2array(gps(:, "speed_3d"));
gpsvel2d = table2array(gps(:, "speed_2d"));
gpsvel(:, 3) = sqrt(gpsvel2d.^2 - gpsvel3d.^2);
gpsvel(:, 1) = gpsvel2d;
N = size(accel,1);
p = zeros(N,3);
q = zeros(N,1,'quaternion');
gpsIdx = 1;

for ii = 1:N
    
    % Predict the filter forward one time step
    predict(filt,1./fs_imu);
    
    % Fuse accelerometer and gyroscope readings
    fuseaccel(filt,accel(ii,:),Racc);
    fusegyro(filt,gyro(ii,:),Rgyro);
    if ii > 1
        ypr_last = quat2eul(q(ii-1));
        gpsvel(gpsIdx, 1:2) = gpsvel(gpsIdx,1) * [cos(ypr_last(1)) sin(ypr_last(1))];
    end

    % Fuse GPS once per second
    if ~mod(ii,fs_imu)
        fusegps(filt,lla(gpsIdx,:),Rpos,gpsvel(gpsIdx,:),Rvel);
        gpsIdx = gpsIdx + 1;
    end
    
    % Log the current pose estimate
    [p(ii,:),q(ii)] = pose(filt);
    
end

%% result
% compare location
% [lat,lon,alt] = local2latlon(p(:, 1), p(:, 2), p(:, 3), [gps.lat(1), gps.lon(1), gps.alt(1)])
subplot(121)
geoplot(gps.lat, gps.lon)
subplot(122)
plot(p(:, 1), p(:, 2))
% quat2eul(q)