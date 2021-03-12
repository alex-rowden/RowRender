#include "LICPrecompute.h"

#include <glm/gtx/string_cast.hpp>
int LICPrecompute() {
	struct Gbuffer {
		GLuint frame_buffer, normal_tex, tangent_tex,
			color_tex, frag_pos_tex, color_array_tex, ellipsoid_coordinates_tex,
			depth_render_buf, force_framebuffer, force_renderbuffer, force_tex, lic_color_tex,
			lic_accum_framebuffer[2], lic_accum_renderbuffer[2],
			ssao_tex, ssao_framebuffer, ssao_renderbuffer,
			ssao_blur_tex, ssao_blur_framebuffer, ssao_blur_renderbuffer;
		GLuint pboIDs[2], lic_tex[2], lic_framebuffer[2], lic_renderbuffer[2], lic_accum_tex[2];
		Texture2D color_texture, frag_pos_texture, tangent_texture,
			normal_texture, ellipsoid_coordinates_texture, force_texture,
			lic_texture[2], lic_accum_texture[2], lic_color_texture,
			ssao_texture, ssao_blur_texture ;
	};

	Gbuffer *buffer = new Gbuffer();
	glm::uvec2 resolution = glm::uvec2(2048, 2048);
	std::string filename = "LIC_image_" + std::to_string(resolution.x)
		+ "_" + std::to_string(resolution.y) + ".bmp";

	//Set Framebuffer Attributes
	glBindFramebuffer(GL_FRAMEBUFFER, buffer->frame_buffer);

	glBindTexture(GL_TEXTURE_2D, buffer->normal_tex);
	buffer->normal_texture = Texture2D();
	buffer->normal_texture.SetTextureID(buffer->normal_tex);
	buffer->normal_texture.giveName("normal_tex");

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer->normal_tex, 0);


	glBindTexture(GL_TEXTURE_2D, buffer->color_tex);
	buffer->color_texture = Texture2D();
	buffer->color_texture.SetTextureID(buffer->color_tex);
	buffer->color_texture.giveName("albedo_tex");

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, buffer->color_tex, 0);

	glBindTexture(GL_TEXTURE_2D, buffer->frag_pos_tex);
	buffer->frag_pos_texture = Texture2D();
	buffer->frag_pos_texture.SetTextureID(buffer->frag_pos_tex);
	buffer->frag_pos_texture.giveName("fragPos_tex");

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, buffer->frag_pos_tex, 0);

	glBindBuffer(GL_PIXEL_PACK_BUFFER, buffer->pboIDs[0]);
	glBufferData(GL_PIXEL_PACK_BUFFER, resolution.x * resolution.y * 4 * sizeof(float), 0, GL_STREAM_READ);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, buffer->pboIDs[1]);
	glBufferData(GL_PIXEL_PACK_BUFFER, resolution.x * resolution.y * 4 * sizeof(float), 0, GL_STREAM_READ);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, buffer->ellipsoid_coordinates_tex);
	buffer->ellipsoid_coordinates_texture = Texture2D();
	buffer->ellipsoid_coordinates_texture.SetTextureID(buffer->ellipsoid_coordinates_tex);
	buffer->ellipsoid_coordinates_texture.giveName("ellipsoid_coordinates_tex");

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, buffer->ellipsoid_coordinates_tex, 0);

	glBindTexture(GL_TEXTURE_2D, buffer->tangent_tex);
	buffer->tangent_texture = Texture2D();
	buffer->tangent_texture.SetTextureID(buffer->tangent_tex);
	buffer->tangent_texture.giveName("tangent_tex");

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT4, GL_TEXTURE_2D, buffer->tangent_tex, 0);

	const int numAttachments = 5;

	GLenum DrawBuffers[numAttachments];
	for (int i = 0; i < numAttachments; ++i)
		DrawBuffers[i] = GL_COLOR_ATTACHMENT0 + i; //Sets appropriate indices for each color buffer

	glDrawBuffers(numAttachments, DrawBuffers);


	glBindRenderbuffer(GL_RENDERBUFFER, buffer->depth_render_buf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution.x, resolution.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer->depth_render_buf);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "framebuffer broke" << std::endl;
		return 0;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, buffer->force_framebuffer);

	glBindTexture(GL_TEXTURE_2D, buffer->force_tex);
	buffer->force_texture = Texture2D();
	buffer->force_texture.SetTextureID(buffer->force_tex);
	buffer->force_texture.giveName("force_tex");

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer->force_tex, 0);

	glBindTexture(GL_TEXTURE_2D, buffer->lic_color_tex);
	buffer->lic_color_texture = Texture2D();
	buffer->lic_color_texture.SetTextureID(buffer->lic_color_tex);
	buffer->lic_color_texture.giveName("lic_color_tex");

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, resolution.x, resolution.y, 0, GL_RGB, GL_FLOAT, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, buffer->lic_color_tex, 0);

	GLenum drawBuffer[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, drawBuffer);

	glBindRenderbuffer(GL_RENDERBUFFER, buffer->force_renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution.x, resolution.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer->force_renderbuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "lic prepass framebuffer broke" << std::endl;
		return 0;
	}

	for (int i = 0; i < 2; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, buffer->lic_framebuffer[i]);

		glBindTexture(GL_TEXTURE_2D, buffer->lic_tex[i]);
		buffer->lic_texture[i] = Texture2D();
		buffer->lic_texture[i].SetTextureID(buffer->lic_tex[i]);
		buffer->lic_texture[i].giveName("lic_tex[" + std::to_string(i) + "]");

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer->lic_tex[i], 0);

		drawBuffer[0] = { GL_COLOR_ATTACHMENT0 };
		glDrawBuffers(1, drawBuffer);

		glBindRenderbuffer(GL_RENDERBUFFER, buffer->lic_renderbuffer[i]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution.x, resolution.y);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer->lic_renderbuffer[i]);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cout << "lic framebuffer broke" << std::endl;
			return 0;
		}
	}
	for (int i = 0; i < 2; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, buffer->lic_accum_framebuffer[i]);

		glBindTexture(GL_TEXTURE_2D, buffer->lic_accum_tex[i]);
		buffer->lic_accum_texture[i] = Texture2D();
		buffer->lic_accum_texture[i].SetTextureID(buffer->lic_accum_tex[i]);
		buffer->lic_accum_texture[i].giveName("lic_accum_tex[" + std::to_string(i) + "]");

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, resolution.x, resolution.y, 0, GL_RGBA, GL_FLOAT, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, buffer->lic_accum_tex[i], 0);

		//drawBuffer = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACMENT0};
		glDrawBuffers(1, drawBuffer);

		glBindRenderbuffer(GL_RENDERBUFFER, buffer->lic_accum_renderbuffer[i]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution.x, resolution.y);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, buffer->lic_accum_renderbuffer[i]);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cout << "lic accumulation framebuffer broke" << std::endl;
			return 0;
		}
	}


	
	
	Model quad = Model(". / Content / Models / quad / quad_centered.obj", true);
	quad.setModel();

	ShaderProgram model_shader = ShaderProgram({
		ShaderProgram::Shaders::FRAG_ELLIPSOID,
		ShaderProgram::Shaders::VERT_ELLIPSOID });
	ShaderProgram prepass_shader = ShaderProgram({
		ShaderProgram::Shaders::LIC_PREPASS_FRAG,
		ShaderProgram::Shaders::LIC_PREPASS_VERT});
	ShaderProgram lic_shader = ShaderProgram({
		ShaderProgram::Shaders::LIC_FRAG,
		ShaderProgram::Shaders::LIC_PREPASS_VERT });
	ShaderProgram lic_accum_shader = ShaderProgram({
		ShaderProgram::Shaders::LIC_ACCUM_FRAG,
		ShaderProgram::Shaders::LIC_PREPASS_VERT });

	if (true) {
		// Make the BYTE array, factor of 3 because it's RBG.
		unsigned char* pixels = new unsigned char[3 * resolution.x * resolution.y];

		glReadPixels(0, 0, resolution.x, resolution.y, GL_RGB, GL_UNSIGNED_BYTE, pixels);
		stbi_flip_vertically_on_write(true);
		int save_result = stbi_write_png
		(
			filename.c_str(),
			resolution.x, resolution.y,
			3, pixels, resolution.x * 3
		);
		if (save_result == 0) {
			std::cout << "shit" << std::endl;
		}
		delete[] pixels;
	}
	return 1;
}