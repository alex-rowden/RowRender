#version 430

out vec4 ssao_blur_tex;

in vec2 TexCoords;

uniform sampler2D ssao_tex;

void main(){
	 vec2 texelSize = 1.0 / vec2(textureSize(ssao_tex, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(ssao_tex, TexCoords + offset).r;
        }
    }
    ssao_blur_tex.r = result / (4.0 * 4.0);
    ssao_blur_tex.a = 1;
}