//FRAGMENT SHADER

#version 420 core

out vec4 FragColor;

in vec2 TexCoord;

struct Gaussian {
	float x_coord;
	float y_coord;
	float sigma;
	float amplitude;
};

#define MAX_GAUSSIANS 200

uniform Gaussian gaussians[MAX_GAUSSIANS];


//uniform sampler2D texture_diffuse1;
uniform highp sampler2D volume0;
uniform highp sampler2D volume1;
uniform sampler2D volume2;
uniform sampler2D volume3;
uniform sampler2D volume4;
uniform sampler2D volume5;
uniform sampler2D normal0;
uniform sampler2D normal1;
uniform sampler2D normal2;
uniform sampler2D normal3;
uniform sampler2D normal4;
uniform sampler2D normal5;
uniform highp sampler2D fhp;
uniform highp sampler2D bhp;
uniform highp sampler2D depth_tex;
uniform sampler2D noise;

uniform vec3 viewPos;
uniform vec2 IsoValRange;
uniform float StepSize;
uniform float increment;
uniform vec3 volume_size;
uniform vec3 box_min;
uniform vec3 box_max;
uniform float base_opac;
uniform vec3 color1;
uniform vec3 color2;
uniform vec3 color3;
uniform vec3 color4;
uniform vec3 color5;
uniform vec3 color6;
uniform vec3 shade_color;
uniform float shade_opac;
uniform float enable_intersection;
uniform int numTex;
uniform float zNear;
uniform float zFar;
uniform float spec_term, bubble_term, bubble_min, bubble_max, max_opac, min_opac, step_mod, tune;
uniform float fcp;

uniform int enabledVolumes;

uniform vec2 sincosLightTheta;
uniform vec2 lightDirP;
uniform vec2 sincosHalfwayTheta;
uniform vec2 HalfwayVecP;
uniform float ambientStrength, diffuseStrength, specularStrength, shininess;
uniform int num_gaussians;

#define EPSILON 1e-8
#define DISTANCE_EPSILON 1e-3
#define M_PIf 3.1415926535897932384626433

float sdot(vec2 a, vec2 b) {
	return  sin(a.x) * sin(b.x) * cos(a.y - b.y) + cos(a.x) * cos(b.x);
}
//For use on a sincos vector and the normal vector. Uses precomputed sines and cosines
float sdot(vec2 sincosa, vec2 sincosnorm, float a, float phi) {
	return sincosa.x * sincosnorm.x * cos(a - phi) + sincosa.y * sincosnorm.y;
}

float signed_distance_sphere(Gaussian gaus, vec3 position) {
	vec3 sphere_center = vec3(gaus.x_coord, gaus.y_coord, gaus.amplitude);
	return distance(sphere_center, position) - gaus.amplitude;
}

