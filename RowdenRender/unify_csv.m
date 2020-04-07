combined_csv = read_all_csv({
    "AlexWifi-2-4-19(2).csv",
    "AlexWifi-2-4-19.csv",
    "AlexWifi-2-5-19.csv",
    "AlexWifi-2-6-19.csv",
    "AlexWifi-2-26-19.csv",
    "AlexWifi-2-27-19.csv",
    "EricWifi-2-4-19(2).csv",
    "EricWifi-2-4-19.csv",
    "EricWifi-2-5-19.csv",
    "EricWifi-2-6-19.csv",
    "EricWifi-2-26-19.csv",
    "EricWifi-2-27-19(2).csv",
    "EricWifi-2-27-19.csv"
    });
writetable(combined_csv, "combined_data.csv");