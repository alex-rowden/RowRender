#include "CampusWifiVisualization.h"
#include "AVWilliamsWifiVisualization.h"
#include "NetworkMapper.h"
//#include "DefferedRenderingDemo.h"
#include "LICPrecompute.h"
#include <string>
bool campus = false;
bool use_vr = false;
int main(int argc, char *argv[]) {
	if (argc > 1) {
		if (std::string(argv[1]) == "campus")
			campus = true;
		else if (std::string(argv[1]) == "server") {
			if (argc > 2)
				return NetworkMapper(true, atoi(argv[2]));
			else
				return NetworkMapper(true);
		}
		else if (std::string(argv[1]) == "client") {
			if (argc > 2)
				return NetworkMapper(false, atoi(argv[2]));
			else
				return NetworkMapper(false);
		}
		for (int i = 1; i < argc; i++) {
			if (std::string(argv[i]) == "vr")
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