vec2 signed_distance_gauss(Gaussian gaus, vec3 currPos, vec3 ray, float iso) {
	//From Mathematica 
	//	(mux mx + muy my - mx x0 - my y0)/(
	//	mx ^ 2 + my ^ 2) - \[Sqrt](1 / (mx ^ 2 + my ^ 2) ^ 2(-muy ^ 2 mx ^ 2 +
	//	2 mux muy mx my - mux ^ 2 my ^ 2 - 2 muy mx my x0 + 2 mux my ^ 2 x0 -
	//	my ^ 2 x0 ^ 2 + 2 muy mx ^ 2 y0 - 2 mux mx my y0 + 2 mx my x0 y0 -
	//	mx ^ 2 y0 ^ 2 + 2 mx ^ 2 sig ^ 2 log(C - height / A] +
	//	2 my ^ 2 sig ^ 2 log(C - height / A])
	/*
	float sig_sq = gaus.sigma * gaus.sigma;
	float mx_sq = ray.x * ray.x;
	float my_sq = ray.y * ray.y;
	float mux_sq = gaus.x_coord * gaus.x_coord;
	float muy_sq = gaus.y_coord * gaus.y_coord;


	float square_ray_length = (mx_sq + my_sq);
	if (square_ray_length < EPSILON) {
		square_ray_length = EPSILON;
	}
	float leading_coefficient = (ray.x * (gaus.x_coord - currPos.x) + ray.y * (gaus.y_coord - currPos.y))/ square_ray_length;
	
	float in_squareroot =  -muy_sq * mx_sq;
	in_squareroot = in_squareroot + 2 * gaus.x_coord * gaus.y_coord * ray.x * ray.y;
	in_squareroot = in_squareroot - mux_sq * my_sq;
	in_squareroot = in_squareroot - 2 * gaus.y_coord * ray.x * ray.y * currPos.x;
	in_squareroot = in_squareroot + 2 * gaus.x_coord * my_sq * currPos.x;
	in_squareroot = in_squareroot - my_sq * currPos.x * currPos.x;
	in_squareroot = in_squareroot + 2 * gaus.y_coord * mx_sq * currPos.y;
	in_squareroot = in_squareroot - 2 * gaus.x_coord * ray.x * ray.y * currPos.y;
	in_squareroot = in_squareroot + 2 * ray.x * ray.y* currPos.x * currPos.y;
	in_squareroot = in_squareroot - mx_sq * currPos.y * currPos.y;

	float log_factor = log(min((isoval)/gaus.amplitude, 1.0f));
	in_squareroot = in_squareroot - 2 * mx_sq * sig_sq * log_factor;
	in_squareroot = in_squareroot - 2 * my_sq * sig_sq * log_factor;
	if (in_squareroot < 0) {
		//in_squareroot =;
	}
	
	float sqrt_term = sqrt(in_squareroot / (square_ray_length * square_ray_length));
	return vec2(leading_coefficient + sqrt_term, leading_coefficient - sqrt_term);
	*/
	float first_term = (gaus.x_coord * ray.x + gaus.y_coord * ray.y - ray.x * currPos.x - ray.y * currPos.y - ray.z * currPos.z) / (ray.x * ray.x + ray.y * ray.y + ray.z * ray.z);
	float sqrt_term = sqrt((-gaus.y_coord * gaus.y_coord) * ray.x * ray.x + 2 * gaus.x_coord * gaus.y_coord * ray.x * ray.y - gaus.x_coord * gaus.x_coord * ray.y * ray.y - 
		gaus.x_coord * gaus.x_coord * ray.z * ray.z - gaus.y_coord * gaus.y_coord * ray.z * ray.z - 2 * gaus.y_coord * ray.x * ray.y * currPos.x + 2 * gaus.x_coord * ray.y * ray.y * currPos.x +
		2 * gaus.x_coord * ray.z * ray.z * currPos.x - ray.y * ray.y * currPos.x * currPos.x - ray.z * ray.z * currPos.x * currPos.x + 2 * gaus.y_coord * ray.x * ray.x * currPos.y -
		2 * gaus.x_coord * ray.x * ray.y * currPos.y + 2 * gaus.y_coord * ray.z * ray.z * currPos.y + 2 * ray.x * ray.y * currPos.x * currPos.y - ray.x * ray.x * currPos.y * currPos.y - 
		ray.z * ray.z * currPos.y * currPos.y - 2 * gaus.x_coord * ray.x * ray.z * currPos.z - 2 * gaus.y_coord * ray.y * ray.z * currPos.z + 2 * ray.x * ray.z * currPos.x * currPos.z + 
		2 * ray.y * ray.z * currPos.y * currPos.z - ray.x * ray.x * currPos.z * currPos.z - ray.y * ray.y * currPos.z * currPos.z + 2 * ray.x * ray.x * gaus.sigma * gaus.sigma * log(gaus.amplitude / iso) + 
		2 * ray.y * ray.y * gaus.sigma * gaus.sigma * log(gaus.amplitude / iso) + 2 * ray.z * ray.z * gaus.sigma * gaus.sigma * log(gaus.amplitude / iso)) / 
		((ray.x * ray.x + ray.y * ray.y + ray.z * ray.z) * (ray.x * ray.x + ray.y * ray.y + ray.z * ray.z));
	return vec2(first_term + sqrt_term, first_term - sqrt_term);
}

