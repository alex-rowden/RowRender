function [table] = getAVWData()
    first_floor = readAVW('.\Content\Data\AVW1.txt', 1);
    second_floor = readAVW('.\Content\Data\AVW2.txt', 2);
    third_floor = readAVW('.\Content\Data\AVW3.txt', 3);
    fourth_floor = readAVW('.\Content\Data\AVW4.txt', 4);
    table = [first_floor; second_floor; third_floor; fourth_floor];
end