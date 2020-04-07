function [z] = gaussian(X, Y, sigma, mu_x, mu_y, amplitude)
%UNTITLED3 Summary of this function goes here
%   Detailed explanation goes here
    X = X.';
    z = amplitude * exp(-((X-mu_x).^2/(2 * sigma^2) + (Y - mu_y).^2/ (2 * sigma^2)));

end

