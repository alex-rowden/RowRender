#include "VR_Wrapper.h"

std::string GetTrackedDeviceString(vr::IVRSystem* pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError* peError = NULL)
{
	uint32_t unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, NULL, 0, peError);
	if (unRequiredBufferLen == 0)
		return "";

	char* pchBuffer = new char[unRequiredBufferLen];
	unRequiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, pchBuffer, unRequiredBufferLen, peError);
	std::string sResult = pchBuffer;
	delete[] pchBuffer;
	return sResult;
}

void VR_Wrapper::initialize() {
	EVRInitError eError = VRInitError_None;
	vr_pointer = VR_Init(&eError, VRApplication_Scene);
	if (eError != VRInitError_None) {
		vr_pointer = NULL;
		std::cerr << "Unable to init VR runtime: " << VR_GetVRInitErrorAsEnglishDescription(eError) << std::endl;
		exit(EXIT_FAILURE);
	}

	std::cout << GetTrackedDeviceString(vr_pointer, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_TrackingSystemName_String) << std::endl;
	std::cout << GetTrackedDeviceString(vr_pointer, vr::k_unTrackedDeviceIndex_Hmd, vr::Prop_SerialNumber_String) << std::endl;
}

void VR_Wrapper::ProcessVREvent(const vr::VREvent_t& event) {
	switch (event.eventType)
	{
	case vr::VREvent_TrackedDeviceDeactivated:
	{
		printf("Device %u detached.\n", event.trackedDeviceIndex);
	}
	break;
	case vr::VREvent_TrackedDeviceUpdated:
	{
		printf("Device %u updated.\n", event.trackedDeviceIndex);
	}case vr::VREvent_TrackedDeviceActivated:
	{
		printf("Device %u activated.\n", event.trackedDeviceIndex);
	}case vr::VREvent_ButtonPress:
		printf("%s Button %s pressed\n", EHand2str(left_or_right(event)), ButtonCode2str(event));
		if (left_or_right(event) == VR_Wrapper::EHand::Right) {
			if (ButtonCode2Button(event) == VR_Wrapper::Button::Trigger) {
				std::cout << "Run Ray Picking" << std::endl;
				ray_picker_enable = true;
			}if (ButtonCode2Button(event) == VR_Wrapper::Button::B) {
				//adjusted_height += 1.01;
			}if (ButtonCode2Button(event) == VR_Wrapper::Button::A) {
				//adjusted_height -= 1.01;
			}
		}
	case vr::VREvent_ButtonTouch:
		printf("%s Button %s touched\n", EHand2str(left_or_right(event)), ButtonCode2str(event));
		break;
	}

}

void VR_Wrapper::SaveControllerIDs() {
	for (unsigned int id = 0; id < k_unMaxTrackedDeviceCount; id++) {
		ETrackedDeviceClass trackedDeviceClass =
			vr_pointer->GetTrackedDeviceClass(id);
		if (trackedDeviceClass !=
			ETrackedDeviceClass::TrackedDeviceClass_Controller ||
			!vr_pointer->IsTrackedDeviceConnected(id))
			continue;

		//Confirmed that the device in question is a connected controller
		auto hand = left_or_right(id);
		if (hand == EHand::Right) {
			RightDeviceId = id;
		}
		else if(hand == EHand::Left) {
			LeftDeviceId = id;
		}
	}
}

glm::mat4 getDeviceLocation(uint32_t id) {

}

VR_Wrapper::Button VR_Wrapper::ButtonCode2Button(const vr::VREvent_t& event) {
	switch (event.data.controller.button) {
	case k_EButton_A: {
		EHand hand = left_or_right(event);
		return (hand == EHand::Left) ? Button::X : (hand == EHand::Right) ? Button::A : Button::None;
		break;
	}
	case k_EButton_ApplicationMenu:
	{
		EHand hand = left_or_right(event);
		return (hand == EHand::Left) ? Button::Y : (hand == EHand::Right) ? Button::B : Button::None;
		break;
	}case k_EButton_Grip:
		return Button::Grip;
		break;
	case k_EButton_SteamVR_Trigger:
		return Button::Trigger;
		break;
	case k_EButton_Axis0:
		return Button::ThumbRightLeft;
	case k_EButton_Axis3:
		return Button::ThumbUpDown;
	default:
		std::cout << event.data.controller.button <<std::endl;
		return Button::None;//vr_pointer->GetButtonIdNameFromEnum();
	}
}

