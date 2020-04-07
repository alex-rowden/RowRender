
function write_data(out_filename, output_freqs, sig_datas)
    out_file = fopen(out_filename, 'w');
    fwrite(out_file, size(sig_datas('all'), 1), 'int');
    fwrite(out_file, size(sig_datas('all'), 2), 'int');
    fwrite(out_file, length(output_freqs), 'int');
    
    for i=1:length(output_freqs)
        fwrite(out_file, sig_datas(output_freqs{i}) * 255/100, 'uint8');
    end
end   