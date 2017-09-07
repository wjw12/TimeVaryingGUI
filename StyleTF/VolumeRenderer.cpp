#include "VolumeRenderer.h"

void VolumeRenderer::Init(int screenWidth, int screenHeight)
{
	//glEnable(GL_DEPTH_TEST | GL_FRAMEBUFFER_SRGB | GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	camera.Init(screenWidth, screenHeight);
	volume.Init();
	ShaderManager::Init();

	renderer = new OpenGLRenderer(screenWidth, screenHeight, volume, camera);

	currentTimestep = 0;
}


void VolumeRenderer::Update()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	

	camera.Update();
	
	// If dataset is time variant advance time and update GPU's texture
	if (volume.timesteps > 1)
	{
		clock_t currentTime = clock();
		float time = (currentTime - oldTime) / (float) CLOCKS_PER_SEC;

		if (time > volume.timePerFrame)
		{
			if (currentTimestep < volume.timesteps - 2)
				currentTimestep++;
			else
				currentTimestep = 0;

			oldTime = currentTime;

			renderer->UpdateTexture(currentTimestep, volume);
		}	
	}

	renderer->Draw(volume, camera);

	glutSwapBuffers();
}