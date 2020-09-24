#include "CampusWifiVisualization.h"
#include "AVWilliamsWifiVisualization.h"
#include "DefferedRenderingDemo.h"
#include <string>
bool campus = false;
int main(int argc, char *argv[]) {
	if (argc > 1) {
		if (std::string(argv[1]) == "campus")
			campus = true;
	}
	if(campus)
		return CampusWifiVisualization();
	else if(true){
		return AVWilliamsWifiVisualization();
	}
	else {
		return DefferedRenderingDemo();
	}
}