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
#include <time.h>
#include <direct.h>
#include <functional>
#include <glm/gtc/matrix_access.hpp>
#include <optix_cuda_interop.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define DEBUG true
#define BENCHMARK false
#define MY_PI 3.1415926535897932384626433
int counter = 10;
float increment = 0.05;
float scale = 1.0;
bool update = true;
//int size = 800;
bool animated = true;
const char* animation_file = "test_1.txt";
float speed = 6.0f;
glm::vec2 resolution = glm::vec2(2560, 1440);
glm::vec3 rand_dim = glm::vec3(50, 50, 50);

optix::Buffer amplitude_buffer, ray_buffer;

GLuint volume_textureId, volume_textureId2, random_texture_id, transferFunction_textureId, depth_mask_id, normal_textureId, ray_texId;
optix::TextureSampler volume_texture, volume_texture2, transferFunction_texture, depth_mask, random_texture, normal_texture, ray_texture;


void createGeometry(optix::Context& context, glm::vec3 volume_size, glm::mat4 transform) {
	try {
		const char* ptx = sutil::getPtxStringDirect("RowdenRender", "box.cu");
	
		optix::Program box_bounds = context->createProgramFromPTXString(ptx, "box_bounds");
		optix::Program box_intersect = context->createProgramFromPTXString(ptx, "box_intersect");
	
		// Create box
		optix::Geometry box = context->createGeometry();
		box->setPrimitiveCount(1u);
		box->setBoundingBoxProgram(box_bounds);
		box->setIntersectionProgram(box_intersect);

		//optix::float3 box_min = optix::make_float3(-volume_size.x / 2.f, -volume_size.y / 2.f, 0.f - volume_size.z / 2.f);	// volume is at origin
		//optix::float3 box_max = optix::make_float3(volume_size.x / 2.f, volume_size.y / 2.f, 0.f + volume_size.z / 2.f);
		optix::float3 box_min = optix::make_float3(25, 25, 0);	// volume is at origin
		optix::float3 box_max = optix::make_float3(75, 75, 50);


		optix::float3 volume_v1 = optix::make_float3(50, 0.f, 0.f); //scaling factors
		optix::float3 volume_v2 = optix::make_float3(0.f, 50, 0.f);
		optix::float3 volume_v3 = optix::make_float3(0.f, 0.f, 3);
		//optix::float3 volume_v3 = optix::make_float3(0.f, 0.f, 3.0);
		/*
		optix::float3 volume_v1 = optix::make_float3(50.0f, 0.f, 0.f);
		optix::float3 volume_v2 = optix::make_float3(0.f, 50.0f, 0.f);
		optix::float3 volume_v3 = optix::make_float3(0.f, 0.f, 25.0f);
		*/
		volume_v1 *= 1.0f / dot(volume_v1, volume_v1);
		volume_v2 *= 1.0f / dot(volume_v2, volume_v2);
		volume_v3 *= 1.0f / dot(volume_v3, volume_v3);
		std::cout << volume_v1.x << ", " << volume_v1.y << ", " << volume_v1.z << std::endl;
		std::cout << volume_v2.x << ", " << volume_v2.y << ", " << volume_v2.z << std::endl;
		std::cout << volume_v3.x << ", " << volume_v3.y << ", " << volume_v3.z << std::endl;
		box["box_min"]->setFloat(box_min);
		box["box_max"]->setFloat(box_max);
		box["v1"]->setFloat(volume_v1);
		box["v2"]->setFloat(volume_v2);
		box["v3"]->setFloat(volume_v3);
		box["voxel_size"]->setFloat(.01, .01, .01);

		box["scene_epsilon"]->setFloat(1e-4f);
		box["volumeRaytraceStepSize"]->setFloat(.11/3.0f);

		context["box_min"]->setFloat(box_min);
		context["box_max"]->setFloat(box_max);
		context["v1"]->setFloat(volume_v1);
		context["v2"]->setFloat(volume_v2);
		context["v3"]->setFloat(volume_v3);
		context["volumeRaytraceStepSize"]->setFloat(.11/10.0f);

		//context["hg_anchor"]->setFloat( 0, 0, 0);
		context["hg_anchor"]->setFloat( -(float)1024 *  .0037/ 2.f, -(float)1024 *.0037 / 2.f, -5.0f );
		context["hg_v1"]->setFloat(1, 0, 0);
		context["hg_v2"]->setFloat(0, 1, 0);	// axis aligned
		context["hg_normal"]->setFloat(0, 0, 1);


		
		optix::float3 volume_end_points[8];
		// voxel size
		optix::float3 voxel_size = optix::make_float3(.01,.01,.01);

		// determine end points of the volume
		float volume_x[2] = { -0.5f * volume_size.x, 0.5f * volume_size.x };
		float volume_y[2] = { -0.5f * volume_size.y, 0.5f * volume_size.y };
		float volume_z[2] = { -0.5f * volume_size.z, 0.5f * volume_size.z };
		for (unsigned int it_x = 0; it_x < 2; ++it_x) {
			for (unsigned int it_y = 0; it_y < 2; ++it_y) {
				for (unsigned int it_z = 0; it_z < 2; ++it_z) {
					unsigned int idx = it_z * 4 + it_y * 2 + it_x;
					volume_end_points[idx] = optix::make_float3(volume_x[it_x], volume_y[it_y], volume_z[it_z]);
				}
			}
		}

		optix::float3 hg_normal = optix::normalize(optix::make_float3(0,0,1));
		optix::float3 hg_anchor = optix::make_float3(0, 0, 0);
		//optix::float3 hg_anchor = optix::make_float3(-(float)1024* .0037 / 2.f, -(float)1024 * .0037 / 2.f, 5.0f);
		float min_dist = 1e10;
		float max_dist = 0;
		for (unsigned int it_v = 0; it_v < 8; ++it_v) {
			float dist = dot(volume_end_points[it_v] - hg_anchor, hg_normal);
			if (dist > max_dist) max_dist = dist;
			if (dist < min_dist) min_dist = dist;
		}


		context["vol_hg_dist"]->setFloat(min_dist, max_dist);
		
		
		ptx = sutil::getPtxStringDirect("RowdenRender", "volume_render.cu");
		optix::Material box_matl = context->createMaterial();
		optix::Program box_ch = context->createProgramFromPTXString(ptx, "closest_hit");
		
		box_matl->setClosestHitProgram(0, box_ch);

		// Create GIs for each piece of geometry
		std::vector<optix::GeometryInstance> gis;
		gis.push_back(context->createGeometryInstance(box, &box_matl, &box_matl + 1));

		// Place all in group
		optix::GeometryGroup geometrygroup = context->createGeometryGroup(gis.begin(), gis.end());
		geometrygroup->setChildCount(static_cast<unsigned int>(gis.size()));
		geometrygroup->setChild(0, gis[0]);
	
		geometrygroup->setAcceleration( context->createAcceleration("NoAccel") );

		context["top_object"]->set(geometrygroup);
	}
	catch (optix::Exception e) {
		std::cout << e.getErrorString() << std::endl;
	}
}

void createContext(optix::Context&context, Window&w) {
	int rtxOn = 1;
	rtGlobalSetAttribute(RT_GLOBAL_ATTRIBUTE_ENABLE_RTX, sizeof(rtxOn), &rtxOn);
	context = optix::Context::create();
	std::vector<int> devices;
	devices.emplace_back(0);
	context->setRayTypeCount(1);
	context["radiance_ray_type"]->setUint(0);
	context->setEntryPointCount(1);
	context->setStackSize(2100);
	if (DEBUG) {
		context->setPrintEnabled(true);
		context->setExceptionEnabled(RT_EXCEPTION_ALL, true);
	}
	context["scene_epsilon"]->setFloat(1.e-4f);
	context["opacity_correction"]->setFloat(.65f);
	optix::float2 output_buffer_dim = optix::make_float2(resolution.x, resolution.y);
	amplitude_buffer = sutil::createOutputBuffer(context, RT_FORMAT_FLOAT4, output_buffer_dim.x, output_buffer_dim.y, false);
	context["amplitude_buffer"]->set(amplitude_buffer);
	float volumeRaytraceStepSize = .11;
	optix::float3 volume_size = optix::make_float3(.01f, .01f, .01f);
	float volumeDiagLength = sqrt(volume_size.x * volume_size.x + volume_size.y * volume_size.y + volume_size.z * volume_size.z);
	unsigned int maxCompositionSteps = (unsigned int)std::ceil(volumeDiagLength / volumeRaytraceStepSize);


	context["element_hologram_dim"]->setUint(resolution.x, resolution.y);
	context["half_num_rays_per_element_hologram"]->setUint( resolution.x/ 2, resolution.y/ 2);
	context["num_rays_per_element_hologram"]->setUint(resolution.x, resolution.y);
	context["pixel_pitch"]->setFloat(.0037);
	context["ray_interval"]->setFloat(10e-4);

	std::string kernel = "volume_render.cu";
	try
	{


	const char* ptx = sutil::getPtxStringDirect("RowdenRender", kernel.c_str());
	
	//set output
	//optix::Buffer buffer = sutil::createOutputBuffer(context, RT_FORMAT_UNSIGNED_BYTE4, w.width, w.height, false);
	//context["output_buffer"]->set(buffer);
	context->setRayGenerationProgram(0, context->createProgramFromPTXString(ptx, "camera"));

	// Exception program
	optix::Program exception_program = context->createProgramFromPTXString(ptx, "exception");
	context->setExceptionProgram(0, exception_program);
	
	// Miss program
	const std::string miss_name = "miss";
	context->setMissProgram(0, context->createProgramFromPTXString(ptx, miss_name));
	}
	catch (optix::Exception e)
	{
		std::cout << e.getErrorString() << std::endl;
	}
	const glm::vec3 default_color = glm::vec3(1.0f, 1.0f, 1.0f);
}

