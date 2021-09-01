#pragma once
#include "RowRender.h"
#include "WifiData.h"
#include "AVWWifiData.h"

#include "VR_Wrapper.h"
#include "DataBuffer.h"
#include "LineIntegralConvolution.h"

#include <random>
#include <fstream>
#include <iostream>
#include <chrono>
#include <time.h>
#include <numeric>

#include <glm/gtx/matrix_decompose.hpp>

#include <glm/gtc/matrix_access.hpp>
#include <windows.h>

using namespace vr;

GLuint fhp_tex, bhp_tex;
bool nearest_router_on = false;
bool jittered = true;
int color_offset = 205;
bool show_analytics = true;
bool demo_mode = true;
float scaling_factor = 2.5;

struct gBuffer {
	GLuint frame_buffer, normal_tex, tangent_tex, 
		color_tex, frag_pos_tex, color_array_tex,  ellipsoid_coordinates_tex,
		depth_render_buf, force_framebuffer, force_renderbuffer, force_tex, lic_color_tex,
		lic_accum_framebuffer, lic_accum_renderbuffer, 
		ssao_tex, ssao_framebuffer, ssao_renderbuffer, 
		ssao_blur_tex, ssao_blur_framebuffer, ssao_blur_renderbuffer;
	GLuint pboIDs[2], lic_tex[2], lic_framebuffer[2], lic_renderbuffer[2], lic_accum_tex;
	Texture2D color_texture, frag_pos_texture, tangent_texture,
		normal_texture, ellipsoid_coordinates_texture, force_texture,
		lic_texture[2], lic_accum_texture, lic_color_texture,
		ssao_texture, ssao_blur_texture;
};

struct eyeBuffer {
	GLuint frame_buffer, screen_tex, depth_render_buf;
	Texture2D screenTexture;
};

struct minimapBuffer {
	GLuint framebuffer, depth_render_buff, minimap_tex;
	Texture2D minimapTexture;
};
 
void maxSeperatedColors(int n, std::vector<glm::vec4>& out, bool ordered = true, float offset=0) {
	float increment = 360.0f / (n);
	int incrementor = ceil(n / 2.0f) * increment;
	if (ordered)
		incrementor = increment;
	
	for (int i = 0; i < n; i++) {
		float v = (incrementor * i) + offset;
		if ((n % 2 || i % 2) && !ordered)
			offset += increment;
		while (v > 360) {
			v = v - 360;
		}
		out[i] = glm::vec4(hsv2rgb(glm::vec3(v, .6f, 1.f)), 1);
	}
}

Lights setPointLights(int num_lights, float intensity, float linear, float quadratic) {
	Lights ret = Lights();
	std::vector<glm::vec3> controlPoints({
		glm::vec3(-11.45, 11.6, .8),
		glm::vec3(-11.45, 15.75, 0.8),
		glm::vec3(8.9, 15.75, 0.8),
		glm::vec3(8.9, -14.35, 0.8),
		glm::vec3(-11.45, -14.35, 0.8),
		glm::vec3(-11.45, -10.24, 0.8),
		glm::vec3(3.9, -10.24, 0.8),
		glm::vec3(3.9, 11.6, 0.8),
		glm::vec3(-11.45, 11.6, .8)
		}
	);
	for (int i = 0; i < controlPoints.size(); i++)
		controlPoints[i] *= scaling_factor;
	std::vector<float> distances(controlPoints.size());
	distances.at(0) = 0;
	for (int i = 1; i < controlPoints.size(); i++) {
		distances[i] = glm::distance(
			controlPoints.at(i - 1),
			controlPoints.at(i)
		);
	}
	float total_distance = std::accumulate(distances.begin(), distances.end(), 0);
	float equal_distance = total_distance / (num_lights - 1);
	int curr_light = 0;
	float curr_distance = 0;
	ret.addPointLight(controlPoints.at(0), intensity, linear, quadratic,
		glm::vec3(1), glm::vec3(1), glm::vec3(1));
	curr_light++;
	int j = 1;
	while (curr_distance < total_distance) {
		if (curr_distance + distances.at(j) > equal_distance * curr_light) {
			float dist_between_points = distances.at(j);
			float dist_along_line = equal_distance * curr_light - curr_distance;
			float t = dist_along_line / dist_between_points;
			ret.addPointLight(
				glm::lerp(controlPoints.at(j - 1),
					controlPoints.at(j), 
					t),
				intensity, linear, quadratic, 
				glm::vec3(1), glm::vec3(1), glm::vec3(1)
			);
			curr_light++;
		}
		else {
			curr_distance += distances.at(j++);
		}
	}
	return ret;
}

std::ostream& operator<< (std::ostream& out, const glm::vec3& vec) {
	out << "{"
		<< vec.x << " " << vec.y << " " << vec.z
		<< "}";

	return out;
}



void writeBinaryVector(std::vector<bool> vec, std::ofstream&outfile) {
	size_t size = vec.size();
	outfile.write((char*)&size, sizeof(size_t));
	for (int i = 0; i < size; i++) {
		if (vec.at(i)) {
			outfile.write("1", 1);
		}
		else {
			outfile.write("0", 1);
		}
	}
}

void readBinaryVector(std::vector<bool>& vec, std::ifstream&infile) {
	size_t size = 0;
	infile.read((char*)&size, sizeof(size_t));
	vec.resize(size);
	char bit = '0';
	for (int i = 0; i < size; i++) {
		infile.read(&bit, 1);
		if (bit == '1')
			vec.at(i) = true;
		else
			vec.at(i) = false;
	}
}
template <typename T>
void writeDictionary(std::map<std::string, T> dict, std::ofstream&outfile) {
	size_t size = dict.size();
	outfile.write((char*)&size, sizeof(size_t));
	
	for (std::pair<std::string, T> keyval: dict) {
		outfile << keyval.first	<< std::endl;
		outfile << keyval.second<< std::endl;
	}
}
template <typename T>
void readDictionary(std::map<std::string, T>& dict, std::ifstream&infile) {
	infile >> std::ws;
	size_t size = 0;
	infile.read((char*)&size, sizeof(size_t));
	
  	std::string key;
	T value;
	for (int i = 0; i < size; i++) {
		infile >> key;
		infile >> value;
		dict[key] = value;
	}
	
}

float calculateMask(int j, int step_num, bool use_mask) {
	const int NUM_STEPS = 8; //NOTE: This is set to be the same as in LIC.frag
	float mask = 0;
	float total_num_steps = NUM_STEPS * 3;
	float current_step = NUM_STEPS * step_num + j;
	if (use_mask) {
		mask = cos(.5 * 3.1415 * (total_num_steps - current_step) / (total_num_steps));
		return (0.5 + 0.5 * mask);
	}return total_num_steps - current_step;
}

float getTotalMaskForStep(int step_num, bool use_mask) {
	float ret = 0;
	const int NUM_STEPS = 8; //NOTE: This is set to be the same as in LIC.frag

	
	for (int j = 0; j < NUM_STEPS; j++){
		ret += calculateMask(j + NUM_STEPS * step_num , step_num, use_mask);
	}
	return ret;
}

void readStateFile(std::string filename, std::vector<bool>&routers, 
					std::vector<bool>&wifinames, std::vector<bool>&freqs) {
	std::ifstream infile(filename.c_str(), std::ios::in);
	if (!infile.is_open())
		std::cout << "File not found";
	else {
		readBinaryVector(routers, infile);
		readBinaryVector(wifinames, infile);
		readBinaryVector(freqs, infile);
	}
	infile.close();
}

void createFramebuffer(glm::vec2 resolution, gBuffer* buffer, bool resize) {
	//generate framebuffers during resizing
	if (!resize) {
		glGenFramebuffers(1, &buffer->frame_buffer);
		glGenTextures(1, &buffer->normal_tex);
		glGenTextures(1, &buffer->color_tex);
		glGenTextures(1, &buffer->frag_pos_tex);
		glGenBuffers(2, buffer->pboIDs);
		glGenTextures(1, &buffer->ellipsoid_coordinates_tex);
		glGenTextures(1, &buffer->tangent_tex);
		glGenRenderbuffers(1, &buffer->depth_render_buf);
		glGenFramebuffers(1, &buffer->force_framebuffer);
		glGenTextures(1, &buffer->force_tex);
		glGenTextures(1, &buffer->lic_color_tex);
		glGenRenderbuffers(1, &buffer->force_renderbuffer);
		glGenFramebuffers(2, buffer->lic_framebuffer);
		glGenTextures(2, buffer->lic_tex);
		glGenRenderbuffers(2, buffer->lic_renderbuffer);
		glGenFramebuffers(1, &buffer->lic_accum_framebuffer);
		glGenTextures(1, &buffer->lic_accum_tex);
		glGenRenderbuffers(1, &buffer->lic_accum_renderbuffer);
		glGenFramebuffers(1, &buffer->ssao_framebuffer);
		glGenTextures(1, &buffer->ssao_tex);
		glGenRenderbuffers(1, &buffer->ssao_renderbuffer);
		glGenFramebuffers(1, &buffer->ssao_blur_framebuffer);
		glGenTextures(1, &buffer->ssao_blur_tex);
		glGenRenderbuffers(1, &buffer->ssao_blur_renderbuffer);
	}
	//Set Framebuffer Attributes
	glBindFramebuffer(GL_FRAMEBUFFER, buffer->frame_buffer);
	
	glBindTexture(GL_TEXTURE_2D, buffer->normal_tex);
	buffer->normal_texture = Texture2D();
	buffer->normal_texture.SetTextureID(buffer->normal_tex);
	buffer->normal_texture.giveName("normal_tex");
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer->normal_tex, 0);	
	
	
	glBindTexture(GL_TEXTURE_2D, buffer->color_tex);
	buffer->color_texture = Texture2D();
	buffer->color_texture.SetTextureID(buffer->color_tex);
	buffer->color_texture.giveName("albedo_tex");
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, buffer->color_tex, 0);	
	
	glBindTexture(GL_TEXTURE_2D, buffer->frag_pos_tex);
	buffer->frag_pos_texture = Texture2D();
	buffer->frag_pos_texture.SetTextureID(buffer->frag_pos_tex);
	buffer->frag_pos_texture.giveName("fragPos_tex");

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, buffer->frag_pos_tex, 0);
	
	glBindBuffer(GL_PIXEL_PACK_BUFFER, buffer->pboIDs[0]);
	glBufferData(GL_PIXEL_PACK_BUFFER, resolution.x * resolution.y * 4 * sizeof(float), 0, GL_STREAM_READ);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, buffer->pboIDs[1]);
	glBufferData(GL_PIXEL_PACK_BUFFER, resolution.x * resolution.y * 4 * sizeof(float), 0, GL_STREAM_READ);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, buffer->ellipsoid_coordinates_tex);
	buffer->ellipsoid_coordinates_texture = Texture2D();
	buffer->ellipsoid_coordinates_texture.SetTextureID(buffer->ellipsoid_coordinates_tex);
	buffer->ellipsoid_coordinates_texture.giveName("ellipsoid_coordinates_tex");

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, buffer->ellipsoid_coordinates_tex, 0);
	
	glBindTexture(GL_TEXTURE_2D, buffer->tangent_tex);
	buffer->tangent_texture = Texture2D();
	buffer->tangent_texture.SetTextureID(buffer->tangent_tex);
	buffer->tangent_texture.giveName("tangent_tex");

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, buffer->tangent_tex, 0);
	
	const int numAttachments = 5;

	GLenum DrawBuffers[numAttachments];
	for (int i = 0; i < numAttachments; ++i)
		DrawBuffers[i] = GL_COLOR_ATTACHMENT0 + i; //Sets appropriate indices for each color buffer
	
	glDrawBuffers(numAttachments, DrawBuffers);

	
	glBindRenderbuffer(GL_RENDERBUFFER, buffer->depth_render_buf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution.x, resolution.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer->depth_render_buf);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "framebuffer broke" << std::endl;
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, buffer->force_framebuffer);

	glBindTexture(GL_TEXTURE_2D, buffer->force_tex);
	buffer->force_texture = Texture2D();
	buffer->force_texture.SetTextureID(buffer->force_tex);
	buffer->force_texture.giveName("force_tex");

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer->force_tex, 0);
	
	glBindTexture(GL_TEXTURE_2D, buffer->lic_color_tex);
	buffer->lic_color_texture = Texture2D();
	buffer->lic_color_texture.SetTextureID(buffer->lic_color_tex);
	buffer->lic_color_texture.giveName("lic_color_tex");

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, resolution.x, resolution.y, 0, GL_RGB, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, buffer->lic_color_tex, 0);

	GLenum drawBuffer[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, drawBuffer);

	glBindRenderbuffer(GL_RENDERBUFFER, buffer->force_renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution.x, resolution.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer->force_renderbuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "lic prepass framebuffer broke" << std::endl;
		return;
	}

	for (int i = 0; i < 2; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, buffer->lic_framebuffer[i]);
	
		glBindTexture(GL_TEXTURE_2D, buffer->lic_tex[i]);
		buffer->lic_texture[i] = Texture2D();
		buffer->lic_texture[i].SetTextureID(buffer->lic_tex[i]);
		buffer->lic_texture[i].giveName("lic_tex[" + std::to_string(i) + "]");

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer->lic_tex[i], 0);
	
		drawBuffer[0] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, drawBuffer);

		glBindRenderbuffer(GL_RENDERBUFFER, buffer->lic_renderbuffer[i]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution.x, resolution.y);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer->lic_renderbuffer[i]);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cout << "lic framebuffer broke" << std::endl;
			return;
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, buffer->lic_accum_framebuffer);

	glBindTexture(GL_TEXTURE_2D, buffer->lic_accum_tex);
	buffer->lic_accum_texture = Texture2D();
	buffer->lic_accum_texture.SetTextureID(buffer->lic_accum_tex);
	buffer->lic_accum_texture.giveName("lic_accum_tex");

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer->lic_accum_tex, 0);
	
	//drawBuffer = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACMENT0};
	glDrawBuffers(1, drawBuffer);

	glBindRenderbuffer(GL_RENDERBUFFER, buffer->lic_accum_renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution.x, resolution.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer->lic_accum_renderbuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "lic accumulation framebuffer broke" << std::endl;
		return;
	}
	

	glBindFramebuffer(GL_FRAMEBUFFER, buffer->ssao_framebuffer);

	glBindTexture(GL_TEXTURE_2D, buffer->ssao_tex);
	buffer->ssao_texture = Texture2D();
	buffer->ssao_texture.SetTextureID(buffer->ssao_tex);
	buffer->ssao_texture.giveName("ssao_tex");

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, resolution.x, resolution.y, 0, GL_RED, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer->ssao_tex, 0);
	
	glDrawBuffers(1, drawBuffer);

	glBindRenderbuffer(GL_RENDERBUFFER, buffer->ssao_renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution.x, resolution.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer->ssao_renderbuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ssao framebuffer broke" << std::endl;
		return;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, buffer->ssao_blur_framebuffer);

	glBindTexture(GL_TEXTURE_2D, buffer->ssao_blur_tex);
	buffer->ssao_blur_texture = Texture2D();
	buffer->ssao_blur_texture.SetTextureID(buffer->ssao_blur_tex);
	buffer->ssao_blur_texture.giveName("ssao_blur_tex");

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, resolution.x, resolution.y, 0, GL_RED, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer->ssao_blur_tex, 0);

	glDrawBuffers(1, drawBuffer);

	glBindRenderbuffer(GL_RENDERBUFFER, buffer->ssao_blur_renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution.x, resolution.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer->ssao_blur_renderbuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ssao blur framebuffer broke" << std::endl;
		return;
	}
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void createEyeFramebuffer(glm::vec2 resolution, eyeBuffer* buffer, bool resize) {
	if (!resize) {
		glGenFramebuffers(1, &buffer->frame_buffer);
		glGenTextures(1, &buffer->screen_tex);
		glGenRenderbuffers(1, &buffer->depth_render_buf);

	}
	glBindFramebuffer(GL_FRAMEBUFFER, buffer->frame_buffer);

	
	glBindTexture(GL_TEXTURE_2D, buffer->screen_tex);
	buffer->screenTexture = Texture2D();
	buffer->screenTexture.SetTextureID(buffer->screen_tex);
	//buffer->screenTexture.giveName("ScreenTexture");

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer->screen_tex, 0);

	glBindRenderbuffer(GL_RENDERBUFFER, buffer->depth_render_buf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution.x, resolution.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer->depth_render_buf);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "framebuffer broke" << std::endl;
		return;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void createFramebuffers(glm::vec2 resolution, gBuffer *buffers, eyeBuffer *eyes, bool use_vr, bool resize) {
	
	createFramebuffer(resolution, &buffers[0], resize);
	createEyeFramebuffer(resolution, &eyes[0], resize);
	if (use_vr)
		createEyeFramebuffer(resolution, &eyes[1], resize);
	eyes[0].screenTexture.giveName("screenTexture");
}

