#include "CampusWifiVisualization.h"
#include "AVWilliamsWifiVisualization.h"
#include <string>
bool campus = true;
int main(int argc, char *argv[]) {
	if (argc > 1) {
		if (std::string(argv[1]) == "campus")
			campus = true;
	}
	if(campus)
		return CampusWifiVisualization();
	else {
		return AVWilliamsWifiVisualization();
	}
}