#include "RowRender.h"

optix::float3 make_float3(glm::vec3 a) {
	return optix::make_float3(a.x, a.y, a.z);
}optix::float4 make_float4(glm::vec4 a) {
	return optix::make_float4(a.x, a.y, a.z, a.w);
}glm::vec3 make_vec3(optix::float3 a) {
	return glm::vec3(a.x, a.y, a.z);

}