void updateNoise(LineIntegralConvolution& lic, int num_samples, int num_routers, Texture2D *noise, Texture2D *antialiased_textures, glm::uvec2 lic_texture_res, float an) {
	//lic(glm::uvec2(64, 256));
	for (int i = 0; i < num_routers; i++) {
		lic.clear();
		lic.fillWithNoise(num_samples/num_routers);
		//lic.convolve(fall_off);

		noise[i] = Texture2D(lic.getTexture().data(), lic_texture_res.y, lic_texture_res.x);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, an);
		noise[i].setTexMinMagFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
		noise[i].giveName("noise_tex[" + std::to_string(i)  + "]");
		antialiased_textures[1 + i] = noise[i];
	}
	
}


void rayPicker(glm::vec3 ray, glm::vec3 ray_coord, glm::vec3 camera_pos ,glm::vec3& intersection, ShaderProgram &s, float scaling_factor) {
	//std::cout << glm::to_string(ray_coord + camera_pos) << std::endl;
	float plane_height = 0;
	float z_coord = ray_coord.z + camera_pos.z;
	plane_height = s.getFloor(ray_coord + camera_pos, scaling_factor) * scaling_factor;

	//std::cout << plane_height << std::endl;
	//std::cout << z_coord << std::endl;

	float t = (plane_height - z_coord) / ray.z;
	//std::cout << t << std::endl;
	intersection = ray_coord + camera_pos + t * ray;
	//intersection = camera_pos + ray * .1f;
	return;
}

std::string getLocationString(glm::vec3 position) {
	std::string ret("Floor ");
	if (position.z > 5 || position.z < 0) {
		return std::string("Outside");
	}
	ret += std::to_string((int)floor(position.z));
	ret += " ";
	if (position.y > 10) {
		ret += "West Hallway";
	}
	else if (position.y > -8.5) {
		ret += "Central Hallway";
	}
	else {
		ret += "East Hallway";
	}
	return ret;
}

void startNearestRouters(std::vector<bool>&wifinames, 
	std::vector<bool>&routers, std::vector<bool>&freqs,
	Camera&camera, AVWWifiData&wifi,int&num_routers, 
	std::map<std::string, float>&deferred_shading_floats,
	bool&start_render, bool&num_routers_changed, bool&updated_routers
	) {
	start_render = true;
	num_routers_changed = true;
	std::fill(freqs.begin(), freqs.end(), true);
	wifi.setNearestNRouters(num_routers, camera.getPosition(), wifinames, routers, freqs);
	deferred_shading_floats["delta_theta"] = 180.f / wifi.getActiveFreqs(freqs).size();
	updated_routers = true;
	nearest_router_on = true;
}

void reset(AVWWifiData wifi,std::vector<bool> &wifinames, std::vector<bool> &freqs,
	std::vector<bool> &routers) {
	wifinames.resize(wifi.getNumWifiNames());
	std::fill(wifinames.begin(), wifinames.end(), true);
	wifi.setAvailableFreqs(wifi.getWifinames());
	freqs.resize(wifi.getAvailableFreqs().size());
	std::fill(freqs.begin(), freqs.end(), true);
	wifi.setAvailableMacs(wifi.getWifinames(), wifi.getAvailableFreqs());
	routers.resize(wifi.getAvailablesMacs().size());
	std::fill(routers.begin(), routers.end(), false);
}

void createMinimapBuffer(glm::uvec2 resolution, minimapBuffer& buffer) {
	glGenFramebuffers(1, &buffer.framebuffer);
	glGenRenderbuffers(1, &buffer.depth_render_buff);
	glGenTextures(1, &buffer.minimap_tex);

	glBindFramebuffer(GL_FRAMEBUFFER, buffer.framebuffer);

	glBindTexture(GL_TEXTURE_2D, buffer.minimap_tex);
	buffer.minimapTexture = Texture2D();
	buffer.minimapTexture.SetTextureID(buffer.minimap_tex);
	buffer.minimapTexture.setDims(resolution.x, resolution.y, 4);
	buffer.minimapTexture.giveName("minimap_tex");

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer.minimap_tex, 0);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	
	glBindRenderbuffer(GL_RENDERBUFFER, buffer.depth_render_buff);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution.x, resolution.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer.depth_render_buff);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "framebuffer broke" << std::endl;
		return;
	}
}

