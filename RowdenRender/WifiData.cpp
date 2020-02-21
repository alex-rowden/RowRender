#include "WifiData.h"
#include <iostream>
#include <string>
#include <fstream>
#include "delaunator.hpp"
#include <optixu/optixu_math_namespace.h>

bool WifiData::loadCSV(const char* str) {
	std::ifstream inputFile(str);
	std::string line;
	//Open the file
	if (inputFile.is_open())
	{
		//Get header of the file
		getline(inputFile, line);
		std::string header = line;

		//Keep reading lines until the end of the file
		while (inputFile.good())
		{
			//Read the values
			std::string unixTimeStr;
			getline(inputFile, unixTimeStr, ',');
			if (unixTimeStr == "")
				continue;

			long long unixTime = atoll(unixTimeStr.c_str());

			std::string BSSIDStr;
			getline(inputFile, BSSIDStr, ',');

			std::string signalStrengthStr;
			getline(inputFile, signalStrengthStr, ',');

			std::string SSIDStr;
			getline(inputFile, SSIDStr, ',');

			std::string longitudeStr;
			getline(inputFile, longitudeStr, ',');
			float longitude = atof(longitudeStr.c_str());

			std::string latitiudeStr;
			getline(inputFile, latitiudeStr, ',');
			float latitude = atof(latitiudeStr.c_str());

			std::string GPSAccuracyStr;
			getline(inputFile, GPSAccuracyStr, ',');

			std::string APCapabilitiesStr;
			getline(inputFile, APCapabilitiesStr, ',');

			std::string channelStr;
			getline(inputFile, channelStr, ',');

			std::string frequencyStr;
			getline(inputFile, frequencyStr);

			if (longitude < minLon)
				minLon = longitude;
			if (longitude > maxLon)
				maxLon = longitude;
			if (latitude < minLat)
				minLat = latitude;
			if (latitude > maxLat)
				maxLat = latitude;

			//Convert to a Wifi Entry
			WifiEntry* entry = new WifiEntry();
			entry->unixTime = unixTime;
			entry->BSSID = BSSIDStr;
			entry->signalStrength = atoi(signalStrengthStr.c_str());
			entry->SSID = SSIDStr;
			entry->longitude = longitude;
			entry->latitude = latitude;
			entry->GPSAccuracy = (unsigned char)atoi(GPSAccuracyStr.c_str());
			entry->frequency = atoi(frequencyStr.c_str());

			std::string id = SSIDStr;

			//Make sure the ID entry exists
			if (NetIDToWifiEntries.find(id) == NetIDToWifiEntries.end())
			{
				NetIDToWifiEntries.insert(std::make_pair(id, std::vector<WifiEntry*>()));
				NetIDToMacToEntries.insert(std::make_pair(id, std::map < std::string, std::vector<WifiEntry*>>()));
				NetIDs.emplace(id);
			}

			if (NetIDToMacToEntries.at(id).find(BSSIDStr) == NetIDToMacToEntries.at(id).end())
			{
				NetIDToMacToEntries.at(id).insert(std::make_pair(BSSIDStr, std::vector<WifiEntry*>()));
			}

			//Add the new entry for the ID
			NetIDToWifiEntries[id].emplace_back(entry);
			NetIDToMacToEntries.at(id).at(BSSIDStr).emplace_back(entry);

		}

	}
	else
	{
		std::cout << "ERROR: Could not open " << str << std::endl;
		return false;
	}
	return true;
}

unsigned long WifiData::get_indx(int x, int y, int z) {
	return x + y * this->numLatCells + this->numLonCells * this->numLatCells * z;
}

unsigned char WifiData::sample_tex(std::vector<unsigned char>& intensities, int x, int y, int z) {
	if (x < 0) {
		x = 0;
	}
	else if (x >= numLatCells) {
		x = numLatCells - 1;
	}if (y < 0) {
		y = 0;
	}else if (y >= numLonCells) {
		y = numLonCells - 1;
	}if (z < 0) {
		z = 0;
	}
	else if (z >= numSlices) {
		z = numSlices - 1;
	}
	return intensities.at(get_indx(x, y, z));
}
float WifiData::sample_tex(std::vector<float>& intensities, int x, int y, int z) {
	if (x < 0) {
		x = 0;
	}
	else if (x >= numLatCells) {
		x = numLatCells - 1;
	}if (y < 0) {
		y = 0;
	}else if (y >= numLonCells) {
		y = numLonCells - 1;
	}if (z < 0) {
		z = 0;
	}
	else if (z >= numSlices) {
		z = numSlices - 1;
	}
	return intensities.at(get_indx(x, y, z));
}