void createOptixTextures(optix::Context& context, glm::vec3 volume_size, std::vector<unsigned char> volumeRaw, std::vector<short> normals, int tex_num = 1) {
	try {
		
		glGenTextures(1, &volume_textureId);
		glBindTexture(GL_TEXTURE_3D, volume_textureId);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_COMPRESSED_RED, volume_size.x, volume_size.y, volume_size.z, 0, GL_RED, GL_UNSIGNED_BYTE, (void*)volumeRaw.data());
		//glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, volume_size.x, volume_size.y, volume_size.z, 0, GL_RED, GL_UNSIGNED_BYTE, (void*)volumeRaw.data());
		glBindTexture(GL_TEXTURE_3D, 0);
		// create optix 3D texture sampler

		volume_texture = context->createTextureSamplerFromGLImage(volume_textureId, RT_TARGET_GL_TEXTURE_3D);
		volume_texture->setFilteringModes(RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_LINEAR);
		volume_texture->setWrapMode(0, RT_WRAP_CLAMP_TO_BORDER);
		volume_texture->setWrapMode(1, RT_WRAP_CLAMP_TO_BORDER);
		volume_texture->setWrapMode(2, RT_WRAP_CLAMP_TO_BORDER);
		
		context["volumeTextureId" + std::to_string(tex_num)]->setInt(volume_texture->getId());

		glGenTextures(1, &normal_textureId);
		glBindTexture(GL_TEXTURE_3D, normal_textureId);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_COMPRESSED_RG, volume_size.x, volume_size.y, volume_size.z, 0, GL_RG, GL_SHORT, (void*)normals.data());
		//glTexImage3D(GL_TEXTURE_3D, 0, GL_RG16F, volume_size.x, volume_size.y, volume_size.z, 0, GL_RG, GL_SHORT, (void*)normals.data());
		glBindTexture(GL_TEXTURE_3D, 0);
		// create optix 3D texture sampler
		normal_texture = context->createTextureSamplerFromGLImage(normal_textureId, RT_TARGET_GL_TEXTURE_3D);
		normal_texture->setFilteringModes(RT_FILTER_LINEAR, RT_FILTER_NEAREST, RT_FILTER_LINEAR);
		normal_texture->setWrapMode(0, RT_WRAP_CLAMP_TO_BORDER);
		normal_texture->setWrapMode(1, RT_WRAP_CLAMP_TO_BORDER);
		normal_texture->setWrapMode(2, RT_WRAP_CLAMP_TO_BORDER);
		context["normalTextureId" + std::to_string(tex_num)]->setInt(normal_texture->getId());
		
		// 3. create transfer function 1D texture
		int transferFunctionSize = 2;
		std::vector<float> transferFunction = std::vector<float>();
		std::string line;
		std::ifstream transferfunction("C:/Users/alrowden/source/repos/RowdenRender/RowdenRender/gaus.1dt");
		//std::ifstream transferfunction("C:/Users/alrowden/source/repos/RowdenRender/RowdenRender/seperate_colors.1dt");

		if (transferfunction.is_open()) {
			std::getline(transferfunction, line);
			int num_points = atoi(line.c_str());
			for (int i = 0; i < num_points; i++) {
				std::getline(transferfunction, line);
				float r, g, b, a;
				sscanf_s(line.c_str(), "%f %f %f %f", &r, &g, &b, &a);
				transferFunction.emplace_back(r);
				transferFunction.emplace_back(g);
				transferFunction.emplace_back(b);
				transferFunction.emplace_back( a/1.0f);
			}
		}
		transferFunctionSize = transferFunction.size() / 4;
		optix::float4* tf_data_dummy2D = new optix::float4[transferFunctionSize * transferFunctionSize];
		for (unsigned int i = 0; i < transferFunctionSize; ++i) {
			memcpy(&tf_data_dummy2D[i * transferFunctionSize], &transferFunction[0], transferFunctionSize * sizeof(optix::float4));
		}
		glGenTextures(1, &transferFunction_textureId);
		glBindTexture(GL_TEXTURE_2D, transferFunction_textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, transferFunctionSize, transferFunctionSize, 0, GL_RGBA, GL_FLOAT, (void*)tf_data_dummy2D);
		glBindTexture(GL_TEXTURE_2D, 0);
		delete[] tf_data_dummy2D;
		// create optix 2D texture sampler
		transferFunction_texture = context->createTextureSamplerFromGLImage(transferFunction_textureId, RT_TARGET_GL_TEXTURE_2D);
		transferFunction_texture->setFilteringModes(RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_NONE);
		transferFunction_texture->setWrapMode(0, RT_WRAP_CLAMP_TO_EDGE);
		transferFunction_texture->setWrapMode(1, RT_WRAP_CLAMP_TO_EDGE);
		context["transferFunction_texId"]->setInt(transferFunction_texture->getId());

		
		
		
	}
	catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}
}

//any old render function
void render(Model mesh, ShaderProgram *sp) {
	counter = 90;
	if (counter > 100)
		counter -= 100;
	glClearColor(counter/100.0f, counter / 100.0f, counter / 80.0f, 1.0);
	
	for (Mesh* m : mesh.getMeshes()) {
		m->Render();
	}
}

void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

#include <windows.h>

std::string getexepath()
{
	char result[MAX_PATH];
	return std::string(result, GetModuleFileName(NULL, result, MAX_PATH));
}

void MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204 || id == 131154) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}

glm::vec3 getHeatMapColor(float value)

{

	const int NUM_COLORS = 5;

	static float color[NUM_COLORS][3] = { { 0,0,1 },{ 0,1,1 },{ 0,1,0 },{ 1,1,0 },{ 1,0,0 } };

	// A static array of 4 colors:  (blue,   green,  yellow,  red) using {r,g,b} for each.    
	int idx1;        // |-- Our desired color will be between these two indexes in "color".

	int idx2;        // |

	float fractBetween = 0;  // Fraction between "idx1" and "idx2" where our value is.    
	if (value <= 0) { idx1 = idx2 = 0; }    // accounts for an input <=0
	else if (value >= 1) { idx1 = idx2 = NUM_COLORS - 1; }    // accounts for an input >=1
	else
	{

		value = value * (NUM_COLORS - 1);        // Will multiply value by 3.

		idx1 = floor(value);                  // Our desired color will be after this index.

		idx2 = idx1 + 1;                        // ... and before this index (inclusive).

		fractBetween = value - float(idx1);    // Distance between the two indexes (0-1).

	}

	glm::vec3 ret;
	ret.r = (color[idx2][0] - color[idx1][0]) * fractBetween + color[idx1][0];
	ret.g = (color[idx2][1] - color[idx1][1]) * fractBetween + color[idx1][1];
	ret.b = (color[idx2][2] - color[idx1][2]) * fractBetween + color[idx1][2];

	return ret;

}

