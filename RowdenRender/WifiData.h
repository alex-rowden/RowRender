#pragma once
#include "RowRender.h"
#include <map>
#include <set>
#include "delaunator.hpp"
class WifiData
{
public: 
	struct WifiEntry
{
	float longitude;
	float latitude;
	unsigned char GPSAccuracy;
	int frequency;
	int signalStrength;
	std::string SSID;
	std::string BSSID;
	long long unixTime;
};
private:
	std::map<std::string, std::vector<WifiEntry*>> NetIDToWifiEntries;
	std::map <std::string,
		std::map <std::string, std::vector<WifiEntry*>>> NetIDToMacToEntries;
	std::set<std::string> NetIDs;
	float area(int x1, int y1, int x2, int y2, int x3, int y3);
	bool isInside(int x1, int y1, int x2, int y2, int x3, int y3, int x, int y);

	float minLat = std::numeric_limits<float>::min();
	float minLon = std::numeric_limits<float>::min();
	float maxLat = std::numeric_limits<float>::max();
	float maxLon = std::numeric_limits<float>::max();
public:
	void Finalize(float latLonDist);
	void ComputeIDIntensities(std::string netID);
	bool loadCSV(const char*);
	float **GetIDIntensities(std::string, std::string);
	glm::vec2 getLatVec() { return glm::vec2(minLat, maxLat); }
	glm::vec2 getLonVec() { return glm::vec2(minLon, maxLon); }
	float longitudeRange, latitudeRange;
	int numLonCells, numLatCells;
	std::map<std::string, float **> netIDToIntensities;
	std::map<std::string, std::map<std::string, float **>> netIDToMacToIntensities;
	std::map<std::string, std::vector<WifiEntry*>> getNetIDToWifiEntries() { return NetIDToWifiEntries; }
	std::map<std::string, std::map<std::string, std::vector<WifiEntry*>>> getNetIDToMacToEntires() { return NetIDToMacToEntries; }


};

