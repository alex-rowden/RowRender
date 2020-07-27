#pragma once
#include "RowRender.h"
#include <map>

struct WifiDataEntry {
	glm::vec2 location;
	std::string MAC;
	int floor, RSSI, freq, linkQuality, security, authAlg, cipherAlg;
};

struct VolumeData {
	std::vector<glm::vec3> data;
	glm::uvec3 dimensions;
	unsigned int& height = dimensions[0];
	unsigned int& width = dimensions[1];
	unsigned int& depth = dimensions[2];
	int getSize() { return dimensions.x * dimensions.y * dimensions.z; }
};

class AVWWifiData
{
private:
	std::map<std::string, std::map<std::string, std::vector<WifiDataEntry>>> floorToWifiNameToEntries;
	std::map<std::string, std::map<std::string, std::vector<WifiDataEntry>>> wifiNameToMacToEntries;
	std::vector<std::string> available_macs;
	std::vector<std::string> wifinames;
	std::vector<float> color_indices;
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
	std::vector<float> getColorIndices() { return color_indices; }
	bool loadVolume(std::string, VolumeData&data);
};