int AVWilliamsWifiVisualization(bool use_vr) {

	bool start_render = false;
	std::string gui_type = "demo";
	//initialize glfw
	glfwInit();
	glfwSetErrorCallback(error_callback);
	
	VR_Wrapper vr = VR_Wrapper();

	glm::uvec2 render_resolution = glm::uvec2(0, 0);
	glm::uvec2 screen_size = glm::uvec2(0, 0);
	glm::mat4 camera_offset = glm::mat4(1);
	if (use_vr) {
		vr.initialize();
		vr.resetZeroPose();
		vr.initCompositor();
		char path[100];	
		GetCurrentDirectory(MAX_PATH, path);
		std::cout << std::string(path) + "/Content/VR Stuff/actions.json" << std::endl;
		vr.SetActionManifestPath(std::string(path) + "/Content/VR Stuff/actions.json");
		vr.setActionHandles();
		camera_offset = vr.getSeatedZeroPoseToStandingPose();
		render_resolution = vr.getRenderTargetSize();
		std::cout << "resolution: " << glm::to_string(render_resolution) << std::endl;
	}
	//Open and setup window
	
	Window w = Window("AV Williams Wifi Visualization", render_resolution.x, render_resolution.y);
	screen_size = glm::uvec2(w.width, w.height);
	if (!use_vr) {
		render_resolution = screen_size;
	}
	//glViewPort(0, 0, resolution.x, resolution.y);
	//w.SetViewportSize(resolution.x, resolution.y);
	//if(!use_vr)
	//	w.setFullScreen(true);
	//initialize GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) //load GLAD
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}

	//setup IMGUI
	setupDearIMGUI(w.window);

	// During init, enable debug output
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);
	glViewport(0, 0, render_resolution.x, render_resolution.y);

	//Allow Resizing
	w.SetFramebuferSizeCallback();

	//Disable V-SYNC
	//glfwSwapInterval(0);

	//Enable Transparency
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//Enable Text Rendering
	TextRenderer tr = TextRenderer();
	TextRenderer popupText = TextRenderer();
	TextRenderer analyticsText = TextRenderer();
	int font_size = 64;
	float hue_start = 0;
	tr.SetCharacterSize(font_size);
	popupText.SetCharacterSize(font_size);
	analyticsText.SetCharacterSize(font_size);
	float pixel_size = font_size * 109 / 72.0f;
	int font_height = ceil(
		pixel_size *
		(tr.arial_face->bbox.yMax - tr.arial_face->bbox.yMin) /
		tr.arial_face->units_per_EM
	);int font_width = ceil(
		pixel_size *
		(tr.arial_face->bbox.xMax - tr.arial_face->bbox.xMin) /
		tr.arial_face->units_per_EM
	);
	int max_chars = 6;
	
	float texcoord_scale = .004 / scaling_factor;
	//Load shaders
	ShaderProgram model_shader		= ShaderProgram({ 
		ShaderProgram::Shaders::FRAG_ELLIPSOID, 
		ShaderProgram::Shaders::VERT_ELLIPSOID });
	ShaderProgram skybox_shader		= ShaderProgram({ 
		ShaderProgram::Shaders::SKY_FRAG, 
		ShaderProgram::Shaders::SKY_VERT });
	ShaderProgram instance_shader	= ShaderProgram({ 
		ShaderProgram::Shaders::INSTANCE_FRAG_COLOR,
		ShaderProgram::Shaders::INSTANCE_VERT_COLOR });
	ShaderProgram ground_shader		= ShaderProgram({ 
		ShaderProgram::Shaders::NO_LIGHT_FRAG, 
		ShaderProgram::Shaders::NO_LIGHT_VERT });
	ShaderProgram volume_shader		= ShaderProgram({ 
		ShaderProgram::Shaders::VOLUME_FRAG_3D, 
		ShaderProgram::Shaders::VOLUME_VERT_3D});
	ShaderProgram deferred_shader	= ShaderProgram({ 
		ShaderProgram::Shaders::DEFFERED_RENDER_ELLIPSOID_FRAG, 
		ShaderProgram::Shaders::DEFFERED_RENDER_ELLIPSOID_VERT });
	ShaderProgram render2quad		= ShaderProgram({ 
		ShaderProgram::Shaders::SCREEN_FRAG, 
		ShaderProgram::Shaders::SCREEN_VERT });
	ShaderProgram quad_shader		= ShaderProgram({ 
		ShaderProgram::Shaders::QUAD_RENDER_FRAG, 
		ShaderProgram::Shaders::QUAD_RENDER_VERT });
	ShaderProgram force_shader = ShaderProgram({
		ShaderProgram::Shaders::LIC_PREPASS_FRAG,
		ShaderProgram::Shaders::LIC_PREPASS_VERT });
	ShaderProgram lic_shader = ShaderProgram({
		ShaderProgram::Shaders::LIC_FRAG,
		ShaderProgram::Shaders::LIC_PREPASS_VERT });
	ShaderProgram lic_accum_shader = ShaderProgram({
		ShaderProgram::Shaders::LIC_ACCUM_FRAG,
		ShaderProgram::Shaders::LIC_PREPASS_VERT });
	ShaderProgram ssao_shader = ShaderProgram({
		ShaderProgram::Shaders::SSAO_FRAG,
		ShaderProgram::Shaders::SSAO_VERT});
	ShaderProgram ssao_blur_shader = ShaderProgram({
		ShaderProgram::Shaders::SSAO_BLUR_FRAG,
		ShaderProgram::Shaders::SCREEN_VERT
		});
	ShaderProgram minimap_shader = ShaderProgram(std::vector<std::string>({
		"shaders/minimap.frag",
		"shaders/minimap.vert"
		}));


	//Setup Skybox
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	std::vector<std::string> skybox_files;

	skybox_files.emplace_back("./Content/Textures/Skyboxes/miramar_lf.png");
	skybox_files.emplace_back("./Content/Textures/Skyboxes/miramar_rt.png");
	skybox_files.emplace_back("./Content/Textures/Skyboxes/miramar_bk.png");
	skybox_files.emplace_back("./Content/Textures/Skyboxes/miramar_ft.png");
	skybox_files.emplace_back("./Content/Textures/Skyboxes/miramar_up.png");
	skybox_files.emplace_back("./Content/Textures/Skyboxes/miramar_dn.png");

	Texture2D skybox_tex = Texture2D(skybox_files);

	Model skybox = Model("./Content/Models/cube/cube.obj");
	skybox.setModel();
	skybox.getMeshes().at(0)->setTexture(&skybox_tex, 0);


	Model AVW("./Content/Models/AVW_sliced.obj", true);
	AVW.setModel(true);
	Texture2D white = Texture2D(glm::vec4(1,1,1,1));
	white.giveName("texture_diffuse");
	//AVW.getMeshes()[0]->setTexture(white, 0);
	
	for (int i = 0; i < AVW.getMeshes().size(); i++) {
		AVW.getMeshes()[i]->setTexture(&white);
	}

	//AVW.getMeshes()[0]->addTexture(text);
	glm::mat4 avw_transform(1);
	avw_transform = glm::rotate(avw_transform, glm::radians(90.f), 
		glm::vec3(1, 0, 0));
	glm::vec3 avw_scale(2, 1, 2);
	avw_scale *= scaling_factor;
	avw_transform = glm::scale(avw_transform, avw_scale);

	Texture2D popup_background("./Content/Textures/popup_background.png");
	popup_background.setTexMinMagFilter(GL_NEAREST, GL_NEAREST);
	//setup instance rendering
	Model Sphere("./Content/Models/sphere.obj");
	Sphere.setModel();
	glm::mat4 wifi_transform(1);
	wifi_transform = glm::scale(wifi_transform, glm::vec3(1,1,1));
	glm::mat4 glyph_transform(1);
	glyph_transform = glm::scale(glyph_transform, glm::vec3(1));
	bool updated_routers = true;

	//setup hand model
	Model LeftHand("./Content/Models/controller_l.fbx");
	LeftHand.setModel();
	Model RightHand("./Content/Models/controller_r.fbx");
	RightHand.setModel();

	Model Cylinder("./Content/Models/cylinder.obj");
	Cylinder.setModel();

	//Setup ground rendering
	Model quad("./Content/Models/quad/quad_centered.obj", true);
	quad.setModel(true);
	Model ground_quad("./Content/Models/quad/quad_centered.obj", false);
	ground_quad.setModel(false);
	ground_quad.getMeshes().at(0)->addTexture(&white);
	glm::mat4 ground_transform(1);
	ground_transform = glm::scale(ground_transform, scaling_factor * glm::vec3(15, 19, 2));
	ground_transform = glm::translate(ground_transform, glm::vec3(0, 0, -.101));
	//quad.getMeshes().at(0)->setTexture(Texture2D(glm::vec4(50, 50, 50, 255) * (1 / 255.0f)), 0);
	

	//Setup volume rendering
	Model screen_quad("./Content/Models/quad/quad_centered.obj");
	screen_quad.setModel();
	

	//Setup Camera
	//Camera camera = Camera(glm::vec3(-11.54, 11.6, .4575),glm::vec3(-10.54, 11.6, .4575) , 60.0f, w.width / (float)w.height);
	//Camera camera = Camera(glm::vec3(.467506, 11.501509, 2.478917),glm::vec3(.467506 - .970493, 11.501509 + .020823, 2.478917 - .240227) , 60.0f, w.width / (float)w.height);
	Camera camera = Camera(scaling_factor * glm::vec3(-.262911, 11.58143, 2.414213), scaling_factor * glm::vec3(-.262911, 11.58143, 2.414213) + glm::vec3(.89027, -.430448, 0), 60.0f, w.width / (float)w.height);
	w.SetCamera(&camera);
	w.setSpeed(scaling_factor);

	//setUp SSAO Kernel
	std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
	std::default_random_engine generator;
	//std::vector<glm::vec3> ssaoKernel;
	for (unsigned int i = 0; i < 64; ++i)
	{
		glm::vec3 sample(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator)
		);
		float scale = (float)i / 64.0;
		scale = glm::lerp(0.1f, 1.0f, scale * scale);
		sample *= scale;
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		//ssaoKernel.push_back(sample);
		ssao_shader.Use();
		ssao_shader.SetUniform(("samples[" + std::to_string(i) + "]").c_str(), sample);
		
	}
	std::vector<glm::vec3> ssaoNoise;
	for (unsigned int i = 0; i < 16; i++)
	{
		glm::vec3 noise(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			0.0f);
		ssaoNoise.push_back(noise);
	}
	Texture2D ssaoNoise_texture;
	unsigned int noiseTexture;
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	ssaoNoise_texture.SetTextureID(noiseTexture);
	ssaoNoise_texture.setDims(4, 4, 3);
	ssaoNoise_texture.giveName("ssaoNoise");

	

	//Setup Light
	Lights lights = Lights();
	lights.addDirLight(glm::vec3(-1, .2, .2), glm::vec3(1, 1, 1));

	//Model Lights
	Lights modelLights = setPointLights(20, 1, .1, .01);
	int totalLights = 100;
	int numLights = 20;
	//modelLights.addPointLight(glm::vec3(-15, 15, 0), .7, glm::vec3(1));
	//modelLights.addPointLight(glm::vec3(0, 15, -15), .7, glm::vec3(1));
	//modelLights.addPointLight(glm::vec3(0,0,0), .2, glm::vec3(1,1,1));

	std::map<std::string, float> deferred_shading_floats = {
		{"ambient_coeff", .2f},
		{"spec_coeff", .0},
		{"diffuse_coeff", .7f},
	}, instance_shading_floats = {
		{"ambient_coeff", .3f},
		{"spec_coeff", .4},
		{"diffuse_coeff", .3f},
	}, lic_shading_floats;
	std::map<std::string, int> deferred_shading_ints = { 
		{"shininess", 32} 
	}, instance_shading_ints = {
		{"shininess", 16}
	}, lic_shading_ints;
	

	//send light params to model shader
	deferred_shader.SetUniforms(deferred_shading_floats);
	deferred_shader.SetUniforms(deferred_shading_ints);

	//send light params to instance shader
	instance_shader.SetUniforms(instance_shading_floats);
	instance_shader.SetUniforms(instance_shading_ints);

	
	float constant = 1, linear = .77, quadratic = .123;
	
	int odd_or_even_frame = 0;


	//load avw wifi data
	ShaderProgram router_shaders[2] = { deferred_shader, force_shader };

	AVWWifiData wifi(router_shaders, 2);
	Ellipsoid ellipsoid;
	wifi.loadEllipsoid("./Content/Data/one_router.elipsoid", ellipsoid);
	

	VolumeData volume;

	wifi.loadWifi("./Content/Data/AVW1.txt", "1");
	wifi.loadWifi("./Content/Data/AVW2.txt", "2");
	wifi.loadWifi("./Content/Data/AVW3.txt", "3");
	wifi.loadWifi("./Content/Data/AVW4.txt", "4");
	wifi.pruneEntries();
	wifi.setupStructures();
	std::vector<glm::vec4> wifi_colors(wifi.getNumWifiNames());
	maxSeperatedColors(wifi.getNumWifiNames(), wifi_colors);

	Texture2D wifi_tex = Texture2D(&wifi_colors, wifi_colors.size(), 1);
	wifi_tex.setTexMinMagFilter(GL_NEAREST, GL_NEAREST);
	wifi_tex.setTexParameterWrap(GL_CLAMP, GL_CLAMP);
	std::vector<glm::mat4> wifi_transforms;
	wifi_tex.giveName("wifi_colors");
	Sphere.getMeshes().at(0)->setTexture(&wifi_tex, 0);
	Texture2D frequency_texture = Texture2D("./Content/Textures/texton_paper_aspect.png");
	frequency_texture.giveName("frequency_tex");
	float an = 0.0f;

	Texture2D widget_background = Texture2D("./Content/Textures/quad_background.png");
	
	


	frequency_texture.Bind();
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &an); 
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, an);
	frequency_texture.setTexMinMagFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	frequency_texture.setTexParameterWrap(GL_REPEAT);
	//AVW.getMeshes().at(0)->addTexture(frequency_texture);
	quad.getMeshes().at(0)->setTexture(&frequency_texture);

	Texture2D minimap_base[4] = {
		Texture2D("./Content/Textures/AVWFloorPlans/1c.png"),
		Texture2D("./Content/Textures/AVWFloorPlans/2c.png"),
		Texture2D("./Content/Textures/AVWFloorPlans/3c.png"),
		Texture2D("./Content/Textures/AVWFloorPlans/4c.png"),
	};
	int oldFloor = minimap_shader.getFloor(camera.getPosition(), scaling_factor);
	minimap_base[oldFloor].name = "minimap_base_tex";
	int currFloor = oldFloor;
	for (int i = 0; i < 4; i++) {
		quad.getMeshes().at(0)->addTexture(&minimap_base[i]);
	}
	minimapBuffer minimap_buffer;
	createMinimapBuffer(minimap_base[0].getDims(), minimap_buffer);

	minimap_shader.SetUniform("bl", scaling_factor * glm::vec2(-13.21, -18));
	minimap_shader.SetUniform("tr", scaling_factor * glm::vec2(10.25, 17.659));
	minimap_shader.SetUniform("aspect", ((float)minimap_base[0].getDims().y / minimap_base[0].getDims().x));

	//std::vector<glm::vec4> heatmap = std::vector<glm::vec4>(2);
	//heatmap[0] = glm::vec4(1, 1, 1, 1);
	//heatmap[1] = glm::vec4(0, 0, 0, 1);
	//Texture2D heatmap_tex = Texture2D(&heatmap, 2, 1);
	//heatmap_tex.giveName("ellipsoid_tex");
	//heatmap_tex.setTexMinMagFilter(GL_LINEAR, GL_LINEAR);
	//heatmap_tex.setTexParameterWrap(GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
	//AVW.getMeshes().at(0)->addTexture(heatmap_tex);
	//AVW.getMeshes().at(0)->addTexture(wifi_tex);


	//Setup ImGUI variables

	
	glm::vec3 wifi_scale = glm::vec3(30, 34.792, 1);
	glm::vec3 wifi_translate = glm::vec3(-15., -16.042, -.833);
	static std::vector<bool> wifinames(wifi.getNumWifiNames());
	std::fill(wifinames.begin(), wifinames.end(), true);
	wifi.setAvailableFreqs(wifi.getSelectedNames(wifinames));
	wifi.setAvailableMacs(wifi.getSelectedNames(wifinames));
	
	static std::vector<bool> routers(wifi.getAvailablesMacs().size());
	float transparency = 1;

	std::fill(routers.begin(), routers.end(), false);

	static std::vector<bool> freqs(wifi.getAvailableFreqs().size());
	std::fill(freqs.begin(), freqs.end(), true);

	glEnable(GL_DEPTH_TEST);


	gBuffer buffer[1];
	eyeBuffer eyes[2];
	createFramebuffers(render_resolution, buffer, eyes, use_vr, false);
	quad.getMeshes().at(0)->addTexture(&buffer[0].color_texture);
	quad.getMeshes().at(0)->addTexture(&buffer[0].frag_pos_texture);
	quad.getMeshes().at(0)->addTexture(&buffer[0].normal_texture);
	quad.getMeshes().at(0)->addTexture(&buffer[0].ellipsoid_coordinates_texture);
	quad.getMeshes().at(0)->addTexture(&buffer[0].tangent_texture);
	quad.getMeshes().at(0)->addTexture(&buffer[0].force_texture);
	quad.getMeshes().at(0)->addTexture(&buffer[0].lic_texture[0]);
	quad.getMeshes().at(0)->addTexture(&buffer[0].lic_texture[1]);
	quad.getMeshes().at(0)->addTexture(&buffer[0].lic_accum_texture);
	quad.getMeshes().at(0)->addTexture(&buffer[0].lic_color_texture);
	quad.getMeshes().at(0)->addTexture(&eyes[0].screenTexture);
	quad.getMeshes().at(0)->addTexture(&ssaoNoise_texture);
	quad.getMeshes().at(0)->addTexture(&buffer[0].ssao_texture);
	quad.getMeshes().at(0)->addTexture(&buffer[0].ssao_blur_texture);
	quad.getMeshes().at(0)->addTexture(&wifi_tex);
	quad.getMeshes().at(0)->addTexture(&white);
	quad.getMeshes().at(0)->addTexture(&frequency_texture);

	int num_fb = 1;
	if (use_vr)
		num_fb = 2;
	//main render loop
	
	vr::Hmd_Eye curr_eye = vr::Hmd_Eye::Eye_Left;
	glm::mat4 ProjectionMat, ViewMat;

	std::map<std::string, glm::vec4> saved_colors;
	int old_num_colors = -1;

	float linear_term = 1, thickness = .056, distance_mask = 0, billboard_scale = .4;
	
	bool send_data = false, polling_pbo = false, renderPopup = false;
	std::vector<std::string> popup_text;

	int num_routers = 1;
	int num_samples = 102400;
	
	glm::uvec2 lic_texture_res(2048 , 2048);
	int old_num_samples = num_samples;

	LineIntegralConvolution lic(lic_texture_res); 
	
	//lic.convolve(fall_off);
	Texture2D noise[20];
	for (int i = 0; i < 20; i++) {
		lic.clear();
		lic.fillWithNoise(num_samples);
		noise[i] = Texture2D(lic.getTexture().data(), lic_texture_res.y, lic_texture_res.x);
		noise[i].setTexMinMagFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
		noise[i].setTexParameterWrap(GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, an);

		noise[i].giveName("noise_tex[" 
			+ std::to_string(i) + "]");
		quad.getMeshes().at(0)->setTexture(&noise[i]);
	}
	glm::vec4* fragPosArray = (glm::vec4*)malloc(sizeof(glm::vec4) * render_resolution.x * render_resolution.y);
	if (!fragPosArray) {
		std::cerr << "Malloc Error" << std::endl;
	}
	GLsync block = 0;
	deferred_shading_floats.clear();
	deferred_shading_floats = {
		{ "extent", 1 },
		{ "frequency", .973 },
		{ "linear_term", 1 },
		{ "thickness", .018 },
		{ "u_stretch", 3.654 },
		{ "v_stretch", 3.654 },
		{ "delta_theta", 180.f / wifi.getActiveFreqs(freqs).size() },
		{ "distance_mask", 0 },
		{ "alpha_boost", 20 },
		{ "density", .025 },
		{ "frag_pos_scale", 150},
		{ "cling", .9},
		{ "tunable", .005}
	};
	lic_shading_floats = {
		{ "alpha_boost", 20 },
		{ "density", .025 },
		{ "frag_pos_scale", 150},
		{ "learning_rate", .134 * scaling_factor},
	};
	std::map<std::string, bool> deferred_shading_bools = {
		{ "contour_on", false},
		{ "display_names", false },
		{ "lic_on", true },
		{ "group_frequencies", false },
		{ "texton_background", false },
		{ "invert_colors", false },
		{ "frequency_bands", false },
		{ "shade_instances", false},
		{ "anti_aliasing", true},
		{ "use_mask", true},
		{ "screen_space_lic", true},
		{ "cull_discontinuities", true},
		{ "multirouter", true},
		{ "color_weaving", true},
		{ "blending", true},
		{ "render_textons", false}
	}, lic_shading_bools = {
		{ "screen_space_lic", true},
		{ "procedural_noise", false},
		{ "cull_discontinuities", true},
		{ "use_mask", true}, 
		{ "multirouter", true},
		{ "last_pass", false}
	};
	deferred_shading_ints = {
		{"num_point_lights", 20},
		{"num_frequencies", wifi.getActiveFreqs(freqs).size()},
		{"num_contours", 6},
		{"power", 1}
	}, lic_shading_ints = {
		{ "num_ellpsoids", num_routers},
		{ "power", 1}
	};

	std::map<std::string, float> ssao_shading_floats = {
		{"radius", .05 },
		{"bias", .025}
	};
	
	int kernelSize = 6;
	bool ssao_on = false;
	glBindFramebuffer(GL_FRAMEBUFFER, buffer[0].ssao_blur_framebuffer);
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0, 0, 0, 0);
	const int num_antialiased_textures = 23;
	Texture2D antialiased_textures[num_antialiased_textures] = {
		frequency_texture,
		//buffer[0].lic_texture[0], buffer[0].lic_texture[0],
		noise[0]};
	bool num_samples_changed = false, num_routers_changed = false,
		render_gui = true;
	std::vector<std::string> analytics_text;
	bool trigger_nearest_router = false, update_config = false;

	static int config_ind = -1;
	const int num_configs = 7;
	if (use_vr) {
		vr.left_hand->counter_min.x = 0;
		vr.left_hand->counter_max.x = num_configs - 1;
	}
	
	while (!glfwWindowShouldClose(w.getWindow())) {
		if (w.sleeping) {
			glfwPollEvents();
			continue;
		}
		if (w.getResized()) {
			render_resolution.x = w.width;
			render_resolution.y = w.height;
			createFramebuffers(render_resolution, buffer, eyes, use_vr, true);
			w.setResized(false);
		}
		//clear default framebuffer
		if (use_vr) {
			vr.SaveControllerIDs();
			vr.updateHMDPoseMatrix();
		}
		for (int i = 0; i < num_fb; i++) {
			if (i == 0) {
				curr_eye = vr::Hmd_Eye::Eye_Left;
			}
			else {
				curr_eye = vr::Hmd_Eye::Eye_Right;
			}
			ProjectionMat = camera.getProjection();
			ViewMat = camera.getView();
			if (use_vr) {
				ProjectionMat = vr.getProjectionMatrix(curr_eye);
				ViewMat = vr.getViewMatrix(curr_eye) * camera_offset * ViewMat;

			}
			glBindFramebuffer(GL_FRAMEBUFFER, eyes[i].frame_buffer);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			//render skybox
			skybox_shader.Use();
			skybox_shader.SetUniform("projection", ProjectionMat);
			skybox_shader.SetUniform("view", glm::mat4(glm::mat3(ViewMat)));

			glDepthMask(GL_FALSE);
			//TODO: Check if outside before rendering skybox?
			render(skybox, &skybox_shader);
			glDepthMask(GL_TRUE);

			

			//render building model
			glBindFramebuffer(GL_FRAMEBUFFER, buffer[0].frame_buffer);

			if (i == 0 && w.signal && !polling_pbo) {
				glReadBuffer(GL_COLOR_ATTACHMENT2);
				glBindBuffer(GL_PIXEL_PACK_BUFFER, buffer->pboIDs[odd_or_even_frame]);
				glReadPixels(0, 0, render_resolution.x, render_resolution.y, GL_BGRA, GL_FLOAT, 0);
				block = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
				//odd_or_even_frame = (odd_or_even_frame + 1) % 2;
				glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
				polling_pbo = true;
			}
			else if (i == 0 && polling_pbo && glClientWaitSync(block, GL_SYNC_FLUSH_COMMANDS_BIT, 0)) {
				glBindBuffer(GL_PIXEL_PACK_BUFFER, buffer->pboIDs[int(odd_or_even_frame)]);
				fragPosArray = (glm::vec4*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);

				//fragPosArray = (glm::vec4*)src;

				if (fragPosArray) {
					glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
				}
				polling_pbo = false;
				w.signal = false;
				send_data = true;
			}

			

			//Model Prepass
			glBindFramebuffer(GL_READ_FRAMEBUFFER, eyes[i].frame_buffer);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			model_shader.Use();
			model_shader.SetUniform("model", avw_transform);
			model_shader.SetUniform("normalMatrix", glm::mat3(glm::transpose(glm::inverse(avw_transform))));
			model_shader.SetUniform("camera", ViewMat);
			model_shader.SetUniform("projection", ProjectionMat);
			Lights modelLights = setPointLights(totalLights, constant, linear, quadratic);
			model_shader.SetUniform("viewPos", camera.getPosition());
			model_shader.SetUniform("distance_mask", deferred_shading_floats["distance_mask"]);
			model_shader.SetUniform("texcoord_scale", 1.f);
			//glDepthMask(GL_FALSE);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			AVW.Render(&model_shader, avw_transform, ViewMat, ProjectionMat);
			//render(AVW, &model_shader);
			//render ground plane
			glDisable(GL_CULL_FACE);
			ground_shader.Use();
			ground_shader.SetUniform("projection", ProjectionMat);
			ground_shader.SetUniform("camera", ViewMat);
			ground_shader.SetUniform("model", ground_transform);
			ground_shader.SetUniform("heatmap", 0);
			ground_shader.SetUniform("flip_texture", false);
			render(ground_quad, &ground_shader);
			
			//vr widget prepass if requested (and in vr)
			if (use_vr && vr.quad_transform != glm::mat4(1)) {
				glm::mat4 transform = 
					vr.quad_transform *
					glm::scale(
						glm::rotate(
							glm::mat4(1),
							glm::radians(90.0f),
							glm::vec3(1, 0, 0)),
						glm::vec3(.12 * scaling_factor))
					;;
				//vr.getViewMatrix(curr_eye) * camera_offset * ViewMat
				model_shader.SetUniform("model", transform);
				model_shader.SetUniform("camera", ViewMat);
				model_shader.SetUniform("normalMatrix", glm::mat3(glm::transpose(glm::inverse(transform))));
				model_shader.SetUniform("texcoord_scale", texcoord_scale);
				quad.getMeshes().at(0)->setTexture(&widget_background, 0);
				render(quad, &model_shader);
				quad.getMeshes().at(0)->setTexture(&white, 0);
			}
			//glDepthMask(GL_TRUE);
			//glClear(GL_DEPTH_BUFFER_BIT);
			
			
			glDisable(GL_DEPTH_TEST);
			
			//Do SSAO prepasses if desired
			if (ssao_on) {
				glBindFramebuffer(GL_FRAMEBUFFER, buffer[0].ssao_framebuffer);
				glClear(GL_COLOR_BUFFER_BIT);
				ssao_shader.Use();
				ssao_shader.SetUniform("kernelSize", kernelSize);
				ssao_shader.SetUniforms(ssao_shading_floats);

				ssao_shader.SetUniform("ViewMat", (ViewMat));
				ssao_shader.SetUniform("projection", ProjectionMat);
				ssao_shader.SetUniform("noiseScale", glm::vec2(render_resolution) / 4.0f);
				render(quad, &ssao_shader);
				glBindFramebuffer(GL_FRAMEBUFFER, buffer[0].ssao_blur_framebuffer);
				glClear(GL_COLOR_BUFFER_BIT);
				ssao_blur_shader.Use();
				render(quad, &ssao_blur_shader);
			}
			//Do LIC precomputation if required
			if (deferred_shading_bools["lic_on"] && start_render) {
				int num_forces = num_routers;
				int num_iters = 3;
				if (lic_shading_bools["multirouter"])
					num_forces = 1;
				if (lic_shading_bools["max_lic"]) {
					num_forces = 1;
				}
				float masks[3] = {0,0,0};
				if (true) {
					float running_total = 0;
					for (int i = 1; i < 3; i++) {
						running_total += getTotalMaskForStep(i - 1, lic_shading_bools["use_mask"]);
						masks[i] = running_total;
					}
				}
				for (int i = 0; i < num_forces; i++) {
					glClearColor(0, 0, 0, 0);
					force_shader.Use();
					glBindFramebuffer(GL_FRAMEBUFFER, buffer[0].force_framebuffer);
					glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

					force_shader.SetUniform("ellipsoid_transform", wifi.ellipsoid_transform);
					force_shader.SetUniform("radius_stretch", wifi.radius_stretch);
					force_shader.SetUniform("ellipsoid_index_offset", i);
					force_shader.SetUniform("num_routers", num_routers);
					force_shader.SetUniform("num_ellipsoids", num_routers / num_forces);
					force_shader.SetUniform("extent", deferred_shading_floats["extent"]);
					force_shader.SetUniform("invert_colors", deferred_shading_bools["invert_colors"]);
					force_shader.SetUniform("max_lic", lic_shading_bools["max_lic"]);
					render(quad, &force_shader);
					glClearColor(1, 1, 1, 0);
					//glDisable(GL_BLEND);
					
					for (int j = 0; j < num_iters; j++) {
						lic_shader.Use();
						lic_shading_bools["last_pass"] = (j == (num_iters - 1));
						lic_shader.SetUniforms(lic_shading_floats);
						lic_shader.SetUniforms(lic_shading_ints);
						lic_shader.SetUniforms(lic_shading_bools);
						lic_shader.SetUniform("camera", ViewMat);
						lic_shader.SetUniform("projection", ProjectionMat);
						//if(wifi.getNumWifiNames() + i < wifi_colors.size())
							//lic_shader.SetUniform("color", glm::vec3(wifi_colors.at(wifi.getNumWifiNames() + i)));
						lic_shader.SetUniform("ellipsoid_index_offset", i);
						lic_shader.SetUniform("router_num", i);
						lic_shader.SetUniform("step_num", j);
						lic_shader.SetUniform("curr_mask", masks[j]);
						//lic_shader.SetUniform("multirouter", lic_shading_bools["multirouter"]);

						//glClearColor(1, 1, 1, 0);
						glBindFramebuffer(GL_FRAMEBUFFER, buffer[0].lic_framebuffer[j % 2]);
						glClear(GL_COLOR_BUFFER_BIT);

						render(quad, &lic_shader);
						//if (j == 0) {
						//	noise[i].giveName("temp_name");
						//}
						//buffer[0].lic_texture.giveName("noise_tex[" + std::to_string(i) + "]");
					}
					//glEnable(GL_BLEND);
					//noise[i].giveName("noise_tex[" + std::to_string(i) + "]");
					//buffer[0].lic_texture.giveName("lic_tex");
					//glEnable(GL_BLEND);
					//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
					glClearColor(1, 1, 1, 0);
					if (i == 0) {
						glBindFramebuffer(GL_FRAMEBUFFER, buffer[0].lic_accum_framebuffer);
						glClear(GL_COLOR_BUFFER_BIT);
					}

					glBindFramebuffer(GL_FRAMEBUFFER, buffer[0].lic_accum_framebuffer);

					//glClear(GL_COLOR_BUFFER_BIT);

					lic_accum_shader.Use();
					lic_accum_shader.SetUniform("camera", ViewMat);
					lic_accum_shader.SetUniform("projection", ProjectionMat);
					lic_accum_shader.SetUniform("lic_index", (num_iters + 1) % 2);
					lic_accum_shader.SetUniform("force_index", 0);
					render(quad, &lic_accum_shader);
				}
				deferred_shader.SetUniform("force_index", 0);
				deferred_shader.SetUniform("num_lic", num_iters % 2);

			}
			glClearColor(0, 0, 0, 0);
			//Do deferred Rendering with all the needed precomputed textures
			deferred_shader.Use();
			deferred_shader.SetEllipsoid(ellipsoid);
			deferred_shader.SetUniforms(deferred_shading_floats);
			deferred_shader.SetUniforms(deferred_shading_ints);
			deferred_shader.SetUniforms(deferred_shading_bools);
			deferred_shader.SetUniform("camera", ViewMat);
			deferred_shader.SetUniform("projection", ProjectionMat);
			deferred_shader.SetUniform("alpha_boost", lic_shading_floats["alpha_boost"]);
			deferred_shader.SetUniform("power", lic_shading_ints["power"]);
			
			wifi_transform = glm::translate(glm::mat4(1), wifi_translate);
			wifi.ellipsoid_transform = glm::scale(glm::mat4(1), scaling_factor * glm::vec3(1)) * wifi_transform * glm::scale(glm::mat4(1), wifi_scale);
			deferred_shader.SetUniform("ellipsoid_transform", wifi.ellipsoid_transform);
			wifi.radius_stretch = scaling_factor * scaling_factor * wifi_scale;
			

			deferred_shader.SetUniform("radius_stretch", wifi.radius_stretch);
			deferred_shader.SetUniform("viewPos", camera.getPosition());
			
			glm::vec3 selectedPos;
			//If requested, find selected position, set it, and set popup render text
			if (send_data) {
				glm::vec2 index = glm::ivec2(w.currX, (render_resolution.y - w.currY));
				selectedPos = fragPosArray[int(index.y * render_resolution.x + index.x)];
				selectedPos = glm::vec3(selectedPos.b, selectedPos.g, selectedPos.r);
				deferred_shader.SetUniform("selectedPos", selectedPos);
				renderPopup = wifi.getNumActiveRouters(routers) > 0;
				wifi.setRenderText(popup_text, selectedPos, routers);
				//std::cout << glm::to_string(selectedPos) << std::endl;
				send_data = false;
				for (int i = 0; i < popup_text.size(); i++) {
					popupText.RenderText(glm::uvec2(0, font_height * popup_text.size() - (i + 1) * font_height),
						glm::uvec2(2560, popup_text.size() * font_height),
						popup_text.at(i), i == 0);
				}
				
				Texture2D text(popupText.tex, popupText.height, popupText.width);
				text.giveName("quad_texture");
				text.setTexMinMagFilter(GL_LINEAR, GL_LINEAR);
				quad.getMeshes().at(0)->setTexture(&text);
				//std::cout << glm::to_string(fragPosArray[int(index.y * resolution.x + index.x)]) << std::endl;
			}
			deferred_shader.SetLights(modelLights, camera.getPosition() + vr.adjusted_height, deferred_shading_ints["num_point_lights"], scaling_factor);

			//update minimap
			glViewport(0, 0, minimap_base[0].getDims().x, minimap_base[0].getDims().y);
			glBindFramebuffer(GL_FRAMEBUFFER, minimap_buffer.framebuffer);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			currFloor = minimap_shader.getFloor(camera.getPosition(), scaling_factor);
			if(oldFloor != currFloor) {
				minimap_base[oldFloor].name = "texture_diffuse";
				minimap_base[currFloor].name = "minimap_base_tex";
				
				oldFloor = currFloor;
			}

			minimap_shader.SetUniform("playerPos", camera.getPosition());
			minimap_shader.SetUniform("playerRadius", .02f);
			minimap_shader.Use();
			render(quad, &minimap_shader);
			glViewport(0, 0, render_resolution.x, render_resolution.y);
			glBindFramebuffer(GL_FRAMEBUFFER, eyes[i].frame_buffer);
			//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			deferred_shader.Use();
			glDepthMask(GL_FALSE);
 			render(quad, &deferred_shader);
			glDepthMask(GL_TRUE);
			glEnable(GL_DEPTH_TEST);

			

			//update noise texture if necessary
			if (num_samples_changed || num_routers_changed) {
				updateNoise(lic, num_samples, num_routers, noise, antialiased_textures, lic_texture_res, an);
				for (int i = 0; i < num_routers; i++) {
				//	quad.getMeshes().at(0)->setTexture(&noise[i]);
				}
				num_samples_changed = false;
				num_routers_changed = false;
			}
			
			//Render wifi popup if necessary
			if (renderPopup) {
				glm::mat4 quad_transform(1);
				quad.getMeshes().at(0)->setTexture(&popup_background, 0);

				glm::vec3 pos = glm::lerp(selectedPos, camera.getPosition(), .1f);
				quad_transform = glm::translate(quad_transform, pos);
				quad_shader.SetUniform("camera", ViewMat);
				quad_shader.SetUniform("projection", ProjectionMat);
				quad_shader.SetUniform("quad_center", pos);
				quad_shader.SetUniform("billboardSize", glm::vec2(popupText.width/(float)render_resolution.x, popupText.height/(float)render_resolution.y) * billboard_scale);
				quad_shader.SetUniform("num_routers", wifi.getNumActiveRouters(routers));
				quad_shader.SetUniform("model", quad_transform);
				render(quad, &quad_shader);
			}

			//Update the router structure if needed (preserver colors)
			if (updated_routers) {
				wifi.updateRouterStructure(routers, wifinames, freqs, router_shaders, 2, camera.getPosition(), nearest_router_on);
				updated_routers = false;
				if (nearest_router_on) {
					std::vector<glm::vec4> new_wifi_colors(wifi.getNumActiveRouters(routers));
					std::vector<glm::vec4> new_wifi_colors_rearranged(wifi.getNumActiveRouters(routers));
					std::vector<int> vec_indices(wifi.getNumActiveRouters(routers));
					std::vector<int> unused_indices(wifi.getNumActiveRouters(routers));
					for (int i = 0; i < unused_indices.size(); i++) { unused_indices[i] = i; }
					maxSeperatedColors(wifi.getNumActiveRouters(routers), new_wifi_colors, jittered, color_offset);
					std::vector<std::string> available_macs = wifi.getAvailablesMacs();
					//Remember colors between updates
					if (saved_colors.size() == num_routers) {
						int count = 0;
						int used_router_num = 0;
						std::vector<std::string> router_names(saved_colors.size());
						for (auto router : routers) {
							if (router) {
								router_names[used_router_num] = available_macs[count];
								auto map_location = saved_colors.find(router_names[used_router_num]);
								if (map_location != saved_colors.end()) {
									vec_indices[used_router_num] = std::distance(saved_colors.begin(), map_location);
									auto location = std::find(unused_indices.begin(), unused_indices.end(), vec_indices[used_router_num]);
									unused_indices.erase(location);
								}
								else {
									vec_indices[used_router_num] = -1;
								}
								used_router_num++;
								count++;
							}
							
						}
						
						std::vector<std::string>old_names;
						for (auto name2color : saved_colors)
							old_names.push_back(name2color.first);
						for (int i = 0; i < vec_indices.size(); i++) {
							if (vec_indices[i] == -1) { vec_indices[i] = unused_indices.back(); unused_indices.pop_back(); }
							new_wifi_colors_rearranged[i] = saved_colors[old_names[vec_indices[i]]];
						}
						
					}
					else {
						new_wifi_colors_rearranged = new_wifi_colors;
					}
					saved_colors.clear();
					int i = 0, count = 0;
					for (auto router : routers) {
						if (router) {
							saved_colors[available_macs[count]] = new_wifi_colors_rearranged[i++];
							count++;
						}
						
					}
					wifi_colors.resize(wifi.getNumActiveRouters(routers) + wifi.getNumWifiNames());
					for (int i = 0; i < new_wifi_colors.size(); i++) {
						wifi_colors[wifi.getNumWifiNames() + i] = new_wifi_colors_rearranged[i];
					}
					wifi_tex.updateTexture(&wifi_colors, wifi.getNumActiveRouters(routers) + wifi.getNumWifiNames(), 1);
					auto router_strings = wifi.getRouterStrings();
					for (int i = 0; i < router_strings.size(); i++) {
						tr.RenderText(glm::uvec2(0, font_height * router_strings.size() - (i + 1) * font_height), glm::uvec2(2560, router_strings.size() * font_height), router_strings[router_strings.size() - i - 1], i==0);
					}

					Texture2D *text = new Texture2D(tr.tex, tr.height, tr.width);
					text->giveName("text_tex");
					quad.getMeshes().at(0)->setTexture(text);
				}
			}
			//render wifi samples if requested
			if (deferred_shading_bools["shade_instances"]) {
				
				instance_shader.Use();
				wifi_transforms.clear();
				if (deferred_shading_bools["shade_instances"])
					wifi_transforms = wifi.getTransforms(wifinames, routers, wifi_scale);

				std::vector<float> wifi_color_indices = wifi.getColorIndices();
				if (deferred_shading_bools["shade_instances"])
					Sphere.getMeshes().at(0)->SetInstanceTransforms(wifi_transforms, wifi_color_indices);
				instance_shader.SetUniform("projection", ProjectionMat);
				instance_shader.SetUniform("view", ViewMat);
				//wifi_transform = glm::scale(glm::mat4(1), wifi_scale);

				instance_shader.SetUniform("transform", wifi_transform);
				instance_shader.SetUniform("model_transform", glyph_transform);
				instance_shader.SetUniform("normalMat", glm::mat3(1));
				instance_shader.SetLights(lights);
				instance_shader.SetUniform("viewPos", camera.getPosition());

				glBindFramebuffer(GL_FRAMEBUFFER, eyes[i].frame_buffer);
				if (wifi_transforms.size() > 0)
					render(Sphere, &instance_shader);
			}
			
			//set analytics text if requested
			if (show_analytics) {
				analytics_text.clear();
				analytics_text.emplace_back(std::to_string(wifi.getNumRoutersWithSignalFromSet(camera.getPosition(), deferred_shading_floats["extent"])) + " Displayed routers with signal strength");
				analytics_text.emplace_back(std::to_string(wifi.getNumRoutersWithSignal(camera.getPosition(), deferred_shading_floats["extent"])) + ": Number of routers with signal strength");
				if (wifi.routers.size() > 0)
					analytics_text.emplace_back("Interference: " + wifi.getInterferenceString());
				analytics_text.emplace_back(getLocationString(camera.getPosition()));
			}
			
			//If in VR, render controllers and laser
			if (use_vr) {
				ground_shader.Use();
				ground_shader.SetUniform("camera", vr.getViewMatrix(curr_eye));
				glm::mat4 controller_rotation = glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::mat4(1), glm::radians(60.0f), glm::vec3(1, 0, 0));
				glm::mat4 left_hand_transform = vr.getControllerPose(vr.LeftDeviceId) * controller_rotation * glm::scale(glm::mat4(1), 1.0f * glm::vec3(-1, -1, 1));
				

				if (show_analytics) {
					analyticsText.RenderText(glm::uvec2(0,0), glm::uvec2(2560, 1440), analytics_text);
				}
				glm::mat4 right_hand_transform = vr.getControllerPose(vr.RightDeviceId) * controller_rotation * glm::scale(glm::mat4(1), 1.0f * glm::vec3(-1, -1, 1));
				
				
				if (i == 0) {
					
					static bool render_quad = false;
					if (vr.left_hand->grip) {
						render_quad = !render_quad;
						vr.quad_transform = glm::mat4();
					}
					if (vr.left_hand->grip_pressed) {
						//glm::decompose(vr.getControllerPose(vr.LeftDeviceId), scale, orientation, position, skew, perspective);
						
						if (render_quad) {
							auto inv_view = glm::inverse(camera_offset * camera.getView());
							vr.quad_transform = inv_view * vr.getControllerPose(vr.LeftDeviceId);
						}
					}
					
					if (vr.right_hand->a) {
						camera.setPosition(camera.getPosition() + glm::vec3(0, 0, -scaling_factor));
					}if (vr.right_hand->b) {
						camera.setPosition(camera.getPosition() + glm::vec3(0, 0, scaling_factor));
					}
				}

				if (vr.right_hand->grip_pressed) {
					glm::mat4 transform = glm::inverse(camera_offset * camera.getView()) *
						vr.getControllerPose(vr.RightDeviceId) *
						glm::scale(
							glm::rotate(
								glm::mat4(1),
								glm::radians(90.0f),
								glm::vec3(1, 0, 0)),
							glm::vec3(minimap_buffer.minimapTexture.getAspectRatio(),1,1) * .2f);
					//vr.getViewMatrix(curr_eye) * camera_offset * ViewMat
					ground_shader.SetUniform("model", transform);
					ground_shader.SetUniform("camera", ViewMat);
					ground_shader.SetUniform("flip_texture", false);
					//ground_shader.SetUniform("normalMatrix", glm::mat3(glm::transpose(glm::inverse(transform))));
					//model_shader.SetUniform("texcoord_scale", texcoord_scale);
					//quad.getMeshes().at(0)->setTexture(&minimap_buffer.minimapTexture, 0);
					std::vector<Texture2D*> mmap;
					mmap.emplace_back(&minimap_buffer.minimapTexture);
					quad.Render(&ground_shader, mmap);
				}if (!vr.left_hand->trigger && vr.right_hand->trigger) {
					//glm::vec3 forward = vr.getControllerPose(vr.RightDeviceId) *controller_rotation* glm::vec4(-1, 0, 0,0);
					glm::vec3 forward, pos2;
					glm::quat  orientation;
					glm::vec3 skew, scale;
					glm::vec4 perspective;
					glm::vec3 position;

					glm::decompose(vr.getControllerPose(vr.RightDeviceId), scale, orientation, position, skew, perspective);
					auto inv_view = glm::inverse(camera_offset * camera.getView());
					position = glm::vec3(inv_view * glm::vec4(position, 1));

					forward = glm::normalize(glm::vec3(inv_view * vr.getControllerPose(vr.RightDeviceId) * glm::rotate(glm::mat4(1), glm::radians(60.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::mat4(1), glm::radians(120.0f), glm::vec3(1, 0, 0)) * glm::vec4(0, 0, -1, 0)));
					rayPicker(forward, position, glm::vec3(0), vr.teleport_position, minimap_shader, scaling_factor);

					deferred_shader.SetUniform("selectedPos", glm::vec3(vr.teleport_position.r, vr.teleport_position.g, vr.teleport_position.b));
					glm::mat4 cylinder_mat = right_hand_transform *
						glm::rotate(glm::mat4(1),
							glm::radians(-120.0f), glm::vec3(1, 0, 0)) *
						glm::scale(glm::mat4(1), glm::vec3(-.2, -100, -.2));

					std::vector<Texture2D*> cyl;
					cyl.emplace_back(&white);
					ground_shader.SetUniform("model", cylinder_mat * glm::scale(glm::mat4(1), .01f * glm::vec3(1, 1, 1)));

					Cylinder.Render(&ground_shader, cyl);
				}
				else {
					ground_shader.SetUniform("model", right_hand_transform * glm::scale(glm::mat4(1), .01f * glm::vec3(1, 1, 1)));
					ground_shader.SetUniform("flip_texture", false);
					render(RightHand, &ground_shader);
				}
				
				if (vr.left_hand->trigger && !vr.right_hand->trigger) {
					//glm::vec3 forward = vr.getControllerPose(vr.RightDeviceId) *controller_rotation* glm::vec4(-1, 0, 0,0);
					glm::vec3 forward, pos2;
					glm::quat  orientation;
					glm::vec3 skew, scale;
					glm::vec4 perspective;
					glm::vec3 position;

					glm::decompose(vr.getControllerPose(vr.LeftDeviceId), scale, orientation, position, skew, perspective);
					auto inv_view = glm::inverse(camera_offset * camera.getView());
					position = glm::vec3(inv_view * glm::vec4(position, 1));

					forward = glm::normalize(glm::vec3(inv_view * vr.getControllerPose(vr.LeftDeviceId) * glm::rotate(glm::mat4(1), glm::radians(60.0f), glm::vec3(1, 0, 0)) * glm::rotate(glm::mat4(1), glm::radians(120.0f), glm::vec3(1, 0, 0)) * glm::vec4(0, 0, -1, 0)));
					rayPicker(forward, position, glm::vec3(0), vr.teleport_position, minimap_shader, scaling_factor);

					deferred_shader.SetUniform("selectedPos", glm::vec3(vr.teleport_position.r, vr.teleport_position.g, vr.teleport_position.b));
					glm::mat4 cylinder_mat = left_hand_transform *
						glm::rotate(glm::mat4(1),
							glm::radians(-120.0f), glm::vec3(1, 0, 0)) *
						glm::scale(glm::mat4(1), glm::vec3(-.2, -100, -.2));

					std::vector<Texture2D*> cyl;
					cyl.emplace_back(&white);
					ground_shader.SetUniform("model", cylinder_mat * glm::scale(glm::mat4(1), .01f * glm::vec3(1, 1, 1)));

					Cylinder.Render(&ground_shader, cyl);
				}
				else {
					ground_shader.SetUniform("model", left_hand_transform * glm::scale(glm::mat4(1), .01f * glm::vec3(1, 1, 1)));
					ground_shader.SetUniform("flip_texture", false);
					render(LeftHand, &ground_shader);
				}
				if (vr.right_hand->trigger == vr.left_hand->trigger) {
					if(!vr.right_hand->trigger && vr.teleport_position != glm::vec3())
						camera.setPosition(glm::vec3(vr.teleport_position.x, vr.teleport_position.y, camera.getPosition().z));

					vr.teleport_position = glm::vec3();
					deferred_shader.SetUniform("selectedPos", glm::vec3(vr.teleport_position.r, vr.teleport_position.g, vr.teleport_position.b));

				}
				
				vr.composite(curr_eye, eyes[i].screen_tex);

				
			}
		}
		glFlush(); 
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, screen_size.x, screen_size.y);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		render2quad.Use();
		render(quad, &render2quad);
		
		if (!demo_mode && vr.left_hand->a) {
			startNearestRouters(wifinames, routers, freqs,
				camera, wifi, num_routers, deferred_shading_floats,
				start_render, num_routers_changed, updated_routers);
		}
		
		//Render ImGUI
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		
		if (render_gui) {
			

			ImGui::Begin("Gui Selector");
			ImGui::SetWindowFontScale(1.5);
			if(ImGui::Button("Debug")) {
				gui_type = "debug";
			}
			ImGui::SameLine();
			if (ImGui::Button("Editor")) {
				gui_type = "editor";
			}
			ImGui::SameLine();
			if (ImGui::Button("Demo")) {
				gui_type = "demo";
			}
			ImGui::End();
			

			if (gui_type == "debug") {
				ImGui::Begin("Rendering Terms");
				ImGui::SetWindowFontScale(1.5);

				if (ImGui::SliderInt("Number of Routers", &num_routers, 1, 20)) {
					if (!demo_mode && use_vr) {
						vr.right_hand->joystick_counter.x = num_routers;
					}
				}
				if (ImGui::Button("Nearest Routers")) {
					startNearestRouters(wifinames, routers, freqs,
						camera, wifi, num_routers, deferred_shading_floats,
						start_render, num_routers_changed, updated_routers);
				}
				if (ImGui::Button("Reset")) {
					reset(wifi, wifinames, freqs, routers);
				}if (ImGui::TreeNode("Lighting Settings")) {
					ImGui::SliderInt("Total Lights", &totalLights, 1, 200);
					ImGui::SliderInt("Lights Shown", &deferred_shading_ints["num_point_lights"], 1, totalLights);
					ImGui::SliderFloat("constant", &constant, 0, 1);
					ImGui::SliderFloat("linear", &linear, 0, 1);
					ImGui::SliderFloat("quadratic", &quadratic, 0, 1);
					ImGui::TreePop();
				}if (ImGui::TreeNode("Texton Settings")) {
					ImGui::Checkbox("Render Textons", &deferred_shading_bools["render_textons"]);
					ImGui::Checkbox("texton_background", &deferred_shading_bools["texton_background"]);

					if (ImGui::Checkbox("Antialiasing", &deferred_shading_bools["anti_aliasing"])) {
						for (int i = 0; i < num_routers + 1; i++) {
							Texture2D texture = antialiased_textures[i];
							texture.Bind();
							if (deferred_shading_bools["anti_aliasing"]) {
								glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, an);
								texture.setTexMinMagFilter(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
							}
							else {
								glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY, 1.0f);
								texture.setTexMinMagFilter(GL_LINEAR, GL_LINEAR);
							}
						}
					}
					ImGui::SliderFloat("u stretch", &deferred_shading_floats["u_stretch"], 0, 10);
					ImGui::SliderFloat("v stretch", &deferred_shading_floats["v_stretch"], 0, 10);

					ImGui::TreePop();
				}
				//ImGui::SliderFloat("extent", &deferred_shading_floats["extent"], 0, 3);
				if(renderPopup)
					ImGui::SliderFloat("BillBoard Scale", &billboard_scale, 0, 1);
				ImGui::Checkbox("Shade Instances", &deferred_shading_bools["shade_instances"]);
				ImGui::Checkbox("Jittered Colors", &jittered);
				ImGui::Checkbox("Line Integral Convolution", &deferred_shading_bools["lic_on"]);
				if (deferred_shading_bools["lic_on"]) {
					ImGui::SliderFloat("Fragment Position Scale", &lic_shading_floats["frag_pos_scale"], 0, 300);
					ImGui::SliderFloat("Rate", &lic_shading_floats["learning_rate"], 0, .9);
					ImGui::Checkbox("Use LIC Mask", &lic_shading_bools["use_mask"]);
					ImGui::SliderFloat("Alpha_Boost", &lic_shading_floats["alpha_boost"], 1, 30);
					ImGui::SliderInt("Power", &lic_shading_ints["power"], 1, 8);
					ImGui::Checkbox("Screenspcace LIC", &lic_shading_bools["screen_space_lic"]);
					if (deferred_shading_bools["screen_space_lic"]) {
						ImGui::SliderFloat("Vector Threshold", &lic_shading_floats["tunable"], 0, 3);
						ImGui::Checkbox("Hug Walls", &lic_shading_bools["cull_discontinuities"]);
					}
					ImGui::Checkbox("Max LIC", &lic_shading_bools["max_lic"]);
					ImGui::Checkbox("Multirouter", &lic_shading_bools["multirouter"]);
					if (lic_shading_bools["multirouter"]) {
						ImGui::Checkbox("Use Color Weaving", &deferred_shading_bools["color_weaving"]);
						if (deferred_shading_bools["color_weaving"]) {
							ImGui::Checkbox("Use Blending", &deferred_shading_bools["blending"]);
						}
					}
					ImGui::Checkbox("Procedural Noise", &lic_shading_bools["procedural_noise"]);
					if (lic_shading_bools["procedural_noise"]) {
						ImGui::SliderFloat("Density", &lic_shading_floats["density"], 0, 1);
						if (!lic_shading_bools["screen_space_lic"])
							ImGui::SliderFloat("Cling Factor", &deferred_shading_floats["cling"], 0, 1);
					}
					else {
						num_samples_changed = ImGui::SliderInt("num_samples", &num_samples, 12800, 102400 * 16);
					}
					

				}
				else {
					ImGui::Checkbox("frequency_bands", &deferred_shading_bools["frequency_bands"]);
					ImGui::Checkbox("Display Names", &deferred_shading_bools["display_names"]);
					if (deferred_shading_bools["display_names"]) {
						ImGui::SliderInt("Number of Dashes", &deferred_shading_ints["num_contours"], 1, 20);
					}
					ImGui::SliderFloat("Contour Frequency", &deferred_shading_floats["frequency"], 0, 1);
					ImGui::SliderFloat("Linear Term", &deferred_shading_floats["linear_term"], 0, 1);
					ImGui::SliderFloat("thickness", &deferred_shading_floats["thickness"], 0, .1);
				}
				ImGui::Checkbox("Invert Color Representation", &deferred_shading_bools["invert_colors"]);
				ImGui::SliderFloat("distance mask", &deferred_shading_floats["distance_mask"], 0, 3);
				if (ImGui::SliderInt("Hue Start", &color_offset, 0, 360)) {
					std::vector<glm::vec4> new_colors(num_routers);

					maxSeperatedColors(num_routers, new_colors, jittered, color_offset);
					for (int i = 0; i < num_routers; i++)
						wifi_colors[i + wifi.getNumWifiNames()] = new_colors[i];
					wifi_tex.updateTexture(&wifi_colors, wifi.getNumActiveRouters(routers) + wifi.getNumWifiNames(), 1);
				}
				//ImGui::SliderFloat("Z Boost", &z_boost, 1, 10);
				if (ImGui::TreeNode("SSAO Terms")) {
					if (ImGui::Checkbox("SSAO Enabled", &ssao_on)) {
						if (!ssao_on) {
							glBindFramebuffer(GL_FRAMEBUFFER, buffer[0].ssao_blur_framebuffer);
							glClearColor(1, 1, 1, 1);
							glClear(GL_COLOR_BUFFER_BIT);
							glClearColor(0, 0, 0, 0);
							glBindFramebuffer(GL_FRAMEBUFFER, 0);
						}
					}
					if (ssao_on) {
						ImGui::SliderInt("Kernel Size", &kernelSize, 1, 64);
						ImGui::SliderFloat("Bias", &ssao_shading_floats["bias"], 0, 1);
						ImGui::SliderFloat("Radius", &ssao_shading_floats["radius"], 0, 3);
					}
					ImGui::TreePop();
				}

				if (ImGui::TreeNode("Wifi Names")) {
					for (int i = 0; i < wifinames.size(); i++) {
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(wifi_colors.at(i).r, wifi_colors.at(i).g, wifi_colors.at(i).b, wifi_colors.at(i).a));
						if (ImGui::Selectable(wifi.getWifiName(i).c_str(), wifinames.at(i)))
						{
							if (glfwGetKey(w.window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS) {    // Clear selection when CTRL is not held
								std::fill(wifinames.begin(), wifinames.end(), false);
							}
							wifinames.at(i) = !wifinames.at(i);
							updated_routers = true;
							nearest_router_on = false;
							if (!wifinames.at(i))
								wifi.fillRouters(wifi.getWifiName(i).c_str(), routers, false);
							else
								wifi.fillRouters(wifi.getWifiName(i).c_str(), routers, true);
							wifi.setAvailableFreqs(wifi.getSelectedNames(wifinames));
							std::fill(freqs.begin(), freqs.end(), true);
							wifi.setAvailableMacs(wifi.getSelectedNames(wifinames));

						}
						ImGui::PopStyleColor();
					}
					ImGui::TreePop();
				}if (ImGui::TreeNode("Frequencies")) {
					for (int i = 0; i < wifi.getAvailableFreqs().size(); i++) {
						if (ImGui::Selectable(std::to_string(wifi.getAvailableFreqs().at(i)).c_str(), freqs.at(i))) {
							if (glfwGetKey(w.window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS)    // Clear selection when CTRL is not held
								std::fill(freqs.begin(), freqs.end(), false);
							freqs.at(i) = !freqs.at(i);
							if (freqs.at(i)) {
								std::fill(routers.begin(), routers.end(), false);
							}
							updated_routers = true;
							nearest_router_on = false;
							wifi.setAvailableMacs(wifi.getSelectedNames(wifinames), wifi.getSelectedFreqs(freqs));
						}
					}
					ImGui::TreePop();
				}

				if (ImGui::TreeNode("Routers")) {

					for (int i = 0; i < wifi.getAvailablesMacs().size(); i++) {
						if (ImGui::Selectable(wifi.getAvailablesMacs().at(i).c_str(), routers.at(i)))
						{
							if (glfwGetKey(w.window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS)    // Clear selection when CTRL is not held
								std::fill(routers.begin(), routers.end(), false);
							routers.at(i) = !routers.at(i);
							updated_routers = true;
							nearest_router_on = false;
						}
					}
					ImGui::TreePop();
				}
				if(!use_vr)
					ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
				ImGui::End();
			}
			if (nearest_router_on && render_gui) {
				ImGui::Begin("Nearest Routers");
				ImGui::SetWindowFontScale(1.5);

				for (int i = 0; i < wifi.getRouterStrings().size(); i++) {
					glm::vec4 color = wifi_colors[i + wifi.getNumWifiNames()];
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(color.r, color.g, color.b, 1));
					ImGui::Text(wifi.getRouterStrings().at(i).c_str());
					ImGui::PopStyleColor();
				}
				ImGui::End();
			}
			if (!use_vr && show_analytics) {
				ImGui::Begin("Analytics");
				ImGui::SetWindowFontScale(1.5);

				for (auto& text : analytics_text)
					ImGui::Text(text.c_str());
				ImGui::End();
			}
			if (gui_type == "editor") {
				ImGui::Begin("Selector");
				ImGui::SetWindowFontScale(1.5);

				if (ImGui::Button("Display Routers")) {
					start_render = true;
					num_routers_changed = true;
					//std::fill(freqs.begin(), freqs.end(), true);
					//wifi.setRouters(wifinames, routers, freqs);
					num_routers = wifi.getNumActiveRouters(routers);
					deferred_shading_floats["delta_theta"] = 180.f / wifi.getActiveFreqs(freqs).size();
					updated_routers = true;
					nearest_router_on = true;
				}
				char filename[100];
				ImGui::InputText("Filename", filename, 100);
				if (ImGui::Button("Save Set")) {
					//wifi.deactivateExtra(routers, wifinames, freqs);
					std::ofstream outfile(std::string("Content/RouterSets/") + filename, std::ios::out);
					wifi.writeRouters(outfile);
					outfile.close();
				}if (ImGui::Button("Load Set")) {
					std::ifstream infile(filename, std::ios::in);
					reset(wifi, wifinames, freqs, routers);
					wifi.readRouters(infile, wifinames, routers, freqs);
					start_render = true;
					num_routers = wifi.getNumActiveRouters(routers);
					//wifi.setAvailableMacs(wifi.getSelectedNames(wifinames));

					deferred_shading_floats["delta_theta"] = 180.f / wifi.getActiveFreqs(freqs).size();
					updated_routers = true;
					nearest_router_on = true;
					num_routers_changed = true;
					infile.close();
				}
				
				if (ImGui::Button("Save Configuration")) {
					std::ofstream outfile(std::string("Content/Config/") + filename, std::ios::out);
					lic_shading_floats["learning_rate"] /= scaling_factor;
					writeDictionary(deferred_shading_bools, outfile);
					writeDictionary(deferred_shading_floats, outfile);
					writeDictionary(deferred_shading_ints, outfile);
					
					writeDictionary(lic_shading_bools, outfile);
					writeDictionary(lic_shading_floats, outfile);
					writeDictionary(lic_shading_ints, outfile);

					writeDictionary(ssao_shading_floats, outfile);

					outfile << num_samples;
					

					outfile.close();
				}if (ImGui::Button("Load Configuration")) {
					std::ifstream infile(std::string("Content/Config/") + filename, std::ios::in);
					if (!infile.is_open())
						std::cout << "File not found";
					else {
						readDictionary(deferred_shading_bools, infile);
						readDictionary(deferred_shading_floats, infile);
						readDictionary(deferred_shading_ints, infile);

						readDictionary(lic_shading_bools, infile);
						readDictionary(lic_shading_floats, infile);
						readDictionary(lic_shading_ints, infile);

						readDictionary(ssao_shading_floats, infile);
						lic_shading_floats["learning_rate"] *= scaling_factor;
						infile >> std::ws;
						old_num_samples = num_samples;
						infile >> num_samples;
						if (old_num_samples != num_samples) {
							num_samples_changed = true;
						}
					}
					infile.close();
				}
				if (ImGui::TreeNode("Wifi Names")) {
					for (int i = 0; i < wifinames.size(); i++) {
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(wifi_colors.at(i).r, wifi_colors.at(i).g, wifi_colors.at(i).b, wifi_colors.at(i).a));
						
						if (ImGui::Selectable(wifi.getWifiName(i).c_str(), wifinames.at(i)))
						{
							if (glfwGetKey(w.window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS) {    // Clear selection when CTRL is not held
								std::fill(wifinames.begin(), wifinames.end(), false);
							}
							wifinames.at(i) = !wifinames.at(i);
							updated_routers = true;
							nearest_router_on = false;
							if (!wifinames.at(i))
								wifi.fillRouters(wifi.getWifiName(i).c_str(), routers, false);
							else
								wifi.fillRouters(wifi.getWifiName(i).c_str(), routers, true);
							wifi.setAvailableFreqs(wifi.getSelectedNames(wifinames));
							std::fill(freqs.begin(), freqs.end(), true);
							wifi.setAvailableMacs(wifi.getSelectedNames(wifinames));

						}
						ImGui::PopStyleColor();
					}
					ImGui::TreePop();
				}if (ImGui::TreeNode("Frequencies")) {
					for (int i = 0; i < wifi.getAvailableFreqs().size(); i++) {
						if (ImGui::Selectable(std::to_string(wifi.getAvailableFreqs().at(i)).c_str(), freqs.at(i))) {
							if (glfwGetKey(w.window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS)    // Clear selection when CTRL is not held
								std::fill(freqs.begin(), freqs.end(), false);
							freqs.at(i) = !freqs.at(i);
							if (freqs.at(i)) {
								std::fill(routers.begin(), routers.end(), false);
							}
							updated_routers = true;
							nearest_router_on = false;
							wifi.setAvailableMacs(wifi.getSelectedNames(wifinames), wifi.getSelectedFreqs(freqs));
						}
					}
					ImGui::TreePop();
				}

				if (ImGui::TreeNode("Routers")) {

					for (int i = 0; i < wifi.getAvailablesMacs().size(); i++) {
						if (ImGui::Selectable(wifi.getAvailablesMacs().at(i).c_str(), routers.at(i)))
						{
							if (glfwGetKey(w.window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS)    // Clear selection when CTRL is not held
								std::fill(routers.begin(), routers.end(), false);
							routers.at(i) = !routers.at(i);
							updated_routers = true;
							nearest_router_on = false;
						}
					}
					ImGui::TreePop();
				}
				ImGui::End();
			}
			if (gui_type == "demo") {
				ImGui::Begin("Demo");
				ImGui::SetWindowFontScale(1.5);

				bool update = false;
				static bool set_use_case = false;
				
				static int set_ind = -1;
				static int use_case_ind = -1;

				const int num_use_cases = 11;
				const char* const use_cases[num_use_cases] = {
					"Localize 1",				//0
					"Localize 2",				//1
					"Coverage - Contour",		//2
					"Coverage - Layered LIC",	//3
					"Interference - Mixed LIC",	//4
					"Interference - Max LIC",	//5
					"Test Use Case",			//6
					"Mixed LIC Explanation",	//7
					"Max LIC Explanation",		//8
					"Three Way Interference",	//9
					"5 routers",				//10
				};

				if (ImGui::Combo("Use Cases", &use_case_ind,
					use_cases, num_use_cases)) {
					set_use_case = true;
					glm::vec3 new_camera_pos = glm::vec3(-.262911, 11.58143, 2.414213);
					glm::vec3 new_camera_dir = glm::vec3(.89027, -.430448, 0);

					switch (use_case_ind) {
					case 0:
						config_ind = 0;
						set_ind = 8;
						new_camera_pos = glm::vec3(
							1.639865, 11.4868, 2.414213
						);
						new_camera_dir = glm::vec3(
							-.986163, -.129348, 0
						);
						break;
					case 1:
						config_ind = 1;
						set_ind = 9;
						new_camera_pos = glm::vec3(
							-7.166709, 11.403663, 2.414213
						);
						new_camera_dir = glm::vec3(
							.948736, -.301454, 0
						);
						break;
					case 2:
						config_ind = 0;
						set_ind = 7;
						new_camera_pos = glm::vec3(
							2.842, 10.2, 3.38
						);
						new_camera_dir = glm::vec3(
							.88, .454, 0
						);
						break;
					case 3:
						config_ind = 1;
						set_ind = 7;
						new_camera_pos = glm::vec3(
							8.658, 15.663932, .347995
						);
						new_camera_dir = glm::vec3(
							-.995577, -.079694, 0
						);
						break;
					case 4:
						config_ind = 4;
						set_ind = 4;
						break;
					case 5:
						config_ind = 5;
						set_ind = 4;
						break;
					case 6:
						config_ind = 0;
						set_ind = 0;
						break;
					case 7:
						config_ind = 2;
						set_ind = 10;
						new_camera_pos = glm::vec3(
							2.438988, 14.275912, 2.414213
						);
						new_camera_dir = glm::vec3(
							0.995877, -.043959, 0
						);
						break;
					case 8:
						config_ind = 3;
						set_ind = 10;
						new_camera_pos = glm::vec3(
							2.438988, 14.275912, 2.414213
						);
						new_camera_dir = glm::vec3(
							0.995877, -.043959, 0
						);
						break;
					case 9:
						config_ind = 6;
						set_ind = 11;
						new_camera_pos = glm::vec3(
							-1.284, 15.486, 1.543
							);
						new_camera_dir = glm::vec3(
							.981, -.104, 0
						);
						break;
					case 10:
						config_ind = 6;
						set_ind = 12;
						new_camera_pos = glm::vec3(
							-4.63, 11.4789, 2.414213
							);
						new_camera_dir = glm::vec3(
							.99, .063, 0
						);
						break;
					}

					camera.setPosition(new_camera_pos * scaling_factor);
					camera.setDirection(new_camera_dir);
				}
				else {
					set_use_case = false;
				}


				
				const char* const configs[num_configs] = 
				{
					"Contour Lines",	//0
					"Layered LIC",		//1
					"Mixed LIC",		//2
					"Max LIC",			//3
					"Mixed w/ Textons",	//4
					"Max w/ Textons",	//5
					"Coverage Test"		//6
				};
				
				if (ImGui::Combo("Configuration", &config_ind, 
					configs, num_configs
				) || set_use_case || update_config) {
					update_config = false;
					std::string filename = "";
					const char* const config_filenames[num_configs] = {
						"contour.cfg",
						"layered_lic.cfg",
						"mixed_lic.cfg",
						"max_lic.cfg",
						"interference_mixed.cfg",
						"interference_max.cfg",
						"coverage.cfg"
					};
					filename = std::string("Content/Config/") + config_filenames[config_ind];
					std::ifstream infile(filename, std::ios::in);
					if (!infile.is_open())
						std::cout << "File not found";
					else {
						readDictionary(deferred_shading_bools, infile);
						readDictionary(deferred_shading_floats, infile);
						readDictionary(deferred_shading_ints, infile);

						readDictionary(lic_shading_bools, infile);
						readDictionary(lic_shading_floats, infile);
						readDictionary(lic_shading_ints, infile);
						lic_shading_floats["learning_rate"] *= scaling_factor;
						readDictionary(ssao_shading_floats, infile);
						infile >> std::ws;
						old_num_samples = num_samples;
						infile >> num_samples;
						if (old_num_samples != num_samples) {
							num_samples_changed = true;
						}
					}
					start_render = true;
					num_routers_changed = true;
					//std::fill(freqs.begin(), freqs.end(), true);
					//wifi.setRouters(wifinames, routers, freqs);
					num_routers = wifi.getNumActiveRouters(routers);
					deferred_shading_floats["delta_theta"] = 180.f / wifi.getActiveFreqs(freqs).size();
					updated_routers = true;
					nearest_router_on = true;
					if (use_vr)
						vr.left_hand->setCounterx(config_ind);
				}
				std::string filename = "";
				
				const int num_router_sets = 13;
				const char* const router_sets[num_router_sets] =
				{
					"2 Routers",			//0
					"2 Routers Alt",		//1
					"Frequencey Analysis",	//2	
					"Find Router",			//3
					"Interference",			//4
					"Wifiname Coverage",	//5
					"Frequency Coverage",	//6
					"Coverage Test",		//7
					"Localization 1",		//8
					"Localization 2",		//9
					"Stacked",				//10
					"3 Way Interference",	//11
					"5 routers"				//12
				};
				if (ImGui::Combo("Router Set", &set_ind, router_sets, 
					num_router_sets) || set_use_case) {

					const char* const set_filenames[num_router_sets] = {
						"2_routers.txt",
						"closer_routers.txt",
						"sparse.txt",
						"",
						"interference.txt",
						"coverage_name.txt",
						"coverage_freq.txt",
						"gap.txt",
						"localize1.txt",
						"localize3.txt",
						"stacked_routers.txt",
						"three_way_interference.txt",
						"5_routers.txt"
					};
					filename = set_filenames[set_ind];
					if(filename == ""){
						int rand_num = floor(randomFloats(generator) * 5) + 1;
						filename = "localize" + std::to_string(rand_num) + ".txt";
						
					}
					
					
					std::ifstream infile(("Content/RouterSets/" + filename).c_str(), std::ios::in);
					reset(wifi, wifinames, freqs, routers);
					wifi.readRouters(infile, wifinames, routers, freqs);
					start_render = true;
					num_routers = wifi.getNumActiveRouters(routers);
					//wifi.setAvailableMacs(wifi.getSelectedNames(wifinames));

					deferred_shading_floats["delta_theta"] = 180.f / wifi.getActiveFreqs(freqs).size();
					updated_routers = true;
					nearest_router_on = true;
					num_routers_changed = true;
					infile.close();
					
				}
				
				
				ImGui::Checkbox("Line Integral Convolution", &deferred_shading_bools["lic_on"]);
				ImGui::End();
			}
		}if (render_gui) {
			ImGui::Begin("Minimap");
			ImGui::SetWindowFontScale(1.5f);
			ImTextureID minimap = (void*)minimap_buffer.minimapTexture.getID();
			//ImTextureID minimap = (void*)minimap_buffer.minimapTexture.getID();
			
			glm::uvec2 dims = minimap_buffer.minimapTexture.getDims();
			dims.x /= 6;
			dims.y /= 6;
			ImVec2 pos = ImGui::GetCursorScreenPos();
			ImGui::Image(minimap, ImVec2(dims.x , dims.y));
			if (ImGui::IsItemHovered())
			{
				auto io = ImGui::GetIO();
				
				ImGui::BeginTooltip();
				float region_sz = 32.0f;
				float region_x = io.MousePos.x - pos.x - region_sz * 0.5f; if (region_x < 0.0f) region_x = 0.0f; else if (region_x > dims.x - region_sz) region_x = dims.x - region_sz;
				float region_y = io.MousePos.y - pos.y - region_sz * 0.5f; if (region_y < 0.0f) region_y = 0.0f; else if (region_y > dims.y - region_sz) region_y = dims.y - region_sz;
				float zoom = 4.0f;
				ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
				ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
				ImVec2 uv0 = ImVec2((region_x) / dims.x, (region_y) / dims.y);
				ImVec2 uv1 = ImVec2((region_x + region_sz) / dims.x, (region_y + region_sz) / dims.y);
				ImGui::Image(minimap, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
				ImGui::EndTooltip();
			}
			ImGui::End();
		}
		if (renderPopup) {
			ImGui::Begin("Ellipsoid Distances");
			for (int i = 0; i < popup_text.size(); i++) {
				glm::vec4 color = wifi_colors[i + wifi.getNumWifiNames()];
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(color.r, color.g, color.b, 1));
				ImGui::Text(popup_text.at(i).c_str());
				ImGui::PopStyleColor();
			}
			ImGui::End();
		}

		
		ImGui::Render();

		glViewport(0, 0, render_resolution.x, render_resolution.y);

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		if (use_vr) {
			vr.handle_vr_input();
			//glm::vec3 pos = camera.getPosition();
			//pos.z += vr.adjusted_height;
			//vr.adjusted_height = 0;
			//camera.setPosition(pos);
			if (!demo_mode) {
				num_routers = floor(vr.right_hand->joystick_counter.x);
				int next_config = floor(vr.left_hand->joystick_counter.x);
				//if (next_config != config_ind) {
				//	config_ind = next_config;
				//	update_config = true;
				//}
			}
		}
		ImGuiIO& io = ImGui::GetIO();
		w.release_input = io.WantCaptureKeyboard;
		w.ProcessFrame();
		if (ImGui::IsKeyPressed('H', false) && !io.WantCaptureKeyboard) {
			w.keypressed = 0;
			render_gui = !render_gui;
		}
		if(use_vr)
			vr.handoff();
		glFinish();

	}
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwTerminate();
	if(use_vr)
		vr.terminate();
	std::ofstream light_position_file("lightPositions.txt", std::ios::out);
	for (int i = 0; i < w.lightPositions.size(); i++) {
		light_position_file << glm::to_string(w.lightPositions.at(i)) << std::endl;
	}
	return 1;
}