void WifiData::calculate_neighbors(Neighborhood&neighbors, std::vector<unsigned char>& intensities, int x, int y, int z, unsigned int sample_step) {
	neighbors.right = (int)sample_tex(intensities, x + sample_step, y, z);
	neighbors.left = (int)sample_tex(intensities, x - sample_step, y, z);
	neighbors.up = (int)sample_tex(intensities, x, y + sample_step, z);
	neighbors.down = (int)sample_tex(intensities, x, y - sample_step, z);
	neighbors.front = (int)sample_tex(intensities, x, y, z + sample_step);
	neighbors.back = (int)sample_tex(intensities, x, y, z - sample_step);
};
void WifiData::calculate_neighbors(Neighborhoodf&neighbors, std::vector<float>& intensities, int x, int y, int z, unsigned int sample_step) {
	neighbors.right = sample_tex(intensities, x + sample_step, y, z);
	neighbors.left = sample_tex(intensities, x - sample_step, y, z);
	neighbors.up = sample_tex(intensities, x, y + sample_step, z);
	neighbors.down = sample_tex(intensities, x, y - sample_step, z);
	neighbors.front = sample_tex(intensities, x, y, z + sample_step);
	neighbors.back = sample_tex(intensities, x, y, z - sample_step);
};

short normFloat2Short(float val) {
	return (((val + 1) * (SHRT_MAX - SHRT_MIN)) / 2.0) + SHRT_MIN;
}

glm::ivec3 getTrip(unsigned long indx, int numLatCells, int numLonCells, int numSlices) {
	glm::ivec3 ret = glm::ivec3();
	ret.z = indx / (numLatCells * numLonCells);
	ret.y = (indx - (ret.z * numLatCells * numLonCells)) / numLatCells;
	ret.x = (indx - (ret.z * numLatCells * numLonCells) - (ret.y * numLatCells));
	return ret;
}

