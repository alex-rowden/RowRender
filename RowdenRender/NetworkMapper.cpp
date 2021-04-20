#include "NetworkMapper.h"
#include <future>

HANDLE EventWait;

void FuncWlanAcmNotify(PWLAN_NOTIFICATION_DATA data, PVOID context) {
	if (data->NotificationCode == wlan_notification_acm_scan_complete) {
		if (!SetEvent(EventWait)) {
			std::cout << "Setting event failed" << std::endl;
		}
	}
	else if (data->NotificationCode == wlan_notification_acm_scan_fail) {
		if (!SetEvent(EventWait)) {
			std::cout << "Setting event failed" << std::endl;
		}
	}
}

std::vector<std::string> GetData()
{
	
	DWORD dwClientVersion = (IsWindowsVistaOrGreater() ? 2 : 1);
	//variables used for WlanEnumInterfaces 
	PWLAN_INTERFACE_INFO_LIST	pIfList = NULL;
	PWLAN_INTERFACE_INFO		pIfInfo = NULL;
	//variables used for WlanGetAvailableNetworkList
	PWLAN_AVAILABLE_NETWORK_LIST	pBssList = NULL;
	PWLAN_AVAILABLE_NETWORK			pBssEntry = NULL;
	//VARIABLE USED FOR 
	PWLAN_BSS_LIST					pWlanBssList = NULL;



	// Variables for open handle
	DWORD pdwNegotiatedVersion = 0;
	HANDLE phClientHandle = NULL;
	DWORD hResult = ERROR_SUCCESS;
	DWORD pdwPrevNotifSource = 0;

	// GUID Variable
	GUID guidInterface = { 0 };
	
	//creating session handle for the client to connect to server.
	hResult = WlanOpenHandle(dwClientVersion, NULL, &pdwNegotiatedVersion, &phClientHandle);
	if (hResult != ERROR_SUCCESS)
	{
		switch (hResult) {
		case 1062:
			std::cout << "WLAN Autoconfig Not running";
			break;
		default:
			printf("failed WlanOpenHandle=%d \n", hResult);

		}
		return std::vector<std::string>();
	}

	//Enumerates all the wifi adapters currently enabled on PC.
	//Returns the list of interface list that are enabled on PC.
	hResult = WlanEnumInterfaces(phClientHandle, NULL, &pIfList);
	if (hResult != ERROR_SUCCESS)
	{
		printf("failed WlanEnumInterfaces check adapter is on=%d \n", hResult);
		return std::vector<std::string>();
	}
	//Get the first GUID assume one adapter
	guidInterface = pIfList->InterfaceInfo[0].InterfaceGuid;

	hResult = WlanRegisterNotification(phClientHandle,
		WLAN_NOTIFICATION_SOURCE_ACM,
		TRUE,
		(WLAN_NOTIFICATION_CALLBACK)FuncWlanAcmNotify,
		NULL,
		NULL,
		&pdwPrevNotifSource);


	if (hResult != ERROR_SUCCESS)
	{
		printf("failed WlanRegisterNotification=%d \n", hResult);
		printf("######## FuncWlanScan<---######## \n \n");
		return std::vector<std::string>();
	}
	hResult = WlanScan(phClientHandle, &guidInterface, NULL, NULL, NULL);
	if (hResult != ERROR_SUCCESS)
	{
		printf("failed WlanScan check adapter is enabled=%d \n ", hResult);

	}
	Sleep(1000);
	hResult = WlanGetAvailableNetworkList(phClientHandle,
		&guidInterface,
		NULL,
		NULL,
		&pBssList);

	std::vector<std::string> output;
	if (hResult == ERROR_SUCCESS && pBssList) {


		for (unsigned int i = 0; i < pBssList->dwNumberOfItems; i++)
		{
			pWlanBssList = NULL;
			hResult = WlanGetNetworkBssList(phClientHandle,
				&guidInterface,
				&(pBssList->Network[i].dot11Ssid),
				pBssList->Network[i].dot11BssType,
				pBssList->Network[i].bSecurityEnabled,
				NULL,
				&pWlanBssList);
			if (hResult != ERROR_SUCCESS || !pBssList) {
				printf("Failed to get BssList\n");
				break;
			}
			else {
				LONG rssi = pWlanBssList->wlanBssEntries[0].lRssi;
				unsigned char* dot_mac = pWlanBssList->wlanBssEntries[0].dot11Bssid;
				char mac_addr[19];
				sprintf_s(mac_addr, "%02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x",
					dot_mac[0], dot_mac[1], dot_mac[2], dot_mac[3], dot_mac[4], dot_mac[5], dot_mac[6], dot_mac[7]);
				printf("%s:%s = %ld\n", pBssList->Network[i].dot11Ssid.ucSSID, mac_addr, rssi);

				std::stringstream ss;
				ss << pBssList->Network[i].dot11Ssid.ucSSID <<
					" " << mac_addr <<
					" " << rssi <<
					" " << pWlanBssList->wlanBssEntries[0].ulChCenterFrequency <<
					" " << pWlanBssList->wlanBssEntries[0].uLinkQuality <<
					" " << pBssList->Network[i].bSecurityEnabled <<
					" " << pBssList->Network[i].dot11DefaultAuthAlgorithm <<
					" " << pBssList->Network[i].dot11DefaultCipherAlgorithm;
				std::string address = ss.str();

				output.emplace_back(address);

				if (!pBssList) {
					break;
				}
			}
		}
	}
	return output;
}



