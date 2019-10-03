#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optix_device.h>

#define M_PIf 3.1415926535897932384626433

using namespace optix;

struct PerRayData_hologram {
	float3 f2b_color;
	float f2b_opaque;
	unsigned int depth;
};

rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, t_hit, rtIntersectionDistance, );
rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );

rtBuffer<float4, 2> amplitude_buffer;


rtDeclareVariable(int, volumeTextureId, , );
rtDeclareVariable(int, transferFunction_texId, , );
rtDeclareVariable(int, depth_mask_id, , );

rtDeclareVariable(float3, texcoord, attribute texcoord, );
rtDeclareVariable(float3, front_hit_point, attribute front_hit_point, );
rtDeclareVariable(float3, back_hit_point, attribute back_hit_point, );

// information about the volume
rtDeclareVariable(float3, box_min, , );	// opposite corners of the volume
rtDeclareVariable(float3, box_max, , );
rtDeclareVariable(float3, v1, , );		// edges of the plane in which a slice is put, has been scaled by 1/dot(v1, v1)
rtDeclareVariable(float3, v2, , );
rtDeclareVariable(float3, v3, , );

// step size
rtDeclareVariable(float, volumeRaytraceStepSize, , );

// lighting_stuff

rtDeclareVariable(float, ambientStrength, , );
rtDeclareVariable(float3, lightPos, , );
rtDeclareVariable(float, specularStrength, , );
rtDeclareVariable(float, shininess, , );

rtDeclareVariable(float, zFar, , );
rtDeclareVariable(float, zNear, , );

RT_PROGRAM void dummy() {
	//rtPrintf("%d, %d\n", launch_index.x, launch_index.y);
	amplitude_buffer[launch_index] = make_float4(.7, 0, .9, .6);
}

rtDeclareVariable(float3, hg_normal, , );	// normalized

RT_PROGRAM void closest_hit() {
	//float t = -dot(ray.origin, hg_normal) / dot(ray.direction, hg_normal);
	//float3 interLoc = ray.origin + t * ray.direction;
	//location_buffer[launch_index] = interLoc;
	// init phase
	const float3 fhp = front_hit_point;
	//float phase_u = dot(fhp - box_min, v1);
	//float phase_v = dot(fhp - box_min, v2);
	//initPhase_buffer[launch_index] = optix::rtTex2D<float>(initPhaseTextureId, phase_u, phase_v);
	// composite from front to back
	const float3 bhp = back_hit_point;
	unsigned int num_steps = floorf(sqrtf(dot(bhp - fhp, bhp - fhp)) / volumeRaytraceStepSize);
	float3 color_composited = make_float3(0.f, 0.f, 0.f);
	float opaque_composited = 0.f;
	//float opacities[300];
	//int counter = 0;
	//rtPrintf("%d\n", num_steps);
	for (unsigned int s = 0; s < num_steps; ++s) {
		float3 texPoint = fhp + (s + 0.5) * volumeRaytraceStepSize * ray.direction;
		
		float vol_u = dot(texPoint - box_min, v1);
		float vol_v = dot(texPoint - box_min, v2);
		float vol_w = dot(texPoint - box_min, v3);
		float3 show = texPoint - box_min;
		
		

		float volume_scalar = optix::rtTex3D<float>(volumeTextureId, vol_u, vol_v, vol_w);

		float plus_u, minus_u, plus_v, minus_v, plus_w, minus_w;
		float region_increment = volumeRaytraceStepSize / 5.0f;

		plus_u = optix::rtTex3D<float>(volumeTextureId, vol_u + region_increment, vol_v, vol_w);
		minus_u = optix::rtTex3D<float>(volumeTextureId, vol_u - region_increment, vol_v, vol_w);

		plus_v = optix::rtTex3D<float>(volumeTextureId, vol_u, vol_v + region_increment, vol_w);
		minus_v = optix::rtTex3D<float>(volumeTextureId, vol_u, vol_v - region_increment, vol_w);

		plus_w = optix::rtTex3D<float>(volumeTextureId, vol_u, vol_v, vol_w + region_increment);
		minus_w = optix::rtTex3D<float>(volumeTextureId, vol_u, vol_v, vol_w - region_increment);

		float3 normal = -optix::normalize(normalize(v1) * (plus_u - minus_u) / (2 * region_increment) + normalize(v2) * (plus_v - minus_v) / (2 * region_increment) + normalize(v3) * (plus_w - minus_w) / (2 * region_increment));

		float4 voxel_val_tf = optix::rtTex2D<float4>(transferFunction_texId, volume_scalar, volume_scalar);
		//float3 color_self = make_float3(fabs(normal.x), fabs(normal.y), fabs(normal.z));//make_float3(voxel_val_tf);
		float3 lightDir = (lightPos - (texPoint - box_min));
		
		//float depth = optix::rtTex2D<float>(depth_mask_id, vol_u, vol_v) * 2.0f - 1;
		//if (depth != -1 && depth != 1) {
			//rtPrintf("%f\n", depth);
		//}
		//if(depth < 1)
			//rtPrintf("%f\n", depth);
		//float distance = 10000;
		//if ((zFar + zNear - depth * (zFar - zNear)) > 0) {
		//	distance = (2.0 * zNear * zFar) / (zFar + zNear - depth * (zFar - zNear));
		//	//rtPrintf("%f, %f\n", texPoint.z, distance);
		//}
		//rtPrintf("%f\n", (texPoint-box_min).z);
		//if ((texPoint-box_min).z < distance * 25.0) {
		//	break;
		//}
		
		float3 color_self = make_float3(0);
		float opaque_self = 0;
		//rtPrintf("%f\n", lightDir.x * lightDir.x + lightDir.y * lightDir.y + lightDir.z * lightDir.z);
		if (lightDir.x * lightDir.x + lightDir.y * lightDir.y + lightDir.z * lightDir.z < -1) {
			color_self = make_float3(0, 1, 0);
			opaque_self = 1;
		}
		else {
			lightDir = normalize(lightDir);
			float diffuse = fmax(dot(normal, lightDir), 0.0f);

			float3 viewDir = normalize(ray.origin - (texPoint));
			float3 reflectDir = reflect(-lightDir, normal);
			float spec = specularStrength * pow(fmax(dot(viewDir, reflectDir), 0.0f), shininess);
			color_self = (ambientStrength + diffuse) * make_float3(voxel_val_tf) + spec * make_float3(1, 1, 1);
			opaque_self = voxel_val_tf.w;
		}
		
		/*
		if (opaque_self > .01) {
			opaque_self = 1.0f;
			//rtPrintf("%f, %f, %f\n", normal.x, normal.y, normal.z);
			if (fabs(normal.x) < 0.01 && fabs(normal.y) < 0.01 && fabs(normal.z) < 0.01) {
				opaque_self = 0;
			}
		}
		*/
		//opaque_self = 1.f - powf(1.f - opaque_self, opacity_correction);
		// amp = amp_in + (1-opaque) * amp_self
		// opqaue = opaque_in + (1-opqaue) * opqaue_self
		color_composited = color_composited + (1.f - opaque_composited) * opaque_self * color_self;
		opaque_composited = opaque_composited + (1.f - opaque_composited) * opaque_self;
		//if (counter	 < 300 && opaque_composited > 1e-3 ) {
			//opacities[counter] = opaque_self;
			//counter++;
		//}
		
		if (opaque_composited > 0.99) break;
	}
	if (opaque_composited > .9) {
		//rtPrintf("%f\n", opaque_composited);
		//for (int i = 0; i < counter; i++) {
			//rtPrintf("%f\n", opacities[i]);
		//}
		//rtPrintf("END\n");
	}
	amplitude_buffer[launch_index] = make_float4(sqrt(color_composited.x), sqrt(color_composited.y), sqrt(color_composited.z), (opaque_composited));
}


