#pragma once
#include "RowRender.h"
#include "WifiData.h"
#include "Window.h"
#include "Shape.h"
#include "Mesh.h"
#include "Lights.h"
#include "ShaderProgram.h"
#include "Texture2D.h"
#include "Model.h"
#include <tinyxml2.h>
#include <optixu/optixpp_namespace.h>
#include <optixu/optixu_math_stream_namespace.h>
#include <sutil/sutil.h>
#include <random>
#include <fstream>
#include <iostream>
#include <chrono>

#define DEBUG true
#define MY_PI 3.1415926535897932384626433
int counter = 10;
float increment = 0.05;
float scale = 1.0;
bool update = false;

optix::Buffer amplitude_buffer;
optix::Buffer location_buffer;
optix::Buffer initPhase_buffer;
optix::Buffer compDepth_buffer;
optix::Buffer compAmplitude_buffer;
optix::Buffer compLocation_buffer;

GLuint volume_textureId, randInitPhase_textureId, transferFunction_textureId, random_texture_id;
optix::TextureSampler volume_texture, randInitPhase_texture, transferFunction_texture, random_texture;




int main() {
	
	WifiData wifi;

	wifi.loadCSV("../RowdenRender/Content/Data/EricWifi-2-4-19.csv");
	wifi.loadCSV("../RowdenRender/Content/Data/AlexWifi-2-4-19.csv");
	wifi.loadCSV("../RowdenRender/Content/Data/EricWifi-2-4-19(2).csv");
	wifi.loadCSV("../RowdenRender/Content/Data/AlexWifi-2-4-19(2).csv");
	wifi.loadCSV("../RowdenRender/Content/Data/EricWifi-2-5-19.csv");
	wifi.loadCSV("../RowdenRender/Content/Data/AlexWifi-2-5-19.csv");
	wifi.loadCSV("../RowdenRender/Content/Data/EricWifi-2-6-19.csv");
	wifi.loadCSV("../RowdenRender/Content/Data/AlexWifi-2-6-19.csv");
	wifi.loadCSV("../RowdenRender/Content/Data/EricWifi-2-7-19.csv");
	wifi.loadCSV("../RowdenRender/Content/Data/AlexWifi-2-26-19.csv");
	wifi.loadCSV("../RowdenRender/Content/Data/EricWifi-2-26-19.csv");
	wifi.loadCSV("../RowdenRender/Content/Data/AlexWifi-2-27-19.csv");
	wifi.loadCSV("../RowdenRender/Content/Data/EricWifi-2-27-19.csv");
	wifi.loadCSV("../RowdenRender/Content/Data/EricWifi-2-27-19(2).csv");
	
	wifi.Finalize(.00001f);
	//for (auto freq : wifi.frequencies) {
	for (int i = 0; i < 1; i++) {
		wifi.ComputeIDIntensities("umd");

		float** intensities = wifi.GetIDIntensities("umd");

		float maxIntensity = 0;
		
		int num_cells = 1;
		std::vector<glm::vec4> pixels;
		std::vector<float> use_intensities;
		use_intensities.resize(wifi.numLonCells * wifi.numLatCells * num_cells);

		for (int i = 0; i < wifi.numLonCells; i++)
		{
			for (int j = 0; j < wifi.numLatCells; j++)
			{
				float intensity = intensities[i][j];
				if (intensity > maxIntensity)
					maxIntensity = intensity;
			}
		}
		if (maxIntensity == 0){
			continue;
		}
		int curr_height = 0;//(num_cells - 1) / 2.0; //Just do the top half
		for (int h = 0; h < num_cells; h++) {
			for (int i = 0; i < wifi.numLonCells; i++)
			{
				for (int j = 0; j < wifi.numLatCells; j++)
				{
					float intensity = intensities[i][j] / maxIntensity;
					intensity = intensity - (increment * fabs(h - curr_height));
					if (intensity <= 0.0f)
					{
						use_intensities.at(j + wifi.numLatCells * i + wifi.numLatCells * wifi.numLonCells * h) = 0;
					}
					else
					{
						use_intensities.at(j + wifi.numLatCells * i + wifi.numLatCells * wifi.numLonCells * h) = intensity;
					}
				}
			}
		}
		
		std::fstream out = std::fstream("umd.raw", std::ios::out | std::ios::binary);
		out.write(reinterpret_cast<char*>(&wifi.numLatCells), sizeof(int));
		out.write(reinterpret_cast<char*>(&wifi.numLonCells), sizeof(int));
		out.write(reinterpret_cast<char*>(&num_cells), sizeof(int));
		for (int i = 0; i < use_intensities.size(); i++) {
			UINT8 test = (UINT8)(use_intensities.at(i) * 255);
			out << test;
		}
		out.close();
	}
	/*
	out = std::fstream("uninterpolated_big_all.ppm", std::ios::out);
	out << "P3" << std::endl;
	
	out << wifi.numLatCells << std::endl;
	out << wifi.numLonCells << std::endl;

	out << "255" << std::endl;
	int kernel_size = 1;
	for (int i = 0; i < wifi.numLonCells; i++) {
		for (int j = 0; j < wifi.numLatCells; j++) {
			int test = 0;
			int num_samples = 0;
			for (int kernel_i = i - kernel_size; kernel_i < i + kernel_size; kernel_i++) {
				for (int kernel_j = j - kernel_size; kernel_j < j + kernel_size; kernel_j++) {
					if (kernel_i < 0 || kernel_i > wifi.numLonCells || kernel_j < 0 || kernel_j > wifi.numLatCells) {
						continue;
					}
					int temp = (int)(use_intensities.at(kernel_j + kernel_i * wifi.numLatCells) * 255);
					
					if (temp > test)
						test = temp;
				}
			}
			out << test << " " << test << " " << test << "\n";
		}
	}
	out.close();

	*/
	

	return 0;
} 