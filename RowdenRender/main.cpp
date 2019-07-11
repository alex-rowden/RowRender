#pragma once
#include "RowRender.h"
#include "WifiData.h"
#include "Window.h"
#include "Shape.h"
#include "Mesh.h"
#include "ShaderProgram.h"
#include "Texture2D.h"
#include "Model.h"
#include <tinyxml2.h>

#include <fstream>
int counter = 10;
float increment = 0.05;
//any old render function
void render(Model mesh, ShaderProgram *sp) {
	if (counter > 100)
		counter -= 100;
	glClearColor(counter/100.0f, counter / 100.0f, counter / 100.0f, 1.0);
	
	mesh.Render(sp);
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
	if (type == 33361) {
		return;
	}
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
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
	
	if (false) {
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
				glm::translate(glm::mat4(1), glm::vec3(-lon * 2000, 0, lat * 2000)) *
				//glm::rotate((float)DEG2RAD(-90.0f), glm::vec3(0, 0, 1)) *
				glm::rotate(glm::mat4(1), (float)glm::radians(-90.0f), glm::vec3(1, 0, 0)) *
				//glm::rotate((float)DEG2RAD(90.0f), glm::vec3(0, 1, 0)) *
				glm::scale(glm::mat4(1), glm::vec3(-sx, -sy, sz) * scale);
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
					*/
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
				glm::scale(glm::mat4(1), glm::vec3(-sx, -sy, sz) * scale);
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
					*/
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
			*/
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
	glm::vec3 vertex;
	filestream.read((char*)& parts, sizeof(size_t));
	Shape* buildings = new Shape();
	for (size_t j = 0; j < parts; j++) {
		filestream.read((char*)& count, sizeof(size_t));
		for (size_t i = 0; i < count; i += 3) {
			filestream.read((char*)& vertex.x, sizeof(float));
			filestream.read((char*)& vertex.y, sizeof(float));
			filestream.read((char*)& vertex.z, sizeof(float));
			buildings->addVertex(vertex);
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
	completeCampus->addMesh(new Mesh(buildings));
	completeCampus->setModel();
	filestream.close();

	}
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

int main() {
	glfwInit();
	glfwSetErrorCallback(error_callback);
	Window w = Window("Better Window");
	
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) //load GLAD
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}
	// During init, enable debug output
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDebugMessageCallback(MessageCallback, 0);

	w.SetFramebuferSizeCallback();

	//Shape cube = Shape(Shape::PREMADE::CUBE);
	

	//Mesh mesh = Mesh(&cube);


	ShaderProgram sp = ShaderProgram({ShaderProgram::Shaders::FRAGMENT, ShaderProgram::Shaders::VERTEX});
	ShaderProgram light_sp = ShaderProgram({ShaderProgram::Shaders::LIGHT_FRAG, ShaderProgram::Shaders::LIGHT_VERT});
	

	//mesh.SetData();
	//
	Texture2D texture = Texture2D("Content\\Textures\\campusMapSat.png");
	//texture.setTexParameterWrap(GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	Model model;
	LoadCampusModel(&model);
	Model light = Model("Content\\Models\\cube\\cube.obj");
	light.setModel();
	Model campusMap = Model("Content\\Models\\cube\\cube.obj");
	campusMap.setModel();
	//campusMap.getMeshes().at(0)->setTexture(texture, 0);
	glm::mat4 transformation =  glm::scale(glm::mat4(1), glm::vec3(-0.256f, 0.3f, -0.388998f));

	WifiData wifi;

	wifi.loadCSV("Content/Data/EricWifi-2-4-19.csv");
	wifi.loadCSV("Content/Data/AlexWifi-2-4-19.csv");
	wifi.loadCSV("Content/Data/EricWifi-2-4-19(2).csv");
	wifi.loadCSV("Content/Data/AlexWifi-2-4-19(2).csv");
	wifi.loadCSV("Content/Data/EricWifi-2-5-19.csv");
	wifi.loadCSV("Content/Data/AlexWifi-2-5-19.csv");
	wifi.loadCSV("Content/Data/EricWifi-2-6-19.csv");
	wifi.loadCSV("Content/Data/AlexWifi-2-6-19.csv");
	wifi.loadCSV("Content/Data/EricWifi-2-7-19.csv");
	wifi.loadCSV("Content/Data/AlexWifi-2-26-19.csv");
	wifi.loadCSV("Content/Data/EricWifi-2-26-19.csv");
	wifi.loadCSV("Content/Data/AlexWifi-2-27-19.csv");
	wifi.loadCSV("Content/Data/EricWifi-2-27-19.csv");
	wifi.loadCSV("Content/Data/EricWifi-2-27-19(2).csv");
	
	wifi.Finalize(.0005f);
	wifi.ComputeIDIntensities("umd");

	float ** intensities = wifi.GetIDIntensities("umd");

	float maxIntensity = 0;

	int num_cells = 50;
	std::vector<glm::vec4> pixels;
	std::vector<float> use_intensities;
	pixels.resize(wifi.numLonCells * wifi.numLatCells * num_cells);
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

	int curr_height = (num_cells - 1) / 2.0;
	for (int h = 0; h < num_cells; h++) {
		for (int i = 0; i < wifi.numLonCells; i++)
		{
			for (int j = 0; j < wifi.numLatCells; j++)
			{
				float intensity = intensities[i][j] / maxIntensity;
				intensity = intensity - (increment * fabs(h - curr_height));
				//std::cout << increment * fabs(h - curr_height) << std::endl;
				if (intensity <= 0.0f)
				{
					pixels.at(j + wifi.numLatCells * i + wifi.numLatCells * wifi.numLonCells * h) = (glm::vec4(0, 0, 0, 0));
					use_intensities.at(j + wifi.numLatCells * i + wifi.numLatCells * wifi.numLonCells * h) = 0;
					//pixels.emplace_back(glm::vec4(0.5, 0.5, 0.5, 0.0));
				}
				else
				{
					glm::vec4 color = glm::vec4(getHeatMapColor(intensity), intensity);
					//glm::vec4 color = glm::vec4(0, 0, 1, 1) * (1.0f - intensity) +
					//	glm::vec4(1, 0, 0, 1) * intensity;
					pixels.at(j + wifi.numLatCells * i + wifi.numLatCells * wifi.numLonCells * h) = (color);
					use_intensities.at(j + wifi.numLatCells * i + wifi.numLatCells * wifi.numLonCells * h) = intensity;
					//pixels.emplace_back(color);
				}
			}
		}
	}
	Shape myShape;
	makeVolumetricShapeGPU(&myShape, use_intensities, wifi, num_cells, .65f);
#if(false)
	std::fstream out = std::fstream("wifi_data.raw", std::ios::binary|std::ios::out);

	for (int i = 0; i < use_intensities.size(); i++) {
		UINT8 test = (UINT8)(use_intensities.at(i) * 255);
		out << test;
	}
	out.close();
#endif	
	Mesh volume = Mesh(&myShape);

	Model vol = Model();
	vol.addMesh(&volume);
	vol.setModel();
	Texture2D wifi_intensities = Texture2D(&pixels, wifi.numLonCells, wifi.numLatCells);
	campusMap.getMeshes().at(0)->setTexture(wifi_intensities, 0);

	Camera camera = Camera(glm::vec3(0, 10, 10), glm::vec3(0, 0, 0), 45.0f, 800/600.0f);
	w.SetCamera(&camera);
	glm::mat4 projection;
	w.scale = glm::vec3(7.6151, .03, 6.21);
	glm::mat4 campusTransform = glm::scale(glm::mat4(1), w.scale);
	w.translate = glm::vec3(-.29, -.409, .1);
	campusTransform = glm::translate(campusTransform, w.translate);
	//projection = glm::perspective(glm::radians(45.0f), 800/600.0f, 0.1f, 1000.0f);
	glm::mat4 light_transform = glm::translate(glm::mat4(1.0f), glm::vec3(3, 3, 3));
	while (!glfwWindowShouldClose(w.getWindow())) //main render loop
	{
		//transformation = glm::translate(transformation, glm::vec3(0, 0, -3));
		//transformation = glm::rotate(transformation, glm::radians(10 * (float)glfwGetTime()), glm::vec3(.5f, 1.0f,0));

		sp.SetUniform4fv("model", transformation);
		sp.SetUniform3fv("normalMatrix", glm::mat3(glm::transpose(glm::inverse(transformation * camera.getView()))));
		sp.SetUniform4fv("camera", camera.getView());
		sp.SetUniform4fv("projection", camera.getProjection());
		sp.SetUniform3f("lightColor", glm::vec3(1, 1, 1));
		sp.SetUniform3f("viewPos", camera.getPosition());
		light_sp.SetUniform4fv("model",  light_transform);
		light_sp.SetUniform4fv("camera", camera.getProjection() * camera.getView());
		sp.SetUniform3f("lightPos", glm::vec3(3, 3, 3));
		
		//texture.Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//render(model, &sp);
		//render(light, &light_sp);
		campusTransform = glm::scale(glm::mat4(1), glm::vec3(.1f * glm::vec3(1, 1, .5)));
		campusTransform = glm::translate(campusTransform, w.translate);
		sp.SetUniform4fv("model", campusTransform);
		sp.SetUniform3fv("normalMatrix", glm::mat3(glm::transpose(glm::inverse(campusTransform * camera.getView()))));
		//render(campusMap, &sp);
		render(vol, &sp);
		w.ProcessFrame(&camera);
	}
	glfwTerminate(); //Shut it down!

	return 0;
} 