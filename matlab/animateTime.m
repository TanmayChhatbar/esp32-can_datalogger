function animateTime(hf, outfile)

% collect axes
cha = collectAxes(hf);

tstart = xlim;
tend = tstart(2);
tstart = tstart(1);

% add ylines
for c = cha
    axes(c);

    if ~exist('xlines', 'var')
        xlines = xline(tstart, '--', 'handlevisibility','off');
    else
        xlines(end+1) = xline(tstart, '--', 'handlevisibility','off');
    end
end

% make new temp folder
[folder, file, ext] = fileparts(outfile);
tmp_folder = fullfile(folder, 'tmp');
if ~isdir(tmp_folder)
    mkdir(tmp_folder);
end

% animate
fps = 30;

n = round(seconds(tend - tstart) * 30);
timestamps = tstart + seconds((1:n) / fps);

% hl = arrayfun(@(~) copyobj(hf, 0), 1:n);
parfor i = 1:n
    h = copyobj(hf, 0);
    fontsize(24,'points')
    arrayfun(@(c) xline(c, timestamps(i), '--', ...
        'LineWidth', 2, ...
        'HandleVisibility', 'off'), collectAxes(h));
    
    exportgraphics(h, fullfile(tmp_folder, sprintf('%04d', i) + ".png"), ...
        'Resolution', 300);

    close(h);
end

% compile
cmd = sprintf('ffmpeg -y -framerate 30 -i "./tmp/%%04d.png" -c:v libx264 -pix_fmt yuv420p -vf "pad=ceil(iw/2)*2:ceil(ih/2)*2" "./figs/%s"', outfile);
% cmd = sprintf('ffmpeg -framerate 30 -i "./tmp/%%04d.png" -c:v libx264 -pix_fmt yuv420p "./figs/%s"', outfile);
system(cmd);

% remove tmp files
rmdir('./tmp', 's')
end

function cha = collectAxes(hf)
    ch = hf.Children();
    cha = [];

    for c = ch'
        if isa(c, 'matlab.graphics.axis.Axes')
            cha = [cha, c];
        end
    end
end