const char* VR_Wrapper::ButtonCode2str(const vr::VREvent_t& event) {
	switch (ButtonCode2Button(event)) {
	case Button::A:
		return "A";
		break;
	case Button::B:
		return "B";
		break;
	case Button::X:
		return "X";
		break;
	case Button::Y:
		return "Y";
		break;
	case Button::Grip:
		return "Grip";
		break;
	case Button::Trigger:
		return "Trigger";
		break;
	case Button::ThumbRightLeft:
		return "Thumb RightLeft";
	case Button::ThumbUpDown:
		return "Thumb UpDown";
	default:
		return "";//vr_pointer->GetButtonIdNameFromEnum();
	}
}

const char* VR_Wrapper::EHand2str(const EHand hand) {
	switch (hand) {
	case EHand::Right:
		return "Right";
		break;
	case EHand::Left:
		return "Left";
		break;
	default:
		return "EHand2str Error";
		break;
	}
}

VR_Wrapper::EHand VR_Wrapper::left_or_right(const vr::VREvent_t& event) {
	return left_or_right(event.trackedDeviceIndex);
}

VR_Wrapper::EHand VR_Wrapper::left_or_right(vr::TrackedDeviceIndex_t trackedDeviceIndex) {
	ETrackedDeviceClass trackedDeviceClass =
		vr_pointer->GetTrackedDeviceClass(trackedDeviceIndex);
	if (trackedDeviceClass != ETrackedDeviceClass::TrackedDeviceClass_Controller) {
		return EHand::None; //this is a placeholder, but there isn't a controller 
		   //involved so the rest of the snippet should be skipped
	}
	ETrackedControllerRole role =
		vr_pointer->GetControllerRoleForTrackedDeviceIndex(trackedDeviceIndex);
	if (role == TrackedControllerRole_Invalid) {
		return EHand::None;
		// The controller is probably not visible to a base station.
		//    Invalid role comes up more often than you might think.
	}
	else if (role == TrackedControllerRole_LeftHand) {
		// Left hand
		return EHand::Left;
	}
	else if (role == TrackedControllerRole_RightHand) {
		// Right hand
		return EHand::Right;
	}
	else {
		return EHand::None;
	}
}

glm::mat4 VR_Wrapper::getControllerPose(uint32_t id) {
	TrackedDevicePose_t trackedDevicePose;
	VRControllerState_t controllerState;
	/*
	bool ret = vr_pointer->GetControllerStateWithPose(
		TrackingUniverseSeated, id, &controllerState,
		sizeof(controllerState), &trackedDevicePose);
		*/glm::mat4 controllerPose = hand[(int)left_or_right(id)].pose;
	//if (!ret)
	//	std::cerr << "Did not recieve tracking for controller " << id << std::endl;
		return controllerPose;//ConvertSteamVRMatrixToMatrix4(trackedDevicePose.mDeviceToAbsoluteTracking);
}

void VR_Wrapper::setActionHandles()
{
	vr::VRInput()->GetActionSetHandle("/actions/demo", &m_actionSet);

	auto ret = vr::VRInput()->GetInputSourceHandle("/user/hand/left", &hand[0].source);
	ret = vr::VRInput()->GetActionHandle("/actions/demo/in/hand_left", &hand[0].pose_handle);
	
	vr::VRInput()->GetInputSourceHandle("/user/hand/right", &hand[1].source);
	vr::VRInput()->GetActionHandle("/actions/demo/in/hand_right", &hand[1].pose_handle);
	
	ret = vr::VRInput()->GetActionHandle("/actions/demo/in/select", &trigger_right);

	if (ret != vr::EVRInputError::VRInputError_None) {
		std::cout << "boop " << ret << std::endl;
	}
}

