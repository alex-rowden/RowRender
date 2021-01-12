#version 430

#define NUM_STEPS 12
#define MAX_ROUTERS 20

out vec4 LIC;

in vec2 TexCoord;

uniform sampler2D normal_tex,
				  tangent_tex,
				  fragPos_tex,
				  ellipsoid_coordinates_tex,
				  force_tex,
				  lic_tex[2],
				  noise_tex[MAX_ROUTERS];

uniform int ellipsoid_index_offset, num_ellipsoids, router_num, power, step_num;
uniform bool screen_space_lic, procedural_noise, cull_discontinuities, use_mask;
uniform float alpha_boost, learning_rate,
frag_pos_scale, density;

uniform mat4 projection, camera;

uniform vec3 color;

float rand(vec2 st) {
	return step(fract(sin(dot(st.xy,
		vec2(12.9898, 78.233))) *
		43758.5453123), density);
}

float rand(vec3 p3)
{
	p3 = fract(p3 * .1031);
	p3 += dot(p3, p3.yzx + 33.33);
	float ret = fract((p3.x + p3.y) * p3.z) - density;
	if (ret < 0)
		return 0.f;
	else
		return ret / (1 - density);
}

float noise(vec2 n, int ellipsoid_num) {
	if (procedural_noise) {
		vec2 i = floor(n);
		vec2 f = fract(n);
		float a = rand(i);
		float b = rand(i + vec2(1.0, 0.0));
		float c = rand(i + vec2(0.0, 1.0));
		float d = rand(i + vec2(1.0, 1.0));
		vec2 u = smoothstep(0., 1., f);
		return mix(a, b, u.x) +
			(c - a) * u.y * (1.0 - u.x) +
			(d - b) * u.x * u.y;
	}
	return texture(noise_tex[ellipsoid_num], n).r;
}
float noise(vec3 x) {
	vec3 i = floor(x);
	vec3 f = fract(x);
	f = f * f * (3.0 - 2.0 * f);

	return	 mix(mix(mix(rand(i + vec3(0, 0, 0)),
						rand(i + vec3(1, 0, 0)), f.x),
					mix(rand(i + vec3(0, 1, 0)),
						rand(i + vec3(1, 1, 0)), f.x), f.y),
				mix(mix(rand(i + vec3(0, 0, 1)),
						rand(i + vec3(1, 0, 1)), f.x),
					mix(rand(i + vec3(0, 1, 1)),
						rand(i + vec3(1, 1, 1)), f.x), f.y), f.z);
}

float calculateMask(int dir, int j) {
	float mask = 0;
	if (use_mask) {
		mask = cos(2. * 3.1415 * (NUM_STEPS - j) / (NUM_STEPS));
		return (0.5 + 0.5 * mask);
	}return NUM_STEPS - dir * j;
}


vec2 getScreenCoords(vec3 fragPos) {
	vec4 screenSpaceCoords = projection * camera * vec4(fragPos.xyz, 1);
	screenSpaceCoords /= screenSpaceCoords.w;
	vec2 fakeTex = screenSpaceCoords.xy * .5 + vec2(.5);
	return fakeTex;
}

