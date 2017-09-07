#include "OpenGLRenderer.h"

OpenGLRenderer::OpenGLRenderer(int screenWidth, int screenHeight, VolumeDataset &volume, Camera &camera)
{
	currTexture3D = GenerateTexture(volume);

	raycaster = new GPURaycaster(screenWidth, screenHeight, volume);
	transferFunction.Init(" ", volume);
}


// Updates the texture by copying the block corresponding to the current timestep to GPU memory
void OpenGLRenderer::UpdateTexture(int currentTimestep, VolumeDataset &volume)
{
	glBindTexture(GL_TEXTURE_3D, currTexture3D);

	if (!volume.littleEndian)
		glPixelStoref(GL_UNPACK_SWAP_BYTES, true);

	if (volume.elementType == "MET_UCHAR")
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, volume.xRes, volume.yRes, volume.zRes, 0,  GL_RED, GL_UNSIGNED_BYTE, volume.memblock3D + (textureSize * currentTimestep));

	else if (volume.elementType == "SHORT")
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, volume.xRes, volume.yRes, volume.zRes, 0, GL_RED, GL_UNSIGNED_SHORT, volume.memblock3D + (textureSize * currentTimestep));

	else if (volume.elementType == "FLOAT")
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, volume.xRes, volume.yRes, volume.zRes, 0, GL_RED, GL_FLOAT, volume.memblock3D + (textureSize * currentTimestep));

	glPixelStoref(GL_UNPACK_SWAP_BYTES, false);

	glBindTexture(GL_TEXTURE_3D, 0);
}


// Generates the original 3D texture
GLuint OpenGLRenderer::GenerateTexture(VolumeDataset &volume)
{
	GLuint tex;
	textureSize = volume.xRes * volume.yRes * volume.zRes * volume.bytesPerElement;

	glEnable(GL_TEXTURE_3D);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_3D, tex);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);


	// Reverses endianness in copy
	if (!volume.littleEndian)
		glPixelStoref(GL_UNPACK_SWAP_BYTES, true);

	if (volume.elementType == "MET_UCHAR")
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, volume.xRes, volume.yRes, volume.zRes, 0,  GL_RED, GL_UNSIGNED_BYTE, volume.memblock3D);

	else if (volume.elementType == "SHORT")
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, volume.xRes, volume.yRes, volume.zRes, 0, GL_RED, GL_UNSIGNED_SHORT, volume.memblock3D);

	else if (volume.elementType == "FLOAT")
		glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, volume.xRes, volume.yRes, volume.zRes, 0, GL_RED, GL_FLOAT, volume.memblock3D);

	glPixelStoref(GL_UNPACK_SWAP_BYTES, false);

	GLenum err = glGetError();
	
	glBindTexture(GL_TEXTURE_3D, 0);

	// render to a texture ------------------------------------------------------
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	// generate texture
	glGenTextures(1, &texColorBuffer);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1280, 720, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);

	// attach it to currently bound framebuffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

	// generate render buffer
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1280, 720);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	// configure frame buffer
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texColorBuffer, 0);

	// attach render buffer to frame buffer
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

	float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		// positions   // texCoords
		-1.0f, 1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 0.0f,
		1.0f, -1.0f, 1.0f, 0.0f,

		-1.0f, 1.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f
	};
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	return tex;
}


void OpenGLRenderer::Draw(VolumeDataset &volume, Camera &camera)
{

	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glEnable(GL_DEPTH_TEST); 
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // necessary, don't forget!

	GLuint shaderProgramID = ShaderManager::UseShader(TFShader);
	raycaster->Raycast(currTexture3D, transferFunction, shaderProgramID, camera);


	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	shaderProgramID = ShaderManager::UseShader(ProcessShader);
	glActiveTexture(GL_TEXTURE3);
	int uniformLoc;
	uniformLoc = glGetUniformLocation(shaderProgramID, "screenTexture");
	glUniform1i(uniformLoc, 3);
	
	glBindVertexArray(quadVAO);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);
	glDrawArrays(GL_TRIANGLES, 0, 6);


}

