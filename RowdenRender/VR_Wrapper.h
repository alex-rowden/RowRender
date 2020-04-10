#pragma once
#include "RowRender.h"

using namespace vr;

class VR_Wrapper
{
public:
	uint32_t LeftDeviceId, RightDeviceId;

	enum class EHand
	{
		Left = 0,
		Right = 1,
		None = -1,
	};

	enum class Button {
		X = 0,
		Y = 1,
		A = 2,
		B = 3,
		Trigger = 4,
		Grip = 5,

		None = -1,
	};
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
	void handle_vr_input();
	void ProcessVREvent(const vr::VREvent_t& event);
	EHand left_or_right(const vr::VREvent_t& event);
	EHand left_or_right(vr::TrackedDeviceIndex_t trackedDeviceIndex);
	const char* EHand2str(const EHand hand);
	const char* ButtonCode2str(const vr::VREvent_t&event);
	Button ButtonCode2Button(const vr::VREvent_t& event);
	void SaveControllerIDs();
	glm::mat4 getControllerPose(uint32_t id);

	glm::uvec2 getRenderTargetSize();
	glm::mat4 getProjectionMatrix(Hmd_Eye eye);
	glm::mat4 getViewMatrix(Hmd_Eye eye);
	void updateHMDPoseMatrix();
	glm::mat4 ConvertSteamVRMatrixToMatrix4(const HmdMatrix34_t& matPose);
	struct ControllerInfo_t
	{
		vr::VRInputValueHandle_t m_source = vr::k_ulInvalidInputValueHandle;
		vr::VRActionHandle_t m_actionPose = vr::k_ulInvalidActionHandle;
		vr::VRActionHandle_t m_actionHaptic = vr::k_ulInvalidActionHandle;
		glm::mat4 m_rmat4Pose;
		//CGLRenderModel *m_pRenderModel = nullptr;
		std::string m_sRenderModelName;
		bool m_bShowController;
	};

	
	ControllerInfo_t m_rHand[2];
};

