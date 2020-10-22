#include "AVWWifiData.h"
#include <glm/gtc/matrix_access.hpp>

void tokenizer(std::string str, const char* delim, std::vector<std::string>& out) {
	char* next_token;
	char* token = strtok_s(const_cast<char*>(str.c_str()), delim, &next_token);
	while (token != nullptr) {
		out.push_back(std::string(token));
		token = strtok_s(nullptr, delim, &next_token);
	}
}

glm::vec3 AVWWifiData::ellipsoidCoordinates(glm::vec3 fragPos, Ellipsoid ellipsoid) {
	glm::vec3 modified_coords = glm::vec3(ellipsoid.mu);
	modified_coords = glm::vec3(ellipsoid_transform * glm::vec4(glm::vec3(modified_coords), 1));
	modified_coords = (fragPos - modified_coords);

	return modified_coords;
}

float AVWWifiData::ellipsoidDistance(glm::vec3 fragPos, glm::mat4 ellipsoid_transform, Ellipsoid ellipsoid) {
	glm::vec3 modified_coords = glm::mat3(ellipsoid.axis) * ellipsoidCoordinates(fragPos, ellipsoid);
	modified_coords = (modified_coords * modified_coords) / (9.0f * abs(glm::mat3(ellipsoid.axis) * abs(radius_stretch * glm::vec3(ellipsoid.r))));

	float distance = 0;
	for (int i = 0; i < 3; i++) {
		distance += modified_coords[i];
	} 
	return sqrt(distance);
}


AVWWifiData::AVWWifiData()
{
	std::cerr << "Wifi data must be linked to shader program" << std::endl;
}

AVWWifiData::AVWWifiData(ShaderProgram p) {
	block_index = glGetUniformBlockIndex(p.getShader(), "EllipsoidBlock");
	glUniformBlockBinding(p.getShader(), block_index, bindingPoint);
	glGenBuffers(1, &uniformBuffer);
	
}

bool AVWWifiData::loadEllipsoid(std::string filename, Ellipsoid&ret, float wifi_num) {
	std::ifstream file = std::ifstream(filename, std::ios::in | std::ios::binary);
	if (!file)
		return false;
	file.read(reinterpret_cast<char*>(glm::value_ptr(ret.mu)), 3 * sizeof(float));
	glm::mat3 axis;
	file.read(reinterpret_cast<char*>(glm::value_ptr(axis)), 9 * sizeof(float));
	file.read(reinterpret_cast<char*>(glm::value_ptr(ret.r)), 3 * sizeof(float));
	ret.mu = glm::vec4(ret.mu.y, ret.mu.x, ret.mu.z, wifi_num);
	ret.r = sqrt(abs(glm::vec4(ret.r.y, ret.r.x, ret.r.z, 0) / 3.0f));
	ret.axis = glm::mat3(glm::row(axis, 0), glm::row(axis, 1), glm::row(axis, 2));
	if (ret.r.z < .01) {
		ret.r.z = .01;
	}
	return true;
}

int AVWWifiData::getNumActiveRouters(std::vector<bool> routers) {
	int ret = 0;
	for (int i = 0; i < routers.size(); i++)
		if (routers[i]) ret++;
	return ret;
}

bool AVWWifiData::loadVolume(std::string filename, VolumeData&data) {
	std::ifstream file = std::ifstream(filename, std::ios::in | std::ios::binary);
	if (!file)
		return false;
	unsigned int dims[3];
	file.read(reinterpret_cast<char*>(dims), 3 * 4 * sizeof(char)); //I know that uint is 4 bytes in matlab and I'm not sure about it here so this is how I am doing it
	data.dimensions = glm::uvec3(dims[0], dims[1], dims[2]);
	unsigned long total_size = data.getSize();
	data.data.resize(total_size);
	file.read(reinterpret_cast<char*>(data.data.data()), data.data.size() * sizeof(float));
	return true;
}

float AVWWifiData::ellipsoidDistance(glm::vec3 fragPos, Ellipsoid ellipsoid) {
	glm::vec3 modified_coords = glm::vec3(ellipsoid.mu);
	modified_coords = glm::vec3((ellipsoid_transform * glm::vec4(modified_coords, 1)));
	modified_coords = glm::mat3(ellipsoid.axis) * (fragPos - modified_coords);

	float distance = 0;
	for (int i = 0; i < 3; i++) {
		distance += (modified_coords[i] * modified_coords[i]) / (9 * abs(glm::mat3(ellipsoid.axis) * abs(radius_stretch * glm::vec3(ellipsoid.r)))[i]);
	}
	return distance;
}

