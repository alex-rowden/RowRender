#include "Window.h"

Window::Window(const char *name, int resolution_x, int resolution_y) {
	SetVersion(4, 2);

	bool window_made = makeWindow(resolution_x, resolution_y, name);
	glfwSetWindowUserPointer(window, this);
	glfwSetCursorPos(window, width / 2.0f, height / 2.0f);
	if (!window_made) {
		std::cout << "Failed to create window" << std::endl;
		glfwTerminate();
		exit(-1);
		return;
	}
}

GLFWwindow* Window::getWindow() {
	return window;
}

//Call in order to resize the window
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	Window* this_window = (Window*)glfwGetWindowUserPointer(window);
	this_window->width = width;
	this_window->height = height;
}

void standard_mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	/*
	Window *win = static_cast<Window *>(glfwGetWindowUserPointer(window)); 
	if (win->firstMouse) // this bool variable is initially set to true
	{
		win->lastTime = glfwGetTime();
		win->lastX = xpos;
		win->lastY = ypos;
		win->firstMouse = false;
	}
	
	win->lastX = xpos;
	win->lastY = ypos;

	float sensitivity = 0.001f;
	
	glfwSetCursorPos(win->getWindow(), win->width / 2.0f, win->height / 2.0f);

	win->horizontalAngle = sensitivity * float(win->width / 2.0f - xpos);
	win->verticalAngle = sensitivity * float(win->height / 2.0f - ypos);


	
	win->camera->setDirection(glm::vec3(cos(win->verticalAngle) * sin(win->horizontalAngle), sin(win->verticalAngle), cos(win->verticalAngle) * cos(win->horizontalAngle)));
	win->camera->setRight(glm::vec3(sin(win->horizontalAngle - 3.14f / 2.0f), 0, cos(win->horizontalAngle - 3.14f / 2.0f)));
	win->camera->setUp(glm::cross(win->camera->getDirection(), win->camera->getRight()));
	*/

	Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
	int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	if (state == GLFW_PRESS && !ImGui::GetIO().WantCaptureMouse)
	{

		if (win->firstMouse) // this bool variable is initially set to true
		{
			win->lastX = xpos;
			win->lastY = ypos;
			win->firstMouse = false;
		}
		float xoffset = xpos - win->lastX;
		float yoffset = win->lastY - ypos; // reversed since y-coordinates range from bottom to top
		win->lastX = xpos;
		win->lastY = ypos;

		float sensitivity = 0.001f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		win->camera->yaw += yoffset;
		win->camera->pitch += xoffset;
		float pitch = win->camera->pitch;
		float yaw = win->camera->yaw;

		if (pitch > 90.0f)
			win->camera->pitch = 90.0f;
		if (pitch < -90.0f)
			win->camera->pitch = -90.0f;

		glm::vec3 front;
		double xzLen = cos(pitch);
		front.x = xzLen * cos(yaw);
		front.y = sin(pitch);
		front.z = xzLen * sin(-yaw);

		win->camera->setDirection(glm::normalize(front));
	}
	else {
		win->firstMouse = true;
	}
	
}

void Window::standardInputProcessor(GLFWwindow* window) { //Go to processInputFunction, no extra steps needed
	float currentFrame = glfwGetTime();
	float deltaTime = currentFrame - lastTime;
	lastTime = currentFrame;
	float speed = this->speed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		speed *= 2;
	}
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera->moveForward(speed);
	}
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera->moveForward(-speed);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera->moveRight(speed);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera->moveRight(-speed);
	}if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		camera->moveUp(speed);
	}if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		camera->moveUp(-speed);
	}if (glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
		if(j == 1)
			scale[i] += .00001;
		else {
			translate[i] += .1;
		}
	}if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
		if(j == 1)
			scale[i] -= .00001;
		else {
			translate[i] -= .1;
		}
	}if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && !pressed) {
		pressed = true;
		i += 1;
		i %= 3;
	}else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && !pressed) {
		pressed = true;
		j += 1;
		j %= 2;
	}if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		std::cout << glm::to_string(camera->getPosition())<< std::endl;
		std::cout << glm::to_string(camera->getDirection())<< std::endl;
		
		//signal = true;
	}
	else {
		pressed = false;
	}
}

void Window::SetVersion(int version_major, int version_minor) {
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, version_major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, version_minor);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //Use core-profile (access a smaller subset of open-gl features
}

void Window::SetFramebuferSizeCallback() {
	glfwSetWindowSizeCallback(window, framebuffer_size_callback); //Set framebuffer callback
}

void Window::SetVersion(float version) {
	SetVersion((int)version, (int)(version * 10) % 10);
}

void Window::setFullScreen(bool set) {
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	width = mode->width;
	height = mode->height;
	if(set)
		glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, mode->width, mode->height, mode->refreshRate);
	else {
		glfwSetWindowMonitor(window, NULL, 0, 0, mode->width, mode->height, mode->refreshRate);

	}
	
}

bool Window::makeWindow(int width, int height, std::string title) {
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	glfwWindowHint(GLFW_RED_BITS, mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

	if (width == 0) { width = mode->width; }
	if (height == 0) { height = mode->height; }

	window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
	this->width = width;
	this->height = height;
	if (window == NULL) {
		
		return false;
	}
	glfwMakeContextCurrent(window); //focus on the new window
	return true;
}

void Window::SetViewportSize(int width, int height) {
	glViewport(0, 0, width, height); //Set viewport to full window size
}

void Window::SetCamera(Camera* _camera) {
	camera = _camera;
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, standard_mouse_callback);
}

//Process each input frame, by default uses standard input processor
void Window::ProcessFrame(bool useStandard) {
	glfwMakeContextCurrent(window); //focus on the new window
	standardInputProcessor(window); //get keypresses etc.
	glfwSwapBuffers(window); //dual buffer swap
	glfwPollEvents();//get polled events
}

//allow custom input processing function, standard process by default not used
void Window::ProcessFrame(void (*processInputFunc)(GLFWwindow *), bool useStandard) { 
	processInputFunc(window);
	ProcessFrame(camera);
}