rtDeclareVariable(uint2, element_hologram_dim, , );		// number of pixels in each element hologram
rtDeclareVariable(uint2, num_rays_per_element_hologram, , );
rtDeclareVariable(uint2, half_num_rays_per_element_hologram, , );
rtDeclareVariable(float, pixel_pitch, , );
rtDeclareVariable(float, ray_interval, , );		// ray interval in radian
rtDeclareVariable(float3, hg_anchor, , );	// lower left corner of the hologram
rtDeclareVariable(float3, hg_v1, , );	// normalized
rtDeclareVariable(float3, hg_v2, , );	// normalized


rtDeclareVariable(unsigned int, radiance_ray_type, , );
rtDeclareVariable(float, scene_epsilon, , );
rtDeclareVariable(rtObject, top_object, , );

// ray direction jitter random texture
rtDeclareVariable(int, random_texture, , );	// [0, 1] ^ 4

rtDeclareVariable(float3, eye, , );
rtDeclareVariable(float3, U, , );
rtDeclareVariable(float3, V, , );
rtDeclareVariable(float3, W, , );

RT_PROGRAM void camera() {
	// if outside buffer range, paint it black
	amplitude_buffer[launch_index] = make_float4(0.f, 0.f, 0.f, 0.0f);
	//location_buffer[launch_index] = make_float3(0.f, 0.f, 0.f);
	//initPhase_buffer[launch_index] = -1.f;
	size_t2 screen = amplitude_buffer.size();

	float2 d = make_float2(launch_index) / make_float2(screen) * 2.f - 1.f;
	//float3 angle = make_float3(cos(d.x) * sin(d.y), -cos(d.y), sin(d.x) * sin(d.y));
	float3 ray_origin = eye;
	float3 ray_direction = normalize(d.x * -normalize(U) + d.y * -normalize(V) +  -normalize(W));

	optix::Ray ray(ray_origin, ray_direction, radiance_ray_type, scene_epsilon);
	//rtPrintf("%f, %f, %f\n", rayDirection.x, rayDirection.y, rayDirection.z);
	PerRayData_hologram prd;
	prd.f2b_color = make_float3(0.f, 0.f, 0.f);
	prd.f2b_opaque = 0.f;
	prd.depth = 0;
	rtTrace(top_object, ray, prd);
}

rtDeclareVariable(float, fov, , );





// exception program
// did not hit anything, mark as not hit
// zero amplitude,
RT_PROGRAM void exception() {
	rtPrintExceptionDetails();
	amplitude_buffer[launch_index] = make_float4(0.f, 1.f, 0.f, 1.0f);

}

// miss program
// did not hit anything, mark as not hit
// zero amplitude
RT_PROGRAM void miss() {
	//amplitude_buffer[launch_index] = optix::make_float4(0, 1, 1, 1);
}