std::vector<int> AVWWifiData::getActiveFreqs(std::vector<bool> freqs) {
	std::vector<int> out;
	for (int i = 0; i < freqs.size(); i++) {
		if (freqs.at(i))
			out.emplace_back(frequencies.at(i));
	}
	return out;
}

void AVWWifiData::setRenderText(std::vector<std::string>&text, glm::vec3 samplePosition, std::vector<bool> router_bools) {
	text.clear();
	text = std::vector<std::string>(getNumActiveRouters(router_bools));
	for (int i = 0; i < getNumActiveRouters(router_bools); i++) {
		std::string router_string = router_strings.at(i);
		auto router = routers[i];
		auto first_delim = router_string.find(":");
		auto second_delim = router_string.find(":", first_delim);
		std::string prefix = router_string.substr(0, second_delim);
		float inv_dist = 1 - ellipsoidDistance(samplePosition, router);
		if (inv_dist < 0)
			inv_dist = 0;
		text[i] = prefix + ":" + std::to_string( inv_dist * 100) + "%";
	}
}

void AVWWifiData::setNearestNRouters(int n, glm::vec3 position, std::vector<bool>& wifinames, std::vector<bool>& routers, std::vector<bool>&freqs) {
	std::vector<float> min_distances(n);
	std::fill(min_distances.begin(), min_distances.end(), std::numeric_limits<float>().max());
	std::vector<int> min_indices(n);
	bool empty = true;
	for (int i = 0; i < wifinames.size(); i++) {
		if (wifinames.at(i)) {
			empty = false;
			break;
		}
	}
	int wifinum = 0, router_num = 0, swap_ind = 0, curr_router, swap_freq_ind;
	Ellipsoid currEllipsoid;
	float currDist, swap_dist;
	std::vector<int> active_freqs = getActiveFreqs(freqs);
	std::vector<int> freq_indices(n);
	for (auto name2Mac : wifiNameToMacToEntries) {
		if (wifinames[wifinum++] || empty) {
			for (auto mac2Entries : name2Mac.second) {
				auto loc = std::find(active_freqs.begin(), active_freqs.end(),
					mac2Entries.second.at(0).freq);
				if(loc == active_freqs.end())
					continue;
				std::string wifiname = name2Mac.first;
				if (!wifiname.compare(""))
					wifiname = "empty";
				if (!loadEllipsoid("./Content/Data/AVW_data/"
					+ wifiname + "/"
					+ mac2Entries.first + ".ellipsoid",
					currEllipsoid)) {
					std::cerr << "Error loading ellipsoid" << std::endl;
				}
				currDist = ellipsoidDistance(position, currEllipsoid);
				curr_router = router_num++;
				auto curr_freq = std::distance(active_freqs.begin(), loc);
				for (int i = 0; i < n; i++) {
					if (currDist < min_distances[i]) {
						swap_dist = min_distances[i];
						swap_ind = min_indices[i];
						swap_freq_ind = freq_indices[i];
						min_distances[i] = currDist;
						min_indices[i] = curr_router;
						freq_indices[i] = curr_freq;
						currDist = swap_dist;
						curr_router = swap_ind;
						curr_freq = swap_freq_ind;
					}
				}
			}
		}
		std::fill(routers.begin(), routers.end(), false);
		std::fill(freqs.begin(), freqs.end(), false);
		for (auto ind : min_indices) {
			routers[ind] = true;
		}for (auto ind : freq_indices) {
			freqs[ind] = true;
		}
	}

}

