#pragma once
#include "RowRender.h"

using namespace vr;

class VR_Wrapper
{
public:
	uint32_t LeftDeviceId, RightDeviceId;
	enum class Joystick_Mode {
		Scroll,
		Impulse,
		Movement
	};
	struct ControllerButtons {
		bool trigger, a, b, grip, grip_pressed, a_pressed, b_pressed;
		glm::vec2 joystick_raw_position, joystick_counter,
			counter_min = glm::vec2(1, 0),
			counter_max = glm::vec2(19, 19);
		Joystick_Mode mode = Joystick_Mode::Impulse;
		void setCounter(glm::vec2 a) {
			if (a.x < counter_min.x)
				a.x = counter_min.x;
			else if (a.x > counter_max.x)
				a.x = counter_max.x;
			if (a.y < counter_min.y)
				a.y = counter_min.y;
			else if (a.y > counter_max.y)
				a.y = counter_max.y;

			this->joystick_counter = a;
		}
		void setCounter(float a, float b) {
			setCounter(glm::vec2(a, b));
		};
		void setCounterx(float a) {
			setCounter(glm::vec2(a, this->joystick_counter.y));
		};
		void setCountery(float a) {
			setCounter(glm::vec2(this->joystick_counter.x, a));
		}
	}controllers[2], 
		*right_hand = &controllers[1],
		*left_hand = &controllers[0];
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
		ThumbRightLeft = 6,
		ThumbUpDown = 7,
		None = -1,
	};
	IVRSystem* vr_pointer = NULL;
	VRActionHandle_t m_actionSet, trigger_right,
		trigger_left, a_button, b_button, x_button,
		y_button, joysticks[2], grip_right, grip_left;
	vr::TrackedDevicePose_t trackedDevicePose[vr::k_unMaxTrackedDeviceCount];
	glm::mat4 DevicePose[vr::k_unMaxTrackedDeviceCount];
	char DevClassChar[k_unMaxTrackedDeviceCount];
	int validPoseCount = 0;
	std::string strPoseClasses;
	float adjusted_height = 0;
	glm::vec3 teleport_position;
	glm::mat4 quad_transform;
	float counter_speed = 5;
	float joystick_threshold = .5;
	char edges[2][2] = { { 0,0 }, {0,0} }; //[controller][x,y]
	//VR_Wrapper();
	void initialize();
	bool initCompositor();
	void terminate();
	void SetActionManifestPath(std::string);
	void composite(EVREye eye, GLuint tex);
	void handoff();
	void handle_vr_input();
	glm::mat4 getSeatedZeroPoseToStandingPose();
	bool resetZeroPose();
	void ProcessVREvent(const vr::VREvent_t& event);
	EHand left_or_right(const vr::VREvent_t& event);
	EHand left_or_right(vr::TrackedDeviceIndex_t trackedDeviceIndex);
	const char* EHand2str(const EHand hand);
	const char* ButtonCode2str(const vr::VREvent_t& event);
	Button ButtonCode2Button(const vr::VREvent_t& event);
	void SaveControllerIDs();
	glm::mat4 getControllerPose(uint32_t id);
	void setActionHandles();
	void UpdateActionState();
	glm::uvec2 getRenderTargetSize();
	glm::mat4 getProjectionMatrix(Hmd_Eye eye);
	glm::mat4 getViewMatrix(Hmd_Eye eye);
	void updateHMDPoseMatrix();
	glm::vec3 getCameraPosition();
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
	struct ControllerData {
		vr::VRActionHandle_t pose_handle = vr::k_ulInvalidActionHandle;
		vr::VRInputValueHandle_t source = vr::k_ulInvalidInputValueHandle;
		glm::mat4 pose;
	}; ControllerData hand[2];

	ControllerInfo_t m_rHand[2];
};
