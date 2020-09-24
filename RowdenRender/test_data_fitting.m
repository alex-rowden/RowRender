data = getAVWData();
step = .0005;
x_range = min(data.x):step:max(data.x);
y_range = min(data.y):step:max(data.y);
z_range = min(data.floor):.1:max(data.floor);
all_umd_routers = data(strcmp(data.wifi_name, "umd"), :);
macs = unique(all_umd_routers.mac(:));
output = zeros(length(y_range),length(x_range),length(z_range));
for mac_index = 1:size(macs,1) 
    [mu, axis, r] = fitEllipsoid(macs(mac_index), all_umd_routers);
    v = plotEllipsoid(mu, axis, r, x_range, y_range, z_range);
    output = max(v, output);
end
write3Dtex("umd_routers_small.tex", output);
