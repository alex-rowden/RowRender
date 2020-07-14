#pragma once
#include "RowRender.h"
#include <map>

struct WifiDataEntry {
	glm::vec2 location;
	std::string MAC;
	int floor, RSSI, freq, linkQuality, security, authAlg, cipherAlg;
};

class AVWWifiData
{
private:
	std::map<std::string, std::map<std::string, std::vector<WifiDataEntry>>> floorToWifiNameToEntries;
	std::map<std::string, std::map<std::string, std::vector<WifiDataEntry>>> wifiNameToMacToEntries;
	std::vector<std::string> available_macs;
	std::vector<std::string> wifinames;
	int numRouters = 0;
public:

	void loadWifi(std::string filename, std::string floor);
	std::vector<glm::mat4> getTransforms(std::vector<bool> wifiname, std::vector<bool> routers, glm::vec3 scale);
	int getNumWifiNames() { return wifiNameToMacToEntries.size(); }
	int getNumRouters() { return numRouters; }
	std::string getWifiName(int i) { return wifinames.at(i); }
	std::vector<std::string>getSelectedNames(std::vector<bool> names);
	void setAvailableMacs(std::vector<std::string> names);
	std::vector<std::string>getAvailablesMacs() { return available_macs; }
	void setWifiNames();
	void fillRouters(std::string wifiname, std::vector<bool> &routers, bool onoff);
	inline int findIndexToEntry(std::string wifiname);
};