void AVWWifiData::loadWifi(std::string filename, std::string floor) {
	int max_rssi = -1000;
	int min_rssi = 1000;
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
				if (max_rssi < entry.RSSI)
					max_rssi = entry.RSSI;
				if (min_rssi > entry.RSSI)
					min_rssi = entry.RSSI;

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

void AVWWifiData::updateRouterStructure(std::vector<bool>router_bools, std::vector<bool> wifinames, std::vector<bool> freqs, ShaderProgram model_shader, bool nearest_router) {
	int wifinum = 0;
	int i = 0;
	delete[] routers;
	routers = new Ellipsoid[getNumActiveRouters(router_bools)];
	router_strings.empty();
	router_strings.resize(getNumActiveRouters(router_bools));
	for (auto NameToMacToEntries : wifiNameToMacToEntries) {
		if (!wifinames.at(wifinum++))
			continue;
		for (auto MacToEntries : NameToMacToEntries.second) {
			int index = findIndexToEntry(MacToEntries.first);
			if (!router_bools.at(index)) {
				continue;
			}
			if (MacToEntries.second.size() > 3) {
				if (i > MAX_ROUTERS) {
					std::cerr << "Selected too many routers" << std::endl;
					return;
				}
				std::string wifiname = NameToMacToEntries.first;
				if (!wifiname.compare(""))
					wifiname = "empty";
				float router_index = 0;
				if (nearest_router)
					router_index = (i + getNumWifiNames() + 1 ) / (float)(getNumActiveRouters(router_bools) + getNumWifiNames() + 1);
				else
					router_index = wifinum / (float)(getNumWifiNames() + 1);
				if (!loadEllipsoid("./Content/Data/AVW_data/"
					+ wifiname + "/"
					+ MacToEntries.first + ".ellipsoid",
					routers[i], router_index)) {
					
					std::cerr << "Error loading ellipsoid" << std::endl;
				}
				else {
					std::vector<int> active_freqs = getActiveFreqs(freqs);
					auto it = std::find(active_freqs.begin(), active_freqs.end(), MacToEntries.second.at(0).freq);
					int index = std::distance(active_freqs.begin(), it);

					routers[i].r.w = (float)index;
					router_strings[i] = wifiname + ": " + MacToEntries.first + ": " + std::to_string(MacToEntries.second.at(0).freq);
					i++;
				}
			}
			else {
				std::cout << "not enough samples for interpolation" << std::endl;
			}
		}
	}
	//model_shader.setEllipsoids(routers);
	glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
	glBufferData(GL_UNIFORM_BUFFER, i * sizeof(Ellipsoid), routers, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint, uniformBuffer);
	model_shader.SetUniform1i("num_routers", i);
}



void AVWWifiData::setAvailableMacs(std::vector<std::string> names) {
	setAvailableMacs(names, frequencies);
}
void AVWWifiData::setAvailableMacs(std::vector<std::string> names,
	std::vector<int> freqs) {
	available_macs = std::vector<std::string>();
	for (auto name : names) {
		for (auto mac2Entry : wifiNameToMacToEntries.at(name)) {
			if(std::find(freqs.begin(), freqs.end(), mac2Entry.second.at(0).freq) != freqs.end())
				available_macs.emplace_back(mac2Entry.first);
		}
	}
	return;
}

void AVWWifiData::setAvailableFreqs(std::vector<std::string> names) {
	available_freqs = std::vector<int>();
	for (auto name : names) {
		for (auto mac2entry : wifiNameToMacToEntries.at(name)) {
			if (mac2entry.second.size() < 4)
				continue;
			for (auto entry : mac2entry.second) {
				if (std::find(available_freqs.begin(), available_freqs.end()
					, entry.freq) == available_freqs.end()) {
					available_freqs.push_back(entry.freq);
				}
			}
		}
	}
	std::sort(available_freqs.begin(), available_freqs.end());
}

std::vector<std::string>AVWWifiData::getSelectedNames(std::vector<bool>names) {
	std::vector<std::string> out;
	for (int i = 0; i < names.size(); i++) {
		if (names.at(i))
			out.emplace_back(getWifiName(i));
	}
	return out;
}
std::vector<int>AVWWifiData::getSelectedFreqs(std::vector<bool>freqs) {
	std::vector<int> out;
	for (int i = 0; i < freqs.size(); i++) {
		if (freqs.at(i))
			out.emplace_back(getAvailableFreqs().at(i));
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
				lastEntry = Entries;
				float size = ((Entries.RSSI + 100) / 30.0f) * .1;
				glm::mat4 translation = glm::translate(glm::mat4(1), glm::vec3(Entries.location.y * scale.x, Entries.location.x * scale.y, Entries.floor * scale.z));
				out.push_back(glm::scale(translation, glm::vec3(size)));
				color_indices.push_back((float)wifinum/(wifiNameToMacToEntries.size() + 1));
			}
		}
	}
	return out;
}

void AVWWifiData::pruneEntries() {
	for (auto name : wifiNameToMacToEntries) {
		for (auto mac : name.second) {
			if (mac.second.size() < 4) {
				wifiNameToMacToEntries[name.first].erase(mac.first);
			}
		}
		if (wifiNameToMacToEntries[name.first].size() == 0) {
			wifiNameToMacToEntries.erase(name.first);
		}
	}
}

void AVWWifiData::setupStructures() {
	for (auto element : wifiNameToMacToEntries) {
		wifinames.emplace_back(element.first);
		for (auto mac2entries : element.second) {
			for (auto entry : mac2entries.second) {
				if (std::find(frequencies.begin(), frequencies.end(), entry.freq) == frequencies.end()) {
					frequencies.emplace_back(entry.freq);
				}
			}
		}
	}
	std::sort(frequencies.begin(), frequencies.end());
	return;
}

void AVWWifiData::fillRouters(std::string wifiname, std::vector<bool> &routers, bool onoff){
	for (auto elem : wifiNameToMacToEntries[wifiname]) {
		int index = findIndexToEntry(elem.first);
		routers.at(index) = onoff;
	}
	return;
}