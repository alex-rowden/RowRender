#include "TriangleStripMesh.h"

struct face {
	int tri_strip;
	long num;
	std::vector<int> indices;
		
};

std::vector<glm::vec3> vertices;
std::vector<glm::vec3> normals;
std::vector<glm::vec3> tangents;
std::vector<glm::vec2> texCoords;
std::vector<face> faces;

TriangleStripMesh(std::string path) {
	std::ifstream inputFile;
	inputFile.open(path);
	//preliminary scan
	int num_vert = 0, num_nvert = 0, num_faces = 0, tempi = 0;

	if (inputFile.is_open()) {
		while (!inputFile.eof())
		{
			std::string buff;
			getline(inputFile, buff);

			/*   Vertex */
			if (buff.at(0) == 'v')
			{
				/*   Normal Vertex */
				if (buff.at(1) == 'n')
					num_nvert++;
				/*   Texture ignore it */
				if (buff.at(1) == 't')
					;
				/*   Vertex */
				else
					num_vert++;
			}
			/*   Face or triangle strip */
			else if ((buff.at(0) == 'f') || (buff.at(0) == 't') || (buff.at(0) == 'q'))
			{
				num_faces++;
				strtok((char*)buff.c_str(), " ");
				tempi = 0;
				while (strtok(NULL, " ") != NULL) tempi++;
			}
		}
	}
	else {
		std::cerr << "Error opening file at: " << path << std::endl;
		return;
	}
	inputFile.close();
	inputFile.open(path);
	vertices.resize(num_vert);
	faces.resize(num_faces + 1);

	if (num_nvert > 0) {
		normals.resize(num_nvert);
		tangents.resize(num_nvert);
	}
	else {
		normals.resize(num_vert);
		tangents.resize(num_vert);
	}

	if (num_nvert == 0)
		initialize_normals(num_vert);



	glm::vec3 vert_holder;
	std::string buff;
	std::vector<face> pfaces = faces;
	//std::vector<int> cp_index;

	int normal = 0, texture = 0, normal_and_texture = 0;
	int num_tris = 0, old_tris = 0, num = 0, int vertex = 0;

	int faces_counter = 0;
	faces.emplace_back(new face);
	std::vector<glm::vec3> norm(1);

	if (inputFile.is_open()) {
		std::string character_code;
		getline(inputFile, character_code, ' ');
		if (character_code == "v" || character_code == "vn" || character_code == "vt") {
			getline(inputFile, buff, ' ');
			vert_holder.x = atof(buff.c_str());
			getline(inputFile, buff, ' ');
			vert_holder.y = atof(buff.c_str());
			getline(inputFile, buff, ' ');
			vert_holder.z = atof(buff.c_str());
			if (character_code == "vn") { //normal vertex
				normals.emplace_back(vert_holder);
			}
			else if (character_code == "vt") { //texture vertex
				texCoords.emplace_back(glm::vec2(vert_holder));
			}
			else { //vertex
				vertices.emplace_back(vert_holder);
				//save_bounding_box();
			}
		}
		else if (character_code == "f" || character_code == "t" || character_code == "q") {
			normal = 0, texture = 0, normal_and_texture = 0;
			if (character_code == "t") {

				normal_triangle(-1, glm::vec3(0), norm.begin());
				num_tris -= 2;
				faces.at(faces_counter).tri_strip = 1;
				old_tris = -2;
			}
			else if (character_code == "q") {
				faces.at(faces_counter).tri_strip = 2;
			}
			else {
				faces.at(faces_counter).tri_strip = 0;
				old_tris = num_tris;
				normal_triangle(-1, glm::vec3(0), norm.begin());
			}
			num = 0;
			std::string rest_of_line;
			getline(inputFile, rest_of_line);

			std::istringstream iss(rest_of_line);
			std::vector<std::string> elements{
				std::istream_iterator<std::string>{iss},
				std::istream_iterator<std::string>{}
			};
			num += elements.size();
			faces.at(faces_counter).indices = std::vector<int>(num * 2);

			faces.at(faces_counter).num = num * 2;
			for (auto e : elements) {
				vertex = atoi(e.c_str()) - 1;
				if (vertex < 0) {
					std::cerr << "Something went wrong in vertex atoi" << std::endl;
					throw 1;
				}
				if (num_nvert == 0) {
					normal_triangle(vertex, vertices[vertex], normals.begin());
					faces.at(faces_counter).indices.emplace_back(vertex);
					faces.at(faces_counter).indices.emplace_back(vertex);
					num_tris++;
				}
				else {
					faces.at(faces_counter).indices.emplace_back(0);
					faces.at(faces_counter).indices.emplace_back(vertex);
					num_tris++;
				}
			}
			if (old_tris >= 0) {
				num_tris = old_tris + 1;
				old_tris = -1;
			}
			faces_counter++;
			faces.at(faces_counter) = face();
		}

	}
	inputFile.close();
	if (num_vert == 0) {
		normalize(num_vert);
	}if (num_tris == -2) {
		num_tris = num_tris - 2;
	}
	std::cout << "Vertices: " << vertices.size() << std::endl;
	std::cout << "Normals: " << normals.size() << std::endl;
	std::cout << "Faces: " << faces.size() << std::endl;
}

void normalize(int num_normals) {
	int min = num_normals;
	if (min > normals.size())
		min = normals.size();
	for (int i = 0; i < min; i++) {
		normals.at(i) = glm::normalize(normals.at(i));
	}
}

void initialize_normals(int num_normals) {
	int min = num_normals;
	if (min > normals.size())
		min = normals.size();
	for (int i = 0; i < min; i++) {
		normals.at(i) = glm::vec3(0);
	}
}

/*   Compute the normal for a triangle */
void get_normal(glm::vec3 c0,
	glm::vec3 c1,
	glm::vec3 vertex,
	glm::vec3&normal)
{
	glm::vec3 vector1, vector2, cross;

	vector1 = c0 - c1;

	vector2 = c1 - vertex;

	cross = glm::cross( vector1, vector2);

	normal = cross;

}

void normal_triangle(int id, glm::vec3 vertex,
	std::vector<glm::vec3>::iterator normal)
{
	static int last[2];
	static int index;
	glm::vec3 n;
	static glm::vec3 c[2];
	int start;

	if (id == -1)
	{
		/*   Start of triangle strip */
		last[0] = -1;
		last[1] = -1;
		index = 0;
		start = 1;
	}
	else if (last[index] == -1)
	{
		/*   Filling in the first 2 vertices of the strip */
		last[index] = id;
		c[index] = vertex;
		index = ~index;
	}
	else if (last[~index] == id)
		/*   Degenerate triangle because of a swap */
		index = ~index;
	else
	{
		/*   At a real triangle */
			
		get_normal(c[0], c[1], vertex, n);
		/*   If start, do the last 2 vertices, since they were the first two in
				the strip
		*/
		if (start)
		{
			normal[last[index]].x += n.x; normal[last[index]].y += n.y; normal[last[index]].z += n.z;
			normal[last[!index]].x += n.x; normal[last[!index]].y += n.y; normal[last[!index]].z += n.z;
			start = ~start;
		}

		last[index] = id;

		c[index] = vertex;
		index = ~index;
		normal[id].x += n.x; normal[id].y += n.y; normal[id].z += n.z;
	}
}
