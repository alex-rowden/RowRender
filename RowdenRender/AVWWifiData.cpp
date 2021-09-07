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

AVWWifiData::AVWWifiData(ShaderProgram* p, int num_shaders) {
	block_index = new GLuint[num_shaders];
	bindingPoint = new GLuint[num_shaders];
	glGenBuffers(1, &uniformBuffer);
	for (int i = 0; i < num_shaders; i++) {
		bindingPoint[i] = 0;
		block_index[i] = glGetUniformBlockIndex(p[i].getShader(), "EllipsoidBlock");
		glUniformBlockBinding(p[i].getShader(), block_index[i], bindingPoint[i]);
		std::cout << bindingPoint[i] << std::endl;
	}
	
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
		ret.r.z = .8;
	}
	ret.mu.z += 1;
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

void AVWWifiData::setNearestNRouters(int n, glm::vec3 position, std::vector<bool>& wifi_bools, std::vector<bool>& routers, std::vector<bool>&freqs, bool old_data) {
	std::vector<float> min_distances(n);
	std::fill(min_distances.begin(), min_distances.end(), std::numeric_limits<float>().max());
	std::vector<int> min_indices(n);
	bool empty = true;
	for (int i = 0; i < wifinames.size(); i++) {
		if (wifi_bools.at(i)) {
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
		std::string wifiname = name2Mac.first;
		if (!wifiname.compare(""))
			wifiname = "empty";
		if (wifi_bools[wifinum++] || empty) {
			for (auto mac2Entries : name2Mac.second) {
				auto EntriesList = mac2Entries.second.first;
				if (!old_data)
					EntriesList = mac2Entries.second.second;
				if (EntriesList.size() < 4) {
					//router_num++;
					continue;
				}
				currEllipsoid = listMac2Routers[old_data ? 0: 1][mac2Entries.first];
				

				auto loc = std::find(active_freqs.begin(), active_freqs.end(),
					EntriesList.at(0).freq);
				if (loc == active_freqs.end())
					continue;



				currDist = ellipsoidDistance(position, currEllipsoid);
				curr_router = findIndexToEntry(mac2Entries.first);
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
	}
	std::fill(routers.begin(), routers.end(), false);
	std::fill(freqs.begin(), freqs.end(), false);
	for (auto ind : min_indices) {
		routers[ind] = true;
	}for (auto ind : freq_indices) {
		freqs[ind] = true;
	}

}

void AVWWifiData::setRouters(std::vector<bool>& wifinames, std::vector<bool>& routers, std::vector<bool>& freqs) {
	
	int wifinum = 0, router_num = 0, swap_ind = 0, curr_router, swap_freq_ind;
	Ellipsoid currEllipsoid;
	std::vector<int> active_freqs = getActiveFreqs(freqs);
	for (auto name2Mac : wifiNameToMacToEntries) {
		if (wifinames[wifinum++]) {
			for (auto mac2Entries : name2Mac.second) {
				auto loc = std::find(active_freqs.begin(), active_freqs.end(),
					mac2Entries.second.at(0).freq);
				if (loc == active_freqs.end())
					continue;
				std::string wifiname = name2Mac.first;
				if (!wifiname.compare(""))
					wifiname = "empty";
				//currEllipsoid = mac2routers[mac2Entries.first];
				curr_router = router_num++;
				auto curr_freq = std::distance(active_freqs.begin(), loc);
				
			}
		}
		std::fill(routers.begin(), routers.end(), false);
		std::fill(freqs.begin(), freqs.end(), false);
		
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
			int numSamples = 1;
						
			if (!legacy) {
				numSamples = atoi(countStr.c_str());
			}
			
			glm::vec2 location = glm::vec2(xPos, yPos);
			for(int sample = 0; sample < numSamples; sample++){
				if (!legacy) {
					getline(inputFile, countStr);
				}
				int count = atoi(countStr.c_str());
				for (int i = 0; i < count; i++)
				{
					std::string entireLine;
					getline(inputFile, entireLine);

					std::vector<std::string> parts;
					tokenizer(entireLine, " ", parts);
					int numParts = parts.size();
					if (numParts == 2) {
						continue;
					}
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

					std::string macAddress = parts.at(numParts - 7);
					std::string rssiStr = parts.at(numParts - 6);
					std::string freqStr = parts.at(numParts - 5);
					std::string linkQualityStr = parts.at(numParts - 4);
					std::string securityEnabledStr = parts.at(numParts - 3);
					std::string authAlgorithmStr = parts.at(numParts - 2);
					std::string cipherAlgorithmStr = parts.at(numParts - 1);

					WifiDataEntry entry;

					entry.location = glm::vec3(location, atof(floor.c_str()));
					float offset = .03, dist_between_samples = .125; 
					if (!legacy)
						entry.location.z += offset + sample * dist_between_samples;
					entry.MAC = macAddress;
					entry.RSSI = atoi(rssiStr.c_str());
					entry.freq = atoi(freqStr.c_str());
					entry.linkQuality = atoi(linkQualityStr.c_str());
					entry.security = (bool)atoi(securityEnabledStr.c_str());
					entry.authAlg = atoi(authAlgorithmStr.c_str());
					entry.cipherAlg = atoi(cipherAlgorithmStr.c_str());

					entry.floor = glm::floor(atof(floor.c_str()));
					if (entry.floor == 2 && !legacy)
						entry.location.x -= .015;
					//else if(entry.floor == 3 && !legacy)


					if (entry.RSSI > 0 || entry.RSSI < -120) {
						continue;
					}

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

					if (wifiNameToMacToEntries.find(networkName) == wifiNameToMacToEntries.end()) {
						wifiNameToMacToEntries.insert(std::make_pair(networkName, std::map<std::string, std::pair<std::vector<WifiDataEntry>,
							std::vector<WifiDataEntry>>>()));
					}if (wifiNameToMacToEntries.at(networkName).find(macAddress) == wifiNameToMacToEntries.at(networkName).end()) {
						wifiNameToMacToEntries.at(networkName).insert(std::make_pair(macAddress,
							std::pair<std::vector<WifiDataEntry>, std::vector<WifiDataEntry>>()));
						numRouters++;
					}
					if (legacy)
						wifiNameToMacToEntries.at(networkName).at(macAddress).first.emplace_back(entry);
					else
						wifiNameToMacToEntries.at(networkName).at(macAddress).second.emplace_back(entry);
					floorToWifiNameToEntries.at(floor).at(networkName).emplace_back(entry);
				}
			}
			
		}
	}
	else
	{
		std::cout << "Could not open " << filename << std::endl;
	}


	inputFile.close();
}

void AVWWifiData::writeRouters(std::ofstream &out) {
	out << router_strings.size() << std::endl;
	for (auto str : router_strings) {
		out << str << std::endl;
	}
}

void AVWWifiData::readRouters(std::ifstream& in, std::vector<bool>& wifinames, std::vector<bool>& router_bools, std::vector<bool>& freqs) {
	routers_read = true;
	size_t size;
	in >> size;
	router_strings.resize(size);
	std::vector<std::string> names(size);
	std::vector<std::string> MAC(size);
	std::string name, addr, dump;
	for (int i = 0; i < size; i++) {
		in >> name;
		in >> addr;
		in >> dump;
		name = name.substr(0, name.length() - 1);
		addr = addr.substr(0, addr.length() - 1);
		names.at(i) = name;
		MAC.at(i) = addr;
		router_strings[i] = name + ": " + addr + ": " + dump;
	}
	
	std::fill(wifinames.begin(), wifinames.end(), false);
	std::fill(freqs.begin(), freqs.end(), false);
	std::fill(router_bools.begin(), router_bools.end(), false);

	for (int i = 0; i < MAC.size(); i++) {
		auto it = wifiNameToMacToEntries.find(names.at(i));
		int name_index = std::distance(wifiNameToMacToEntries.begin(), it);
		//std::cout << name_index << std::endl;
		wifinames[name_index] = true;
	}

	//freqs.resize(available_freqs.size());
	for (int i = 0; i < MAC.size(); i++) {
		auto it = wifiNameToMacToEntries.find(names.at(i));
		int curr_freq = it->second.at(MAC.at(i)).at(0).freq;
		auto freq_it = std::find(available_freqs.begin(), available_freqs.end(), curr_freq);
		int freq_ind = std::distance(available_freqs.begin(), freq_it);
		freqs[freq_ind] = true;
	}
	//router_bools.resize(available_macs.size());
	for (int i = 0; i < MAC.size(); i++) {
		auto it = std::find(available_macs.begin(), available_macs.end(), MAC.at(i));
		int router_ind = std::distance(available_macs.begin(), it);
		router_bools.at(router_ind) = true;
	}
	
}

void AVWWifiData::updateRouterStructure(std::vector<bool>router_bools, std::vector<bool> wifinames, std::vector<bool> freqs, bool old_data, ShaderProgram* model_shader, int num_shaders, glm::vec3 position, bool nearest_router) {
	int wifinum = 0;
	int i = 0;
	routers.clear();
	routers.resize(getNumActiveRouters(router_bools));
	router_strings.empty();
	router_strings.resize(getNumActiveRouters(router_bools));
	for (auto NameToMacToEntries : wifiNameToMacToEntries) {
		if (!wifinames.at(wifinum++))
			continue;
		for (auto MacToEntries : NameToMacToEntries.second) {
			int index = findIndexToEntry(MacToEntries.first);
			if (index >= router_bools.size() || !router_bools.at(index)) {
				continue;
			}
			auto MacList = MacToEntries.second.first;
			if (!old_data)
				MacList = MacToEntries.second.second;
			if (MacList.size() > 3) {
				if (router_counter > MAX_ROUTERS) {
					std::cerr << "Selected too many routers" << std::endl;
					return;
				}
				std::string wifiname = NameToMacToEntries.first;
				if (!wifiname.compare(""))
					wifiname = "empty";
				
				float router_index = 0;
				if (nearest_router)
					router_index = (i + getNumWifiNames() + 1) / (float)(getNumActiveRouters(router_bools) + getNumWifiNames() + 1);
				else
					router_index = wifinum / (float)(getNumWifiNames() + 1);
				
				routers[router_counter] = (old_data ? listMac2Routers[0][MacToEntries.first] :
					listMac2Routers[1][MacToEntries.first]);
				routers[router_counter].mu.w = router_index;
				std::vector<int> active_freqs = getActiveFreqs(freqs);
				int search_freq = MacList.at(0).freq;
				
				auto it = std::find(active_freqs.begin(), active_freqs.end(), search_freq);
				int index = std::distance(active_freqs.begin(), it);

				routers[router_counter].r.w = (float)index;
				router_strings[router_counter] = wifiname + ": " + MacToEntries.first + ": " + std::to_string(search_freq);


				router_counter++;
				i++;
				
			}
			else {
 				std::cout << "not enough samples for interpolation" << std::endl;
			}
		}
	}
	sortRouters(position);
	//model_shader.setEllipsoids(routers);
	uploadRouters(num_shaders, model_shader);
	
}



void AVWWifiData::sortRouters(glm::vec3 position) {
	std::vector<std::vector<Ellipsoid>>organization;
	for (auto router : routers) {
		bool found = false;
		for (int i = 0; i < organization.size(); i++) {
			if (organization[i][0].r.w == router.r.w) {
				found = true;
				organization[i].emplace_back(router);
			}
		}
		if (!found) {
			std::vector<Ellipsoid> temp;
			temp.emplace_back(router);
			organization.emplace_back(temp);
		}
	}
	std::sort(organization.begin(), organization.end(),
		[](std::vector<Ellipsoid> a, std::vector<Ellipsoid> b) {
			return a.size() > b.size();
		}
	);

	int ittr = 0;
	for (auto vec: organization) {
		for (auto elem : vec) {
			routers[ittr++] = elem;
		}
	}
}

void AVWWifiData::uploadRouters( int num_shaders, ShaderProgram *model_shader) {
	glBindBuffer(GL_UNIFORM_BUFFER, uniformBuffer);
	for (int j = 0; j < num_shaders; j++) {
		glBufferData(GL_UNIFORM_BUFFER, routers.size() * sizeof(Ellipsoid), routers.data(), GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, bindingPoint[j], uniformBuffer);
		model_shader[0].SetUniform("num_routers", (int)routers.size());
	}
}


int AVWWifiData::getNumRoutersWithSignalFromSet(glm::vec3 position, float extent) {
	int counter = 0;
	for (auto &router : routers) {
		if (ellipsoidDistance(position, router) <= extent) {
			counter++;
		}
	}
	return counter;
}
int AVWWifiData::getNumRoutersWithSignal(glm::vec3 position, float extent, bool old_data) {
	int counter = 0;
	for (auto& mac2Router : listMac2Routers[old_data ? 0 : 1]) {
		if (ellipsoidDistance(position, mac2Router.second) <= extent) {
			counter++;
		}
		
	}
	return counter;
}
//TODO: should this be static or dynamic? Implementing static
std::string AVWWifiData::getInterferenceString() {
	std::map<float, int> counter_map;
	for (auto &router : routers) {
		counter_map.insert(std::pair<float, int>((float)router.r.w, 0));
		counter_map[router.r.w]++;
	}
	std::string ret("");
	for (auto key_val : counter_map)
		ret += (std::to_string(key_val.second) + "|");
	ret.erase(ret.end() - 1);
	return ret;
}


void AVWWifiData::setAvailableMacs(std::vector<std::string> names, bool old_data) {
	setAvailableMacs(names, frequencies, old_data);
}
void AVWWifiData::setAvailableMacs(std::vector<std::string> names,
	std::vector<int> freqs, bool old_data) {
	available_macs = std::vector<std::string>();
	
	for (auto name : names) {
		for (auto mac2Entry : wifiNameToMacToEntries.at(name)) {
			auto macList = mac2Entry.second.first;
			if (!old_data)
				macList = mac2Entry.second.second;
			if (!macList.empty() && std::find(freqs.begin(), freqs.end(), macList.at(0).freq) != freqs.end())
				available_macs.emplace_back(mac2Entry.first);
		}
	}
	return;
}

void AVWWifiData::setAvailableFreqs(std::vector<std::string> names, bool old_data) {
	available_freqs = std::vector<int>();
	for (auto name : names) {
		for (auto mac2entry : wifiNameToMacToEntries.at(name)) {
			auto entryList = mac2entry.second.first;
			if (!old_data)
				entryList = mac2entry.second.second;
			for (auto entry : entryList) {
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

inline int AVWWifiData::findIndexToEntry(std::string mac) {
	return std::distance(available_macs.begin(), std::find(available_macs.begin(), available_macs.end(), mac));

}

std::vector<glm::mat4> AVWWifiData::getTransforms(std::vector<bool> wifinames, std::vector<bool> routers, glm::vec3 scale, bool old_data) {
	std::vector<glm::mat4> out;
	color_indices.clear();
	int wifinum = 0;
	for (auto NameToMacToEntries : wifiNameToMacToEntries) {
		if (!wifinames.at(wifinum++))
			continue;
		for (auto MacToEntries : NameToMacToEntries.second) {
			if ((MacToEntries.second.first.size() < 1 && old_data) || (MacToEntries.second.second.size() < 1 && !old_data))
				continue;
			int index = findIndexToEntry(MacToEntries.first);
			if (!routers.at(index)) {
				continue;
			}
			auto EntryList = MacToEntries.second.first;
			if (!old_data)
				EntryList = MacToEntries.second.second;
			for (auto Entries : EntryList) {
				lastEntry = Entries;
				float size = ((Entries.RSSI + 100) / 30.0f) * .1;
				glm::mat4 translation = glm::translate(glm::mat4(1), glm::vec3(Entries.location.y * scale.x, Entries.location.x * scale.y, Entries.location.z * scale.z));
				out.push_back(glm::scale(translation, glm::vec3(size)));
				color_indices.push_back((float)wifinum / (wifiNameToMacToEntries.size() + 1));
			}
		}
	}
	return out;
}

void AVWWifiData::deactivateExtra(std::vector<bool>routers, std::vector<bool>&wifi_bools,
									std::vector<bool>&freqs) {
	std::fill(wifi_bools.begin(), wifi_bools.end(), false);
	std::fill(freqs.begin(), freqs.end(), false);
	int wifinum = 0;
	for (auto wifiToMac : wifiNameToMacToEntries) {
		for (auto MacToEntries : wifiToMac.second) {
			int index = findIndexToEntry(MacToEntries.first);
			if (routers.at(index)) {
				wifi_bools.at(wifinum) = true;
				index = std::distance(frequencies.begin(),
					std::find(frequencies.begin(), frequencies.end(),
						MacToEntries.second.at(0).freq));
				freqs.at(index) = true;
			}
				
		}
		wifinum++;
	}
}

void AVWWifiData::pruneEntries() {
	for (auto name : wifiNameToMacToEntries) {
		for (auto mac : name.second) {
			if (mac.second.first.size() < 4 && mac.second.second.size() < 4) { //TODO: Change this when we get more functionality to only remove when both have too few
				wifiNameToMacToEntries[name.first].erase(mac.first);
			}
		}
		if (wifiNameToMacToEntries[name.first].size() == 0) {
			wifiNameToMacToEntries.erase(name.first);
		}
	}
}

void AVWWifiData::setupStructures() {
	listMac2Routers.resize(2);
	
	for (auto element : wifiNameToMacToEntries) {
		wifinames.emplace_back(element.first);
		for (auto mac2entries : element.second) {
			std::string wifiname = element.first;
			Ellipsoid currEllipsoid;
			
			if (!wifiname.compare(""))
				wifiname = "empty";
			for (int i = 0; i < 2; i++) {
				std::string path_prefix = "AVW_data" ;
				auto EntryList = mac2entries.second.first;
				if (i == 1) {
					EntryList = mac2entries.second.second;
					path_prefix = "new_AVW_data";
				}
				if (!loadEllipsoid("./Content/Data/" + path_prefix  + "/"
					+ wifiname + "/"
					+ mac2entries.first + ".ellipsoid",
					currEllipsoid)) {
					
				}
				else {
					listMac2Routers[i][mac2entries.first] = currEllipsoid;
				}
				for (auto entry : EntryList) {
					if (std::find(frequencies.begin(), frequencies.end(), entry.freq) == frequencies.end()) {
						frequencies.emplace_back(entry.freq);
					}
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