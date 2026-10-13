clear
close all
clc

% GoPro telemetry is in the camera frame.  goProFixFrameIMU and
% goProFixFrameCORI change it to the common bike frame used by the
% calibration scripts: x forward, y right, z down.

filename = "GX010043";
gopro_rootdir = "T:\Media\GoPro";
file = findFile2(filename, gopro_rootdir);
clear filename gopro_rootdir

%% load and fix frame
acc = getGoProTelemetry(file, "ACCL");
gyro = getGoProTelemetry(file, "GYRO");
cori = getGoProTelemetry(file, "CORI");
gps = getGoProTelemetry(file, "GPS5");

acc(:, 2:end) = goProFixFrameIMU(acc(:, 2:end));
gyro(:, 2:end) = goProFixFrameIMU(gyro(:, 2:end));
cori(:, 2:end) = goProFixFrameCORI(cori(:, 2:end));

% telemetrik timestamps are milliseconds.  Give each stream a local time
% origin; the first sample is a more reliable origin than a nominal rate.
t = (gyro(:, 1) - gyro(1, 1)) / 1e3;
t_acc = (acc(:, 1) - acc(1, 1)) / 1e3;
t_cori = (cori(:, 1) - cori(1, 1)) / 1e3;
t_gps = (gps(:, 1) - gps(1, 1)) / 1e3;

acc = interp1(t_acc, acc(:, 2:end), t, 'linear', 'extrap');
q_cori = interp1(t_cori, cori(:, 2:end), t, 'linear', 'extrap');
q_cori = q_cori ./ vecnorm(q_cori, 2, 2);

% GYRO is radians/s.  Smooth only enough to remove the sample-to-sample
% noise; the integration below still uses the original sample timing.
gyro = smoothdata(gyro(:, 2:end), 1, 'movmean', 5);

%% gyro/CORI attitude fusion
% CORI supplies an absolute camera attitude, while the gyro retains the
% fast lean changes.  A short CORI time constant prevents gyro drift.
N = numel(t);
q = zeros(N, 4);
q(1, :) = q_cori(1, :);
tau_cori = 0.35;                 % CORI correction time constant, s

for i = 2:N
    dt = min(max(t(i) - t(i-1), 0), 0.05);
    omega = gyro(i-1, :);
    theta = norm(omega) * dt;
    if theta > 0
        dq = [cos(theta/2), sin(theta/2) * omega / norm(omega)];
        q_pred = quatmultiply(q(i-1, :), dq);
    else
        q_pred = q(i-1, :);
    end

    % Quaternions q and -q are identical rotations.  Match signs before
    % the complementary blend so that interpolation never crosses zero.
    q_meas = q_cori(i, :);
    if dot(q_pred, q_meas) < 0
        q_meas = -q_meas;
    end
    alpha = exp(-dt / tau_cori);
    q(i, :) = alpha * q_pred + (1 - alpha) * q_meas;
    q(i, :) = q(i, :) / norm(q(i, :));
end

eul = quat2eul(q, 'ZYX');
lean_imu = unwrap(eul(:, 3));    % positive is a lean to the right

%% GPS cornering lean
% GPS position gives course and yaw rate without relying on a magnetic
% heading.  The steady-state bike relation is tan(lean) = v*yaw_rate/g.
lat = deg2rad(gps(:, 2));
lon = deg2rad(gps(:, 3));
earth_radius = 6378137;
north = earth_radius * (lat - lat(1));
east = earth_radius * cos(lat(1)) * (lon - lon(1));
north = smoothdata(north, 'movmean', 5);
east = smoothdata(east, 'movmean', 5);

% GPS5 speed is available, but position-derived speed is more dependable
% when the stream has missing or repeated speed samples.
speed = hypot(gradient(north, t_gps), gradient(east, t_gps));
speed = smoothdata(speed, 'movmean', 9);
course = unwrap(atan2(gradient(east, t_gps), gradient(north, t_gps)));
yaw_rate = smoothdata(gradient(course, t_gps), 'movmean', 9);
lean_gps = atan2(speed .* yaw_rate, 9.80665);

lean_gps = interp1(t_gps, lean_gps, t, 'linear', NaN);
speed = interp1(t_gps, speed, t, 'linear', NaN);
yaw_rate = interp1(t_gps, yaw_rate, t, 'linear', NaN);

% The GPS course convention and the GoPro mounting can differ by a sign.
% Determine that sign from sections that are both moving and turning.
turning = speed > 5 & abs(yaw_rate) > deg2rad(1);
if nnz(turning) > 20 && median(lean_imu(turning) .* lean_gps(turning), 'omitnan') < 0
    lean_gps = -lean_gps;
end

%% final lean estimate
% Accelerometer magnitude is used as a quality gate.  During bumps its
% gravity estimate is poor, so use the CORI/gyro attitude by itself.
accel_mag = vecnorm(acc, 2, 2);
g_est = median(accel_mag, 'omitnan');
smooth_road = abs(accel_mag - g_est) < 0.35 * g_est;
gps_ok = turning & smooth_road & abs(lean_gps) < deg2rad(70);

lean = lean_imu;
gps_weight = 0.25;
lean(gps_ok) = angleMean(lean_imu(gps_ok), lean_gps(gps_ok), gps_weight);
lean = smoothdata(lean, 'movmean', 11);

result = table(t, rad2deg(lean), rad2deg(lean_imu), rad2deg(lean_gps), ...
    speed, rad2deg(yaw_rate), gps_ok, ...
    'VariableNames', ["Timestamp", "LeanAngle", "LeanImuCori", ...
    "LeanGps", "Speed", "YawRate", "GpsUsed"]);
outfile = fullfile(file.folder, strrep(file.name, '.MP4', '_lean_angle.csv'));
writetable(result, outfile)

%% plot
figure(1); clf
plot(t, result.LeanAngle, 'LineWidth', 1.5, 'DisplayName', 'fused lean')
hold on
plot(t, result.LeanImuCori, 'DisplayName', 'CORI + gyro')
plot(t, result.LeanGps, 'DisplayName', 'GPS cornering')
hold off
xlabel('time (s)')
ylabel('lean angle (deg, +right)')
legend('Location', 'best')
grid on
fontsize(18, 'points')

%% save

function angle = angleMean(a, b, weight_b)
angle = atan2((1 - weight_b) .* sin(a) + weight_b .* sin(b), ...
    (1 - weight_b) .* cos(a) + weight_b .* cos(b));
end
