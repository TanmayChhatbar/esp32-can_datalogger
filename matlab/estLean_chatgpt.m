function lean_est = estLean_chatgpt(ax, ay, az, wx, wy, wz)
%ESTLEAN Estimate motorcycle lean angle from post-processed IMU data.
%
%   lean_est = estLean(ax, ay, az, wx, wy, wz)
%
% Inputs are vectors:
%   ax, ay, az   accelerometer [m/s^2]
%   wx, wy, wz   angular rate  [rad/s]
%
% Output:
%   lean_est     motorcycle lean angle [rad]
%
% Axis convention assumed:
%   +x = forward
%   +y = left
%   +z = up
%
% IMPORTANT:
%   Set Fs below to the actual IMU sample rate.
%
% Method:
%   - full 3-D quaternion integration of wx, wy, wz
%   - full 3-D accelerometer-vector correction
%   - accelerometer correction is reduced during dynamic acceleration
%   - no atan2(ay,az) accelerometer lean shortcut is used
%
% The accelerometer cannot distinguish gravity from sustained cornering
% acceleration, so gyro propagation is deliberately trusted much more
% during motorcycle dynamics.

    %% Settings
    Fs = 100;                  % <-- SET YOUR SAMPLE RATE [Hz]
    dt = 1/Fs;

    % g = 9.80665;
    g = 1;
    % Accelerometer attitude correction gain [1/s].
    % Smaller = trust gyro more during motorcycle motion.
    kp_acc = 0.5;

    % Accelerometer is progressively rejected when |a| differs from g.
    % accelTol = fractional deviation from g at which correction reaches 0.
    accelTol = 0.20;

    %% Input preparation
    ax = ax(:);
    ay = ay(:);
    az = az(:);

    wx = wx(:);
    wy = wy(:);
    wz = wz(:);

    N = numel(ax);

    assert(numel(ay)==N && numel(az)==N && ...
           numel(wx)==N && numel(wy)==N && numel(wz)==N, ...
           'All inputs must have the same length.');

    lean_est = zeros(N,1);

    %% Initial attitude
    %
    % We need an initial gravity direction. This uses the COMPLETE
    % accelerometer vector for initialization, not a roll atan2 shortcut.
    %
    % q represents body -> navigation rotation:
    %       v_nav = R(q) * v_body
    %
    % Navigation frame has +z upward.

    a0 = [ax(1); ay(1); az(1)];

    if norm(a0) < 1e-6
        error('Initial accelerometer sample is invalid.');
    end

    % Accelerometers at rest measure approximately +g upward under the
    % convention used here.
    b = a0 / norm(a0);
    n = [0; 0; 1];

    % Quaternion rotating measured initial vertical onto navigation vertical.
    q = quatFromTwoVectors(b, n);

    %% Main estimator
    for k = 1:N

        %--------------------------------------------------------------
        % 1. Full 3-axis angular-rate propagation
        %--------------------------------------------------------------
        omega = [wx(k); wy(k); wz(k)];

        dq = rotvecQuat(omega * dt);

        % q maps body -> navigation.
        % Body rotates relative to navigation, therefore propagate using
        % the inverse incremental body rotation.
        q = quatMultiply(q, quatConj(dq));
        q = q / norm(q);

        %--------------------------------------------------------------
        % 2. Full accelerometer-vector correction
        %--------------------------------------------------------------
        a = [ax(k); ay(k); az(k)];
        amag = norm(a);

        if amag > 1e-6

            aHat = a / amag;

            % Expected upward direction expressed in the body frame.
            %
            % nav vertical:
            zNav = [0;0;1];

            % q maps body -> nav, so transpose rotation maps nav -> body.
            Rbn = quatToRotm(q);
            zBodyPred = Rbn.' * zNav;

            % Vector attitude error.
            %
            % This is a genuine 3-D vector residual. No Euler-angle
            % accelerometer calculation is performed.
            eAcc = cross(aHat, zBodyPred);

            %----------------------------------------------------------
            % Reject accelerometer attitude information when the measured
            % specific-force magnitude is inconsistent with gravity.
            %----------------------------------------------------------
            magError = abs(amag - g) / g;

            wAcc = 1 - magError / accelTol;
            wAcc = max(0, min(1, wAcc));

            % Additional robustness:
            % large disagreement between predicted vertical and measured
            % specific force is typical in a hard motorcycle turn.
            alignment = dot(aHat, zBodyPred);

            % Only apply strong correction if the vectors are reasonably
            % consistent. Smooth weighting avoids a hard discontinuity.
            alignWeight = (alignment - 0.7) / (1.0 - 0.7);
            alignWeight = max(0, min(1, alignWeight));

            wAcc = wAcc * alignWeight;

            % Correction expressed as a small body-frame rotation.
            corr = kp_acc * wAcc * eAcc;

            dqCorr = rotvecQuat(corr * dt);

            q = quatMultiply(q, dqCorr);
            q = q / norm(q);
        end

        %--------------------------------------------------------------
        % 3. Extract motorcycle lean from full orientation
        %--------------------------------------------------------------
        Rbn = quatToRotm(q);

        % Body +z axis expressed in navigation coordinates.
        zBodyNav = Rbn(:,3);

        % Body +y axis expressed in navigation coordinates.
        yBodyNav = Rbn(:,2);

        % For the specified body convention, lean is rotation about the
        % longitudinal (+x) axis.
        %
        % This atan2 is ONLY extracting roll from the already estimated
        % 3-D attitude. It is not using accelerometer ay/az to estimate lean.
        lean_est(k) = atan2(-yBodyNav(3), zBodyNav(3));
    end

end


%==========================================================================

function q = rotvecQuat(r)
% Convert a rotation vector to quaternion [w;x;y;z].

    theta = norm(r);

    if theta < 1e-12
        q = [1; 0.5*r];
        q = q / norm(q);
        return
    end

    axis = r/theta;

    q = [cos(theta/2);
         axis*sin(theta/2)];
end


%==========================================================================

function q = quatMultiply(q1,q2)
% Quaternion product q = q1 (*) q2.
% Quaternion ordering is [w;x;y;z].

    w1 = q1(1);
    v1 = q1(2:4);

    w2 = q2(1);
    v2 = q2(2:4);

    q = [w1*w2 - dot(v1,v2);
         w1*v2 + w2*v1 + cross(v1,v2)];
end


%==========================================================================

function qc = quatConj(q)

    qc = [q(1); -q(2:4)];

end


%==========================================================================

function R = quatToRotm(q)
% Quaternion [w;x;y;z] to rotation matrix.

    q = q / norm(q);

    w = q(1);
    x = q(2);
    y = q(3);
    z = q(4);

    R = [ ...
        1 - 2*(y*y + z*z),   2*(x*y - z*w),       2*(x*z + y*w);
        2*(x*y + z*w),       1 - 2*(x*x + z*z),   2*(y*z - x*w);
        2*(x*z - y*w),       2*(y*z + x*w),       1 - 2*(x*x + y*y)];
end


%==========================================================================

function q = quatFromTwoVectors(a,b)
% Quaternion rotating unit vector a onto unit vector b.

    a = a / norm(a);
    b = b / norm(b);

    d = dot(a,b);

    if d < -0.999999
        % Vectors are opposite. Find any perpendicular rotation axis.
        if abs(a(1)) < 0.9
            axis = cross(a,[1;0;0]);
        else
            axis = cross(a,[0;1;0]);
        end

        axis = axis/norm(axis);
        q = [0; axis];
        return
    end

    v = cross(a,b);

    q = [1 + d;
         v];

    q = q/norm(q);
end