#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optix_device.h>
#include "volume_render.h"



using namespace optix;

struct PerRayData {
	;
};

rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );
rtDeclareVariable(float, t_hit, rtIntersectionDistance, );
rtDeclareVariable(uint2, launch_index, rtLaunchIndex, );

rtBuffer<float4, 2> amplitude_buffer;


rtDeclareVariable(int, volumeTextureId1, , );
rtDeclareVariable(int, rayTextureId, , );
rtDeclareVariable(int, normalTextureId1, , );
rtDeclareVariable(int, max_volume, , );
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
rtDeclareVariable(float2, lightDirP, , );
rtDeclareVariable(float3, lightDir, , );
rtDeclareVariable(float2, sincosLightTheta, , );
rtDeclareVariable(float3, CameraDir, , );
rtDeclareVariable(float3, HalfwayVec, , );
rtDeclareVariable(float3, sincosCameraDirTheta, , );
rtDeclareVariable(float2, CameraDirP, , );
rtDeclareVariable(float2, HalfwayVecP, , );

rtDeclareVariable(float2, sincosHalfwayTheta, , );
rtDeclareVariable(float, specularStrength, , );
rtDeclareVariable(float, shininess, , );
rtDeclareVariable(float, diffuseStrength, , );


rtDeclareVariable(float, zFar, , );
rtDeclareVariable(float, zNear, , );

rtDeclareVariable(float2, IsoValRange, , );
rtDeclareVariable(float3, ShadingTerms, , );
rtDeclareVariable(float4, BubbleTerms, , );
rtDeclareVariable(float, tune, , );
rtDeclareVariable(float3, color1, , );
rtDeclareVariable(float3, color2, , );
rtDeclareVariable(float3, color3, , );
rtDeclareVariable(float3, color4, , );
rtDeclareVariable(float3, color5, , );
rtDeclareVariable(float3, color6, , );
rtDeclareVariable(int, numTex, , );
rtDeclareVariable(int, enabledColors, , );
rtDeclareVariable(float4, intersectionColor, , );
rtDeclareVariable(float, debug, , );
rtDeclareVariable(float, increment, , );
rtDeclareVariable(float, step_mod, , );


RT_PROGRAM void dummy() {
	//rtPrintf("%d, %d\n", launch_index.x, launch_index.y);
	amplitude_buffer[launch_index] = make_float4(.7, 0, .9, .6);
}



rtDeclareVariable(float3, hg_normal, , );	// normalized
//For use on two vectors with radius 1 in spherical coordinates
inline float sdot(float2 a, float2 b) {
	return  sin(a.x) * sin(b.x) * cos(a.y - b.y) + cos(a.x) * cos(b.x);
}
//For use on a sincos vector and the normal vector. Uses precomputed sines and cosines
inline float sdot(float2 sincosa, float2 sincosnorm, float a, float phi) {
	return sincosa.x * sincosnorm.x * cos(a - phi) + sincosa.y * sincosnorm.y;
}

