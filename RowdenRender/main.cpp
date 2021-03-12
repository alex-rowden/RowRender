#include "CampusWifiVisualization.h"
#include "AVWilliamsWifiVisualization.h"
#include "DefferedRenderingDemo.h"
#include "LICPrecompute.h"
#include <string>
bool campus = false;
bool use_vr = false;
int main(int argc, char *argv[]) {
	if (argc > 1) {
		for (int i = 1; i < argc; i++) {
			if (std::string(argv[i]) == "campus")
				campus = true;
			else if (std::string(argv[i]) == "vr")
				use_vr = true;
		}
	}
	if(campus)
		return CampusWifiVisualization(use_vr);
	else if(true){
		return AVWilliamsWifiVisualization(use_vr);
	}
	else {
		return LICPrecompute();
	}
}