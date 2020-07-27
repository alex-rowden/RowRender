function write3Dtex(filename, tex)
    out_file = fopen(filename, 'w');
    fwrite(out_file, size(tex, 1), 'int');
    fwrite(out_file, size(tex, 2), 'int');
    fwrite(out_file, size(tex,3), 'int');
    
    fwrite(out_file, tex, 'float');
end