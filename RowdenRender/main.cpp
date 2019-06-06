#include "Window.h"


int counter = 0;
//any old render function
void render() {
	counter += 1;
	if (counter > 100)
		counter -= 100;
	glClearColor(counter/100.0f, counter / 100.0f, counter / 100.0f, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
}




void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

int main() {
	std::cout << "Hello World" << std::endl;
	glfwInit();
	glfwSetErrorCallback(error_callback);
	Window w;
	w.SetVersion(3, 3);
	

	bool window_made = w.makeWindow(800, 600, "helloClass");

	if (!window_made) {
		std::cout << "Failed to create window" << std::endl;
		glfwTerminate();
		return -1;
	}
	
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) //load GLAD
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return false;
	}
	
	w.SetFramebuferSizeCallback();

	while (!glfwWindowShouldClose(w.window)) //main render loop
	{
		

		render();
		w.ProcessFrame();
	}
	glfwTerminate(); //Shut it down!

	return 0;
} 