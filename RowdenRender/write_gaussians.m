function write_gaussians(filename, table, Latitude, Longitude)
    out_file = fopen(filename, 'w');
    scale_factor_lat = [min(Latitude), max(Latitude)];
    scale_factor_lon = [min(Longitude), max(Longitude)];
    range_lon = scale_factor_lon(2) - scale_factor_lon(1);
    range_lat = scale_factor_lat(2) - scale_factor_lat(1);
    fwrite(out_file, size(table, 2), 'int');
    for counter = 1:size(table,2)
       fwrite(out_file, (table(counter).gaussian.mu_x - mean(scale_factor_lon)) / range_lon, 'float');
       fwrite(out_file, (table(counter).gaussian.mu_y - mean(scale_factor_lat)) / range_lat, 'float'); 
       fwrite(out_file, table(counter).gaussian.amplitude, 'float'); 
       fwrite(out_file, (table(counter).gaussian.sigma) / sqrt(mean([range_lat, range_lon])), 'float'); 
    end
    
end