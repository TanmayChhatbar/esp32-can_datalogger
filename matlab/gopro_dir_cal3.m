clear
close all
clc

gopro_rootdir = "T:\Media\GoPro";

%% gyro
filename = "GX010046";
file = findFile2(filename, gopro_rootdir);
gyro = getGoProTelemetry(file, "GYRO");
gyro = array2table(gyro);
gyro.Properties.VariableNames = ["Timestamp", "X", "Y", "Z"];
[gyro.X, gyro.Y, gyro.Z] = goProFixFrameIMU(gyro.X, gyro.Y, gyro.Z);
plot(gyro.Timestamp / 1e3, gyro.X, 'displayname', 'x')
hold on
plot(gyro.Timestamp / 1e3, gyro.Y, 'displayname', 'y')
plot(gyro.Timestamp / 1e3, gyro.Z, 'displayname', 'z')
hold off
legend
fontsize(24, 'points')

%% CORI
% file = file;
cori = getGoProTelemetry(file, "CORI");
cori = array2table(cori);
cori.Properties.VariableNames = ["Timestamp", "W", "X", "Y", "Z"];
t = [cori.W, cori.X, cori.Y, cori.Z];

yaw_int = cumtrapz(gyro.Timestamp / 1e3, gyro.Z);
pitch_int = cumtrapz(gyro.Timestamp / 1e3, gyro.Y);
roll_int = cumtrapz(gyro.Timestamp / 1e3, gyro.X);

eul_int(:, 1) = interp1(gyro.Timestamp, yaw_int, cori.Timestamp);
eul_int(:, 2) = interp1(gyro.Timestamp, pitch_int, cori.Timestamp);
eul_int(:, 3) = interp1(gyro.Timestamp, roll_int, cori.Timestamp);
    
if 0
    seq = combinationsOf1234();
    
    f = [];
    for i = 1:height(seq)
        tmp1 = t(:, abs(seq(i, :))) .* sign(seq(i, :));
        t1 = quat2eul(tmp1);
        t1 = t1 - t1(1, :);
        t1 = bestWrap(t1);
        plot(t1); hold on
        f(end+1) = norm(eul_int - t1);
    end
    [~, is] = sort(f, 'ascend');
    
    i2 = find(sum(seq(is(1:5), :)' < 0) == min(sum(seq(is(1:5), :)' < 0)));
    is = is(i2)
    for i = is
        tmp1 = t(:, abs(seq(i, :))) .* sign(seq(i, :));
        t1 = quat2eul(tmp1);
        t1 = t1 - t1(1, :);
        t1 = bestWrap(t1);
        
        figure
        subplot(311)
        plot(cori.Timestamp, [eul_int(:, 1), t1(:, 1)])
        legend
        subplot(312)
        plot(cori.Timestamp, [eul_int(:, 2), t1(:, 2)])
        legend
        subplot(313)
        plot(cori.Timestamp, [eul_int(:, 3), t1(:, 3)])
        legend
    end
    
    seq(is, :)
else
    % seq_sel = goProFixFrameCORI(1:4)
    t = goProFixFrameCORI(t);
    eul = quat2eul(t);
    eul = eul - eul(1, :);
    eul = bestWrap(eul);
    subplot(311)
    eul = bestWrap(eul);
    plot(cori.Timestamp, [eul_int(:, 1), eul(:, 1)])
    legend
    subplot(312)
    plot(cori.Timestamp, [eul_int(:, 2), eul(:, 2)])
    legend
    subplot(313)
    plot(cori.Timestamp, [eul_int(:, 3), eul(:, 3)])
    legend
end

function t1 = bestWrap(t1)
for j = find(range(t1) > 6.28)
    rng_tmp = range(t1(:, j));
    tmp1 = wrapTo2Pi(t1(:, j));
    if range(tmp1) < rng_tmp
        t1(:, j) = tmp1;
    end
    tmp1 = wrapToPi(t1(:, j));
    if range(tmp1) < rng_tmp
        t1(:, j) = tmp1;
    end
end
end

function seq = combinationsOf1234()
    seq0 = combinations([-1 1], [-2 2], [-3 3], [-4 4]);
    seq0 = table2array(seq0);
    seq = [];
    for j = 1:height(seq0)
        seq = [seq; perms(seq0(j, :))];
    end
end