bool WifiData::loadBinary(const char* filename, std::vector<unsigned char>& intensities, std::vector<short>& phi, std::vector<short>&theta, unsigned int sample_step) {
	int dialation = 3;
	int num_smooths = 10;
	std::string outputf = std::to_string(dialation) + "_" + std::to_string(num_smooths) + std::string(filename);
	std::ifstream f(outputf.c_str(), std::ios::in|std::ios::binary);
	if (f.good()) {
		//size_t size;
		f.read((char*)(&numSlices), sizeof(int));
		f.read((char*)(&numLatCells), sizeof(int));
		f.read((char*)(&numLonCells), sizeof(int));
		//numSlices = size / (numLatCells * numLonCells);
		unsigned long size = numSlices * numLatCells * numLonCells;
		intensities.resize(size);
		phi.resize(size);
		theta.resize(size);
		f.read((char*)(&intensities[0]), size * sizeof(unsigned char));
		f.read((char*)(&phi[0]), size * sizeof(short));
		f.read((char*)(&theta[0]), size * sizeof(short));
		return true;
	}
	if (!loadBinary(filename, intensities))
		return false;
	Neighborhood neighbors = { 0, 0, 0, 0, 0, 0 };
	Neighborhoodf neighborsf = { 0, 0, 0, 0, 0, 0 };
	phi.resize(numLatCells * numLonCells * numSlices);
	theta.resize(numLatCells * numLonCells * numSlices);
	
	std::vector<float>temp_x, temp_y, temp_z;

	std::ofstream out(outputf.c_str(), std::ios::out | std::ios::binary);
	
	temp_x.resize(numLatCells * numLonCells * numSlices);
	temp_y.resize(numLatCells * numLonCells * numSlices);
	temp_z.resize(numLatCells * numLonCells * numSlices);
	/*
	for (int x = 0; x < numLonCells; x++) {
		for (int y = 0; y < numLatCells; y++) {
			for (int z = 0; z < numSlices; z++) {
				calculate_neighbors(neighbors, intensities, x, y, z, 1);
				unsigned long curr_idx = get_indx(x, y, z);
				glm::vec3 normal = glm::vec3((neighbors.right) - neighbors.left, (neighbors.up) - neighbors.down, (neighbors.front) - neighbors.back);
				normal /= 255.0f;
				if (glm::length(normal) != 0) {
					normal = glm::normalize(normal);
				}
				//normal = glm::normalize(normal);
				temp_x.at(curr_idx) = normal.x;
				temp_y.at(curr_idx) = normal.y;
				temp_z.at(curr_idx) = normal.z;
			}
		}
	}
	*/
	for (unsigned long i = 0; i < intensities.size(); i++) {
		glm::ivec3 indices = getTrip(i, numLatCells, numLonCells, numSlices);
		calculate_neighbors(neighbors, intensities, indices.x, indices.y, indices.z, 4);
		glm::vec3 normal = glm::vec3(((neighbors.right) - neighbors.left) * numLatCells / (float)1, ((neighbors.up) - neighbors.down) * numLonCells / (float)1, .01);
		
		if (glm::length(normal) != 0) {
			normal = glm::normalize(normal);
		}
		temp_x.at(i) = normal.x;
		temp_y.at(i) = normal.y;
		temp_z.at(i) = normal.z;
	}

	for (int i = 0; i < num_smooths; i++) {
		for (unsigned long curr_idx = 0; curr_idx < intensities.size(); curr_idx++) {
			glm::ivec3 indices = getTrip(curr_idx, numLatCells, numLonCells, numSlices);
			calculate_neighbors(neighborsf, temp_x, indices.x, indices.y, indices.z, dialation);
			temp_x.at(curr_idx) = (neighborsf.right + neighborsf.left + neighborsf.up + neighborsf.down + neighborsf.front + neighborsf.back) / 6.0f;
			calculate_neighbors(neighborsf, temp_y, indices.x, indices.y, indices.z, dialation);
			temp_y.at(curr_idx) = (neighborsf.right + neighborsf.left + neighborsf.up + neighborsf.down + neighborsf.front + neighborsf.back) / 6.0f;
			calculate_neighbors(neighborsf, temp_z, indices.x, indices.y, indices.z, dialation);
			temp_z.at(curr_idx) = (neighborsf.right + neighborsf.left + neighborsf.up + neighborsf.down + neighborsf.front + neighborsf.back) / 6.0f;
			glm::vec3 normal = glm::vec3(temp_x.at(curr_idx), temp_y.at(curr_idx), temp_z.at(curr_idx));
			if (true) {
				normal = glm::normalize(normal);
				temp_x.at(curr_idx) = normal.x;
				temp_y.at(curr_idx) = normal.y;
				temp_z.at(curr_idx) = normal.z;
			}
		}
		/*
		for (int x = 0; x < numLonCells; x++) {
			for (int y = 0; y < numLatCells; y++) {
				for (int z = 0; z < numSlices; z++) {
					calculate_neighbors(neighborsf, temp_x, x, y, z, dialation);
					unsigned long curr_idx = get_indx(x, y, z);
					temp_x.at(curr_idx) = (neighborsf.right + neighborsf.left + neighborsf.up + neighborsf.down + neighborsf.front + neighborsf.back) / 6.0f;
					calculate_neighbors(neighborsf, temp_y, x, y, z, dialation);
					temp_y.at(curr_idx) = (neighborsf.right + neighborsf.left + neighborsf.up + neighborsf.down + neighborsf.front + neighborsf.back) / 6.0f;
					calculate_neighbors(neighborsf, temp_z, x, y, z, dialation);
					temp_z.at(curr_idx) = (neighborsf.right + neighborsf.left + neighborsf.up + neighborsf.down + neighborsf.front + neighborsf.back) / 6.0f;
					glm::vec3 normal = glm::vec3(temp_x.at(curr_idx), temp_y.at(curr_idx), temp_z.at(curr_idx));
					if(num_smooths == i - 1)
						normal = glm::normalize(normal);
					temp_x.at(curr_idx) = normal.x;
					temp_y.at(curr_idx) = normal.y;
					temp_z.at(curr_idx) = normal.z;
				}
			}
		}
		*/
	}
	/*
	for (int x = 0; x < numLonCells; x++) {
		for (int y = 0; y < numLatCells; y++) {
			for (int z = 0; z < numSlices; z++) {
				unsigned long curr_idx = get_indx(x, y, z);
				phi.at(curr_idx) = normFloat2Short(temp_x.at(curr_idx));
				theta.at(curr_idx) = normFloat2Short(temp_y.at(curr_idx));
			}
		}
	}
	*/
	float max_phi = std::numeric_limits<float>().min();
	float min_phi = std::numeric_limits<float>().max();
	for (unsigned long curr_idx = 0; curr_idx < intensities.size(); curr_idx++) {
		//phi.at(curr_idx) = (temp_x.at(curr_idx));
		
		float t_phi = (acos(temp_z.at(curr_idx)) / M_PIf);
		//theta.at(curr_idx) = (temp_y.at(curr_idx));
		//float t_phi = (asin(temp_y.at(curr_idx) / sin(acos(temp_z.at(curr_idx)))) + M_PI_2f) / M_PIf;
		
		
		if (abs(temp_y.at(curr_idx)) <= 1e-2 && temp_x.at(curr_idx) < 0) {
			//std::cout << t_phi << std::endl;
		}
		

		phi.at(curr_idx) = normFloat2Short(t_phi);
		
		if (t_phi == 1.0f) {
			//t_phi -= 1e-3;
		}
		t_phi = (atan2(temp_y.at(curr_idx), temp_x.at(curr_idx)) + M_PIf) / (2 * M_PIf);
		theta.at(curr_idx) = normFloat2Short(t_phi);
		if (t_phi > max_phi) {
			max_phi = t_phi;
		}
		else if (t_phi < min_phi) {
			min_phi = t_phi;
		}
	
	}
	std::cout << max_phi << std::endl;
	std::cout << min_phi << std::endl;
	size_t size = intensities.size();
	out.write(reinterpret_cast<const char*>(&numSlices), sizeof(int));
	out.write(reinterpret_cast<const char*>(&numLatCells), sizeof(int));
	out.write(reinterpret_cast<const char*>(&numLonCells), sizeof(int));
	out.write(reinterpret_cast<const char *>(&intensities[0]), intensities.size() * sizeof(unsigned char));
	out.write(reinterpret_cast<const char*>(&phi[0]), phi.size() * sizeof(short));
	out.write(reinterpret_cast<const char*>(&theta[0]), theta.size() * sizeof(short));

	//for(unsigned long i = 0; i < phi.size(); i++){
		//out.write((char *)&phi.at(i), sizeof(short));
		//out.write((char *)&theta.at(i), sizeof(short));
		//out.write((char *)&normals_z.at(i), sizeof(short));
	//}
	//out.flush();
	out.close();
	return true;
}

