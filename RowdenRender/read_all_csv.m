function [output_data] = read_all_csv(list)
%READ_ALL_CSV Summary of this function goes here
%   Detailed explanation goes here
     output_data = read_csv(".\Content\Data\" + list{1});
    for file_index = 2:size(list)
        output_data = union(read_csv(".\Content\Data\" + list{file_index}), output_data);
    end
    
end

