function tt = getGoProTelemetry(video_file, type)

telemetry_file = fullfile(video_file.folder, strrep(video_file.name, '.MP4', "."+type));
if ~isfile(telemetry_file)
    cur_dir = cd;
    cd(video_file.folder)
    fprintf("Telemetry not found (" + type + ")\n\tRunning ""telemetrik " + ...
        video_file.name + """ in """ + strrep(video_file.folder, '\', '\\') + """\n")
    status = system("telemetrik --streams " + type + " " + video_file.name);
    if status ~= 0
        error("Could not run telemetrik (https://github.com/kmatzen/telemetrik)")
    end
    cd(cur_dir)
end

% load telemetry
tt = readlines(telemetry_file);
tt = split(tt(tt ~= ""), ',');
tt = str2double(tt);

end