bool WifiData::loadBinary(const char* filename, std::vector<unsigned char>& intensities) {
	try {
		std::ifstream file = std::ifstream(filename, std::ios::in | std::ios::binary);
		if (!file)
			return false;
		unsigned int dims[3];
		file.read(reinterpret_cast<char*>(dims), 3 * 4 * sizeof(char)); //I know that uint is 4 bytes in matlab and I'm not sure about it here so this is how I am doing it
		this->numLatCells = dims[1];//1
		this->numLonCells = dims[0];//0
		this->numSlices = dims[2];
		unsigned long total_size = numLatCells * numLonCells * numSlices;
		intensities.resize(total_size);
		std::vector<char> temp = std::vector<char>();
		temp.resize(total_size);
		file.read(temp.data(), total_size);
		int counter = 0;
		for (auto sample : temp) {
			intensities[counter++] = (unsigned char)sample;
		}
		return true;
	}
	catch (std::exception e) {
		std::cerr << e.what() << std::endl;
		return false;
	}
}

void WifiData::Finalize(float latLonDist)
{
	longitudeRange = maxLon - minLon;
	latitudeRange = maxLat - minLat;


	numLonCells = ceil(longitudeRange / latLonDist);
	numLatCells = ceil(latitudeRange / latLonDist);

}

