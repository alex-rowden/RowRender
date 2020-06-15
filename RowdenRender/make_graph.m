lon_range = min(Longitude):.00001:max(Longitude);
lat_range = min(Latitude):.00001:max(Latitude);
sig_data = zeros(length(lon_range), length(lat_range));
sig_datas = containers.Map('all', sig_data);
for index=1:size(bssid_tables, 2)  
    vals = bssid_tables(index).gaussian;
    if vals.sigma == -1
        vals.sigma = avg_sigma / count;
    end
    z = gaussian(lon_range, lat_range, vals.sigma, vals.mu_x, vals.mu_y, vals.amplitude);
    key = int2str(bssid_tables(index).table.Frequency(1));
    if sig_datas.isKey(key)
        sig_datas(key) = max(sig_datas(key), z);
    else
        sig_datas(key) = z;
    end
    sig_datas('all') = max(sig_datas('all'), z);
end
mesh(sig_datas('all')); 