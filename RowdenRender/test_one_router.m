data = getAVWData(false);
unique_wifi_names = unique(data.wifi_name);
for i=1:size(unique_wifi_names, 1)
    name = unique_wifi_names(i);
    filepath = "Content/Data/MultilevelData/";
    if strcmp(name, "")
        filepath = filepath + "empty/";
    else
        filepath = filepath + name + "/";
    end
    mkdir(filepath);
    data_for_name = data(strcmp(data.wifi_name, name), :);
    unique_macs = unique(data_for_name.mac);
    for j= 1:size(unique_macs, 1)
        mac = unique_macs(j);
        [mu, axis, r] = fitEllipsoid(mac, data);
        filename = filepath + mac;
        fid = fopen(filename + ".ellipsoid", 'w');
        fwrite(fid, mu, 'float');
        fwrite(fid, axis, 'float');
        fwrite(fid, r, 'float');
        fclose(fid);
    end
end