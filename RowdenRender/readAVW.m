function [data_entries] = readAVW(filename, this_floor)
    x = [];
    y = [];
    rssi = [];
    floor = [];
    mac = string([]);
    wifi_name = string([]);
    freq = [];
    link_quality = [];
    security_enabled = [];
    authAlg = [];
    cipherAlg = [];
    fid = fopen(filename, 'r');
    count = 1;
    while(~feof(fid))
        [sample, num] = fscanf(fid, '%f %f %d\n', [1,3]);
        for i=1:sample(3)
            x(count) = sample(1);
            y(count) = sample(2);
            line = fgetl(fid);
            tokens = strsplit(line);
            num = size(tokens, 2);
            offset = 0;
            if(num > 7)
                name = "";
                for j=1:(num-7)
                    name = name + tokens{j};
                    offset = j;
                end
                wifi_name(count)= string(name);
            else
                wifi_name(count) = string("");
            end
            mac(count) = string(tokens{offset + 1});
            rssi(count) = str2double(tokens{offset + 2});
            freq(count) = str2double(tokens{offset + 3});
            link_quality(count) = str2double(tokens{offset + 4});
            security_enabled(count) = str2double(tokens{offset + 5});
            authAlg(count) = str2double(tokens{offset + 6});
            cipherAlg(count) = str2double(tokens{offset + 7});
            floor(count) = this_floor;
            
            count = count + 1;
        end  
    end
    data_entries = table(x',y',floor', rssi', wifi_name', mac', freq', link_quality', security_enabled', authAlg', cipherAlg',...
        'VariableNames', {'x', 'y', 'floor', 'rssi', 'wifi_name', 'mac', 'freq', 'link_quality', 'security_enabled', 'authAlg', 'cipherAlg'});
end