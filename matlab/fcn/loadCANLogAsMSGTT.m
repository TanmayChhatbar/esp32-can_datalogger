function msg_tt = loadCANLogAsMSGTT(ffn, dbc_files)

% load can database(s)
db = canDatabase(dbc_files(1));
for dbc_file = dbc_files(2:end)
    db(end+1) = canDatabase(dbc_file);
end

opts = detectImportOptions(ffn, 'NumHeaderLines',0);
opts = setvartype(opts, opts.VariableNames, "char");
opts = setvartype(opts, "TimeStamp", 'double');
opts = setvartype(opts, "ID", 'uint32');
opts = setvartype(opts, ["Bus", "LEN", "ID"], 'double');
warning off
data = readtable(ffn, opts, "ReadVariableNames", true);
warning on

% rename
data = renamevars(data, 'TimeStamp', 'Time');
data = renamevars(data, 'Dir', 'Remote');
data = renamevars(data, 'LEN', 'Length');

% convert
data.Extended = data.Extended == "true";
data.Remote = data.Remote == "Rx";
data.Error = data.Extended & false; % assume no error frame
data.D1 = hex2dec(data.D1);
data.D2 = hex2dec(data.D2);
data.D3 = hex2dec(data.D3);
data.D4 = hex2dec(data.D4);
data.D5 = hex2dec(data.D5);
data.D6 = hex2dec(data.D6);
data.D7 = hex2dec(data.D7);
data.D8 = hex2dec(data.D8);
data.ID = uint32(data.ID);
data.Length = uint8(data.Length);

% reduce to one data array
data.Data = rowfun(@rwfcn, data).Var1;
data = removevars(data, "D"+string(1:8));

% add missing cols
data.Name = repmat({''}, height(data), 1);
data.Signals = repmat({struct([])}, height(data), 1);

% convert time to datetime
data.Time = datetime(data.Time/1e6, 'ConvertFrom', 'posixtime', 'TimeZone','local');

% resequence
data = data(:, ["Time","ID","Extended","Name","Data","Length","Signals","Error","Remote"]);

data = table2timetable(data);
msg_tt = canMessageTimetable(data,db);

end

%% helpers
function dd = rwfcn(Time, ID, Extended, Remote, Bus, Length, D1, D2, D3, D4, D5, D6, D7, D8, Error)
dd = [];
for i = 1:Length
    dd(end+1) = eval("D"+string(i));
end
dd = {uint8(dd)};
end
