#pragma once
#include "RowRender.h"
#include <map>

const int MAX_ROUTERS = 1200;

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
	std::vector<int> available_freqs;
	std::vector<std::string> wifinames;
	std::vector<int> frequencies;
	std::vector<float> color_indices;
	std::vector<std::string> router_strings;
	
	int numRouters = 0;
	GLuint uniformBuffer = 1, block_index = 1, bindingPoint = 1;
public:
	glm::mat4 ellipsoid_transform;
	glm::vec3 radius_stretch;
	glm::vec3 ellipsoidCoordinates(glm::vec3 fragPos, Ellipsoid ellipsoid);
	float ellipsoidDistance(glm::vec3 fragPos, glm::mat4 ellipsoid_transform, Ellipsoid ellipsoid);
	AVWWifiData();
	AVWWifiData(ShaderProgram);
	void pruneEntries();
	std::vector<std::string> getRouterStrings() { return router_strings; }
	void updateRouterStructure(std::vector<bool>routers, std::vector<bool> wifi_names, std::vector<bool> freqs, ShaderProgram shader, bool nearest_router = false);
	int getNumActiveRouters(std::vector<bool> routers);
	bool loadEllipsoid(std::string filename, Ellipsoid&ret, float wifi_num = 0);
	void loadWifi(std::string filename, std::string floor);
	std::vector<glm::mat4> getTransforms(std::vector<bool> wifiname, std::vector<bool> routers, glm::vec3 scale);
	int getNumWifiNames() { return wifiNameToMacToEntries.size(); }
	int getNumRouters() { return numRouters; }
	std::string getWifiName(int i) { return wifinames.at(i); }
	int getFrequencyAt(int i) { return frequencies.at(i); }
	std::vector<std::string>getSelectedNames(std::vector<bool> names);
	std::vector<int> getSelectedFreqs(std::vector<bool> freqs);
	void setAvailableMacs(std::vector<std::string> names);
	void setAvailableMacs(std::vector<std::string> names, std::vector<int> frequencies);
	void setAvailableFreqs(std::vector<std::string> names);
	std::vector<std::string>getAvailablesMacs() { return available_macs; }
	std::vector<int> getAvailableFreqs() { return available_freqs; }
	std::vector<int> getActiveFreqs(std::vector<bool>);
	void setRenderText(std::vector<std::string>&text, glm::vec3 samplePosition, std::vector<bool> routers);
	void setupStructures();
	void setNearestNRouters(int, glm::vec3, std::vector<bool>&, std::vector<bool>&, std::vector<bool>&);
	void fillRouters(std::string wifiname, std::vector<bool> &routers, bool onoff);
	inline int findIndexToEntry(std::string wifiname);
	std::vector<float> getColorIndices() { return color_indices; }
	bool loadVolume(std::string, VolumeData&data);
	float ellipsoidDistance(glm::vec3 fragPos, Ellipsoid ellipsoid);
	std::map<std::string, std::map<std::string, std::vector<WifiDataEntry>>> getFloorToWifiNameToEntries() {return floorToWifiNameToEntries;}
	std::map<std::string, std::map<std::string, std::vector<WifiDataEntry>>> getWifiNameToMacToEntries() { return wifiNameToMacToEntries; };
	WifiDataEntry lastEntry;
	Ellipsoid *routers;
};

