clear
close all
clc

gopro_rootdir = "T:\Media\GoPro";

%% acc
filename = "GX010044";
file = findFile2(filename, gopro_rootdir);
acc = getGoProTelemetry(file, "ACCL");
acc = array2table(acc);
acc.Properties.VariableNames = ["Timestamp", "X", "Y", "Z"];
[acc.X, acc.Y, acc.Z] = goProFixFrame(acc.X, acc.Y, acc.Z);
plot(acc.Timestamp / 1e3, acc.X, 'displayname', 'x')
hold on
plot(acc.Timestamp / 1e3, acc.Y, 'displayname', 'y')
plot(acc.Timestamp / 1e3, acc.Z, 'displayname', 'z')
hold off
legend
fontsize(24, 'points')

%% gyro
filename = "GX010046";
file = findFile2(filename, gopro_rootdir);
gyro = getGoProTelemetry(file, "GYRO");
gyro = array2table(gyro);
gyro.Properties.VariableNames = ["Timestamp", "X", "Y", "Z"];
[gyro.X, gyro.Y, gyro.Z] = goProFixFrame(gyro.X, gyro.Y, gyro.Z);
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
figure(1235415); clf
subplot(211)
plot(gyro.Timestamp / 1e3, gyro.X, 'displayname', 'x')
hold on
plot(gyro.Timestamp / 1e3, gyro.Y, 'displayname', 'y')
plot(gyro.Timestamp / 1e3, gyro.Z, 'displayname', 'z')
hold off
legend

t = [cori.X, cori.Y, cori.Z, cori.W];
seq0 = combinations([-1 1], [-2 2], [-3 3], [-4 4]);
seq0 = table2array(seq0);
seq = [];
for j = 1:height(seq0)
    seq = [seq; perms(seq0(j, :))];
end
height(seq)
i1s = find(cori.Timestamp > 8e2, 1); % +ve pitch (camera turns down)
i1e = find(cori.Timestamp > 2e3, 1);
i2s = find(cori.Timestamp > 2.7e3, 1); % -ve yaw (camera turns right)
i2e = find(cori.Timestamp > 4.2e3, 1);
i3s = find(cori.Timestamp > 4.8e3, 1); % +ve roll (camera tilts right)
i3e = find(cori.Timestamp > 6.5e3, 1);

yaw_int = cumtrapz(gyro.Timestamp / 1e3, gyro.Z);
pitch_int = cumtrapz(gyro.Timestamp / 1e3, gyro.Y);
roll_int = cumtrapz(gyro.Timestamp / 1e3, gyro.X);
f = [];
for i = 1:height(seq)
    tmp1 = t(i1s:i1e, :) .* sign(seq(i, :));
    t1 = quat2eul(tmp1(:, abs(seq(i, :))));  % range of time for pitch 
    t1 = bestWrap(t1);
    tmp2 = t(i2s:i2e, :) .* sign(seq(i, :));
    t2 = quat2eul(tmp2(:, abs(seq(i, :))));  % range of time for yaw
    t2 = bestWrap(t2);
    tmp3 = t(i3s:i3e, :) .* sign(seq(i, :));
    t3 = quat2eul(tmp3(:, abs(seq(i, :))));  % range of time for roll
    t3 = bestWrap(t3);
    
    r1 = range(t1); r1 = r1(2);
    r2 = range(t2); r2 = r2(3);
    r3 = range(t3); r3 = r3(1);
    if all([r1 r2 r3] > .09) && ...
            (mean(t1(:, 2)) < 0 && ...
            mean(t2(:, 3)) > 0 && ...
            mean(t3(:, 1)) < 0)
        tmp1 = t .* sign(seq(i, :));
        eul = quat2eul(tmp1(:, abs(seq(i, :))));
        eul = bestWrap(eul);
        eul = eul - eul(1, :);
        figure
        subplot(311)
        plot(cori.Timestamp / 1e3, eul(:, 1), 'displayname', 'yaw')
        hold on
        plot(gyro.Timestamp / 1e3, yaw_int, 'displayname', 'integrated')
        hold off
        legend
        subplot(312)
        plot(cori.Timestamp / 1e3, eul(:, 2), 'displayname', 'pitch')
        hold on
        plot(gyro.Timestamp / 1e3, cumtrapz(gyro.Timestamp / 1e3, gyro.Y), 'displayname', 'integrated')
        hold off
        legend
        subplot(313)
        plot(cori.Timestamp / 1e3, eul(:, 3), 'displayname', 'roll')
        hold on
        plot(gyro.Timestamp / 1e3, cumtrapz(gyro.Timestamp / 1e3, gyro.X), 'displayname', 'integrated')
        hold off
        legend
        fontsize(24, 'points')
        title(num2str(i))
    end

end

function t1 = bestWrap(t1)
for j = find(range(t1) > 6.28)
    rng_tmp = range(t1(:, j));
    tmp1 = wrapTo2Pi(t1(:, j));
    if range(tmp1) < rng_tmp
        t1(:, j) = tmp1;
    end
end
end
