function [mu, axis, r] = fitEllipsoid(mac, data)
    router_data = data(strcmp(data.mac, mac), :);
    if(size(router_data, 1) < 3)
        r = [0; 0; 0];
        mu = [0 0 0];
        axis = [1 0 0; 0 1 0; 0 0 1];
        return
    end
    router_coords = [router_data.x, router_data.y, router_data.floor];
    %router_weights = (router_data.rssi/min(router_data.rssi));
    router_weights = (10.^(router_data.rssi/10));
    router_weights = router_weights/sum(router_weights);
    [axis,score,r,tsquared,explained,mu] = pca(router_coords, 'Weights', router_weights);
    r = 3 * sqrt(r);
    if(size(r, 1) == 2)
       r = [r; 0]; 
       axis = [axis.'; [0 0 1]].';
    end
    if(size(r, 1) < 3)
         r = [0;0;0];
         return;
    end
end