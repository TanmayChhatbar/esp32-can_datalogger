clear
close all
clc

% https://github.com/kmatzen/telemetrik

filename = "GX010043";
gopro_rootdir = "T:\Media\GoPro";
file = findFile2(filename, gopro_rootdir);
clear filename gopro_rootdir

%% IMU data
acc = getGoProTelemetry(file, "ACCL");
gyro = getGoProTelemetry(file, "GYRO");

acc(:, 2:end) = goProFixFrameIMU(acc(:, 2:end));
gyro(:, 2:end) = goProFixFrameIMU(gyro(:, 2:end));

% Use the gyro time base and interpolate the accelerometer onto it.
t = (gyro(:, 1) - gyro(1, 1)) / 1e3;
t_acc = (acc(:, 1) - acc(1, 1)) / 1e3;
acc = interp1(t_acc, acc(:, 2:end), t, 'linear', 'extrap');
gyro = gyro(:, 2:end);

%% gyro/accelerometer lean estimate
% The first second must be stationary and upright.  This defines zero lean
% and the accelerometer direction in the local navigation frame.
initial = t < 1;
if nnz(initial) < 10
    error('Not enough samples to initialize the IMU estimator')
end

g0 = mean(acc(initial, :), 1);
g0 = g0 / norm(g0);
g = median(vecnorm(acc, 2, 2), 'omitnan');

% q rotates the current bike frame into the initial upright frame.  CORI
% is intentionally not used; gyro gives the lean motion between samples.
N = numel(t);
q = zeros(N, 4);
q(1, :) = [1 0 0 0];
accel_used = false(N, 1);
kp = 0.7;                         % accelerometer correction gain

for i = 2:N
    dt = min(max(t(i) - t(i-1), 0), 0.05);
    omega = gyro(i-1, :);
    R = quat2rotm(q(i-1, :));

    % A motorcycle accelerometer measures specific force.  In a steady
    % turn it is approximately normal to the bike, not vertical, so do
    % not use it as a gravity measurement while yawing or on a bump.
    acc_norm = norm(acc(i-1, :));
    straight = abs(omega(3)) < deg2rad(2);
    smooth = abs(acc_norm - g) < 0.12 * g;
    if straight && smooth
        acc_meas = acc(i-1, :) / acc_norm;
        acc_pred = (R' * g0')';
        tilt_error = cross(acc_pred, acc_meas);
        omega = omega + kp * tilt_error;
        accel_used(i) = true;
    end

    angle = norm(omega) * dt;
    if angle > 0
        dq = [cos(angle/2), sin(angle/2) * omega / norm(omega)];
        q(i, :) = quatmultiply(q(i-1, :), dq);
        q(i, :) = q(i, :) / norm(q(i, :));
    else
        q(i, :) = q(i-1, :);
    end
end

eul = quat2eul(q, 'ZYX');
lean = rad2deg(unwrap(eul(:, 3)));

result = table(t, lean, accel_used, ...
    'VariableNames', ["Timestamp", "LeanAngle", "AccelUsed"]);
outfile = fullfile(file.folder, strrep(file.name, '.MP4', '_lean_imu_codex.csv'));
writetable(result, outfile)

%% plot
figure(1); clf
plot(result.Timestamp, result.LeanAngle)
xlabel('time (s)')
ylabel('lean angle (deg)')
grid on
fontsize(24, 'points')
