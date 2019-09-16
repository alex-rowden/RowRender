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

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#define DEBUG true
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

optix::Buffer amplitude_buffer;


GLuint volume_textureId, randInitPhase_textureId, transferFunction_textureId;
optix::TextureSampler volume_texture, transferFunction_texture;


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
		optix::float3 box_max = optix::make_float3(75, 75, 25);


		optix::float3 volume_v1 = optix::make_float3(50.0f, 0.f, 0.f);
		optix::float3 volume_v2 = optix::make_float3(0.f, 50.0f, 0.f);
		optix::float3 volume_v3 = optix::make_float3(0.f, 0.f, 25.0f);
		volume_v1 *= 1.0f / dot(volume_v1, volume_v1);
		volume_v2 *= 1.0f / dot(volume_v2, volume_v2);
		volume_v3 *= 1.0f / dot(volume_v3, volume_v3);
		box["box_min"]->setFloat(box_min);
		box["box_max"]->setFloat(box_max);
		box["v1"]->setFloat(volume_v1);
		box["v2"]->setFloat(volume_v2);
		box["v3"]->setFloat(volume_v3);
		box["voxel_size"]->setFloat(.01, .01, .01);

		box["scene_epsilon"]->setFloat(1e-4f);
		box["volumeRaytraceStepSize"]->setFloat(.11);

		context["box_min"]->setFloat(box_min);
		context["box_max"]->setFloat(box_max);
		context["v1"]->setFloat(volume_v1);
		context["v2"]->setFloat(volume_v2);
		context["v3"]->setFloat(volume_v3);
		context["volumeRaytraceStepSize"]->setFloat(.11);

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
	
		//geometrygroup->setAcceleration(context->createAcceleration("Trbvh"));
		geometrygroup->setAcceleration( context->createAcceleration("NoAccel") );

		context["top_object"]->set(geometrygroup);
	}
	catch (optix::Exception e) {
		std::cout << e.getErrorString() << std::endl;
	}
}

