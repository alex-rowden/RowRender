 data = read_csv(".\Content\Data\combined_data.csv");
bssids = data(:, "BSSID");
unique_bssids = unique(bssids);
bssid_tables = struct();
default_sigma = -1;
avg_sigma = 0;
count = 0;
show_n_plots = 0;
for bssid_index=1:size(unique_bssids, 1)
    curr_table =  data(data.BSSID == unique_bssids{bssid_index, :}, :);
    bssid_tables(bssid_index).table = curr_table;
    X = curr_table.Longitude;
    Y = curr_table.Latitude;
    Z = -curr_table.SignalstrengthdBm;
    [amp, ind] = max(Z);
    sum_intensities = sum(Z);
    sigma_est = cov(Z);
    
    if(length(Z) > 3)
        
        [fit, gof] = gaussianFit(X, Y, Z, [amp, sum(X .* Z)/sum_intensities, sum(Y .* Z)/sum_intensities, sigma_est], count < show_n_plots);
        values = coeffvalues(fit);
        gauss.amplitude = values(1);
        gauss.mu_x = values(2);
        gauss.mu_y = values(3);
        gauss.sigma = values(4);
        avg_sigma = avg_sigma + gauss.sigma;
        count = count  + 1;
    elseif(length(Z) == 1)
        gauss.amplitude = Z(1);
        gauss.mu_x = X(1);
        gauss.mu_y = Y(1);
        gauss.sigma = default_sigma;
    elseif(length(Z) == 2)
        [gauss.amplitude, ind] = max(Z);
        gauss.mu_x = X(ind);
        gauss.mu_y = Y(ind);
        gauss.sigma = sigma_est;
        %count = count + 1;
    elseif(length(Z) == 3)
        [gauss.amplitude, ind] = max(Z);
        gauss.mu_x = X(ind);
        gauss.mu_y = Y(ind);
        gauss.sigma = sigma_est;
        %count = count + 1;
    end
    bssid_tables(bssid_index).gaussian = gauss;
end