float **WifiData::GetIDIntensities(std::string netID, std::string mac)
{
	if (mac == "")
		return netIDToIntensities.at(netID);
	else
		return netIDToMacToIntensities.at(netID).at(mac);
}

void WifiData::ComputeIDIntensities(std::string netID)
{
	if (netIDToIntensities.find(netID) != netIDToIntensities.end())
		return;
	//Create counter arrays
	int** numEntries = new int* [numLonCells]();
	float** intensities = new float* [numLonCells]();
	float** intensitiesIntrp = new float* [numLonCells]();
	for (int i = 0; i < numLonCells; i++)
	{
		numEntries[i] = new int[numLatCells]();
		intensities[i] = new float[numLatCells]();
		intensitiesIntrp[i] = new float[numLatCells]();
		for (int j = 0; j < numLatCells; j++)
		{
			numEntries[i][j] = 0;
			intensities[i][j] = 0.0f;
			intensitiesIntrp[i][j] = 0.0f;
		}
	}

	//Go through each WifiEntry
	for (auto wifiInfo : NetIDToWifiEntries.at(netID))
	{
		WifiEntry* entry = wifiInfo;

		float lat = abs(entry->latitude);
		float lon = abs(entry->longitude);

		float intensity = entry->signalStrength;

		//This hella needs to be fixed in the future
		float latPer = ((lat - abs(minLat)) / (float)latitudeRange);
		float lonPer = ((lon - abs(maxLon)) / (float)longitudeRange);

		int latIndex = (int)(latPer * (float)(numLatCells - 1));
		int lonIndex = (int)(lonPer * (float)(numLonCells - 1));

		numEntries[lonIndex][latIndex] ++;
		intensities[lonIndex][latIndex] += abs(intensity);
	}

	//Normalize intensities
	for (int i = 0; i < numLonCells; i++)
	{
		for (int j = 0; j < numLatCells; j++)
		{
			float num = (float)numEntries[i][j];
			if (num == 0.0f)
				intensities[i][j] = 0.0f;
			else
				intensities[i][j] /= num;
		}
	}

	//Interpolate the intensities
	/*double kernel[5][5];
	FilterCreation(kernel);

	for (int i = 0; i < numLonCells; i++)
	{
		for (int j = 0; j < numLatCells; j++)
		{
			for (int ki = -2; ki <= 2; ki++)
			{
				for (int kj = -2; kj <= 2; kj++)
				{
					if (i + ki < 0 ||
						j + kj < 0 ||
						i + ki >= numLonCells ||
						j + kj >= numLatCells)
						continue;
					intensitiesIntrp[i][j] += intensities[i + ki][j + kj] * kernel[ki + 2][kj + 2];
				}
			}
		}
	}*/

	//Generate the coordinates /x,y x y x y
	std::vector<double> coords;// = { -1, 1, 1, 1, 1, -1, -1, -1 };
	std::vector<glm::ivec2> pointIndexes;
	for (int i = 0; i < numLonCells; i++)
	{
		for (int j = 0; j < numLatCells; j++)
		{
			//fill in for only sampled points
			if (intensities[i][j] > 0)
			{
				coords.emplace_back(i);
				coords.emplace_back(j);

				pointIndexes.emplace_back(glm::ivec2(i, j));
			}
		}
	}

	//triangulation happens here
#if(true)
	try{
		delaunator::Delaunator d(coords);

		//Get triangles
		std::vector<glm::ivec3> triangleIndexes;
		for (std::size_t i = 0; i < d.triangles.size(); i += 3)
		{
			glm::ivec3 triangle;
			triangle.x = d.triangles[i];
			triangle.y = d.triangles[i + 1];
			triangle.z = d.triangles[i + 2];

			triangleIndexes.emplace_back(triangle);
		}

		//Go through each triangle and interpolate the wifi strengths
		for (auto triangle : triangleIndexes)
		{
			glm::ivec2 indexOne = pointIndexes.at(triangle.x);
			glm::ivec2 indexTwo = pointIndexes.at(triangle.y);
			glm::ivec2 indexThree = pointIndexes.at(triangle.z);

			float strengthOne = intensities[indexOne.x][indexOne.y];
			float strengthTwo = intensities[indexTwo.x][indexTwo.y];
			float strengthThree = intensities[indexThree.x][indexThree.y];



			//Go through each point in the triangle and use B to get the iterpolated strength
			int minX = std::fmin(indexOne.x, std::fmin(indexTwo.x, indexThree.x));
			int maxX = std::fmax(indexOne.x, std::fmax(indexTwo.x, indexThree.x));
			int minY = std::fmin(indexOne.y, std::fmin(indexTwo.y, indexThree.y));
			int maxY = std::fmax(indexOne.y, std::fmax(indexTwo.y, indexThree.y));

			for (int x = minX; x <= maxX; x++)
			{
				for (int y = minY; y <= maxY; y++)
				{
					//Make sure point is inside the triangle
					bool inside = isInside(
						indexOne.x, indexOne.y, indexTwo.x, indexTwo.y,
						indexThree.x, indexThree.y, x, y);
					glm::ivec2 p = glm::ivec2(x, y);
					if (inside)
					{
						//https://codeplea.com/triangular-interpolation
						float wv1 = ((float)(indexTwo.y - indexThree.y) * (float)(p.x - indexThree.x) + (float)(indexThree.x - indexTwo.x) * (float)(p.y - indexThree.y)) /
							((float)(indexTwo.y - indexThree.y) * (float)(indexOne.x - indexThree.x) + (float)(indexThree.x - indexTwo.x) * (float)(indexOne.y - indexThree.y));
						float wv2 = ((float)(indexThree.y - indexOne.y) * (float)(p.x - indexThree.x) + (float)(indexOne.x - indexThree.x) * (float)(p.y - indexThree.y)) /
							((float)(indexTwo.y - indexThree.y) * (float)(indexOne.x - indexThree.x) + (float)(indexThree.x - indexTwo.x) * (float)(indexOne.y - indexThree.y));
						float wv3 = 1.0f - wv1 - wv2;

						float strength = strengthOne * wv1 + strengthTwo * wv2 + strengthThree * wv3;
						intensities[x][y] += strength;
						intensitiesIntrp[x][y] += 1.0f;

					}

				}
			}



			intensities[indexOne.x][indexOne.y] = strengthOne;
			intensitiesIntrp[indexOne.x][indexOne.y] = 1.0f;
			intensities[indexTwo.x][indexTwo.y] = strengthTwo;
			intensitiesIntrp[indexTwo.x][indexTwo.y] = 1.0f;
			intensities[indexThree.x][indexThree.y] = strengthThree;
			intensitiesIntrp[indexThree.x][indexThree.y] = 1.0f;
		}

		for (int i = 0; i < numLonCells; i++)
		{
			for (int j = 0; j < numLatCells; j++)
			{
				if (intensitiesIntrp[i][j] > 0)
					intensities[i][j] /= intensitiesIntrp[i][j];
			}
		}

	}
	catch (std::exception e)
	{

		std::cout << "Delauney Error " << std::endl;
	}

#endif
	netIDToIntensities.insert(std::make_pair(netID, intensities));

}

/* A utility function to calculate area of triangle formed by (x1, y1),
(x2, y2) and (x3, y3) */
float WifiData::area(int x1, int y1, int x2, int y2, int x3, int y3)
{
	return std::abs((x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2)) / 2.0);
}

/* A function to check whether point P(x, y) lies inside the triangle formed
by A(x1, y1), B(x2, y2) and C(x3, y3) */

bool WifiData::isInside(int x1, int y1, int x2, int y2, int x3, int y3, int x, int y)
{
	/* Calculate area of triangle ABC */
	float A = area(x1, y1, x2, y2, x3, y3);

	/* Calculate area of triangle PBC */
	float A1 = area(x, y, x2, y2, x3, y3);

	/* Calculate area of triangle PAC */
	float A2 = area(x1, y1, x, y, x3, y3);

	/* Calculate area of triangle PAB */
	float A3 = area(x1, y1, x2, y2, x, y);

	/* Check if sum of A1, A2 and A3 is same as A */
	return (A == A1 + A2 + A3);
}
