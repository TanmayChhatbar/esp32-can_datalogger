function file = findFile2(filename, gopro_rootdir)
% find file
filelist = dir(fullfile(gopro_rootdir, '**\*.mp4'));
found_file = strings(0);
is = [];
for i = 1:length(filelist)
    f = filelist(i);
    if contains(f.name, filename)
        found_file(end+1) = string(fullfile(f.folder, f.name));
        is(end+1) = i;
    end
end

if isempty(found_file)
    error("Could not find file: " + filename + ".mp4");
elseif length(found_file) == 1
    file = filelist(is);
elseif length(found_file) > 1
    str = "Multiple files found\n\t";
    for i = 1:length(found_file)
        str = str + strrep(found_file(i), '\', '\\') + "\n\t";
    end
    fprintf(str)
    fprintf("\bContinue? [0-%d] ", length(found_file))
    response = input('', 's');
    if str2num(response) < 1 || str2num(response) > length(found_file)
        fprintf("Exiting\n")
        return
    end
    file = filelist(is(str2num(response))); % Select the chosen file
end
end