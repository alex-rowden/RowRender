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

#include "tutorial.h"


rtBuffer<BasicLight> lights;

rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, );
rtDeclareVariable(float3, shading_normal, attribute shading_normal, );
rtDeclareVariable(float4, obj_color, attribute obj_color, );
rtDeclareVariable(float4, ambient_light_color, , );
rtDeclareVariable(float, phong_exp, , );

rtDeclareVariable(PerRayData_radiance, prd_radiance, rtPayload, );
rtDeclareVariable(PerRayData_shadow, prd_shadow, rtPayload, );

rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, t_hit, rtIntersectionDistance, );
rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );

rtDeclareVariable(float, scene_epsilon, , );
rtDeclareVariable(rtObject, top_object, , );
rtDeclareVariable(rtObject, top_shadower, , );


//
// Pinhole camera implementation
//
rtDeclareVariable(float3, eye, , );
rtDeclareVariable(float3, U, , );
rtDeclareVariable(float3, V, , );
rtDeclareVariable(float3, W, , );
rtDeclareVariable(float3, bad_color, , );
rtBuffer<float4, 2>              output_buffer;

RT_PROGRAM void pinhole_camera()
{
	size_t2 screen = output_buffer.size();

	float2 d = make_float2(launch_index) / make_float2(screen) * 2.f - 1.f;
	float3 ray_origin = eye;
	float3 ray_direction = normalize(d.x * U + d.y * V + W);

	optix::Ray ray(ray_origin, ray_direction, RADIANCE_RAY_TYPE, scene_epsilon);

	PerRayData_radiance prd;
	prd.importance = 1.f;
	prd.depth = 0;

	rtTrace(top_object, ray, prd);

	output_buffer[launch_index] = prd.result;
}

//
// Returns solid color for miss rays
//
rtDeclareVariable(float3, bg_color, , );
RT_PROGRAM void miss()
{
	
	prd_radiance.result = make_float4(bg_color, 0.01f);
}

RT_PROGRAM void closest_hit_radiance()
{
	// intersection vectors
	const float3 h = ray.origin + t_hit * ray.direction;            // hitpoint
	const float3 n = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal)); // normal
	const float3 i = ray.direction;                                            // incident direction

	float reflection = 1.0f;
	float4 result = make_float4(0.0f);
	float3 refraction_color = make_float3(1);

	float3 beer_attenuation;
	if (dot(n, ray.direction) > 0) {
		// Beer's law attenuation
		float3 extinction_constant = make_float3(log(.80f), log(.89f), log(.75f));
		beer_attenuation = exp(extinction_constant * t_hit);
	}
	else {
		beer_attenuation = make_float3(1);
	}
	float max_depth = 100;
	// refraction
	if (prd_radiance.depth < max_depth)
	{
		float3 t;                                                            // transmission direction
		float refraction_index = 1.4f;
		if (refract(t, i, n, refraction_index))
		{

			// check for external or internal reflection
			float cos_theta = dot(i, n);
			if (cos_theta < 0.0f)
				cos_theta = -cos_theta;
			else
				cos_theta = dot(t, n);

			reflection = fresnel_schlick(cos_theta, 3, .1, 1.0f);

			float importance = prd_radiance.importance * (1.0f - reflection) * optix::luminance(refraction_color * beer_attenuation);
			if (importance > .01f) {
				optix::Ray ray(h, t, RADIANCE_RAY_TYPE, scene_epsilon);
				PerRayData_radiance refr_prd;
				refr_prd.depth = prd_radiance.depth + 1;
				refr_prd.importance = importance;

				rtTrace(top_object, ray, refr_prd);
				result += make_float4((1.0f - reflection) * refraction_color, 1) * refr_prd.result;
			}
			else {
				result += make_float4((1.0f - reflection) * refraction_color, 1) * obj_color;
			}
		}
		// else TIR
	}
	float3 reflection_color = make_float3(1);
	// reflection
	if (prd_radiance.depth < max_depth)
	{
		float3 r = reflect(i, n);

		float importance = prd_radiance.importance * reflection * optix::luminance(reflection_color * beer_attenuation);
		if (importance > .01f) {
			optix::Ray ray(h, r, RADIANCE_RAY_TYPE, scene_epsilon);
			PerRayData_radiance refl_prd;
			refl_prd.depth = prd_radiance.depth + 1;
			refl_prd.importance = importance;

			rtTrace(top_object, ray, refl_prd);
			result += make_float4(reflection * reflection_color, 1) * refl_prd.result;
		}
		else {
			result += make_float4(reflection * reflection_color, 1) * obj_color;
		}
	}

	result = result * make_float4(beer_attenuation, 1);
	if(result.w > .5)
		rtPrintf("%f\n", result.w);
	prd_radiance.result = result;
}


