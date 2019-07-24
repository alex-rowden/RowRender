/*
 * Copyright (c) 2018, NVIDIA CORPORATION. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  * Neither the name of NVIDIA CORPORATION nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>
#include <optixu/optixu_aabb_namespace.h>

using namespace optix;

rtBuffer<float4> voxel_buffer;
rtBuffer<float> intensity_buffer;

rtDeclareVariable(float, cutoff_to, , );
rtDeclareVariable(float, cutoff_from, , );
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, );
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );
rtDeclareVariable(float4, obj_color, attribute obj_color, );


//
// Box
//
static __device__ void make_box(const float4& input, float3& boxmin, float3& boxmax) {
	float halfWidth = input.w / 2;
	boxmin.x = input.x - halfWidth; boxmax.x = input.x + halfWidth;
	boxmin.y = input.y - halfWidth; boxmax.y = input.y + halfWidth;
	boxmin.z = input.z - halfWidth; boxmax.z = input.z + halfWidth;
}

static __device__ float3 boxnormal(float t, float3 t0, float3 t1)
{
	float3 neg = make_float3(t == t0.x ? 1 : 0, t == t0.y ? 1 : 0, t == t0.z ? 1 : 0);
	float3 pos = make_float3(t == t1.x ? 1 : 0, t == t1.y ? 1 : 0, t == t1.z ? 1 : 0);
	return pos - neg;
}

static __device__ float4 get_color(float value) {
	if (value > .8) {
		return make_float4(1, 0, 1, .02);
	}
	else {
		return make_float4(0, 0, 0, 0);
	}
}

RT_PROGRAM void box_intersect(int idx)
{
	if (intensity_buffer[idx] == 0) return;
	else if (intensity_buffer[idx] < cutoff_from || intensity_buffer[idx] > cutoff_to) return;
	if (intensity_buffer[idx] < .8) return;

	float3 boxmin, boxmax;
	make_box(voxel_buffer[idx], boxmin, boxmax);

	float3 t0 = (boxmin - ray.origin) / ray.direction;
	float3 t1 = (boxmax - ray.origin) / ray.direction;

	float3 near = fminf(t0, t1);
	float3 far = fmaxf(t0, t1);
	float tmin = fmaxf(near);
	float tmax = fminf(far);

	if (tmin <= tmax) {
		bool check_second = true;
		if (rtPotentialIntersection(tmin)) {
			shading_normal = geometric_normal = boxnormal(tmin, t0, t1);
			obj_color = get_color(intensity_buffer[idx]);
			if (rtReportIntersection(0))
				check_second = false;
		}
		if (check_second) {
			if (rtPotentialIntersection(tmax)) {
				obj_color = get_color(intensity_buffer[idx]);
				shading_normal = geometric_normal = boxnormal(tmax, t0, t1);
				rtReportIntersection(0);
			}
		}
	}
}

RT_PROGRAM void box_bounds(int primIdx, float result[6])
{
	float3 boxmin, boxmax;
	make_box(voxel_buffer[primIdx], boxmin, boxmax);
	optix::Aabb* aabb = (optix::Aabb*)result;
	aabb->set(boxmin, boxmax);
}
