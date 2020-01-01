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
#include <optix_device.h>
using namespace optix;


rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );


rtDeclareVariable(float3, box_min, , );	// also the anchor
rtDeclareVariable(float3, box_max, , );	// opposite corners of the volume
rtDeclareVariable(float3, v1, , );		// edges of the plane in which a slice is put, has been scaled by 1/dot(v1, v1)
rtDeclareVariable(float3, v2, , );
rtDeclareVariable(float3, v3, , );
rtDeclareVariable(float3, voxel_size, , );
rtDeclareVariable(float, scene_epsilon, , );
rtDeclareVariable(float, volumeRaytraceStepSize, , );

rtDeclareVariable(float3, texcoord, attribute texcoord, );
rtDeclareVariable(float3, front_hit_point, attribute front_hit_point, );
rtDeclareVariable(float3, back_hit_point, attribute back_hit_point, );



RT_PROGRAM void box_intersect(int primIdx) {
	float3 rayOrigin_boxMin = box_min - ray.origin;
	float3 rayOrigin_boxMax = box_max - ray.origin;
	float3 t0 = rayOrigin_boxMin / ray.direction;
	float3 t1 = rayOrigin_boxMax / ray.direction;
	float3 near = fminf(t0, t1);
	float3 far = fmaxf(t0, t1);
	float t_min = fmaxf(near);
	float t_max = fminf(far);

	if (t_min < t_max) {

		bool check_second = true;
		// ray intersects volume, enters at t_min, exits at t_max
		if (rtPotentialIntersection(t_min)) {
			check_second = false;
			//rtPrintf("%f, %f\n", t_min, t_max);
			front_hit_point = ray.origin + (t_min + scene_epsilon) * ray.direction;
			back_hit_point = ray.origin + (t_max - scene_epsilon) * ray.direction;
			rtReportIntersection(0);
		}
		if (check_second) {
			if (rtPotentialIntersection(t_max)) {
				front_hit_point = ray.origin + ( scene_epsilon)* ray.direction;
				back_hit_point = ray.origin + (t_max - scene_epsilon) * ray.direction;
				rtReportIntersection(0);
			}
		}

	}
}
RT_PROGRAM void box_bounds(int primIdx, float result[6])
{
	optix::Aabb* aabb = (optix::Aabb*)result;
	//rtPrintf("%f, %f, %f\n", box_min.x, box_min.y, box_min.z);
	//rtPrintf("%f, %f, %f\n", box_max.x, box_max.y, box_max.z);
	aabb->set(box_min, box_max);
}
