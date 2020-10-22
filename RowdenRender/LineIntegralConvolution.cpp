#include "LineIntegralConvolution.h"
float randfloat() {
	return std::rand() / ((float)RAND_MAX + 1);
}

void LineIntegralConvolution::setSamplePoint(glm::uvec2 index, float sample_size) {
	int extent_of_change = ceil(sample_size);
	for (int i = -extent_of_change; i <= extent_of_change; i++) {
		int y_coord = ((int)index.y + i);
		if (y_coord >= resolution.y || y_coord < 0) { continue; }
		
		float val = 1;
		if (abs(i) == extent_of_change) {
			val = extent_of_change - sample_size;
		}
		texture.at(index.x + (y_coord * resolution.x)) += val;
		//if (texture.at(index.x + (y_coord * resolution.x)) > 1) {
		//	texture.at(index.x + (y_coord * resolution.x)) = 1;
		//}
	}
}


void LineIntegralConvolution::fillWithStatifiedNoise(glm::uvec2 strata, unsigned int num_samples) {
	glm::vec2 strata_dims = glm::vec2((float)resolution.x/strata.x, (float)resolution.y/strata.y);
	//std::cout << glm::to_string(strata_dims) << std::endl;
	unsigned int num_samples_per_strata = floor(num_samples / (strata.x * strata.y));
	unsigned int num_stratified_samples = num_samples_per_strata * strata.x * strata.y;
	noise_samples.resize(num_samples);
	for (int i = 0; i < num_samples; i++) {
		int flat_strata_ind = i % (strata.x * strata.y);
		if (i > num_stratified_samples) {
			flat_strata_ind = floor(randfloat() * (strata.x * strata.y));
		}
		glm::uvec2 strata_ind = glm::uvec2(flat_strata_ind % strata.x, flat_strata_ind / strata.x);
		glm::vec2 strata_sample(randfloat() * strata_dims.x, randfloat() * strata_dims.y);
		glm::uvec2 index = glm::uvec2(strata_sample.x + strata_dims.x * strata_ind.x, strata_sample.y + strata_dims.y * strata_ind.y);
		noise_samples.at(i) = index;
		//setSamplePoint(index, sample_size);
	}
}void LineIntegralConvolution::fillWithNoise(unsigned int num_samples) {
	//noise_samples.resize(num_samples);
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(0, 1);
	for (int i = 0; i < num_samples; i++) {
		float x = dis(gen);
		float y = dis(gen);
		glm::uvec2 index = glm::uvec2(floor(resolution.x * x), floor(resolution.y * y));
		if (index.x == resolution.x)
			index.x = resolution.x - 1;
		if (index.y == resolution.y)
			index.y = resolution.y - 1;
		//noise_samples.at(i) = index;
		//setSamplePoint(index, sample_size);
 		texture.at(index.x + index.y * resolution.x) = 1;
	}
}

void LineIntegralConvolution::convolve(float falloff, float min_arc_ratio) {
	float theta_incr = 1 / (float)resolution.x * 2 * PI;
	float max_arc_length = theta_incr;
	float min_arc_length = theta_incr * min_arc_ratio;
	for (int i = 0; i < noise_samples.size(); i++) {
		float val = 1;
		float sample = 0;
		glm::uvec2 index = noise_samples.at(i);
		if (index.y >= resolution.y)
			index.y = resolution.y - 1;
		if (index.x >= resolution.x)
			index.x = resolution.x - 1;
		int counter = 0;
		while (val > 0) {
			
			uint32_t flat_index = index.x + (index.y * resolution.x);
			//std::cout << flat_index << std::endl;
			sample = texture.at(flat_index);
			float r = (index.y) / (float)resolution.y;
			float arc_length = theta_incr * r;
			//if (arc_length < min_arc_length)
			//	arc_length = min_arc_length;
			float ratio = 1/r;
			int extent = 2;//ceil(ratio);
			
			for (int j = -extent; j <= extent; j++) {
				if ((int)index.x + j >= 0 && (int)index.x + j < resolution.x) {
					texture.at(flat_index + j) += val;
					if (texture.at(flat_index + j) > 1)
						texture.at(flat_index + j) = 1;
				}
				
					
			}
			
			val += sample - falloff;
			if (val < 0) { val = 0; }
			if (val > 1) { val = 1; }
			if (index.y == 0)
				break;
			index.y -= 1;
			counter++;
		}
	}  
}

LineIntegralConvolution::LineIntegralConvolution(glm::uvec2 res) {
	resolution = res;
	texture = std::vector<float>(resolution.x * resolution.y);
}