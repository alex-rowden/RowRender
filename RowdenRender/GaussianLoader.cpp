#include "GaussianLoader.h"

bool GaussianLoader::loadGaussians(std::string filename, std::vector<Gaussian>& gaussians) {
	std::ifstream file = std::ifstream(filename, std::ios::in | std::ios::binary);
	if (!file) {
		std::cerr << "File: " << filename << " not found" << std::endl;
		return false;
	}
	unsigned int numGaus = 0;
	try {
		file.read(reinterpret_cast<char*>(&numGaus), sizeof(int));
	}
	catch (std::ifstream::failure f) {
		std::cerr << f.what() << std::endl;
	}
	unsigned int fileSize = numGaus * sizeof(float) * 4;
	Gaussian* gaus_arr;
	gaus_arr = (Gaussian*)malloc(numGaus * 4 * sizeof(float));
	if (!gaus_arr) {
		std::cerr << "Malloc Failed" << std::endl;
	}
	file.read(reinterpret_cast<char*>(gaus_arr), fileSize);
	gaussians.assign(gaus_arr, gaus_arr + numGaus);
	return true;
}