void LoadCampusModel(Model *completeCampus) {
	/*
	if (true) {
		int campusRunningIndex = 0;
		for (int i = 1; i <= 999; i++)
		{
			std::string value = std::to_string(i);
			if (value.length() == 1)
				value = "00" + value;
			else if (value.length() == 2)
				value = "0" + value;

			

			tinyxml2::XMLDocument* buildingKML;
			std::string file = "Content/Models/Buildings/ID_" + value + ".kml";
			buildingKML = new tinyxml2::XMLDocument();
			bool configLoaded = buildingKML->LoadFile(file.c_str());
			if ( buildingKML->ErrorID() != 0)
			{
				std::cerr << "Could not load file: '" << file << "' " << buildingKML->ErrorID() << std::endl;
				continue;
			}
			Model* building = new Model("Content/Models/Buildings/ID_" + value + ".dae");
			tinyxml2::XMLHandle buildingKMLHandle(buildingKML);

			tinyxml2::XMLElement* buildingXML =
				buildingKMLHandle.FirstChildElement("kml").FirstChildElement("Document").FirstChildElement("Placemark").FirstChildElement("Model").ToElement();
			tinyxml2::XMLElement* locationXML = buildingXML->FirstChildElement("Location");
			tinyxml2::XMLElement* longitude = locationXML->FirstChildElement("longitude");
			tinyxml2::XMLElement* latitude = locationXML->FirstChildElement("latitude");

			std::string lonText = longitude->GetText();
			std::string latText = latitude->GetText();
			float lon = std::atof(lonText.c_str());
			float lat = std::atof(latText.c_str());

			//std::cout << lonText << " " << latText << std::endl;

			tinyxml2::XMLElement* scaleXML = buildingXML->FirstChildElement("Scale");
			tinyxml2::XMLElement* scaleX = scaleXML->FirstChildElement("x");
			tinyxml2::XMLElement* scaleY = scaleXML->FirstChildElement("y");
			tinyxml2::XMLElement* scaleZ = scaleXML->FirstChildElement("z");

			float sx = std::atof(scaleX->GetText());
			float sy = std::atof(scaleY->GetText());
			float sz = std::atof(scaleZ->GetText());
			//std::cout << sx << "," << sy << "," << sz << std::endl;

			//KroEngine::WorldObject* buildingObj = new KroEngine::WorldObject();
			//buildingObj->SetModel(building);
			//buildingObj->SetPosition(glm::vec3(lon, 0, lat));
			////buildingObj->SetRotation(glm::toQuat(glm::rotate((float)DEG2RAD(-90.0f), glm::vec3(1, 0, 0))));
			//buildingObj->SetScale(glm::vec3(sx, sy, sz) * 0.5f);
			lon += 76.936594579149414130370132625103f;
			lat -= 38.990750300955419049842021195218f;
			float scale = .0085;
			glm::mat4 transform =
				glm::translate(glm::mat4(1), glm::vec3(-lon * 7500, 0, lat * 7500)) *
				//glm::rotate((float)DEG2RAD(-90.0f), glm::vec3(0, 0, 1)) *
				glm::rotate(glm::mat4(1), (float)glm::radians(-90.0f), glm::vec3(1, 0, 0)) *
				//glm::rotate((float)DEG2RAD(90.0f), glm::vec3(0, 1, 0)) *
				glm::scale(glm::mat4(1), .5f * glm::vec3(-sx, -sy, sz));
			std::vector<Mesh*> parts = building->getMeshes();
			Shape* moved_building = new Shape();
			for (Mesh* part : parts)
			{
				std::vector<float> verts = part->getVerticies();
				if (verts.size() == 0)
					continue;
				std::vector<unsigned int> indices = part->getIndices();
				std::vector<float> normals = part->getNormals();
				//std::vector<float> tangents = part->getTangents();
				std::vector<float> texCoords = part->getTexCoords();

				for (int v = 0; v < verts.size(); v += 3)
				{
					moved_building->addVertex(
						glm::vec3(transform * glm::vec4(
							verts.at(v),
							verts.at(v + 1),
							verts.at(v + 2),
							1)
						)
					);
					moved_building->addNormal(
						glm::normalize(
							glm::vec3(glm::vec4(
								normals.at(v),
								normals.at(v + 1),
								normals.at(v + 2),
								1)
							)
						)
					);
					/*
					completeCampus->AddTangent(
						glm::normalize(
							glm::vec3(glm::vec4(
								tangents.at(v),
								tangents.at(v + 1),
								tangents.at(v + 2),
								1)
							)
						)
					);
					*/ /*
				}
				for (int v = 0; v < texCoords.size(); v += 2) {
					moved_building->addTexCoord(
						glm::vec2(
							texCoords.at(v),
							texCoords.at(v + 1)
						)
					);
				}

				for (int in = 0; in < indices.size(); in += 3)
				{
					moved_building->addIndex(indices.at(in) + campusRunningIndex, indices.at(in + 1) + campusRunningIndex, indices.at(in + 2) + campusRunningIndex);
				}
				
				//std::cout << "NumMeshes: " << completeCampus->getMeshes().size() << std::endl;
				campusRunningIndex += indices.size();
			}
			completeCampus->addMesh(new Mesh(moved_building));
			delete building;
			//campusBuildings.emplace_back(buildingObj);

			
		}for (int i = 1; i <= 46; i++)
		{
			std::string value = std::to_string(i);
			if (value.length() == 1)
				value = "00" + value;
			else if (value.length() == 2)
				value = "0" + value;



			tinyxml2::XMLDocument* buildingKML;
			std::string file = "Content/Models/Buildings/S" + value + ".kml";
			buildingKML = new tinyxml2::XMLDocument();
			bool configLoaded = buildingKML->LoadFile(file.c_str());
			if (buildingKML->ErrorID() != 0)
			{
				std::cerr << "Could not load file: '" << file << "' " << buildingKML->ErrorID() << std::endl;
				continue;
			}
			Model* building = new Model("Content/Models/Buildings/ID_" + value + ".dae");
			tinyxml2::XMLHandle buildingKMLHandle(buildingKML);

			tinyxml2::XMLElement* buildingXML =
				buildingKMLHandle.FirstChildElement("kml").FirstChildElement("Document").FirstChildElement("Placemark").FirstChildElement("Model").ToElement();
			tinyxml2::XMLElement* locationXML = buildingXML->FirstChildElement("Location");
			tinyxml2::XMLElement* longitude = locationXML->FirstChildElement("longitude");
			tinyxml2::XMLElement* latitude = locationXML->FirstChildElement("latitude");

			std::string lonText = longitude->GetText();
			std::string latText = latitude->GetText();
			float lon = std::atof(lonText.c_str());
			float lat = std::atof(latText.c_str());

			//std::cout << lonText << " " << latText << std::endl;

			tinyxml2::XMLElement* scaleXML = buildingXML->FirstChildElement("Scale");
			tinyxml2::XMLElement* scaleX = scaleXML->FirstChildElement("x");
			tinyxml2::XMLElement* scaleY = scaleXML->FirstChildElement("y");
			tinyxml2::XMLElement* scaleZ = scaleXML->FirstChildElement("z");

			float sx = std::atof(scaleX->GetText());
			float sy = std::atof(scaleY->GetText());
			float sz = std::atof(scaleZ->GetText());
			//std::cout << sx << "," << sy << "," << sz << std::endl;

			//KroEngine::WorldObject* buildingObj = new KroEngine::WorldObject();
			//buildingObj->SetModel(building);
			//buildingObj->SetPosition(glm::vec3(lon, 0, lat));
			////buildingObj->SetRotation(glm::toQuat(glm::rotate((float)DEG2RAD(-90.0f), glm::vec3(1, 0, 0))));
			//buildingObj->SetScale(glm::vec3(sx, sy, sz) * 0.5f);
			lon += 76.936594579149414130370132625103f;
			lat -= 38.990750300955419049842021195218f;
			float scale = .0085;
			glm::mat4 transform =
				glm::translate(glm::mat4(1), glm::vec3(-lon * 2000, 0, lat * 2000)) *
				//glm::rotate((float)DEG2RAD(-90.0f), glm::vec3(0, 0, 1)) *
				glm::rotate(glm::mat4(1), (float)glm::radians(-90.0f), glm::vec3(1, 0, 0)) *
				//glm::rotate((float)DEG2RAD(90.0f), glm::vec3(0, 1, 0)) *
				glm::scale(glm::mat4(1), .5f * glm::vec3(-sx, -sy, sz));
			std::vector<Mesh*> parts = building->getMeshes();
			Shape* moved_building = new Shape();
			for (Mesh* part : parts)
			{
				std::vector<float> verts = part->getVerticies();
				if (verts.size() == 0)
					continue;
				std::vector<unsigned int> indices = part->getIndices();
				std::vector<float> normals = part->getNormals();
				//std::vector<float> tangents = part->getTangents();
				std::vector<float> texCoords = part->getTexCoords();

				for (int v = 0; v < verts.size(); v += 3)
				{
					moved_building->addVertex(
						glm::vec3(transform * glm::vec4(
							verts.at(v),
							verts.at(v + 1),
							verts.at(v + 2),
							1)
						)
					);
					moved_building->addNormal(
						glm::normalize(
							glm::vec3(glm::vec4(
								normals.at(v),
								normals.at(v + 1),
								normals.at(v + 2),
								1)
							)
						)
					);
					/*
					completeCampus->AddTangent(
						glm::normalize(
							glm::vec3(glm::vec4(
								tangents.at(v),
								tangents.at(v + 1),
								tangents.at(v + 2),
								1)
							)
						)
					);
					*/ /*
				}
				for (int v = 0; v < texCoords.size(); v += 2) {
					moved_building->addTexCoord(
						glm::vec2(
							texCoords.at(v),
							texCoords.at(v + 1)
						)
					);
				}

				for (int in = 0; in < indices.size(); in += 3)
				{
					moved_building->addIndex(indices.at(in) + campusRunningIndex, indices.at(in + 1) + campusRunningIndex, indices.at(in + 2) + campusRunningIndex);
				}
				
				//std::cout << "NumMeshes: " << completeCampus->getMeshes().size() << std::endl;
				campusRunningIndex += indices.size();
			}
			completeCampus->addMesh(new Mesh(moved_building));
			delete building;
			//campusBuildings.emplace_back(buildingObj);


		}
		completeCampus->setModel();
		size_t pcount = completeCampus->getMeshes().size();
		std::fstream filestream = std::fstream("data.bin", std::fstream::out | std::fstream::binary);
		filestream.write(reinterpret_cast<char*>(&pcount), sizeof(size_t));
		for (Mesh *campusMesh : completeCampus->getMeshes()) {
			std::vector<float> vertices = campusMesh->getVerticies();
			//std::cout << vertices.size() / 3 << std::endl;
			size_t count = vertices.size();
			filestream.write(reinterpret_cast<char*>(&count), sizeof(size_t));
			for (float vertex : vertices) {
				filestream.write(reinterpret_cast<char*>(&vertex), sizeof(float));
			}
			std::vector<float> normals = campusMesh->getNormals();
			count = normals.size();
			filestream.write(reinterpret_cast<char*>(&count), sizeof(size_t));
			for (float normal : normals) {
				filestream.write(reinterpret_cast<char*>(&normal), sizeof(float));
			}
			/*
			std::vector<float> tangents = campusMesh->getTangents();
			count = tangents.size();
			filestream.write(reinterpret_cast<char*>(&count), sizeof(size_t));
			for (float tangent : tangents) {
				filestream.write(reinterpret_cast<char*>(&tangent), sizeof(float));
			}
			*/ /*
			std::vector<float> texCoords = campusMesh->getTexCoords();
			count = texCoords.size();
			filestream.write(reinterpret_cast<char*>(&count), sizeof(size_t));
			for (float texCoord : texCoords) {
				filestream.write(reinterpret_cast<char*>(&texCoord), sizeof(float));
			}

			std::vector<unsigned int> indices = campusMesh->getIndices();
			size_t indCount = indices.size();
			filestream.write(reinterpret_cast<char*>(&indCount), sizeof(size_t));
			for (unsigned int index : indices) {
				filestream.write(reinterpret_cast<char*>(&index), sizeof(unsigned int));
			}
		}
		filestream.close();
	}
	else {
	std::fstream filestream = std::fstream("data.bin", std::fstream::in | std::fstream::binary);
	if (!filestream.is_open()) {
		std::cout << "file stream didn't open. Panic" << std::endl;
	}
	size_t count, parts;
	std::vector<glm::vec3> vertices = std::vector<glm::vec3>();
	glm::vec3 vertex;
	glm::vec3 min_bounds = glm::vec3(0), max_bounds = glm::vec3(0);
	filestream.read((char*)& parts, sizeof(size_t));
	Shape* buildings = new Shape();
	for (size_t j = 0; j < parts; j++) {
		filestream.read((char*)& count, sizeof(size_t));
		for (size_t i = 0; i < count; i += 3) {
			filestream.read((char*)& vertex.x, sizeof(float));
			filestream.read((char*)& vertex.y, sizeof(float));
			filestream.read((char*)& vertex.z, sizeof(float));
			vertices.emplace_back(vertex);
			if (vertex.x > max_bounds.x)
				max_bounds.x = vertex.x;
			if (vertex.y > max_bounds.y)
				max_bounds.y = vertex.y;
			if (vertex.z > max_bounds.z)
				max_bounds.z = vertex.z;
			if (vertex.x < max_bounds.x)
				min_bounds.x = vertex.x;
			if (vertex.y < max_bounds.y)
				min_bounds.y = vertex.y;
			if (vertex.z < max_bounds.z)
				min_bounds.z = vertex.z;
		}
		filestream.read((char*)& count, sizeof(size_t));
		for (size_t i = 0; i < count; i += 3) {
			filestream.read((char*)& vertex.x, sizeof(float));
			filestream.read((char*)& vertex.y, sizeof(float));
			filestream.read((char*)& vertex.z, sizeof(float));
			buildings->addNormal(vertex);
		}
		/*
		filestream.read((char*)& count, sizeof(size_t));
		for (size_t i = 0; i < count; i += 3) {
			filestream.read((char*)& vertex.x, sizeof(float));
			filestream.read((char*)& vertex.y, sizeof(float));
			filestream.read((char*)& vertex.z, sizeof(float));
			buildings->addTangent(vertex);
		}
		*/ 
/*
		filestream.read((char*)& count, sizeof(size_t));
		for (size_t i = 0; i < count; i += 2) {
			filestream.read((char*)& vertex.x, sizeof(float));
			filestream.read((char*)& vertex.y, sizeof(float));
			buildings->addTexCoord(vertex);
		}
		filestream.read((char*)& count, sizeof(size_t));
		unsigned int uint1, uint2, uint3;
		for (size_t i = 0; i < count; i += 3) {
			filestream.read((char*)& uint1, sizeof(unsigned int));
			filestream.read((char*)& uint2, sizeof(unsigned int));
			filestream.read((char*)& uint3, sizeof(unsigned int));
			buildings->addIndex(uint1, uint2, uint3);
		}
	}
	glm::vec3 range = max_bounds - min_bounds;
	//glm::mat4 transform = glm::scale(glm::mat4(1), 1 / (fmax(fmax(range.x, range.y), range.z)) * glm::vec3(1));
	//transform = glm::translate(transform, ( -min_bounds));
	
	for (int i = 0; i < vertices.size(); i++) {
		buildings->addVertex(glm::vec3( glm::vec4(vertices.at(i), 1.0f)));
	}
	completeCampus->addMesh(new Mesh(buildings));
	completeCampus->setModel();
	filestream.close();

	}
	*/
	
	completeCampus->addModel(new Model("Content/Models/Buildings/flat_campus.fbx"));
	completeCampus->setModel();
}

