#include "WifiData.h"
#include <iostream>
#include <string>
#include <fstream>
#include "delaunator.hpp"

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

bool WifiData::loadBinary(const char* filename, std::vector<float>& intensities) {
	try {
		std::ifstream file = std::ifstream(filename, std::ios::in | std::ios::binary);
		if (!file)
			return false;
		char dims[3];
		file.read(dims, 3);
		numLatCells = (unsigned char)dims[0];
		numLonCells = (unsigned char)dims[1];
		numSlices = (unsigned char)dims[2];
		int total_size = numLatCells * numLonCells * numSlices;
		intensities.resize(total_size);
		std::vector<char> temp = std::vector<char>();
		temp.resize(total_size);
		file.read(temp.data(), total_size);
		int counter = 0;
		for (auto sample : temp) {
			intensities[counter++] = (float)((unsigned char)sample / 255.0f);
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
