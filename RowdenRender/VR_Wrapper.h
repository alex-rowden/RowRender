#pragma once
#include "RowRender.h"

using namespace vr;

class VR_Wrapper
{
public:
	IVRSystem* vr_pointer = NULL;
	vr::TrackedDevicePose_t trackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	glm::mat4 DevicePose[vr::k_unMaxTrackedDeviceCount];
	char DevClassChar[k_unMaxTrackedDeviceCount];
	int validPoseCount = 0;
	std::string strPoseClasses;
	//VR_Wrapper();
	void initialize();
	bool initCompositor();
	void terminate();
	void composite(EVREye eye, GLuint tex);
	void handoff();

	glm::uvec2 getRenderTargetSize();
	glm::mat4 getProjectionMatrix(Hmd_Eye eye);
	glm::mat4 getViewMatrix(Hmd_Eye eye);
	void updateHMDPoseMatrix();
	glm::mat4 ConvertSteamVRMatrixToMatrix4(const HmdMatrix34_t& matPose);
};

