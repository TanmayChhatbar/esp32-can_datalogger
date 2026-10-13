function euler_rates = gyroToEulerRates(gyro, eul)
% gyro = [p;q;r]
% eul = [yaw;pitch;roll]
% euler_rates = [yaw_dot;pitch_dot;roll_dot]

theta = eul(2); % pitch
phi   = eul(3); % roll

T = [1, sin(phi)*tan(theta), cos(phi)*tan(theta);
     0, cos(phi),           -sin(phi);
     0, sin(phi)/cos(theta), cos(phi)/cos(theta)];

rpy_rates = T * gyro;

euler_rates = [rpy_rates(3);
               rpy_rates(2);
               rpy_rates(1)];
end