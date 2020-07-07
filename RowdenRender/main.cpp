#include "CampusWifiVisualization.h"
#include "AVWilliamsWifiVisualization.h"
bool campus = false;
int main() {
	if(campus)
		return CampusWifiVisualization();
	else {
		return AVWilliamsWifiVisualization();
	}
}