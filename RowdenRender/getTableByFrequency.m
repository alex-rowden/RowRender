function [table] = getTableByFrequency(in_table, frequency)
    count = 1;
    for counter=1:size(in_table, 2)
       if (in_table(counter).table.Frequency(1) == frequency) || strcmp(frequency, 'all')
          table(count) = in_table(counter);
          count = count + 1;
       end
    end
end