//
// Returns shading normal as the surface shading result
// 
RT_PROGRAM void closest_hit_radiance1()
{
	float3 world_geo_normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, geometric_normal));
	float3 world_shade_normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal));
	float3 ffnormal = faceforward(world_shade_normal, -ray.direction, world_geo_normal);
	float3 hit_point = ray.origin + t_hit * ray.direction;
	float4 color = make_float4(0);

	float3 beer_attenuation;
	if (dot(world_shade_normal, ray.direction) > 0) {
		// Beer's law attenuation
		float3 extinction_constant = make_float3(log(.80f), log(.89f), log(.75f));
		
		beer_attenuation  = exp(extinction_constant * t_hit);
	}
	else {
		beer_attenuation = make_float3(1);
	}

	for (int i = 0; i < lights.size(); i++) {
		BasicLight light = lights[i];
		float3 L = normalize(light.pos - hit_point);
		float nDl = dot(ffnormal, L);

		if (nDl > 0) {
			PerRayData_shadow shadow_prd;
			shadow_prd.attenuation = make_float3(1.0f);
			float Ldist = length(light.pos - hit_point);
			optix::Ray shadow_ray(hit_point, L, SHADOW_RAY_TYPE,
				scene_epsilon, Ldist);
			rtTrace(top_shadower, shadow_ray, shadow_prd);
			float3 light_attenuation = shadow_prd.attenuation;
			if (fmaxf(light_attenuation) > 0.0f) {
				float3 Lc = light.color * light_attenuation;
				//color += obj_color * make_float4(nDl * light.color, 1.0f);

				float3 H = normalize(L - ray.direction);
				float nDh = dot(ffnormal, H);
				if (false)
					color += obj_color * make_float4(Lc * pow(nDh, phong_exp), 1.0);
			}
		}
	}
	float max_depth = 10;
	float3 refraction_color = make_float3(1);
	float importance = prd_radiance.importance * optix::luminance(refraction_color * beer_attenuation);
	//rtPrintf("%f\n", prd_radiance.importance);
	if (prd_radiance.depth < max_depth && importance > .01f) {
		PerRayData_radiance transmit_prd;
		transmit_prd.depth = prd_radiance.depth + 1;
		transmit_prd.importance = importance;
		Ray trans_ray(hit_point, ray.direction, RADIANCE_RAY_TYPE, scene_epsilon);

		rtTrace(top_object, trans_ray, transmit_prd);
		float transmittence_var = 1/(transmit_prd.depth + 50);
		color += transmittence_var * transmit_prd.result;
	}
	//rtPrintf("%f, %f, %f\n", color.w, beer_attenuation.y, beer_attenuation.z);
	prd_radiance.result = make_float4(beer_attenuation, 1.0) * color;
}

RT_PROGRAM void any_hit_shadow() {
	float3 world_normal = normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, shading_normal));
	float nDi = fabs(dot(world_normal, ray.direction));

	prd_shadow.attenuation *= 1 - fresnel_schlick(nDi, 5);
	rtIgnoreIntersection();
	//rtTerminateRay();
}


//
// Set pixel to solid color upon failur
//
RT_PROGRAM void exception()
{
	output_buffer[launch_index] = make_float4(bad_color, 1.0f);
}
