#pragma once
#include "RowRender.h"

using namespace vr;

class VR_Wrapper
{
public:
	IVRSystem* vr_pointer = NULL;

	//VR_Wrapper();
	void initialize();
	bool initCompositor();
	void terminate();
	void composite(EVREye eye, GLuint tex);
	void handoff();

	glm::uvec2 getRenderTargetSize();
};

