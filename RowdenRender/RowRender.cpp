#include "RowRender.h"

//any old render function
void render(Model mesh, ShaderProgram* sp, int i) {
	for (Mesh* m : mesh.getMeshes()) {
		m->Render(sp, i);
	}
}
void render(Model mesh, ShaderProgram* sp) {
	for (Mesh* m : mesh.getMeshes()) {
		m->Render(sp);
	}
}

//Taken from https://stackoverflow.com/a/6930407
glm::vec3 hsv2rgb(glm::vec3 hsv)
{
	double      hh, p, q, t, ff;
	long        i;
	glm::vec3         out;
	if (hsv.g <= 0.0) {       // < is bogus, just shuts up warnings
		out.r = hsv.b;
		out.g = hsv.b;
		out.b = hsv.b;
		return out;
	}
	hh = hsv.r;
	if (hh >= 360.0) hh = 0.0;
	hh /= 60.0;
	i = (long)hh;
	ff = hh - i;
	p = hsv.b * (1.0 - hsv.g);
	q = hsv.b * (1.0 - (hsv.g * ff));
	t = hsv.b * (1.0 - (hsv.g * (1.0 - ff)));

	switch (i) {
	case 0:
		out.r = hsv.b;
		out.g = t;
		out.b = p;
		break;
	case 1:
		out.r = q;
		out.g = hsv.b;
		out.b = p;
		break;
	case 2:
		out.r = p;
		out.g = hsv.b;
		out.b = t;
		break;

	case 3:
		out.r = p;
		out.g = q;
		out.b = hsv.b;
		break;
	case 4:
		out.r = t;
		out.g = p;
		out.b = hsv.b;
		break;
	case 5:
	default:
		out.r = hsv.b;
		out.g = p;
		out.b = q;
		break;
	}
	return out;
}

void error_callback(int error, const char* description) {
	fprintf(stderr, "Error: %s\n", description);
}

void setupDearIMGUI(GLFWwindow* window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 420");
	return;
}

void MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204 || id == 131154 || id == 131140) return;

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

