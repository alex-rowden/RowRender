#pragma once
#include "RowRender.h"

#include "Window.h"
#include "Shape.h"
#include "Mesh.h"
#include "ShaderProgram.h"
#include "Texture2D.h"
#include "Model.h"

#include <fstream>
int counter = 10;
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
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}

void LoadCampusModel(Model *completeCampus) {
	
	if (true) {
		completeCampus = new Model();
		int campusRunningIndex = 0;
		for (int i = 1; i <= 999; i++)
		{
			std::string value = std::to_string(i);
			if (value.length() == 1)
				value = "00" + value;
			else if (value.length() == 2)
				value = "0" + value;

			KroEngine::Model3D* building = new KroEngine::Model3D();
			bool loadedBuilding = building->LoadMesh(
				"Content/Models/Buildings/ID_" + value + ".dae");
			if (!loadedBuilding)
			{
				Debugger::Instance()->WriteLine("ERROR: Failed to load building");
				continue;
			}

			TiXmlDocument* buildingKML;
			std::string file = "Content/Models/Buildings/ID_" + value + ".kml";
			buildingKML = new TiXmlDocument(file.c_str());
			bool configLoaded = buildingKML->LoadFile();
			if (!configLoaded)
			{
				std::cerr << "Could not load file: '" << file << "' " << buildingKML->ErrorDesc() << std::endl;
				continue;
			}
			TiXmlHandle buildingKMLHandle(buildingKML);

			TiXmlElement* buildingXML =
				buildingKMLHandle.FirstChild("kml").FirstChild("Document").FirstChild("Placemark").FirstChild("Model").ToElement();
			TiXmlElement* locationXML = buildingXML->FirstChildElement("Location");
			TiXmlElement* longitude = locationXML->FirstChildElement("longitude");
			TiXmlElement* latitude = locationXML->FirstChildElement("latitude");

			std::string lonText = longitude->GetText();
			std::string latText = latitude->GetText();
			float lon = std::atof(lonText.c_str());
			float lat = std::atof(latText.c_str());

			//std::cout << lonText << " " << latText << std::endl;

			TiXmlElement* scaleXML = buildingXML->FirstChildElement("Scale");
			TiXmlElement* scaleX = scaleXML->FirstChildElement("x");
			TiXmlElement* scaleY = scaleXML->FirstChildElement("y");
			TiXmlElement* scaleZ = scaleXML->FirstChildElement("z");

			float sx = std::atof(scaleX->GetText());
			float sy = std::atof(scaleY->GetText());
			float sz = std::atof(scaleZ->GetText());

			//KroEngine::WorldObject* buildingObj = new KroEngine::WorldObject();
			//buildingObj->SetModel(building);
			//buildingObj->SetPosition(glm::vec3(lon, 0, lat));
			////buildingObj->SetRotation(glm::toQuat(glm::rotate((float)DEG2RAD(-90.0f), glm::vec3(1, 0, 0))));
			//buildingObj->SetScale(glm::vec3(sx, sy, sz) * 0.5f);
			lon += 76.936594579149414130370132625103f;
			lat -= 38.990750300955419049842021195218f;
			float scale = .0085;
			glm::mat4 transform =
				glm::translate(glm::vec3(-lon * 3000.0, 0, lat * 3000.0f)) *
				//glm::rotate((float)DEG2RAD(-90.0f), glm::vec3(0, 0, 1)) *
				glm::rotate((float)DEG2RAD(-90.0f), glm::vec3(1, 0, 0)) *
				//glm::rotate((float)DEG2RAD(90.0f), glm::vec3(0, 1, 0)) *
				glm::scale(glm::vec3(-scale, -scale, scale));

			std::vector<Mesh3D*> parts = building->GetParts();
			for (Mesh3D* part : parts)
			{
				std::vector<float> verts = part->GetVerticies();
				if (verts.size() == 0)
					continue;
				std::vector<unsigned int> indices = part->GetIndices();
				std::vector<float> normals = part->GetNormals();
				std::vector<float> tangents = part->GetTangents();
				std::vector<float> texCoords = part->GetTexCoords();

				for (int v = 0; v < verts.size(); v += 3)
				{
					completeCampus->AddVertex(
						glm::vec3(transform * glm::vec4(
							verts.at(v),
							verts.at(v + 1),
							verts.at(v + 2),
							1)
						)
					);
					completeCampus->AddNormal(
						glm::normalize(
							glm::vec3(glm::vec4(
								normals.at(v),
								normals.at(v + 1),
								normals.at(v + 2),
								1)
							)
						)
					);

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

					completeCampus->AddColor(glm::vec4(1, 1, 1, 1));
				}
				for (int v = 0; v < texCoords.size(); v += 2) {
					completeCampus->AddTexCoord(
						glm::vec2(
							texCoords.at(v),
							texCoords.at(v + 1)
						)
					);
				}

				for (int in = 0; in < indices.size(); in += 3)
				{
					completeCampus->AddTriangle(
						in + campusRunningIndex,
						in + 1 + campusRunningIndex,
						in + 2 + campusRunningIndex);
				}

				campusRunningIndex += indices.size();
			}
			delete building;
			//campusBuildings.emplace_back(buildingObj);


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
	
	glDebugMessageCallback(MessageCallback, 0);

	w.SetFramebuferSizeCallback();

	//Shape cube = Shape(Shape::PREMADE::CUBE);
	

	//Mesh mesh = Mesh(&cube);


	ShaderProgram sp = ShaderProgram({ShaderProgram::Shaders::FRAGMENT, ShaderProgram::Shaders::VERTEX});
	ShaderProgram light_sp = ShaderProgram({ShaderProgram::Shaders::LIGHT_FRAG, ShaderProgram::Shaders::LIGHT_VERT});
	

	//mesh.SetData();
	//
	//Texture2D texture = Texture2D("Content\\Textures\\brick_wall.jpg");
	//texture.setTexParameterWrap(GL_MIRRORED_REPEAT, GL_MIRRORED_REPEAT);
	Model model = Model("Content\\Models\\cube\\cube.obj");
	Model light = Model("Content\\Models\\cube\\cube.obj");
	glm::mat4 transformation = glm::mat4(1.0f);
	
	Camera camera = Camera(glm::vec3(0, 1, 1), glm::vec3(0, 0, 0), 45.0f, 800/600.0f);
	w.SetCamera(&camera);
	glm::mat4 projection;
	//projection = glm::perspective(glm::radians(45.0f), 800/600.0f, 0.1f, 1000.0f);
	glm::mat4 light_transform = glm::translate(glm::mat4(1.0f), glm::vec3(3, 3, 3));
	while (!glfwWindowShouldClose(w.getWindow())) //main render loop
	{
		transformation = glm::mat4(1.0f);
		//transformation = glm::translate(transformation, glm::vec3(0, 0, -3));
		//transformation = glm::rotate(transformation, glm::radians(10 * (float)glfwGetTime()), glm::vec3(.5f, 1.0f,0));
		transformation = glm::scale(transformation, glm::vec3(.5, .5, .05));
		sp.SetUniform4fv("model", transformation);
		sp.SetUniform3fv("normalMatrix", glm::mat3(glm::transpose(glm::inverse(transformation * camera.getView()))));
		sp.SetUniform4fv("camera", camera.getView());
		sp.SetUniform4fv("projection", camera.getProjection());
		sp.SetUniform3f("lightColor", glm::vec3(1, 0, 1));
		sp.SetUniform3f("viewPos", camera.getPosition());
		light_sp.SetUniform4fv("model",  light_transform);
		light_sp.SetUniform4fv("camera", camera.getProjection() * camera.getView());
		sp.SetUniform3f("lightPos", glm::vec3(3, 3, 3));
		//texture.Bind();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		render(model, &sp);
		render(light, &light_sp);
		w.ProcessFrame(&camera);
	}
	glfwTerminate(); //Shut it down!

	return 0;
} 