void main() {
	vec3 front = (texture(fhp, TexCoord).xyz);
	vec3 back = (texture(bhp, TexCoord).xyz);
	//vec3 view_dir = vec3(0);
	vec3 start = vec3(0);
	vec3 end = vec3(0);
	if (abs(back.x) < EPSILON || abs(back.y) < EPSILON || abs(back.z) < EPSILON) {
		FragColor = vec4(0, 0, 1, 0);
		return;
	}

	if (abs(front.x) < EPSILON || abs(front.y) < EPSILON || abs(front.z) < EPSILON) {
		start = viewPos;
		end = back;
		//FragColor = vec4(vec3(1,0,0), 1.0);
	}
	else {
		start = front;
		end = back;
		//FragColor = vec4(vec3(0, 1, 0), 1.0);
	}
	//FragColor = vec4((back - box_min)/50.00, 1.0);
	//return;

	//view_dir = normalize(FragPos - viewPos);
	vec3 color_composited = vec3(0, 0, 0);
	float opaque_composited = 0;

	bool debug = false;


	vec3 view_dir = normalize(end - viewPos);
	float curr_dist = .08 * (abs(texture(noise, vec2(view_dir)).r) + EPSILON);
	float distance = sqrt(dot(end - start, end - start));
	float raw_depth = texture(depth_tex, TexCoord).x * 2.0f - 1;
	float depth = 2.0 * zNear * zFar / (zFar + zNear - raw_depth * (zFar - zNear));
	distance = min(distance - StepSize, depth);
	float upperBoundStep = 5 * StepSize;
	//FragColor = vec4(vec3(distance / 75.0f), 1.0);
	//FragColor = vec4(viewPos / 50.0f, 1);
	//return;
	float nextDistance = upperBoundStep;
	bool above_arr[6] = { false, false, false, false, false, false };
	vec3 color[6] = {color1, color2, color3, color3, color5, color6};
	bool above = false;
	bool firstStep = true;
	float minStep = 1e-4;
	if (num_gaussians == 0) {
		FragColor = vec4(0, 0, 0, 1);
		return;
	}
	for (curr_dist; curr_dist < distance; curr_dist += nextDistance) {
		vec3 texPoint = start + view_dir * curr_dist;


		nextDistance = distance - curr_dist;
		for (int i = 0; i < num_gaussians; i++) {
			Gaussian gauss = gaussians[i];
			vec2 signed_distances = signed_distance_gauss(gauss, texPoint, view_dir, ( IsoValRange.x));
			float signed_distance = (view_dir.x * view_dir.x + view_dir.y * view_dir.y) * min(abs(signed_distances.x), abs(signed_distances.y));

			//if(above)
				//signed_distance = (view_dir.x * view_dir.x + view_dir.y * view_dir.y) * max(signed_distances.x, signed_distances.y);
			vec3 color_self = color[i %6];
			float opaque_self = 0;

			if (abs(signed_distance) < DISTANCE_EPSILON) {
				color_self = color[i % 6];
				opaque_self = base_opac;
				above = true;
			    //signed_distance = max(signed_distances.x, signed_distances.y);
			}
			nextDistance =  max(min(nextDistance, abs(.5 * (view_dir.z + 1) * signed_distance)), minStep);
			color_composited += ((1.f - opaque_composited) * color_self * opaque_self);
			opaque_composited += (1.f - opaque_composited) * opaque_self;

			if (opaque_composited > .99) {
				FragColor = vec4(color_composited, opaque_composited);
				return;
			}

		}
	}

	if (debug) {
		FragColor = vec4(1, 0, 1, 1);
	}
	else {
		FragColor = vec4(color_composited, opaque_composited);
	}
	return;

}