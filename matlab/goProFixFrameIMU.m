function varargout = goProFixFrameIMU(x, y, z)
if nargout ~= 3 && nargout ~= 1
    error('Function requires exactly 1 or 3 output arguments.');
end
if nargin ~= nargout
    error('Function requires same IO argument number');
end
if nargin == 1
    y = x(:, 2);
    z = x(:, 3);
    x = x(:, 1);
end
yo = y;
xo = -z;
zo = x;
if nargout == 1
    varargout{1} = [yo, xo, zo];
end
end