RT_PROGRAM void closest_hit() {
	//amplitude_buffer[launch_index] = make_float4(1, 0, 1, .5);
	//return;
	float2 sample;
	/*
	float max_theta = -3.15f;
	float min_theta = 3.15f;
	for (int i = 0; i < 512; i++) {
		for (int j = 0; j < 512; j++) {
			for (int k = 0; k < 512; k++) {
				sample = optix::rtTex3D<float2>(normalTextureId1, i/512.0f, j/512.0f, k/512.0f);
				//float theta = sample.x * M_PIf;
				float theta = sample.y * M_PIf * 2 - M_PIf;
				if (theta < min_theta) {
					min_theta = theta;
				}
				else if (theta > max_theta) {
					max_theta = theta;
				}
			}
		}
	}
	rtPrintf("%f, %f\n", min_theta, max_theta);
	*/
	const float3 fhp = front_hit_point;
	
	const float3 bhp = back_hit_point;
	float total_distance = (sqrtf(dot(bhp - fhp, bhp - fhp)));
	float3 color_composited = make_float3(0.f, 0.f, 0.f);
	float opaque_composited = 0.f;
	//float epsilon = .1f * volumeRaytraceStepSize;
	//bool show_spec = true;
	float depth = optix::rtTex2D<float>(depth_mask_id, launch_index.x / (float)amplitude_buffer.size().x, (amplitude_buffer.size().y - launch_index.y) / (float)amplitude_buffer.size().y) * 2.0f - 1;
	float distance = 0;
	
	//if ((zFar + zNear - depth * (zFar - zNear)) > 0) {
	distance = 2.0 * zNear * zFar / (zFar + zNear - depth * (zFar - zNear));
	
	float distance_so_far = 0;
	//rtPrintf("%f\n", total_distance);
	while (total_distance > distance_so_far) {
		//rtPrintf("%f\n", distance_so_far);
		float3 texPoint = fhp + (distance_so_far) * ray.direction;// +(epsilon) * (optix::rtTex3D<float>(random_texture, launch_index.x / amplitude_buffer.size().x, launch_index.y / amplitude_buffer.size().y, s / num_steps));
		
		float vol_u = dot(texPoint - box_min, v1);
		float vol_v = dot(texPoint - box_min, v2);
		float vol_w = dot(texPoint - box_min, v3);
		//rtPrintf("%f, %f, %f\n", v1.x, v2.y, v3.z);
		float3 show = texPoint - box_min;

		float volume_scalar;
		if (length(texPoint - ray.origin) > distance) {
			//color_composited = make_float3(1, 0, 0);
			//color_composited = make_float3(0, 0, 0);
			//opaque_composited = 0;
			//flag = true;
			//rtPrintf("%f\n", length(texPoint-box_min));
			//amplitude_buffer[launch_index] = make_float4(length(texPoint-ray.origin)/50.0f, length(texPoint-ray.origin) /50.0f, length(texPoint-ray.origin) /50.0f, 1.0f);
			amplitude_buffer[launch_index] = make_float4((color_composited.x), (color_composited.y), (color_composited.z), (opaque_composited));
			return;
		}
		
		
		
		float4 color;
		bool flag = false;
		bool lighting_enabled = enabledColors &(1<<5);
		float volume_max = optix::rtTex2D<float>(max_volume, vol_u, vol_v) - increment * vol_w;
		distance_so_far += volumeRaytraceStepSize * (1 + step_mod * max(abs(volume_max - IsoValRange.x), abs(volume_max - IsoValRange.y)));
		float4 voxel_val_tf;

		if (volume_max <= IsoValRange.y) {
			//rtPrintf("%f\n", volume_scalar);
			continue;
		}
		for (int i = 0; i < numTex; i++) {
			if (!(enabledColors & (1 << i)))
				continue;

			bool shade_intersection = false;
			switch (i) {
			case 0:
				color = make_float4(color1, 1.0f);
				//color = make_float4(0, 0, 1, 1.0f);
				break;
			case 1:
				color = make_float4(color2, 1.0f);
				break;
			case 2:
				color = make_float4(color3, 1.0f);
				break;
			case 3:
				color = make_float4(color4, 1.0f);
				break;
			case 4:
				color = make_float4(color5, 1.0f);
				break;
			default:
				color = make_float4(1.0);
			}
			volume_scalar = optix::rtTex3D<float>(volumeTextureId1, vol_u, vol_v, i / (float)numTex) - increment * vol_w;
			float4 voxel_val_tf;
			if (volume_scalar <= IsoValRange.y && volume_scalar >= IsoValRange.x) {
				voxel_val_tf = color;
				if (flag == true) {
					voxel_val_tf = intersectionColor;
					shade_intersection = true;
					//return;
				}
				flag = true;
				//voxel_val_tf = make_float4(fabs(normal.x), fabs(normal.y), fabs(normal.z), .99);
				//rtPrintf("%f, %f, %f\n", voxel_val_tf.x, voxel_val_tf.y, voxel_val_tf.z);
			}
			else {
				//voxel_val_tf = make_float4(0, 0, 0, 0);
				continue;
			}

			sample = optix::rtTex3D<float2>(normalTextureId1, vol_u, vol_v, i / 5.0f);
			
			//float top_val = .565;
			//float top_val = .215;
			//float bottom_val = .555;
			//float bottom_val = .2;
			// = optix::rtTex2D<float4>(transferFunction_texId, volume_scalar, volume_scalar);

			
			float phi = sample.x * M_PIf;
			float theta = sample.y * M_PIf * 2 - M_PIf;

			


			float3 color_self = make_float3(0);
			float opaque_self = 0;

			float sinphi = sin(phi);
			float3 normal = make_float3(sinphi * cos(theta), sinphi * sin(theta), cos(phi));
			//normal.z = 1 - increment;
			//normal = normalize(normal);
			float2 normalP = make_float2(phi, theta);
			float2 sincosnorm = make_float2(sin(theta), cos(theta));

			//sin(theta1)sin(theta2)cos(phi1 - pih2) + cos(theta1)cos(theta2)
			//float diffuse = diffuseStrength * fmax(0, sin(theta) * sincosLightTheta.x * cos(phi - lightDirP.x) + cos(theta) * sincosLightTheta.y);
			//float diffuse = diffuseStrength * fmax(0, sdot(lightDirP, normalP));
			float diffuse = diffuseStrength * fmax(0, sdot(sincosLightTheta, sincosnorm, lightDirP.y, phi));
			//float3 viewDir = CameraDir;




			//float spec = specularStrength * pow(fmax(sin(theta) * sincosHalfwayTheta.x * cos(phi - sincosHalfwayTheta.x) + cos(theta) * sincosHalfwayTheta.y, 0), shininess);
			//float spec = specularStrength * pow(fabs(sdot(normalP, HalfwayVecP)), shininess);
			float spec = specularStrength * pow(fabs(sdot(sincosHalfwayTheta, sincosnorm, HalfwayVecP.y, phi)), shininess);
			//rtPrintf("%f, %f\n", HalfwayVecP.x, HalfwayVecP.y);
			if (lighting_enabled) {
				color_self = ambientStrength * make_float3(voxel_val_tf) + diffuse * make_float3(voxel_val_tf) + spec * make_float3(1, 1, 1);
			}
			else {
				color_self = make_float3(fabs(normal.x), fabs(normal.y), fabs(normal.z));;
			}
			

			float bubble_coefficient = 1 - (fabs(dot(ray.direction, normal)));
			//float bubble_coefficient = 1 - (fabs(sdot(CameraDirP, normalP)));

			if (!shade_intersection) {
				if (voxel_val_tf.w > 1e-5) {
					if (bubble_coefficient > BubbleTerms.x) {
						bubble_coefficient = BubbleTerms.x;
					}
					else if (bubble_coefficient < BubbleTerms.y) {
						bubble_coefficient = BubbleTerms.y;
					}

					float norm = (((bubble_coefficient - BubbleTerms.y) / (BubbleTerms.x - BubbleTerms.y)));
					bubble_coefficient = (norm * (BubbleTerms.z - BubbleTerms.w)) + BubbleTerms.w;
					voxel_val_tf.w = ShadingTerms.x;

					//voxel_val_tf.w = 1.0f;
				}
				//bubble_coefficient /= 4;
				//bubble_coefficient += .75;
				//bubble_coefficient = 0 ;
				//if(spec != 0)
					//rtPrintf("%f\n", spec);

				opaque_self = voxel_val_tf.w + (abs(ShadingTerms.y) * bubble_coefficient + ((ShadingTerms.z) * spec));// 0.5f);
				if (ShadingTerms.y > 0) {
					color_self *= powf(1 - bubble_coefficient, 1 / tune);
				}
			}
			else {
				opaque_self = intersectionColor.w;
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
			//rtPrintf("%f, %f, %f\n", color_self.x, color_self.y, color_self.z);
			color_composited += ( (1.f - opaque_composited) * color_self * opaque_self);
			opaque_composited +=  (1.f - opaque_composited) * opaque_self;
			//color_composited = color_self;


			//if (counter	 < 300 && opaque_composited > 1e-3 ) {
				//opacities[counter] = opaque_self;
				//counter++;
			//}
			
			if (opaque_composited > 0.9) {
				amplitude_buffer[launch_index] = make_float4((color_composited.x), (color_composited.y), (color_composited.z), (opaque_composited));
				return;
			}
		}
	}
	
	amplitude_buffer[launch_index] = make_float4((color_composited.x), (color_composited.y), (color_composited.z), (opaque_composited));
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


rtDeclareVariable(float3, eye, , );
rtDeclareVariable(float3, U, , );
rtDeclareVariable(float3, V, , );
rtDeclareVariable(float3, W, , );

/*
rtDeclareVariable(float4, m1, , );
rtDeclareVariable(float4, m2, , );
rtDeclareVariable(float4, m3, , );
rtDeclareVariable(float4, m4, , );
rtDeclareVariable(float4, n1, , );
rtDeclareVariable(float4, n2, , );
rtDeclareVariable(float4, n3, , );
rtDeclareVariable(float4, n4, , );
*/

rtDeclareVariable(float, fov, , );

RT_PROGRAM void camera() {
	// if outside buffer range, paint it black
	amplitude_buffer[launch_index] = make_float4(0.f, 0.f, 0.f, 0.0f);
	//location_buffer[launch_index] = make_float3(0.f, 0.f, 0.f);
	//initPhase_buffer[launch_index] = -1.f;
	size_t2 screen = amplitude_buffer.size();

	float2 d = (make_float2(launch_index) + make_float2(0.5f)) / make_float2(screen) * 2.f - 1.f;
	//rtPrintf("%f, %f\n", d.x, d.y);
	//d.x *= tanf(fov / 2.0f) * screen.x / (float)screen.y;
	//d.y *= 1* tanf(fov / 2.0f);
	//float3 angle = make_float3(cos(d.x) * sin(d.y), -cos(d.y), sin(d.x) * sin(d.y));
	//float3 ray_origin = make_float3(m4);
	float3 ray_origin = eye;
	float3 ray_direction = normalize(-d.x * (U) + -d.y * (V) + -(W));
	//float4 ray_dir = make_float4(d.x, -d.y, zNear, 1.0f);
	//ray_dir = ray_dir.x * n1 + ray_dir.y * n2 + ray_dir.z * n3 + ray_dir.w * n4;
	//ray_dir.z = -1.f;
	//ray_dir.w = 0.f;
	//rtPrintf("help: %f, %f, %f\n", ray_direction.x, ray_direction.y, ray_direction.z);
	//ray_dir = ray_dir.x * m1 + ray_dir.y * m2 + ray_dir.z * m3 + ray_dir.w * m4;
	//float3 ray_direction = normalize(make_float3(ray_dir/ray_dir.w) - ray_origin);
	//rtPrintf("view_project: %f, %f, %f\n", ray_direction);
	//rtPrintf("m1: %f, %f, %f\n", m1.x, m1.y, m1.z);
	//rtPrintf("m2: %f, %f, %f\n", m2.x, m2.y, m2.z);
	//rtPrintf("m3: %f, %f, %f\n\n", m3.x, m3.y, m3.z);
	//float3 ray_direction = normalize(make_float3((d.x * normalize(m1) + d.y * normalize(m2) + normalize(m3) + normalize(m4))));
	optix::Ray ray(ray_origin, ray_direction, radiance_ray_type, scene_epsilon);
	//rtPrintf("%f, %f, %f\n", rayDirection.x, rayDirection.y, rayDirection.z);
	PerRayData prd;
	rtTrace(top_object, ray, prd);
}







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