//straight up stolen from openvr sample
bool GetDigitalActionState(vr::VRActionHandle_t action, vr::VRInputValueHandle_t* pDevicePath = nullptr)
{
	vr::InputDigitalActionData_t actionData;
	vr::VRInput()->GetDigitalActionData(action, &actionData, sizeof(actionData), vr::k_ulInvalidInputValueHandle);
	if (pDevicePath)
	{
		*pDevicePath = vr::k_ulInvalidInputValueHandle;
		if (actionData.bActive)
		{
			vr::InputOriginInfo_t originInfo;
			if (vr::VRInputError_None == vr::VRInput()->GetOriginTrackedDeviceInfo(actionData.activeOrigin, &originInfo, sizeof(originInfo)))
			{
				*pDevicePath = originInfo.devicePath;
			}
		}
	}
	return actionData.bActive && actionData.bState;
}

void VR_Wrapper::UpdateActionState() {
	vr::VRActiveActionSet_t actionSet = { 0 };
	actionSet.ulActionSet = m_actionSet;
	if (vr::VRInput()->UpdateActionState(&actionSet, sizeof(actionSet), 1) != vr::EVRInputError::VRInputError_None) {
		std::cerr << "error updating Action State" << std::endl;
	}
	ray_picker_enable = GetDigitalActionState(trigger_right);
	for (int eHand = 0; eHand < 2; eHand++)
	{
		vr::InputPoseActionData_t poseData;
		if (auto error = vr::VRInput()->GetPoseActionDataForNextFrame(hand[eHand].pose_handle, vr::TrackingUniverseStanding, &poseData, sizeof(poseData), hand[eHand].source) != vr::VRInputError_None
			|| !poseData.bActive || !poseData.pose.bPoseIsValid)
		{
			//m_rHand[eHand].m_bShowController = false;
			std::cerr << "pose invalid: " << error << std::endl;
		}
		else
		{
			//std::cout << "I got a good state" << std::endl;
			hand[eHand].pose = ConvertSteamVRMatrixToMatrix4(poseData.pose.mDeviceToAbsoluteTracking);
		}
	}
}

void VR_Wrapper::handle_vr_input() {
	vr::VREvent_t event;
	while (vr_pointer->PollNextEvent(&event, sizeof(event)))
	{
		ProcessVREvent(event);
	}
	UpdateActionState();
	/*
	for (EHand eHand = EHand::Left; eHand <= EHand::Right; ((int&)eHand)++)
	{
		vr::InputPoseActionData_t poseData;
		if (auto error = vr::VRInput()->GetPoseActionDataForNextFrame(m_rHand[(int)eHand].m_actionPose, vr::TrackingUniverseStanding, &poseData, sizeof(poseData), vr::k_ulInvalidInputValueHandle) != vr::VRInputError_None
			|| !poseData.bActive || !poseData.pose.bPoseIsValid)
		{
			std::cout << "failed to get PoseActionData" << error << std::endl;
			m_rHand[(int)eHand].m_bShowController = false;
		}
		else
		{
			m_rHand[(int)eHand].m_rmat4Pose = ConvertSteamVRMatrixToMatrix4(poseData.pose.mDeviceToAbsoluteTracking);

			vr::InputOriginInfo_t originInfo;
			if (vr::VRInput()->GetOriginTrackedDeviceInfo(poseData.activeOrigin, &originInfo, sizeof(originInfo)) == vr::VRInputError_None
				&& originInfo.trackedDeviceIndex != vr::k_unTrackedDeviceIndexInvalid)
			{
				std::string sRenderModelName = GetTrackedDeviceString(vr_pointer, originInfo.trackedDeviceIndex, vr::Prop_RenderModelName_String);
				if (sRenderModelName != m_rHand[(int)eHand].m_sRenderModelName)
				{
					//m_rHand[eHand].m_pRenderModel = FindOrLoadRenderModel(sRenderModelName.c_str());
					m_rHand[(int)eHand].m_sRenderModelName = sRenderModelName;
				}
			}
		}
	}
	*/
	}