int get3DIndex(int i, int j, int k, int x_dim, int y_dim, int num_cells) {
	if (i >= x_dim || j >= y_dim || k >= num_cells) {
		return -1;
	}
	return i + j * x_dim + k * x_dim * y_dim;
}

float getIntensity(std::vector<float> intensities, int max_index, int index) {
	if (index > max_index) {
		return 0;
	}
	else if (index < 0) {
		return -1;
	}
	return intensities.at(index);
}

glm::vec3 calcNormal(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2)
{
	glm::vec3 edge0 = v1 - v0;
	glm::vec3 edge1 = v2 - v0;

	return glm::normalize(cross(edge0, edge1));
}
glm::vec3 vertexInterp(float isolevel, glm::vec3 p0, glm::vec3 p1, float f0, float f1)
{
	if (abs(f1 - f0) < std::numeric_limits<float>::epsilon()) {
		return p0;
	}
	float t = (isolevel - f0) / (f1 - f0);
	return glm::lerp(p0, p1, t);
}

GLenum glFormatFromBufferFormat(bufferPixelFormat pixel_format, RTformat buffer_format)
{
	if (buffer_format == RT_FORMAT_UNSIGNED_BYTE4)
	{
		switch (pixel_format)
		{
		case BUFFER_PIXEL_FORMAT_DEFAULT:
			return GL_BGRA;
		case BUFFER_PIXEL_FORMAT_RGB:
			return GL_RGBA;
		case BUFFER_PIXEL_FORMAT_BGR:
			return GL_BGRA;
		default:
			throw std::exception("Unknown buffer pixel format");
		}
	}
	else if (buffer_format == RT_FORMAT_FLOAT4)
	{
		switch (pixel_format)
		{
		case BUFFER_PIXEL_FORMAT_DEFAULT:
			return GL_RGBA;
		case BUFFER_PIXEL_FORMAT_RGB:
			return GL_RGBA;
		case BUFFER_PIXEL_FORMAT_BGR:
			return GL_BGRA;
		default:
			throw std::exception("Unknown buffer pixel format");
		}
	}
	else if (buffer_format == RT_FORMAT_FLOAT3)
		switch (pixel_format)
		{
		case BUFFER_PIXEL_FORMAT_DEFAULT:
			return GL_RGB;
		case BUFFER_PIXEL_FORMAT_RGB:
			return GL_RGB;
		case BUFFER_PIXEL_FORMAT_BGR:
			return GL_BGR;
		default:
			throw std::exception("Unknown buffer pixel format");
		}
	else if (buffer_format == RT_FORMAT_FLOAT)
		return GL_LUMINANCE;
	else
		throw std::exception("Unknown buffer format");
}

GLuint optixBufferToGLTexture(optix::Buffer& buffer) {
	// Query buffer information
	RTsize buffer_width_rts, buffer_height_rts;
	buffer->getSize(buffer_width_rts, buffer_height_rts);
	uint32_t width = static_cast<int>(buffer_width_rts);
	uint32_t height = static_cast<int>(buffer_height_rts);
	RTformat buffer_format = buffer->getFormat();

	bool g_disable_srgb_conversion = false;
	GLboolean use_SRGB = GL_FALSE;
	if (!g_disable_srgb_conversion && (buffer_format == RT_FORMAT_FLOAT4 || buffer_format == RT_FORMAT_FLOAT3))
	{
		glGetBooleanv(GL_FRAMEBUFFER_SRGB_CAPABLE_EXT, &use_SRGB);
		if (use_SRGB)
			glEnable(GL_FRAMEBUFFER_SRGB_EXT);
	}

	static unsigned int gl_tex_id = 0;
	if (!gl_tex_id)
	{
		glGenTextures(1, &gl_tex_id);
		glBindTexture(GL_TEXTURE_2D, gl_tex_id);

		// Change these to GL_LINEAR for super- or sub-sampling
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		// GL_CLAMP_TO_EDGE for linear filtering, not relevant for nearest.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	glBindTexture(GL_TEXTURE_2D, gl_tex_id);

	// send PBO or host-mapped image data to texture
	const unsigned pboId = buffer->getGLBOId();
	GLvoid* imageData = 0;
	if (pboId)
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, pboId);
	else
		imageData = buffer->map(0, RT_BUFFER_MAP_READ);

	RTsize elmt_size = buffer->getElementSize();
	if (elmt_size % 8 == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 8);
	else if (elmt_size % 4 == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	else if (elmt_size % 2 == 0) glPixelStorei(GL_UNPACK_ALIGNMENT, 2);
	else                          glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	bufferPixelFormat g_image_buffer_format = BUFFER_PIXEL_FORMAT_DEFAULT;
	GLenum pixel_format = glFormatFromBufferFormat(g_image_buffer_format, buffer_format);
	if (buffer_format == RT_FORMAT_UNSIGNED_BYTE4)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, imageData);
	else if (buffer_format == RT_FORMAT_FLOAT4)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB, width, height, 0, pixel_format, GL_FLOAT, imageData);
	else if (buffer_format == RT_FORMAT_FLOAT3)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F_ARB, width, height, 0, pixel_format, GL_FLOAT, imageData);
	else if (buffer_format == RT_FORMAT_FLOAT)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE32F_ARB, width, height, 0, pixel_format, GL_FLOAT, imageData);
	else
		throw std::exception("Unknown buffer format");

	if (pboId)
		glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
	else
		buffer->unmap();

	return gl_tex_id;
}