float renderLIC(vec3 fragPos, vec3 tangent, vec3 bitangent, vec3 Normal) {
	float alpha_new = 0;
	float scale = 1;
	if (screen_space_lic && !procedural_noise)
		scale = .1;
	else if (screen_space_lic && procedural_noise)
		scale = 100;
	else if (!screen_space_lic && procedural_noise)
		scale = 10;
	

	vec3 oldFragPos = fragPos, oldTangent = tangent, oldNormal = Normal,
		oldBitangent = bitangent, originalFragPos = fragPos, originalTangent = tangent;
	float norm;
	float val;
	float step_size = learning_rate * .02;

	vec2 uv;
	vec3 uvt;

	vec3 projected_frag, projected_coords, newFragPos;
	int dir = -1;
	for (int num_directions = 0; num_directions < 2; num_directions++) {
		fragPos = originalFragPos;
		uvt = fragPos;
		Normal = oldNormal;
		
		tangent = originalTangent;
		oldTangent = tangent;
		bitangent = oldBitangent;
		mat3 wallToWorldCoords = mat3(tangent, bitangent, Normal);
		mat3 worldToWallCoords = transpose(wallToWorldCoords);
		uv = TexCoord;
		int j = 0;
		if (num_directions > 0)
			dir = 1;
		if (dir > 0)
			j = 1;
		for (j; j < NUM_STEPS / 2; j++) {
			if (screen_space_lic) {
				uvt = texture(ellipsoid_coordinates_tex, uv).xyz;
			}
			
			float mask = calculateMask(dir, j);// ;
			//mask *= mask;
			if(step_num != 0){
				val += mask * texture(lic_tex[(step_num + 1) % 2], uv).a;
			}
			else if ( procedural_noise)
				val += noise(frag_pos_scale * scale * uvt.rgb) * mask;
			else{
				float noise_val = 0;
				noise_val += noise(frag_pos_scale * scale * uvt.rg, router_num);
				//noise_val = max(noise_val, noise(frag_pos_scale / 5.0f * scale * uvt.rg));
				val += noise_val * mask;
			}
			norm += mask;
			

			vec3 direction = texture(force_tex, uv).xyz;
			float magnitude = length(direction);
			if(magnitude == 0){
				if(j == 0)
					return 0;
				break;
			}
			vec3 force = dir * (direction) / magnitude;
			
			oldFragPos = fragPos;
			fragPos += step_size * force;
			
			if (screen_space_lic) {
				uv = getScreenCoords(fragPos);
				
				if (uv.x > 1 || uv.y > 1 || uv.x < 0 || uv.y < 0)
					break;
				newFragPos = texture(fragPos_tex, uv).rgb;
				float delta = distance(newFragPos, oldFragPos);
				//check for areas of depth discontinuity
				if (distance(newFragPos, fragPos) > .005) {
					break;
				}
				
				tangent = texture(tangent_tex, uv).rgb;
				//check if we changed surfaces
				if (abs(dot(tangent, oldTangent)) < 1e-3) {
					
					if (cull_discontinuities) {
						float angle = acos(dot(newFragPos - oldFragPos, dir * direction * step_size) / (dir * magnitude * delta * step_size));
						float altitude = sin(angle) * delta;
						float partial_distance = sqrt(delta * delta - altitude * altitude);
						float remaining_distance = step_size - magnitude * partial_distance;
						
						fragPos = oldFragPos + partial_distance * force;
						//fragPos = texture(fragPos_tex, getScreenCoords(fragPos)).rgb;						
						
						Normal = texture (normal_tex, uv).rgb;
						bitangent = cross(Normal, tangent);
						wallToWorldCoords = mat3(tangent, bitangent, Normal);
						worldToWallCoords = transpose(wallToWorldCoords);
						vec3 new_direction = texture2D(force_tex, uv).xyz;
						//if new direction and old direction point toward each other, flip the new direction
						//if (dot(force, dir * new_direction) < 0) {
							
						//	dir *= -1;
						//}
						
						fragPos += 2 * dir * remaining_distance * normalize(new_direction);
						if (any(notEqual(texture(normal_tex, getScreenCoords(fragPos)).rgb, Normal))) {
							dir *= -1; 
							fragPos += 3 * dir * remaining_distance * normalize(new_direction);
						}
						else {
							fragPos -= dir * remaining_distance * normalize(new_direction);
						}
						uv = getScreenCoords(fragPos);
						
						
						oldTangent = tangent;
						newFragPos = texture(fragPos_tex, uv).rgb;
					}
					
					
					
				}
				fragPos = newFragPos;
			}
			else {
				uvt = fragPos;
			}
			
			//projected_frag = 

		}
		dir *= -1;
	}
	alpha_new = float(alpha_boost * pow(val / norm, power));
	
	//alpha_new = sqrt(alpha_new);
	//alpha_new *= alpha_new;
	//alpha_new *= (1 - .9 * distance / extent);
	alpha_new = min(alpha_new, 1);
	return alpha_new;
}

void main(){
	
	vec3 fragPos = texture(fragPos_tex, TexCoord).xyz;
	vec3 normal = texture(normal_tex, TexCoord).xyz;
	if(abs(dot(normal, vec3(0,0,1))) > (1 - 1e-3)){
		LIC = vec4(0,0,0,0);
		return;
	}
	vec3 tangent = texture(tangent_tex, TexCoord).xyz;
	vec3 bitangent = normalize(cross(tangent, normal));

	float lic = renderLIC(fragPos, tangent, bitangent, normal);
	
	LIC = vec4(color, lic);
}