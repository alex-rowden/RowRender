#include "Window.h"

Window::Window(const char *name) {
	SetVersion(3, 3);

	bool window_made = makeWindow(800, 600, name);
	glfwSetWindowUserPointer(window, this);
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
}

void standard_mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	Window *win = static_cast<Window *>(glfwGetWindowUserPointer(window)); 
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

	float sensitivity = 0.05f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	win->camera->yaw += xoffset;
	win->camera->pitch += yoffset;
	float pitch = win->camera->pitch;
	float yaw = win->camera->yaw;

	if (pitch > 89.0f)
		win->camera->pitch = 89.0f;
	if (pitch < -89.0f)
		win->camera->pitch = -89.0f;

	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	win->camera->setDirection(glm::normalize(front));
}

void Window::standardInputProcessor(GLFWwindow* window) { //Go to processInputFunction, no extra steps needed
	float currentFrame = glfwGetTime();
	float deltaTime = currentFrame - lastTime;
	lastTime = currentFrame;
	float speed = .25 * deltaTime;
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
		if(j == 0)
			scale[i] += .01;
		else {
			translate[i] += .01;
		}
	}if (glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
		if(j == 0)
			scale[i] -= .01;
		else {
			translate[i] -= .01;
		}
	}if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS && !pressed) {
		pressed = true;
		i += 1;
		i %= 3;
	}else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS && !pressed) {
		pressed = true;
		j += 1;
		j %= 2;
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
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); //Set framebuffer callback
}

void Window::SetVersion(float version) {
	SetVersion((int)version, (int)(version * 10) % 10);
}

bool Window::makeWindow(int height, int width, std::string title) {
	window = glfwCreateWindow(height, width, title.c_str(), NULL, NULL);
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
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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