optix::float2 make_float2(glm::vec2 a) {
	return optix::make_float2(a.x, a.y);
}optix::float3 make_float3(glm::vec3 a) {
	return optix::make_float3(a.x, a.y, a.z);
}optix::float4 make_float4(glm::vec4 a) {
	return optix::make_float4(a.x, a.y, a.z, a.w);
}
optix::Matrix4x4 make_mat4(glm::mat4 mat) {
	return optix::Matrix4x4(glm::value_ptr(mat));
}

void updateCamera(Window& w, optix::Context& context, Camera& camera) {
	glm::vec4 camera_u, camera_v, camera_w, camera_x;
	
	glm::mat4 ModelToView = glm::inverse(camera.getView());
	glm::mat4 view = camera.getView();
	glm::mat4 projection = camera.getProjection();
	//optix::Matrix4x4 modelToView = make_mat4(ModelToView);
	//std::cout << glm::to_string(ModelToView * glm::vec4(0, 0, 0, 1));
	/*
	int viewport[4];
	void* ray_dirs;
	glGetIntegerv(GL_VIEWPORT, viewport);
	
	rtBufferMap(ray_buffer->get(), &ray_dirs);
	//ray_buffer->map()
	//std::vector<optix::float3>& rays = *reinterpret_cast<std::vector<optix::float3>*>(ray_dirs);
	//rays.resize(((long)viewport[2] - viewport[0]) * (viewport[3] - viewport[1]));
	optix::float3* rays = (optix::float3*)ray_dirs;
	for (int i = viewport[0]; i < viewport[2]; i++) {
		for (int j = viewport[1]; j < viewport[3]; j++) {
			glm::vec3 temp = glm::unProject(glm::vec3(i, j, 1), view, projection, glm::vec4(viewport[0], viewport[1], viewport[2], viewport[3]));
			//std::cout << glm::to_string(temp) << std::endl;
			rays[i + j * (long)viewport[2]] = (make_float3(temp));
		}
	}
	rtBufferUnmap(ray_buffer->get());	
	*/
	
	//printf("%d, %d, %d, %d\n", viewport[0], viewport[1], viewport[2], viewport[3]);
	camera_u = glm::column(ModelToView, 0);
	camera_v = glm::column(ModelToView, 1);
	camera_w = glm::column(ModelToView, 2);
	camera_x = glm::column(ModelToView, 3);
	//camera_w =  (-w.width / 2.0f ) * camera_u + (-w.height / 2.0f) * camera_v + ((w.height / 2.0f) / tan(glm::radians(90.0f) * 0.5f) * camera_w);

	context["eye"]->setFloat(make_float3(camera.getPosition()));
	context["m1"]->setFloat(make_float4(camera_u));
	context["m2"]->setFloat(make_float4(camera_v));
	context["m3"]->setFloat(make_float4(camera_w));
	context["m4"]->setFloat(make_float4(camera_x));
	
	ModelToView = glm::inverse(camera.getProjection());
	//optix::Matrix4x4 modelToView = make_mat4(ModelToView);
	//std::cout << glm::to_string(ModelToView * glm::vec4(0, 0, 0, 1));
	
	camera_u = glm::column(ModelToView, 0);
	camera_v = glm::column(ModelToView, 1);
	camera_w = glm::column(ModelToView, 2);
	camera_x = glm::column(ModelToView, 3);
	//camera_w =  (-w.width / 2.0f ) * camera_u + (-w.height / 2.0f) * camera_v + ((w.height / 2.0f) / tan(glm::radians(90.0f) * 0.5f) * camera_w);

	context["n1"]->setFloat(make_float4(camera_u));
	context["n2"]->setFloat(make_float4(camera_v));
	context["n3"]->setFloat(make_float4(camera_w));
	context["n4"]->setFloat(make_float4(camera_x));
	

}

void updateCamera(Window&w, optix::Context&context, optix::float3 camera_eye, optix::float3 camera_lookat, optix::float3 camera_up, optix::Matrix4x4 camera_rotate)
{
	const float vfov = 90.0f;
	const float aspect_ratio = static_cast<float>(w.width) /
		static_cast<float>(w.height);

	optix::float3 camera_u, camera_v, camera_w;
	sutil::calculateCameraVariables(
		camera_eye, camera_lookat, camera_up, vfov, aspect_ratio,
		camera_u, camera_v, camera_w, true);
	
	const optix::Matrix4x4 frame = optix::Matrix4x4::fromBasis(
		normalize(camera_u),
		normalize(camera_v),
		normalize(-camera_w),
		camera_lookat);
	const optix::Matrix4x4 frame_inv = frame.inverse();
	// Apply camera rotation twice to match old SDK behavior


	sutil::calculateCameraVariables(
		camera_eye, camera_lookat, camera_up, vfov, aspect_ratio,
		camera_u, camera_v, camera_w, true);

	//camera_rotate = optix::Matrix4x4::identity();
	std::cout << camera_u.x << ', ' <<camera_u.y << ', ' << camera_u.z << std::endl;
	std::cout << camera_v.x << camera_v.y << camera_v.z << std::endl;
	std::cout << camera_w.x << camera_w.y << camera_w.z << std::endl;
	std::cout << std::endl;
	std::cout << glm::to_string(w.camera->getView()) << std::endl;
	std::cout << std::endl;
	std::cout << std::endl;

	context["eye"]->setFloat(camera_eye);
	context["U"]->setFloat(camera_u);
	context["V"]->setFloat(camera_v);
	context["W"]->setFloat(camera_w);
}

void printMemUsage() {
	size_t free_byte;

	size_t total_byte;
	
	optix::cudaError_t cuda_status = optix::cudaMemGetInfo(&free_byte, &total_byte);

	if (optix::cudaSuccess != cuda_status) {

		printf("Error: cudaMemGetInfo fails, %s \n", optix::cudaGetErrorString(cuda_status));

		exit(1);

	}



	double free_db = (double)free_byte;

	double total_db = (double)total_byte;

	double used_db = total_db - free_db;

	printf("GPU memory usage: used = %f, free = %f MB, total = %f MB\n",

		used_db / 1024.0 / 1024.0, free_db / 1024.0 / 1024.0, total_db / 1024.0 / 1024.0);
}

void cudaPrint() {
	unsigned int numberOfDevices = 0;
	RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetDeviceCount(&numberOfDevices));
	std::cout << "Number of Devices = " << numberOfDevices << std::endl << std::endl;

	for (unsigned int i = 0; i < numberOfDevices; ++i)
	{
		char name[256];
		RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_NAME, sizeof(name), name));
		std::cout << "Device " << i << ": " << name << std::endl;

		int computeCapability[2] = { 0, 0 };
		RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY, sizeof(computeCapability), &computeCapability));
		std::cout << "  Compute Support: " << computeCapability[0] << "." << computeCapability[1] << std::endl;

		RTsize totalMemory = 0;
		RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_TOTAL_MEMORY, sizeof(totalMemory), &totalMemory));
		std::cout << "  Total Memory: " << (unsigned long long) totalMemory << std::endl;

		int clockRate = 0;
		RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_CLOCK_RATE, sizeof(clockRate), &clockRate));
		std::cout << "  Clock Rate: " << clockRate << " Hz" << std::endl;

		int maxThreadsPerBlock = 0;
		RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_MAX_THREADS_PER_BLOCK, sizeof(maxThreadsPerBlock), &maxThreadsPerBlock));
		std::cout << "  Max. Threads per Block: " << maxThreadsPerBlock << std::endl;

		int smCount = 0;
		RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_MULTIPROCESSOR_COUNT, sizeof(smCount), &smCount));
		std::cout << "  Streaming Multiprocessor Count: " << smCount << std::endl;

		int executionTimeoutEnabled = 0;
		RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_EXECUTION_TIMEOUT_ENABLED, sizeof(executionTimeoutEnabled), &executionTimeoutEnabled));
		std::cout << "  Execution Timeout Enabled: " << executionTimeoutEnabled << std::endl;

		int maxHardwareTextureCount = 0;
		RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_MAX_HARDWARE_TEXTURE_COUNT, sizeof(maxHardwareTextureCount), &maxHardwareTextureCount));
		std::cout << "  Max. Hardware Texture Count: " << maxHardwareTextureCount << std::endl;

		int tccDriver = 0;
		RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_TCC_DRIVER, sizeof(tccDriver), &tccDriver));
		std::cout << "  TCC Driver enabled: " << tccDriver << std::endl;

		int cudaDeviceOrdinal = 0;
		RT_CHECK_ERROR_NO_CONTEXT(rtDeviceGetAttribute(i, RT_DEVICE_ATTRIBUTE_CUDA_DEVICE_ORDINAL, sizeof(cudaDeviceOrdinal), &cudaDeviceOrdinal));
		std::cout << "  CUDA Device Ordinal: " << cudaDeviceOrdinal << std::endl << std::endl;
	}
}

