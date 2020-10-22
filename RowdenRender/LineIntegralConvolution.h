#pragma once
#include "RowRender.h"
#include <random>
class LineIntegralConvolution
{
public:
	LineIntegralConvolution(glm::uvec2 resolution);
	void setSamplePoint(glm::uvec2 index, float sample_size);
	void fillWithNoise(unsigned int numSamples);
	void fillWithStatifiedNoise(glm::uvec2 strata, unsigned int num_samples);
	std::vector<float> getTexture() { return texture; }
	void convolve(float falloff = .1, float min_arc_ratio = .5);
	void clear() { texture.clear(); texture.resize(resolution.x * resolution.y); noise_samples.clear(); }
private:
	
	std::vector<float> texture;
	std::vector<glm::uvec2> noise_samples;
	glm::uvec2 resolution;
};