std::string wifi_sample() {
	std::cout << "Called" << std::endl;
	return "TestOut";
}


void getSamples(std::vector<rpc::client*>client_list) {
	for (int i = 0; i < client_list.size(); i++) {
		std::cout << client_list.at(i)->call("wifi_sample").as<std::string>() << std::endl;
	}
}

int NetworkMapper(bool isServer, int floor) {
	if (isServer) {
		std::cout << "Running" << std::endl;
		rpc::server srv(8080);

		srv.bind("wifi_sample", &wifi_sample);

		srv.run();

		return 0;
	}
	else {
		//Initialize Windowing
		glfwInit();
		glfwSetErrorCallback(error_callback);

		//Create Window 
		Window window("Wifisampling", 0, 0);
		

		//Glad Loading
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) //load GLAD
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
			return false;
		}
		//make window fullscreen
		window.setFullScreen(true);
		glm::uvec2 resolution(window.width, window.height);

		// During init, enable debug output
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(MessageCallback, 0);
		glViewport(0, 0, resolution.x, resolution.y);

		//Allow Resizing
		window.SetFramebuferSizeCallback();

		ShaderProgram screen_shader({
			ShaderProgram::Shaders::SCREEN_FRAG,
			ShaderProgram::Shaders::SCREEN_VERT
			});

		Model quad("./Content/Models/quad/quad_centered.obj");
		quad.setModel(false);
		
		Texture2D floorplan(("./Content/Textures/AVWFloorPlans/" + std::to_string(floor) + "c.png").c_str());
		quad.getMeshes().at(0)->addTexture(&floorplan);
		while (!glfwWindowShouldClose(window.getWindow())) {
			glClear(GL_COLOR_BUFFER_BIT);
			quad.Render(&screen_shader);
			window.ProcessFrame();
			glFinish();
		}

		glfwTerminate();

		std::cout << "Enter Server Addresses" << std::endl;
		std::vector<rpc::client*> client_list;
		
		int counter = 0;
		for (int i = 0; i < 4; i++, counter++) {
			std::string srv = "";
			std::getline(std::cin, srv);
			if (srv.empty()) {
				break;
			}
			std::cout << srv << std::endl;
			rpc::client *c = new rpc::client(srv, 8080);
			client_list.emplace_back(c);
		}
			
		/*
		for (int i = 0; i < counter; i++) {
			if (c->get_connection_state() != rpc::client::connection_state::connected) {
				std::cerr << "Not connected to " << i << std::endl;
				return -1;
			}
		}
		std::vector<std::future<clmdep_msgpack::v1::object_handle>> client_futures;
		*/
		
		/*
		for (int i = 0; i < counter; i++) {
			std::string ret = client_futures[i].get().as<std::string>();
			if (ret.empty()) {
				std::cout << "No return" << std::endl;
			}
			std::cout << ret << std::endl;
		}
		*/
		return 0;
	}
}