void setupDearIMGUI(GLFWwindow *window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");
}

int main() {
	
	clock_t start = clock();
	std::vector<short> normal_x, normal_y, normal2_x, normal2_y;
	std::vector<unsigned char> use_intensities, use_intensities2;
	WifiData wifi;
	int dialation = 1;
	int num_smooths = 1;
	//std::string filename = "sphere_scaled512";
	std::string filename = "umd-secure-cubic";
	wifi.loadBinary((filename + ".raw").c_str(), use_intensities, normal_x, normal_y);
	WifiData wifi2;
	//wifi2.loadBinary("sphere_scaled512.raw", use_intensities2, normal2_x, normal2_y);
	if (BENCHMARK) {
		std::cout << "Loading Data: " << (start - clock()) / CLOCKS_PER_SEC << " seconds" << std::endl;
		start = clock();
	}
	
	std::mt19937::result_type seed = time(0);
	auto generator = std::bind(std::uniform_real_distribution<float>(-1, 1),
		std::mt19937(seed));  // mt19937 is a standard mersenne_twister_engin
	glfwInit();
	glfwSetErrorCallback(error_callback);
	Window w = Window("Better Window", resolution.x, resolution.y);
	
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) //load GLAD
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}
	setupDearIMGUI(w.window);
	// During init, enable debug output
	glEnable(GL_DEBUG_OUTPUT);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	glDebugMessageCallback(MessageCallback, 0);

	w.SetFramebuferSizeCallback();
	glfwSwapInterval(0);
	if (BENCHMARK) {
		std::cout << "GLFW INIT " << ((double)(clock() - start)) / CLOCKS_PER_SEC << " seconds" << std::endl;
		start = clock();
	}
	ShaderProgram sp = ShaderProgram({ShaderProgram::Shaders::FRAGMENT, ShaderProgram::Shaders::VERTEX});
	ShaderProgram campus_map_sp = ShaderProgram({ShaderProgram::Shaders::NO_LIGHT_FRAG, ShaderProgram::Shaders::NO_LIGHT_VERT});
	ShaderProgram screen_shader = ShaderProgram({ ShaderProgram::Shaders::SCREEN_FRAG, ShaderProgram::Shaders::SCREEN_VERT });
	ShaderProgram skybox_shader = ShaderProgram({ ShaderProgram::Shaders::SKY_FRAG, ShaderProgram::Shaders::SKY_VERT });
	ShaderProgram instance_shader = ShaderProgram({ ShaderProgram::Shaders::INSTANCE_FRAG, ShaderProgram::Shaders::INSTANCE_VERT });
	//mesh.SetData();
	//
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	std::vector<std::string> skybox_files;
	
	skybox_files.emplace_back("C:/Users/alrowden/source/repos/RowdenRender/RowdenRender/Content/Textures/Skyboxes/miramar_lf.png");
	skybox_files.emplace_back("C:/Users/alrowden/source/repos/RowdenRender/RowdenRender/Content/Textures/Skyboxes/miramar_rt.png");
	skybox_files.emplace_back("C:/Users/alrowden/source/repos/RowdenRender/RowdenRender/Content/Textures/Skyboxes/miramar_bk.png");
	skybox_files.emplace_back("C:/Users/alrowden/source/repos/RowdenRender/RowdenRender/Content/Textures/Skyboxes/miramar_ft.png");
	skybox_files.emplace_back("C:/Users/alrowden/source/repos/RowdenRender/RowdenRender/Content/Textures/Skyboxes/miramar_up.png");
	skybox_files.emplace_back("C:/Users/alrowden/source/repos/RowdenRender/RowdenRender/Content/Textures/Skyboxes/miramar_dn.png");


	Texture2D texture = Texture2D("Content\\Textures\\CampusMap.png");
	Texture2D skybox_tex = Texture2D(skybox_files);
	//texture.setTexParameterWrap(GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	Model model;
	LoadCampusModel(&model);
	model.getMeshes()[0]->SetUniformColor(glm::vec4(1, 1, 1, 1));
	Model skybox = Model("Content\\Models\\cube\\cube.obj");
	skybox.setModel();
	skybox.getMeshes().at(0)->setTexture(skybox_tex, 0);
	Model campusMap = Model("Content\\Models\\quad\\quad.obj");
	campusMap.setModel();
	campusMap.getMeshes().at(0)->setTexture(texture, 0);
	Model RayTraced = Model("Content\\Models\\quad\\quad_centered.obj");
	RayTraced.setModel();
	Model Tree = Model("Content\\Models\\tree_scaled.FBX");
	Tree.setModel();
	glm::mat4 transformation = glm::scale(glm::mat4(1), scale * glm::vec3(-1, 1, -1));// glm::scale(glm::mat4(1), glm::vec3(-0.256f, 0.3f, -0.388998f));
	
	struct TreeEntry {
		double lat, lon;
		int objID;
	};

	std::vector<TreeEntry *> trees;
	//load in tree positions
	std::ifstream tree_file = std::ifstream("Content/Data/plants.csv");
	if (!tree_file.is_open()) {
		std::cerr << "ERROR OPENING PLANTS.CSV\n" << std::endl;
	}
	std::string line;

	getline(tree_file, line);
	std::string header = line;

	while (!tree_file.eof())
	{
		TreeEntry* curr_tree = new TreeEntry();
		std::string IDstr;
		getline(tree_file, IDstr, ',');
		curr_tree->objID = atoi(IDstr.c_str());

		std::string skip;
		getline(tree_file, skip, ','); //S_ID
		getline(tree_file, skip, ','); //TAGID
		getline(tree_file, skip, ','); //CNAME1
		getline(tree_file, skip, ','); //CNAME2
		getline(tree_file, skip, ','); //CNAME3
		getline(tree_file, skip, ','); //GENUS
		getline(tree_file, skip, ','); //SPECIES
		getline(tree_file, skip, ','); //CULTIVAR

		std::string diameter_str;
		getline(tree_file, diameter_str, ',');
		//curr_tree->diameter = atof(diameter_str.c_str());

		std::string height_str;
		getline(tree_file, height_str, ',');
		//curr_tree->height = atof(height_str.c_str());

		getline(tree_file, skip, ','); //CRADAVG
		getline(tree_file, skip, ','); //TRUNKHEIGHT

		std::string lat_str, lon_str;
		getline(tree_file, lat_str, ',');
		getline(tree_file, lon_str);
		curr_tree->lat = atof(lat_str.c_str());
		curr_tree->lon = atof(lon_str.c_str());

		curr_tree->lon += 76.936594579149414130370132625103f;
		curr_tree->lat -= 38.990750300955419049842021195218f;
		trees.emplace_back(curr_tree);
		//std::cout << trees.size() << std::endl;
	}

	std::vector<glm::mat4> treeTransforms;

	//Create Tree transforms
	for (TreeEntry* treeEntry : trees)
	{
		if (treeTransforms.size() < 0)
			break;
		glm::mat4 transform = //glm::translate(glm::mat4(1), glm::vec3(20, 20, 1));
		glm::translate(glm::mat4(1), glm::vec3(treeEntry->lon * 3000000, treeEntry->lat * 3000000, 1.0f));
		treeTransforms.emplace_back(transform);
	}

	Tree.getMeshes().at(0)->SetInstanceTransforms(treeTransforms);
	if (BENCHMARK) {
		std::cout << "Model Loading " << ((double)(clock() - start)) / CLOCKS_PER_SEC << " seconds" << std::endl;
		start = clock();
	}
	/*
	WifiData wifi;

	std::vector<short> normal_x, normal_y, normal_z;
	std::vector<unsigned char> use_intensities;
	wifi.loadBinary("sphere_scaled512.raw", use_intensities, normal_x, normal_y, normal_z);
	*/

	std::vector<short> normals;//, normals2;
	normals.resize(2 * use_intensities.size());
	//normals2.resize(2 * use_intensities2.size());
	for (unsigned long i = 0; i < use_intensities.size(); i++) {
		normals[i * 2] = normal_x.at(i);
		normals[i * 2 + 1] = normal_y.at(i);
	}
	for (unsigned long i = 0; i < use_intensities2.size(); i++) {
		//normals2[i * 2] = normal2_x.at(i);
		//normals2[i * 2 + 1] = normal2_y.at(i);
	}
	time_t timer = time(NULL);
	struct tm local_time;
	localtime_s(&local_time, &timer);
	std::string foldername = "Output/Render_";
	char str[80];
	strftime(str, sizeof(str), "%b_%d_%y_%H_%M_%S", &local_time);
	foldername.append(str);

	if (!CreateDirectory(foldername.c_str(), NULL)) {
		std::cout << "directory creation failed" << std::endl;
	}
	Shape myShape;
	//makeVolumetricShapeGPU(&myShape, use_intensities, wifi, num_cells, .65f);
	cudaPrint();
#if(false)
		std::fstream out = std::fstream("wifi_data.raw", std::ios::binary | std::ios::out);

		for (int i = 0; i < use_intensities.size(); i++) {
			UINT8 test = (UINT8)(use_intensities.at(i) * 255);
			out << test;
		}
		out.close();
