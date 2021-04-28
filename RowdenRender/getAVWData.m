function [table] = getAVWData(legacy)
    if legacy
        first_floor = readAVW('.\Content\Data\AVW1.txt', 0);
        second_floor = readAVW('.\Content\Data\AVW2.txt', 1);
        third_floor = readAVW('.\Content\Data\AVW3.txt', 2);
        fourth_floor = readAVW('.\Content\Data\AVW4.txt', 3);
    else
        offset = .03;
        step = .15;
        first_floor = readAVW('.\Content\Data\DataCollection1.txt', 0, offset, step);
        second_floor = readAVW('.\Content\Data\DataCollection2.txt', 1, offset, step);
        third_floor = readAVW('.\Content\Data\DataCollection3.txt', 2, offset, step);
        fourth_floor = readAVW('.\Content\Data\DataCollection4.txt', 3, offset, step);
    end
    table = [first_floor; second_floor; third_floor; fourth_floor];

end