glm::mat4 VR_Wrapper::getSeatedZeroPoseToStandingPose() {
	return ConvertSteamVRMatrixToMatrix4(vr_pointer->GetSeatedZeroPoseToStandingAbsoluteTrackingPose());
}

bool VR_Wrapper::resetZeroPose() {
	vr_pointer->ResetSeatedZeroPose();
	return true;
}

bool VR_Wrapper::initCompositor() {
	EVRInitError peError = VRInitError_None;

	if (!VRCompositor())
	{
		std::cerr << "Compositor initialization failed. See log file for details" << std::endl;
		return false;
	}

	return true;
}

void VR_Wrapper::terminate() {
	VR_Shutdown();
	vr_pointer = NULL;
}

void VR_Wrapper::SetActionManifestPath(std::string path)
{
	if (vr::VRInput()->SetActionManifestPath(path.c_str()) != vr::EVRInputError::VRInputError_None) {
		std::cerr << "Error setting action manifest path" << std::endl;
	}
}

void VR_Wrapper::composite(EVREye eye, GLuint tex) {
	Texture_t eye_texture = { (void*)(uintptr_t)tex, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
	EVRCompositorError error = VRCompositor()->Submit(eye, &eye_texture);
	if (error != VRCompositorError_None) {
		std::cerr << "Error in Compositing: " << error << std::endl;
	}
}

void VR_Wrapper::handoff() {
	VRCompositor()->PostPresentHandoff();
}

glm::uvec2 VR_Wrapper::getRenderTargetSize() {
	unsigned int width, height;
	vr_pointer->GetRecommendedRenderTargetSize(&width, &height);
	return glm::uvec2(width, height);
}

glm::mat4 VR_Wrapper::getProjectionMatrix(Hmd_Eye eye) {
	vr::HmdMatrix44_t mat = vr_pointer->GetProjectionMatrix(eye, .1, 1000);

	return glm::mat4(
		mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
		mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
		mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
		mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
	);
}

glm::mat4 VR_Wrapper::ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t& matPose)
{
	glm::mat4 matrixObj = glm::mat4(
		matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
		matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
		matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
		matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
	);
	return matrixObj;
}

void VR_Wrapper::updateHMDPoseMatrix() {
	vr::VRCompositor()->WaitGetPoses(trackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);

	validPoseCount = 0;
	strPoseClasses = "";
	for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
	{
		if (trackedDevicePose[nDevice].bPoseIsValid)
		{
			validPoseCount++;
			DevicePose[nDevice] = ConvertSteamVRMatrixToMatrix4(trackedDevicePose[nDevice].mDeviceToAbsoluteTracking);
			if (DevClassChar[nDevice] == 0)
			{
				switch (vr_pointer->GetTrackedDeviceClass(nDevice))
				{
				case vr::TrackedDeviceClass_Controller:        DevClassChar[nDevice] = 'C'; break;
				case vr::TrackedDeviceClass_HMD:               DevClassChar[nDevice] = 'H'; break;
				case vr::TrackedDeviceClass_Invalid:           DevClassChar[nDevice] = 'I'; break;
				case vr::TrackedDeviceClass_GenericTracker:    DevClassChar[nDevice] = 'G'; break;
				case vr::TrackedDeviceClass_TrackingReference: DevClassChar[nDevice] = 'T'; break;
				default:                                       DevClassChar[nDevice] = '?'; break;
				}
			}
			strPoseClasses += DevClassChar[nDevice];
		}
	}

}

glm::vec3 VR_Wrapper::getCameraPosition() {
	return glm::vec3((DevicePose[vr::k_unTrackedDeviceIndex_Hmd])* glm::vec4(0, 0, 0, 1));
}

glm::mat4 VR_Wrapper::getViewMatrix(Hmd_Eye eye) {
	vr::HmdMatrix34_t matEyeRight = vr_pointer->GetEyeToHeadTransform(eye);
	glm::mat4 matrixObj = glm::mat4(
		matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0,
		matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
		matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
		matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f
	);

	glm::mat4 HMDPose = (DevicePose[vr::k_unTrackedDeviceIndex_Hmd]);


	return glm::inverse(HMDPose * matrixObj);
}