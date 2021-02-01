#include "CampusWifiVisualization.h"
#include "AVWilliamsWifiVisualization.h"
#include "DefferedRenderingDemo.h"
#include <string>
bool campus = false;
bool vr = true;
int main(int argc, char *argv[]) {
	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			if (std::string(argv[i]) == "campus")
				campus = true;
			else if (std::string(argv[i]) == "vr")
				vr = true;
		}
	}
	if(campus)
		return CampusWifiVisualization(vr);
	else if(true){
		return AVWilliamsWifiVisualization(vr);
	}
	else {
		return DefferedRenderingDemo();
	}
}