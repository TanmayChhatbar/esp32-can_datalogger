function q = goProFixFrameCORI(q)
if nargout ~= 1
    error('Function requires exactly 1 output argument.');
end
% assume [w x y z]
seq_sel = [1 -4 2 3];
q = q(:, abs(seq_sel)) .* sign(seq_sel); % from gopro_dir_cal3
end