#endif	


	optix::Context context;
	createContext(context, w);
	glm::mat4 volume_transform = glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(0, 1, 0));
	//RTresult ret = rtBufferCreate(context->get(), RT_BUFFER_INPUT, &ray_buffer);
	ray_buffer = context->createBuffer(RT_BUFFER_INPUT);
	
	rtBufferSetFormat(ray_buffer->get(), RT_FORMAT_USER);
	rtBufferSetElementSize(ray_buffer->get(), sizeof(optix::float3));
	rtBufferSetSize2D(ray_buffer->get(), w.width, w.height);
	context["ray_buffer"]->setBuffer(ray_buffer);
	createGeometry(context, glm::vec3(wifi.numLatCells, wifi.numLonCells, wifi.numSlices), glm::mat4(1));
	context["fov"]->setFloat(glm::radians(90.0f));
	try {
		context->validate();
	}
	catch (optix::Exception e) {
		std::cerr << e.getErrorString() << std::endl;
	}
	printMemUsage();
	createOptixTextures(context, glm::vec3(wifi.numLatCells, wifi.numLonCells, wifi.numSlices), use_intensities, normals);
	//createOptixTextures(context, glm::vec3(wifi2.numLatCells, wifi2.numLonCells, wifi2.numSlices), use_intensities2, normals2, 2);
	//createOptixTextures(context, glm::vec3(wifi.numLatCells, wifi.numLonCells, wifi.numSlices), use_intensities);
	printMemUsage();
	float ambientStrength = .15f;
	glm::vec3 lightDir = normalize(-glm::vec3(0.5, 0.5, 0.5));
	float specularStrength = .4;
	float diffuseStrength = .45;
	float shininess = 256;

	context["ambientStrength"]->setFloat(ambientStrength);
	context["specularStrength"]->setFloat(specularStrength);
	context["diffuseStrength"]->setFloat(diffuseStrength);
	context["shininess"]->setFloat(shininess);

	//setup camera
	Camera camera = Camera(glm::vec3(25, 25, 50), glm::vec3(50, 49.999, 0), 90.0f, w.width/(float)w.height);
	//Camera camera = Camera(glm::vec3(61.5, 41.5, .5), glm::vec3(50, 49, 0), 90.0f, w.width / w.height);
	w.SetCamera(&camera);

	context["lightDir"]->setFloat(make_float3(lightDir));
	optix::float2 lightDirP = optix::make_float2(acos(lightDir.z), atan2(lightDir.y, lightDir.x));
	context["lightDirP"]->setFloat(lightDirP);
	glm::vec2 sincosLightTheta = glm::vec2(sin(lightDirP.x), cos(lightDirP.x));
	context["sincosLightTheta"]->setFloat(make_float2(sincosLightTheta));
	

	//sutil::displayBufferPPM("out.ppm", context["output_buffer"]->getBuffer());

	Mesh volume = Mesh(&myShape);

	Model vol = Model();
	vol.addMesh(&volume);
	vol.setModel();
	//Texture2D wifi_intensities = Texture2D(&pixels, wifi.numLonCells, wifi.numLatCells);
	//campusMap.getMeshes().at(0)->setTexture(wifi_intensities, 0);
	Texture2D hdr_texture = Texture2D();
	hdr_texture.setDims(w.height, w.width, 4);
	//Camera camera = Camera(glm::vec3(0, 10, 10), glm::vec3(0, 0, 0), 45.0f, 800/600.0f);

	glm::mat4 projection;
	//w.scale = glm::vec3(1, .03, 1);
	w.scale = glm::vec3(0, 0, 0);
	w.translate = glm::vec3(0,0,0);
	//w.translate = glm::vec3(0, -.1f, 0);

	w.setSpeed(.5 * 10);

	Lights lights = Lights();
	float toNorm = 1 / 255.0;
	lights.addPointLight(50.0f * glm::vec3(1, 1.5, 0), .1, .3, .003, toNorm * glm::vec3(255, 195, 12), toNorm * glm::vec3(255, 195, 12), toNorm * glm::vec3(255, 195, 12));
	lights.addPointLight(50.0f * glm::vec3(0, 1.5, 0), .1, .2, .003, toNorm * glm::vec3(121, 102, 162), toNorm * glm::vec3(121, 102, 162), toNorm * glm::vec3(121, 102, 162));
	//projection = glm::perspective(glm::radians(45.0f), 800/600.0f, 0.1f, 1000.0f);
	glm::mat4 light_transform = glm::translate(glm::mat4(1.0f), glm::vec3(3, 3, 3));

	glm::mat4 campusTransform;
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> look_ats;
	std::vector<float> distances;
	char char_buffer[100];
	char* next_token;
	if (animated) {
		positions.emplace_back(camera.getPosition());
		look_ats.emplace_back(camera.getPosition() + camera.getDirection());
		std::fstream animation = std::fstream(animation_file, std::ios::in);
		bool odd = false;
		while (animation.getline(char_buffer, 100)) {
			char* parts = strtok_s(char_buffer, ", ", &next_token);
			glm::vec3 vec;
			int axis = 0;
			while (parts != NULL) {
				vec[axis] = atof(parts);
				parts = strtok_s(NULL, ", ", &next_token);
				axis++;
			}
			if (odd) {
				look_ats.emplace_back(vec);
			}
			else {
				positions.emplace_back(vec);
			}
			odd = !odd;
		}
		positions.emplace_back(camera.getPosition());
		look_ats.emplace_back(camera.getPosition() + camera.getDirection());
		for (int i = 0; i < positions.size() - 1; i++) {
			distances.emplace_back(glm::distance(positions.at(i), positions.at(i + 1)));
		}
		animated = false;
	}
	glGenTextures(1, &depth_mask_id);
	glGenTextures(1, &random_texture_id);
	context["zFar"]->setFloat(1000.0f);
	context["zNear"]->setFloat(.1f);

	int fps = 0;
	uint64_t fps_counter = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	//uint64_t start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	unsigned long int num_frames = 0;
	if (BENCHMARK) {
		std::cout << "Setup OptiX " << (double)(clock() - start) / CLOCKS_PER_SEC << " seconds" << std::endl;
		start = clock();
	}
	//Rendering Parameters
	float center = .56;//.56; //.2075
	float width = .01;//.015
	float base_opac = 0;
	float bubble_top = .99;
	float bubble_bottom = .95;
	float bubble_max_opac = .1f;
	float bubble_min_opac = .025f;
	float spec_term = .05;
	float sil_term = .95;
	bool color_aug = false;
	float tune = 1.0f;
	float fov = 90;
	optix::float3 color = optix::make_float3(253 / 255.0f, 117 / 255.0f, 0 / 255.0f);
	while (!glfwWindowShouldClose(w.getWindow())) //main render loop
	{
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin("Rendering Terms");
		
		ImGui::SliderFloat("IsoVal Center", &center, 0.0f, 1.0f);
		ImGui::SliderFloat("IsoVal width", &width, 0.0f, fmin(center/2.0, 1-center/2.0));

		ImGui::SliderFloat("Base Opacity", &base_opac, 0.0f, 1.0f);
		ImGui::SliderFloat("Sillhoutte Term", &sil_term, 0.0f, 1.0f);
		ImGui::SliderFloat("bubble top", &bubble_top, 0.0f, 1.0f);
		ImGui::SliderFloat("bubble bottom", &bubble_bottom, 0.0f, bubble_top);
		ImGui::SliderFloat("bubble max opac", &bubble_max_opac, 0.0f, 1.0f);
		ImGui::SliderFloat("bubble min opac", &bubble_min_opac, 0.0f, bubble_max_opac);
		ImGui::SliderFloat("Debug", &tune, 0.0f, 1.0f);
		ImGui::Checkbox("Shade sillhouette", &color_aug);
		ImGui::SliderFloat("Specular Term", &spec_term, 0.0f, 1.0f);
		ImGui::SliderFloat("FOV", &fov, 0.0f, 90.0f);

		ImGui::ColorEdit3("Volume Base Color", &color.x);
		
		ImGui::End();

		context["IsoValRange"]->setFloat(optix::make_float2(center-width/2.0f, center+width/2.0f));
		context["ShadingTerms"]->setFloat(optix::make_float3(base_opac, color_aug ? sil_term: -sil_term, spec_term));
		context["BubbleTerms"]->setFloat(optix::make_float4(bubble_top, bubble_bottom, bubble_max_opac, bubble_min_opac));
		context["tune"]->setFloat(tune);
		context["color1"]->setFloat(color);
		camera.fov = fov;
		clock_t per_frame = clock();
		glEnable(GL_DEPTH_TEST);
		if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() - fps_counter < 1000) {
			fps++;
		}
		else {
			std::cout << fps << std::endl;
			fps = 0;
			fps_counter = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		}

		if (animated) {
			float distance = speed * num_frames * 1 / 60.0f;
			int i = 0;
			while (distance > distances.at(i)) {
				distance -= distances.at(i);
				i++;
				if (i == distances.size()) {
					animated = false;
					distance = 0;
					i = 0;
					//uint64_t start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

					return 0;
				}
			}

			float step = distance / distances.at(i);
			camera.setPosition(glm::lerp(positions.at(i), positions.at(i + 1), step));
			camera.setDirection(glm::lerp(look_ats.at(i) - positions.at(i), look_ats.at(i + 1) - positions.at(i + 1), step));
		}

		if (w.signal) {
			animated = true;
			start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			w.signal = false;
			update = true;

		}
		if (BENCHMARK) {
			std::cout << "Calculate FPS and Update Animation: " << ((double)(clock() - start)) / CLOCKS_PER_SEC << " seconds" << std::endl;
			start = clock();
		}
		//texture.Bind();
		
		//render(model, &sp);
		//render(light, &light_sp);

		glm::mat4 transformation = glm::translate(glm::mat4(1), glm::vec3(75, 40.7, .9));
		transformation = glm::scale(transformation,   glm::vec3(0.00996, 0.012782, 0.0155));// glm::scale(glm::mat4(1), glm::vec3(-0.256f, 0.3f, -0.388998f));
		//transformation = glm::rotate(transformation, glm::radians(180.0f), glm::vec3(0, 1, 0));

		campusTransform = glm::translate(glm::mat4(1), glm::vec3(0, 0, 0));
		campusTransform = glm::scale(campusTransform, scale * glm::vec3(100, 100, 100));


		skybox_shader.SetUniform4fv("projection", camera.getProjection());
		skybox_shader.SetUniform4fv("view", glm::mat4(glm::mat3(camera.getView())));

		glDepthMask(GL_FALSE);
		render(skybox, &skybox_shader);
		glDepthMask(GL_TRUE);
		if (BENCHMARK) {
			std::cout << "Render Skybox " << ((double)(clock() - start)) / CLOCKS_PER_SEC << " seconds" << std::endl;
			start = clock();
		}
		sp.SetUniform4fv("model", transformation);
		sp.SetUniform3fv("normalMatrix", glm::mat3(glm::transpose(glm::inverse(transformation * camera.getView()))));
		sp.SetUniform4fv("camera", camera.getView());
		sp.SetUniform4fv("projection", camera.getProjection());
		sp.SetLights(lights);
		sp.SetUniform3f("viewPos", camera.getPosition());
		render(model, &sp);
		if (BENCHMARK) {
			std::cout << "Render Campus Model " << ((double)(clock() - start)) / CLOCKS_PER_SEC << " seconds" << std::endl;
			start = clock();
		}
		campus_map_sp.SetUniform4fv("model", campusTransform);
		campus_map_sp.SetUniform4fv("camera", camera.getView());
		campus_map_sp.SetUniform4fv("projection", camera.getProjection());
		render(campusMap, &campus_map_sp);
		if (BENCHMARK) {
			std::cout << "Render Campus Map " << ((double)(clock() - start)) / CLOCKS_PER_SEC << " seconds" << std::endl;
			start = clock();
		}
		instance_shader.Use();
		instance_shader.SetUniform4fv("projection", camera.getProjection());
		instance_shader.SetUniform4fv("view", camera.getView());
		instance_shader.SetUniform4fv("transform", glm::scale(glm::translate(glm::mat4(1), glm::vec3(72.099, 63.9, 0) + w.translate), glm::vec3(.00095, .00159, .00129) + w.scale));
		render(Tree, &instance_shader);
		if (BENCHMARK) {
			std::cout << "Render Trees " << ((double)(clock() - start)) / CLOCKS_PER_SEC << " seconds" << std::endl;
			start = clock();
		}
		GLfloat* depths = new GLfloat[w.width * w.height];

		glReadPixels(0, 0, w.width, w.height, GL_DEPTH_COMPONENT, GL_FLOAT, depths);

		//glBindTexture(GL_TEXTURE_2D, depth_mask_id);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, w.width, w.height, 0, GL_RED, GL_FLOAT, (void*)depths);
		//glBindTexture(GL_TEXTURE_2D, 0);

		std::string filename;
		if (false) {
			BYTE* depth_pix = new BYTE[w.width * w.height];

			for (int i = 0; i < w.width * w.height; i++) {
				depth_pix[i] = (unsigned char)(depths[i] * 255.0f);
			}
		
			filename = std::string(foldername + "/");
			filename.append(std::to_string(num_frames));
			filename.append(".bmp");

			stbi_flip_vertically_on_write(true);
			int save_result = stbi_write_bmp
			(
				filename.c_str(),
				resolution.x, resolution.y,
				1, depth_pix
			);
			if (save_result == 0) {
				std::cout << "shit" << std::endl;
			}
		}

		
		glBindTexture(GL_TEXTURE_2D, depth_mask_id);
		if (num_frames == 0) {
			
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, resolution.x, resolution.y, 0, GL_RED, GL_FLOAT, (void*)depths);
			depth_mask = context->createTextureSamplerFromGLImage(depth_mask_id, RT_TARGET_GL_TEXTURE_2D);
			depth_mask->setFilteringModes(RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_NONE);
			depth_mask->setWrapMode(0, RT_WRAP_CLAMP_TO_EDGE);
			depth_mask->setWrapMode(1, RT_WRAP_CLAMP_TO_EDGE);
			context["depth_mask_id"]->setInt(depth_mask->getId());

			glBindTexture(GL_TEXTURE_3D, 0);
			
			
		}
		else {
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, resolution.x, resolution.y, GL_RED, GL_FLOAT, (void*)depths);
		}
		glBindTexture(GL_TEXTURE_2D, 0);
		if (BENCHMARK) {
			std::cout << "Update Depth Buffer: " << ((double)(clock() - start)) / CLOCKS_PER_SEC << " seconds" << std::endl;
			start = clock();
		}
		

		optix::float3  camera_eye = optix::make_float3(camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);
		
		optix::float3 camera_lookat = camera_eye - optix::make_float3(camera.getDirection().x, camera.getDirection().y, camera.getDirection().z);
		optix::float3 camera_up = optix::make_float3(camera.getUp().x, camera.getUp().y, camera.getUp().z);
		//updateCamera(w, context, camera_eye, camera_lookat, camera_up, optix::Matrix4x4().identity());
		updateCamera(w, context, camera);
		try {
			context->validate();
		}
		catch (optix::Exception e) {
			std::cerr << e.getErrorString() << std::endl;
		}
		//context["lightPos"]->setFloat(make_float3(camera.getPosition()));
		glm::vec3 CameraDir = (normalize(camera.getDirection()));
		context["CameraDir"]->setFloat(make_float3(CameraDir));
		glm::vec2 CameraDirP = glm::vec2(acos(CameraDir.z), atan2(CameraDir.y, CameraDir.x));
		context["CameraDirP"]->setFloat(make_float2(CameraDirP));
		glm::vec2 sincosCameraDir = glm::vec2(sin(CameraDir.x), cos(CameraDir.x));
		context["sincosCameraDir"]->setFloat(make_float2(sincosCameraDir));
		glm::vec3 halfwayVec = normalize(CameraDir + lightDir);
		context["HalfwayVec"]->setFloat(make_float3(halfwayVec));
		glm::vec2 halfwayVecP = glm::vec2(acos(halfwayVec.z), atan2(halfwayVec.y, halfwayVec.x));
		//std::cout << halfwayVecP.x << ", " << halfwayVecP.y << std::endl;
		context["HalfwayVecP"]->setFloat(make_float2(halfwayVecP));
		context["sincosHalfwayTheta"]->setFloat(optix::make_float2(sin(halfwayVecP.x), cos(halfwayVecP.x)));
		if (BENCHMARK) {
			std::cout << "Update OptiX " << ((double)(clock() - start)) / CLOCKS_PER_SEC << " seconds" << std::endl;
			start = clock();
		}
		try {
			if (update) {
				context->launch(0, resolution.x, resolution.y);
				//update = false;
			}
		}
		catch (optix::Exception e) {
			std::cout << e.getErrorString() << std::endl;
		}
		if (BENCHMARK) {
			std::cout << "Optix Render " << ((double)(clock() - start)) / CLOCKS_PER_SEC << " seconds" << std::endl;
			start = clock();
		}
		hdr_texture.SetTextureID(optixBufferToGLTexture(amplitude_buffer));
		RayTraced.getMeshes().at(0)->setTexture(hdr_texture, 0);
		delete[] depths;
		

		glDisable(GL_DEPTH_TEST);
		screen_shader.Use();
		render(RayTraced, &screen_shader);
		if (BENCHMARK) {
			std::cout << "Render Volume to Screen " << ((double)(clock() - start)) / CLOCKS_PER_SEC << " seconds" << std::endl;
			start = clock();
		}
		filename = std::string(foldername + "/");
		filename.append(std::to_string(num_frames++));
		filename.append(".bmp");
		void* data;
		if (false) {
			// Make the BYTE array, factor of 3 because it's RBG.
			BYTE* pixels = new BYTE[3 * w.width * w.height];

			glReadPixels(0, 0, w.width, w.height, GL_RGB, GL_UNSIGNED_BYTE, pixels);
			stbi_flip_vertically_on_write(true);
			int save_result = stbi_write_bmp
			(
				filename.c_str(),
				resolution.x, resolution.y,
				3, pixels
			);
			if (save_result == 0) {
				std::cout << "shit" << std::endl;
			}
			delete[] pixels;
		}
		ImGui::Render();
		
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		//render(vol, &sp);
		w.ProcessFrame(&camera);
		if (BENCHMARK) {
			std::cout << "Full frame " << (double)((clock() - per_frame)) / CLOCKS_PER_SEC << " seconds" << std::endl;
			start = clock();
		}
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate(); //Shut it down!

	return 1;
} 