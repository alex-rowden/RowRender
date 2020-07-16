#include "AVWWifiData.h"


void tokenizer(std::string str, const char* delim, std::vector<std::string>& out) {
	char* next_token;
	char* token = strtok_s(const_cast<char*>(str.c_str()), delim, &next_token);
	while (token != nullptr) {
		out.push_back(std::string(token));
		token = strtok_s(nullptr, delim, &next_token);
	}
}

void AVWWifiData::loadWifi(std::string filename, std::string floor) {
	std::ifstream inputFile;
	inputFile.open(filename);
	if (inputFile.is_open())
	{

		floorToWifiNameToEntries.insert(std::make_pair(floor,
			std::map<std::string, std::vector<WifiDataEntry>>()));

		while (inputFile.good())
		{
			//Read the values
			std::string xPosStr;
			getline(inputFile, xPosStr, ' ');
			std::string yPosStr;
			getline(inputFile, yPosStr, ' ');
			std::string countStr;
			getline(inputFile, countStr);

			float xPos = atof(xPosStr.c_str());
			float yPos = atof(yPosStr.c_str());
			int count = atoi(countStr.c_str());

			glm::vec2 location = glm::vec2(xPos, yPos);

			for (int i = 0; i < count; i++)
			{
				std::string entireLine;
				getline(inputFile, entireLine);

				std::vector<std::string> parts;
				tokenizer(entireLine, " ", parts);
				int numParts = parts.size();

				std::string networkName;
				if (numParts > 7)
				{
					int numNameParts = numParts - 7;
					for (int j = 0; j < numNameParts; j++)
					{
						networkName += parts.at(j);
						if (j < numNameParts - 1)
							networkName += " ";
					}
				}

				//getline(inputFile, networkName, ' ');

				std::string macAddress = parts.at(numParts - 7);
				//getline(inputFile, macAddress, ' ');

				std::string rssiStr = parts.at(numParts - 6);
				//getline(inputFile, rssiStr, ' ');

				std::string freqStr = parts.at(numParts - 5);
				//getline(inputFile, freqStr, ' ');

				std::string linkQualityStr = parts.at(numParts - 4);
				//getline(inputFile, linkQualityStr, ' ');

				std::string securityEnabledStr = parts.at(numParts - 3);
				//getline(inputFile, securityEnabledStr, ' ');

				std::string authAlgorithmStr = parts.at(numParts - 2);
				//getline(inputFile, authAlgorithmStr, ' ');

				std::string cipherAlgorithmStr = parts.at(numParts - 1);
				//getline(inputFile, cipherAlgorithmStr);

				WifiDataEntry entry;
				entry.location = location;
				entry.MAC = macAddress;
				entry.RSSI = atoi(rssiStr.c_str());
				entry.freq = atoi(freqStr.c_str());
				entry.linkQuality = atoi(linkQualityStr.c_str());
				entry.security = (bool)atoi(securityEnabledStr.c_str());
				entry.authAlg = atoi(authAlgorithmStr.c_str());
				entry.cipherAlg = atoi(cipherAlgorithmStr.c_str());
				entry.floor = atoi(floor.c_str());

				if (floorToWifiNameToEntries.at(floor).find(networkName) ==
					floorToWifiNameToEntries.at(floor).end())
				{
					floorToWifiNameToEntries.at(floor).insert(
						std::make_pair(networkName, std::vector<WifiDataEntry>()));
				}

				if (wifiNameToMacToEntries.find(networkName) == wifiNameToMacToEntries.end())
					wifiNameToMacToEntries.insert(std::make_pair(networkName, std::map<std::string, std::vector<WifiDataEntry>>()));
				if (wifiNameToMacToEntries.at(networkName).find(macAddress) == wifiNameToMacToEntries.at(networkName).end()) {
					wifiNameToMacToEntries.at(networkName).insert(std::make_pair(macAddress, std::vector<WifiDataEntry>()));
					numRouters++;
				}

				wifiNameToMacToEntries.at(networkName).at(macAddress).emplace_back(entry);
				floorToWifiNameToEntries.at(floor).at(networkName).emplace_back(entry);
			}


		}
	}
	else
	{
		std::cout << "Could not open " << filename << std::endl;
	}


	inputFile.close();
}

void AVWWifiData::setAvailableMacs(std::vector<std::string> names) {
	available_macs = std::vector<std::string>();
	for (auto name : names) {
		for (auto mac2Entry : wifiNameToMacToEntries.at(name)) {
			available_macs.emplace_back(mac2Entry.first);
		}
	}
	return;
}

std::vector<std::string>AVWWifiData::getSelectedNames(std::vector<bool>names) {
	std::vector<std::string> out;
	for (int i = 0; i < names.size(); i++) {
		if (names.at(i))
			out.emplace_back(getWifiName(i));
	}
	return out;
}

inline int AVWWifiData::findIndexToEntry(std::string wifiname) {
	return std::distance(available_macs.begin(), std::find(available_macs.begin(), available_macs.end(), wifiname));

}

std::vector<glm::mat4> AVWWifiData::getTransforms(std::vector<bool> wifinames, std::vector<bool> routers, glm::vec3 scale) {
	std::vector<glm::mat4> out;
	color_indices.clear();
	int wifinum = 0;
	for (auto NameToMacToEntries : wifiNameToMacToEntries) {
		if (!wifinames.at(wifinum++))
			continue;
		for (auto MacToEntries : NameToMacToEntries.second) {
			int index = findIndexToEntry(MacToEntries.first);
			if (!routers.at(index)) {
				continue;
			}
			for (auto Entries : MacToEntries.second) {
				float size = ((Entries.RSSI + 100) / 30.0f);
				glm::mat4 translation = glm::translate(glm::mat4(1), glm::vec3(Entries.location.y * scale.x, Entries.location.x * scale.y, Entries.floor * scale.z));
				out.push_back(glm::scale(translation, glm::vec3(size)));
				color_indices.push_back((float)wifinum/wifiNameToMacToEntries.size());
			}
		}
	}
	return out;
}

void AVWWifiData::setWifiNames() {
	for (auto element : wifiNameToMacToEntries)
		wifinames.emplace_back(element.first);
	return;
}

void AVWWifiData::fillRouters(std::string wifiname, std::vector<bool> &routers, bool onoff){
	for (auto elem : wifiNameToMacToEntries[wifiname]) {
		int index = findIndexToEntry(elem.first);
		routers.at(index) = onoff;
	}
	return;
}