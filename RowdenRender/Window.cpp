#include "Window.h"

Window::Window(const char *name, int resolution_x, int resolution_y) {
	SetVersion(4, 2);

	bool window_made = makeWindow(resolution_x, resolution_y, name);
	glfwSetWindowUserPointer(window, this);
	glfwSetCursorPos(window, width / 2.0f, height / 2.0f);
	lightPositions = std::vector<glm::vec3>();
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
	this_window->setResized(true);
}


void standard_mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
	int lstate = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
	int rstate = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
	int mstate = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);
	win->currX = xpos;
	win->currY = ypos;
	if ((ImGui::GetCurrentContext() != NULL) && ImGui::GetIO().WantCaptureMouse) return;
	if (lstate == GLFW_PRESS )
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

		float sensitivity = 0.1f;
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		win->camera->yaw += xoffset;
		win->camera->pitch -= yoffset;
		float pitch = win->camera->pitch;
		float yaw = win->camera->yaw;

		if (pitch > 89.f)
			win->camera->pitch = 89.f;
		if (pitch < -89.f)
			win->camera->pitch = -89.f;

		glm::vec3 front;
		double xzLen = cos(glm::radians(pitch));
		front.x = xzLen * cos(glm::radians(yaw));
		front.z = sin(glm::radians(pitch));
		front.y = xzLen * sin(glm::radians(yaw));

		win->camera->setDirection(glm::normalize(front));
	}else if(rstate == GLFW_PRESS){
		if (win->firstMouse) // this bool variable is initially set to true
		{
			win->lastX = xpos;
			win->lastY = ypos;
			win->firstMouse = false;
		}
		win->x_offset = win->lastX - xpos;
		win->y_offset = ypos - win->lastY;
		//win->lastX = xpos;
		//win->lastY = ypos;
		win->button_pressed = true;
		
	}
	else if (rstate == GLFW_RELEASE) {
		win->button_pressed = false;
		win->firstMouse = true;
	}
	else {
		win->firstMouse = true;

	}
	if (win->button_pressed) {
		float currentFrame = glfwGetTime();
		float deltaTime = currentFrame - win->lastTime;
		win->lastTime = currentFrame;
		float speed = deltaTime * win->speed;
		std::cout << win->x_offset << ", " << win->y_offset << std::endl;
		win->camera->moveForward(speed * win->y_offset);
		win->camera->moveRight(speed * win->x_offset / 10.0f);
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
		if (std::find(lightPositions.begin(), lightPositions.end(), camera->getPosition()) ==  lightPositions.end()) {
			lightPositions.emplace_back(camera->getPosition());
		}
		//signal = true;
	}if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
		signal = true;
		std::cout << glm::to_string(glm::vec2(currX, currY)) << std::endl;
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
	glfwPollEvents();//get polled events
	glfwMakeContextCurrent(window); //focus on the new window
	standardInputProcessor(window); //get keypresses etc.
	glfwSwapBuffers(window); //dual buffer swap
	
}

//allow custom input processing function, standard process by default not used
void Window::ProcessFrame(void (*processInputFunc)(GLFWwindow *), bool useStandard) { 
	processInputFunc(window);
	ProcessFrame(camera);
}

