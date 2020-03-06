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