function [v] = plotEllipsoid(mu, axis, r, x_range, y_range, z_range)
    [X_, Y_, Z_] = meshgrid(x_range, y_range, z_range);
    
    A = 0; %Rotation A angle
    B = 0; %Rotation B angle
    
    CA = cos(A);
    CB = cos(B);
    SA = sin(A);
    SB = sin(B);
    
    X = X_ - mu(1);
    Y = Y_ - mu(2);
    Z = Z_ - mu(3);
    
    XYZ = [X(:), Y(:), Z(:)] * axis;
    sz = size(X_);
    X = reshape(XYZ(:, 1), sz);
    Y = reshape(XYZ(:, 2), sz);
    Z = reshape(XYZ(:, 3), sz);
    
    s = (X) * CA * CB + (Y) * SA * CB + (Z) * SB;
    t = (Y) * CA - ((X ) * SA);
    u = (Z ) * CB - (Y) * SA * SB - (X) * CA * SB;
    
    v = s.^2/r(1).^2 + t.^2/r(2).^2 + u.^2/r(3).^2;
    
    
    if nargout == 0
        [faces, verts] = isosurface(X_, Y_, Z_, v, 1);
        p = patch('Faces', faces, 'Vertices', verts, 'FaceColor', 'red', 'EdgeColor', 'none');
        %isonormals(X, Y, Z, v, p);
        camlight;
        lighting gouraud;
    end
    v = max(1 - v, 0);
end