void createContext(optix::Context&context, Window&w) {
	context = optix::Context::create();
	std::vector<int> devices;
	devices.emplace_back(0);
	//context->setDevices(devices.begin(), devices.end());
	context->setRayTypeCount(1);
	context["radiance_ray_type"]->setUint(0);
	context->setEntryPointCount(1);
	context->setStackSize(2100);
	//context->setMaxTraceDepth(5);
	if (DEBUG) {
		context->setPrintEnabled(true);
		//m_context->setPrintLaunchIndex(256, 256);
		context->setExceptionEnabled(RT_EXCEPTION_ALL, true);
	}
	//context["max_depth"]->setInt(100);
	context["scene_epsilon"]->setFloat(1.e-4f);
	context["opacity_correction"]->setFloat(.65f);
	optix::float2 output_buffer_dim = optix::make_float2(resolution.x, resolution.y);
	// buffer to store locations and brightness of intersections
	//location_buffer = sutil::createOutputBuffer(context, RT_FORMAT_FLOAT3, output_buffer_dim.x, output_buffer_dim.y, false);
	amplitude_buffer = sutil::createOutputBuffer(context, RT_FORMAT_FLOAT4, output_buffer_dim.x, output_buffer_dim.y, false);
	//initPhase_buffer = sutil::createOutputBuffer(context, RT_FORMAT_FLOAT, output_buffer_dim.x, output_buffer_dim.y, false);
	//context["location_buffer"]->set(location_buffer);
	context["amplitude_buffer"]->set(amplitude_buffer);
	//context["initPhase_buffer"]->set(initPhase_buffer);
	float volumeRaytraceStepSize = .11;
	optix::float3 volume_size = optix::make_float3(.01f, .01f, .01f);
	float volumeDiagLength = sqrt(volume_size.x * volume_size.x + volume_size.y * volume_size.y + volume_size.z * volume_size.z);
	unsigned int maxCompositionSteps = (unsigned int)std::ceil(volumeDiagLength / volumeRaytraceStepSize);
	//optix::float3 compositionBufferSize = optix::make_float3(2048, std::floorf(100000000 / (sizeof(optix::float3) * maxCompositionSteps * 2048)), maxCompositionSteps);
	// composition buffer
	//compLocation_buffer = context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT3, compositionBufferSize.x, compositionBufferSize.y, compositionBufferSize.z);
	//compAmplitude_buffer = context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_FLOAT3, compositionBufferSize.x, compositionBufferSize.y, compositionBufferSize.z);
	//compDepth_buffer = context->createBuffer(RT_BUFFER_OUTPUT, RT_FORMAT_UNSIGNED_INT, compositionBufferSize.x, compositionBufferSize.y);
	//context["compLocation_buffer"]->set(compLocation_buffer);
	//context["compAmplitude_buffer"]->set(compAmplitude_buffer);
	//context["compDepth_buffer"]->set(compDepth_buffer);
	//context["compositionBufferSize"]->setUint(compositionBufferSize.x, compositionBufferSize.y, compositionBufferSize.z);

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

void createOptixTextures(optix::Context& context, glm::vec3 volume_size, std::vector<float> volumeRaw) {
	try {
		
		glGenTextures(1, &volume_textureId);
		glBindTexture(GL_TEXTURE_3D, volume_textureId);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, volume_size.x, volume_size.y, volume_size.z, 0, GL_RED, GL_FLOAT, (void*)& volumeRaw[0]);
		glBindTexture(GL_TEXTURE_3D, 0);
		// create optix 3D texture sampler
		volume_texture = context->createTextureSamplerFromGLImage(volume_textureId, RT_TARGET_GL_TEXTURE_3D);
		volume_texture->setFilteringModes(RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_NONE);
		volume_texture->setWrapMode(0, RT_WRAP_CLAMP_TO_EDGE);
		volume_texture->setWrapMode(1, RT_WRAP_CLAMP_TO_EDGE);
		volume_texture->setWrapMode(2, RT_WRAP_CLAMP_TO_EDGE);
		context["volumeTextureId"]->setInt(volume_texture->getId());
		/*
		// 2. random initial phase texture
		// assume the size of the texture is 4 x num_rays_per_ele_hg
		optix::uint2 num_rays_per_ele_hg = optix::make_uint2(1024, 1024);
		optix::uint2 randPhaseTextureSize = 4 * num_rays_per_ele_hg;
		float* randPhaseData = new float[randPhaseTextureSize.x * randPhaseTextureSize.y];
		for (unsigned int i = 0; i < randPhaseTextureSize.x * randPhaseTextureSize.y; ++i) {
			randPhaseData[i] = std::rand() * 2.f * MY_PI;
		}
		glGenTextures(1, &randInitPhase_textureId);
		glBindTexture(GL_TEXTURE_2D, randInitPhase_textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, randPhaseTextureSize.x, randPhaseTextureSize.y, 0, GL_RED, GL_FLOAT, (void*)randPhaseData);
		glBindTexture(GL_TEXTURE_2D, 0);
		delete[] randPhaseData;
		// create optix 2D texture sampler
		randInitPhase_texture = context->createTextureSamplerFromGLImage(randInitPhase_textureId, RT_TARGET_GL_TEXTURE_2D);
		randInitPhase_texture->setFilteringModes(RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_NONE);
		randInitPhase_texture->setWrapMode(0, RT_WRAP_REPEAT);
		randInitPhase_texture->setWrapMode(1, RT_WRAP_REPEAT);
		context["initPhaseTextureId"]->setInt(randInitPhase_texture->getId());
		*/
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
				transferFunction.emplace_back( a/3.0f);
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

		// 4. random ray direction jitter texture
		/*
		glGenTextures(1, &random_texture_id);
		glBindTexture(GL_TEXTURE_2D, random_texture_id);
		float* randData = new float[4 * num_rays_per_ele_hg.x * num_rays_per_ele_hg.y];
		for (unsigned int j = 0; j < 4 * num_rays_per_ele_hg.x * num_rays_per_ele_hg.y; ++j) {
			randData[j] = std::rand();	//[0,1]
		}
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, num_rays_per_ele_hg.x, num_rays_per_ele_hg.y, 0, GL_RGBA, GL_FLOAT, (void*)randData);
		delete[] randData;
		// create optix 2D texture sampler
		random_texture = context->createTextureSamplerFromGLImage(random_texture_id, RT_TARGET_GL_TEXTURE_2D);
		random_texture->setFilteringModes(RT_FILTER_LINEAR, RT_FILTER_LINEAR, RT_FILTER_NONE);
		random_texture->setWrapMode(0, RT_WRAP_REPEAT);
		random_texture->setWrapMode(1, RT_WRAP_REPEAT);
		glBindTexture(GL_TEXTURE_2D, 0);
		context["random_texture"]->setInt(random_texture->getId());
		*/
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
	
completeCampus->addModel(new Model("Content/Models/Buildings/campus.fbx"));
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

void makeVolumetricShapeGPU(Shape* shape, std::vector<float> intensities, WifiData wifiData, int num_cells, float isoVal) {
	int numLonCells = wifiData.numLonCells;
	int numLatCells = wifiData.numLatCells;
	int index = 0;
	int threads = 1024;
	int h = 1024 / 4;
	int w = 1024 / 4;

	for (int h = 0; h < num_cells; h++) {
		for (int i = 0; i < numLonCells; i++) {
			for (int j = 0; j < numLatCells; j++) {
				float field[8];

				field[0] = getIntensity(intensities, numLonCells * numLatCells * num_cells, get3DIndex(i, j, h, numLatCells, numLonCells, num_cells));
				field[1] = getIntensity(intensities, numLonCells * numLatCells * num_cells, get3DIndex(i + 1, j, h, numLatCells, numLonCells, num_cells));
				field[2] = getIntensity(intensities, numLonCells * numLatCells * num_cells, get3DIndex(i + 1, j + 1, h, numLatCells, numLonCells, num_cells));
				field[3] = getIntensity(intensities, numLonCells * numLatCells * num_cells, get3DIndex(i, j + 1, h, numLatCells, numLonCells, num_cells));
				field[4] = getIntensity(intensities, numLonCells * numLatCells * num_cells, get3DIndex(i, j, h + 1, numLatCells, numLonCells, num_cells));
				field[5] = getIntensity(intensities, numLonCells * numLatCells * num_cells, get3DIndex(i + 1, j, h + 1, numLatCells, numLonCells, num_cells));
				field[6] = getIntensity(intensities, numLonCells * numLatCells * num_cells, get3DIndex(i + 1, j + 1, h + 1, numLatCells, numLonCells, num_cells));
				field[7] = getIntensity(intensities, numLonCells * numLatCells * num_cells, get3DIndex(i, j + 1, h + 1, numLatCells, numLonCells, num_cells));


				unsigned int cube_index = 0;
				for (int bit = 0; bit < 8; bit++) {
					cube_index += unsigned int(field[bit] < isoVal) << bit;
				}
				glm::vec3 v[8];
				v[0] = glm::vec3(i, j, h);
				v[1] = glm::vec3(i + 1, j, h);
				v[2] = glm::vec3(i + 1, j + 1, h);
				v[3] = glm::vec3(i, j + 1, h);
				v[4] = glm::vec3(i, j, h + 1);
				v[5] = glm::vec3(i + 1, j, h + 1);
				v[6] = glm::vec3(i + 1, j + 1, h + 1);
				v[7] = glm::vec3(i, j + 1, h + 1);

				glm::vec3 vertlist[12];
				vertlist[0] = vertexInterp(isoVal, v[0], v[1], field[0], field[1]);
				vertlist[1] = vertexInterp(isoVal, v[1], v[2], field[1], field[2]);
				vertlist[2] = vertexInterp(isoVal, v[2], v[3], field[2], field[3]);
				vertlist[3] = vertexInterp(isoVal, v[3], v[0], field[3], field[0]);

				vertlist[4] = vertexInterp(isoVal, v[4], v[5], field[4], field[5]);
				vertlist[5] = vertexInterp(isoVal, v[5], v[6], field[5], field[6]);
				vertlist[6] = vertexInterp(isoVal, v[6], v[7], field[6], field[7]);
				vertlist[7] = vertexInterp(isoVal, v[7], v[4], field[7], field[4]);

				vertlist[8] = vertexInterp(isoVal, v[0], v[4], field[0], field[4]);
				vertlist[9] = vertexInterp(isoVal, v[1], v[5], field[1], field[5]);
				vertlist[10] = vertexInterp(isoVal, v[2], v[6], field[2], field[6]);
				vertlist[11] = vertexInterp(isoVal, v[3], v[7], field[3], field[7]);

				int num_verts = wifiData.numVertsTable[cube_index];
				for (int i = 0; i < num_verts; i += 3)
				{
					glm::vec3 v[3];
					v[0] = vertlist[wifiData.triTable[cube_index][i]];
					v[1] = vertlist[wifiData.triTable[cube_index][i + 1]];
					v[2] = vertlist[wifiData.triTable[cube_index][i + 2]];
					shape->addVertex(v[0]);
					shape->addVertex(v[1]);
					shape->addVertex(v[2]);
					shape->addTexCoord(glm::vec2(0, 0));
					shape->addTexCoord(glm::vec2(1, 0));
					shape->addTexCoord(glm::vec2(0, 1));
					shape->addNormal(calcNormal(v[0], v[1], v[2]));
					shape->addNormal(calcNormal(v[0], v[1], v[2]));
					shape->addNormal(calcNormal(v[0], v[1], v[2]));
					shape->addIndex(index, index + 1, index + 2);

					index += 3;
				}
			}
		}
	}
}

void makeVolumetricShape(Shape* shape, std::vector<float> intensities, WifiData wifiData, int num_cells, float isoVal) {
	int numLonCells = wifiData.numLonCells;
	int numLatCells = wifiData.numLatCells;
	int index = 0;
	int threads = 1024;
	int h = 1024 / 4;
	int w = 1024 / 4;

	for (int h = 0; h < num_cells; h++) {
		for (int i = 0; i < numLonCells; i++) {
			for (int j = 0; j < numLatCells; j++) {
				float field[8];

				field[0] = getIntensity(intensities, numLonCells * numLatCells * num_cells, get3DIndex(i, j, h, numLatCells, numLonCells, num_cells));
				field[1] = getIntensity(intensities, numLonCells * numLatCells * num_cells, get3DIndex(i + 1, j, h, numLatCells, numLonCells, num_cells));
				field[2] = getIntensity(intensities, numLonCells * numLatCells * num_cells, get3DIndex(i + 1, j + 1, h, numLatCells, numLonCells, num_cells));
				field[3] = getIntensity(intensities, numLonCells * numLatCells * num_cells, get3DIndex(i, j + 1, h, numLatCells, numLonCells, num_cells));
				field[4] = getIntensity(intensities, numLonCells * numLatCells * num_cells, get3DIndex(i, j, h + 1, numLatCells, numLonCells, num_cells));
				field[5] = getIntensity(intensities, numLonCells * numLatCells * num_cells, get3DIndex(i + 1, j, h + 1, numLatCells, numLonCells, num_cells));
				field[6] = getIntensity(intensities, numLonCells * numLatCells * num_cells, get3DIndex(i + 1, j + 1, h + 1, numLatCells, numLonCells, num_cells));
				field[7] = getIntensity(intensities, numLonCells * numLatCells * num_cells, get3DIndex(i, j + 1, h + 1, numLatCells, numLonCells, num_cells));


				unsigned int cube_index = 0;
				for (int bit = 0; bit < 8; bit++) {
					cube_index += unsigned int(field[bit] < isoVal) << bit;
				}
				glm::vec3 v[8];
				v[0] = glm::vec3(i, j, h);
				v[1] = glm::vec3(i + 1, j, h);
				v[2] = glm::vec3(i + 1, j + 1, h);
				v[3] = glm::vec3(i, j + 1, h);
				v[4] = glm::vec3(i, j, h + 1);
				v[5] = glm::vec3(i + 1, j, h + 1);
				v[6] = glm::vec3(i + 1, j + 1, h + 1);
				v[7] = glm::vec3(i, j + 1, h + 1);

				glm::vec3 vertlist[12];
				vertlist[0] = vertexInterp(isoVal, v[0], v[1], field[0], field[1]);
				vertlist[1] = vertexInterp(isoVal, v[1], v[2], field[1], field[2]);
				vertlist[2] = vertexInterp(isoVal, v[2], v[3], field[2], field[3]);
				vertlist[3] = vertexInterp(isoVal, v[3], v[0], field[3], field[0]);

				vertlist[4] = vertexInterp(isoVal, v[4], v[5], field[4], field[5]);
				vertlist[5] = vertexInterp(isoVal, v[5], v[6], field[5], field[6]);
				vertlist[6] = vertexInterp(isoVal, v[6], v[7], field[6], field[7]);
				vertlist[7] = vertexInterp(isoVal, v[7], v[4], field[7], field[4]);

				vertlist[8] = vertexInterp(isoVal, v[0], v[4], field[0], field[4]);
				vertlist[9] = vertexInterp(isoVal, v[1], v[5], field[1], field[5]);
				vertlist[10] = vertexInterp(isoVal, v[2], v[6], field[2], field[6]);
				vertlist[11] = vertexInterp(isoVal, v[3], v[7], field[3], field[7]);

				int num_verts = wifiData.numVertsTable[cube_index];
				for (int i = 0; i < num_verts; i+=3)
				{
					glm::vec3 v[3];
					v[0] = vertlist[wifiData.triTable[cube_index][i]];
					v[1] = vertlist[wifiData.triTable[cube_index][i + 1]];
					v[2] = vertlist[wifiData.triTable[cube_index][i + 2]];
					shape->addVertex(v[0]);
					shape->addVertex(v[1]);
					shape->addVertex(v[2]);
					shape->addNormal(calcNormal(v[0], v[1], v[2]));
					shape->addNormal(calcNormal(v[0], v[1], v[2]));
					shape->addNormal(calcNormal(v[0], v[1], v[2]));
					shape->addIndex(index, index + 1, index + 2);

					index += 3;
				}
			}
		}
	}
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
	const optix::Matrix4x4 trans = frame * camera_rotate * camera_rotate * frame_inv;

	camera_eye = optix::make_float3(trans * make_float4(camera_eye, 1.0f));
	camera_lookat = optix::make_float3(trans * make_float4(camera_lookat, 1.0f));
	camera_up = optix::make_float3(trans * make_float4(camera_up, 0.0f));

	sutil::calculateCameraVariables(
		camera_eye, camera_lookat, camera_up, vfov, aspect_ratio,
		camera_u, camera_v, camera_w, true);

	camera_rotate = optix::Matrix4x4::identity();
	

	context["eye"]->setFloat(camera_eye);
	context["U"]->setFloat(camera_u);
	context["V"]->setFloat(camera_v);
	context["W"]->setFloat(camera_w);
}


int main() {
	glfwInit();
	glfwSetErrorCallback(error_callback);
	Window w = Window("Better Window", resolution.y, resolution.x);
	
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) //load GLAD
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}
	// During init, enable debug output
	glEnable(GL_DEBUG_OUTPUT);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	glDebugMessageCallback(MessageCallback, 0);

	w.SetFramebuferSizeCallback();
	glfwSwapInterval(0);
	//Shape cube = Shape(Shape::PREMADE::CUBE);
	

	//Mesh mesh = Mesh(&cube);


	ShaderProgram sp = ShaderProgram({ShaderProgram::Shaders::FRAGMENT, ShaderProgram::Shaders::VERTEX});
	ShaderProgram campus_map_sp = ShaderProgram({ShaderProgram::Shaders::NO_LIGHT_FRAG, ShaderProgram::Shaders::NO_LIGHT_VERT});
	ShaderProgram screen_shader = ShaderProgram({ ShaderProgram::Shaders::SCREEN_FRAG, ShaderProgram::Shaders::SCREEN_VERT });
	ShaderProgram skybox_shader = ShaderProgram({ ShaderProgram::Shaders::SKY_FRAG, ShaderProgram::Shaders::SKY_VERT });

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


	Texture2D texture = Texture2D("Content\\Textures\\campusMapSat.png");
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
	glm::mat4 transformation = glm::scale(glm::mat4(1), scale * glm::vec3(-1, 1, -1));// glm::scale(glm::mat4(1), glm::vec3(-0.256f, 0.3f, -0.388998f));
	/*
	struct TreeEntry {
		double lat, lon, height, diameter;
		int objID;
	};

	std::vector<TreeEntry *> trees;
	//load in tree positions
	std::ifstream tree_file = std::ifstream("Content/plants.csv");
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
		curr_tree->diameter = atof(diameter_str.c_str());

		std::string height_str;
		getline(tree_file, height_str, ',');
		curr_tree->height = atof(height_str.c_str());

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
		glm::mat4 transform =
			glm::translate(glm::mat4(1), glm::vec3(-treeEntry->lon * 3000.0, 0, treeEntry->lat * 3000.0f)) *
			//glm::rotate((float)DEG2RAD(-90.0f), glm::vec3(1, 0, 0)) *
			glm::scale(glm::mat4(4), glm::vec3(-0.0009f, 0.0009f, 0.0009f));
		treeTransforms.emplace_back(transform);
	}
	*/

	WifiData wifi;

	std::vector<float> use_intensities;
	wifi.loadBinary("interp.raw", use_intensities);
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
		std::cout << "  Clock Rate: " << clockRate << " kHz" << std::endl;

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
#if(false)
	std::fstream out = std::fstream("wifi_data.raw", std::ios::binary|std::ios::out);

	for (int i = 0; i < use_intensities.size(); i++) {
		UINT8 test = (UINT8)(use_intensities.at(i) * 255);
		out << test;
	}
	out.close();
#endif	

	GLuint output_buffer = 0;
	glGenBuffers(1, &output_buffer);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, output_buffer);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, 4 * w.width * w.height * sizeof(float), 0, GL_STREAM_READ);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	optix::Context context;
	createContext(context, w);
	glm::mat4 volume_transform = glm::rotate(glm::mat4(1), glm::radians(90.0f), glm::vec3(0, 1, 0));
	createGeometry(context, glm::vec3(wifi.numLatCells, wifi.numLonCells, wifi.numSlices), glm::mat4(1));
	try {
		context->validate();
	}
	catch (optix::Exception e) {
		std::cerr << e.getErrorString() << std::endl;
	}
	createOptixTextures(context, glm::vec3(wifi.numLatCells, wifi.numLonCells, wifi.numSlices), use_intensities);
	//createOptixTextures(context, glm::vec3(wifi.numLatCells, wifi.numLonCells, wifi.numSlices), use_intensities);

	float ambientStrength = .1;
	optix::float3 lightPos = optix::make_float3(0.5, 0.5, 0.5);
	float specularStrength = .1;
	float shininess = 16;

	context["ambientStrength"]->setFloat(ambientStrength);
	
	context["specularStrength"]->setFloat(specularStrength);
	context["shininess"]->setFloat(shininess);
	//setup camera
	Camera camera = Camera(glm::vec3(50, 50, 50), glm::vec3(50, 49, 0), 90.0f, w.width/w.height);
	w.SetCamera(&camera);

	context["lightPos"]->setFloat(optix::make_float3(25, 25, 25));

	
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
	w.scale = .01f * glm::vec3(1.3, 1.6, 2.4);
	w.translate = glm::vec3(54, 45.2, 2.9);
	//w.translate = glm::vec3(0, -.1f, 0);
	
	w.setSpeed(.5 * 10);
	
	Lights lights = Lights();
	float toNorm = 1 / 255.0;
	lights.addPointLight(50.0f * glm::vec3(2, 1.5, 0), .1, .3, .003, toNorm * glm::vec3(255, 195, 12), toNorm * glm::vec3(255, 195, 12), toNorm * glm::vec3(255, 195, 12));
	lights.addPointLight(0.0f *  glm::vec3(-2, 1.5, 0), .1, .2, .003, toNorm * glm::vec3(121, 102, 162), toNorm * glm::vec3(121, 102, 162), toNorm * glm::vec3(121, 102, 162));
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
			char *parts = strtok_s(char_buffer, ", ", &next_token);
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
		animated = true;
	}
	
	int fps = 0;
	uint64_t fps_counter = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	uint64_t start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	unsigned long int num_frames = 0;
	while (!glfwWindowShouldClose(w.getWindow())) //main render loop
	{
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
			float distance = speed * num_frames * 1/60.0f;
			int i = 0;
			while (distance > distances.at(i)) {
				distance -= distances.at(i);
				i++;
				if (i == distances.size()) {
					animated = false;
					distance = 0;
					i = 0;
					break;
				}
			}
			
			float step = distance / distances.at(i);
			camera.setPosition(glm::lerp(positions.at(i), positions.at(i + 1), step));
			camera.setDirection(glm::lerp(look_ats.at(i) - positions.at(i), look_ats.at(i + 1) - positions.at(i+1), step));
		}

		if (w.signal) {
			animated = true;
			start = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
			w.signal = false;
			update = true;

		}
		optix::float3  camera_eye = optix::make_float3(camera.getPosition().x, camera.getPosition().y, camera.getPosition().z);
		optix::float3 camera_lookat = camera_eye - optix::make_float3(camera.getDirection().x, camera.getDirection().y, camera.getDirection().z);
		optix::float3 camera_up = optix::make_float3(0.0f, 0.0f, 1.0f);
		updateCamera(w, context, camera_eye, camera_lookat, camera_up, optix::Matrix4x4().identity());
		try {
			if (update) {
				context->launch(0, resolution.x, resolution.y);
				//update = false;
			}
		}
		catch (optix::Exception e) {
			std::cout << e.getErrorString() << std::endl;
		}
		hdr_texture.SetTextureID(optixBufferToGLTexture(amplitude_buffer));
		RayTraced.getMeshes().at(0)->setTexture(hdr_texture, 0);
		
		
		//texture.Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//render(model, &sp);
		//render(light, &light_sp);

		glm::mat4 transformation = glm::translate(glm::mat4(1), w.translate);
		transformation = glm::scale(transformation, scale * w.scale);// glm::scale(glm::mat4(1), glm::vec3(-0.256f, 0.3f, -0.388998f));
		//transformation = glm::rotate(transformation, glm::radians(180.0f), glm::vec3(0, 1, 0));

		


		campusTransform = glm::translate(glm::mat4(1), glm::vec3(0,0,0));
		campusTransform = glm::scale(campusTransform, scale * glm::vec3(100,100,100));
		
		
		skybox_shader.SetUniform4fv("projection", camera.getProjection());
		skybox_shader.SetUniform4fv("view", glm::mat4(glm::mat3(camera.getView())));
		
		glDepthMask(GL_FALSE);
		render(skybox, &skybox_shader);
		glDepthMask(GL_TRUE);
		sp.SetUniform4fv("model", transformation);
		sp.SetUniform3fv("normalMatrix", glm::mat3(glm::transpose(glm::inverse(transformation* camera.getView()))));
		sp.SetUniform4fv("camera", camera.getView());
		sp.SetUniform4fv("projection", camera.getProjection());
		sp.SetLights(lights);
		sp.SetUniform3f("viewPos", camera.getPosition());
		render(model, &sp);
		campus_map_sp.SetUniform4fv("model", campusTransform);
		campus_map_sp.SetUniform4fv("camera", camera.getView());
		campus_map_sp.SetUniform4fv("projection", camera.getProjection());
		render(campusMap, &campus_map_sp);

		GLfloat* depths = new GLfloat[w.width * w.height];

		glReadPixels(0, 0, w.width, w.height, GL_DEPTH_COMPONENT, GL_FLOAT, depths);

		BYTE* depth_pix = new BYTE[w.width * w.height];

		for (int i = 0; i < w.width * w.height; i++) {
			depth_pix[i] = (unsigned char)(depths[i] * 255.0f);
		}
		std::string filename = std::string(foldername + "/");
		filename.append(std::to_string(num_frames));
		filename.append(".bmp");
		
		stbi_flip_vertically_on_write(true);
		//int save_result = stbi_write_bmp
		//(
		//	filename.c_str(),
		//	resolution.x, resolution.y,
		//	1, depth_pix
		//);
		//if (save_result == 0) {
		//	std::cout << "shit" << std::endl;
		//}
		
		
		glDisable(GL_DEPTH_TEST);
		screen_shader.Use();
		render(RayTraced, &screen_shader);
		filename = std::string(foldername + "/");
		filename.append(std::to_string(num_frames++));
		filename.append(".bmp");
		void* data;
		if (true) {
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
		}
		//render(vol, &sp);
		w.ProcessFrame(&camera);
	}
	glfwTerminate(); //Shut it down!

	return 0;
} 