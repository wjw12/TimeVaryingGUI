#ifndef GPU_RAYCASTER_H
#define GPU_RAYCASTER_H

#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Camera.h"
#include "GLM.h"
#include <vector>
#include "VolumeDataset.h"
#include "TransferFunction.h"
#include <string>

class GPURaycaster
{
public:
	int maxRaySteps;
	float rayStepSize, gradientStepSize;
	glm::vec3 lightPosition;


	GPURaycaster(int screenWidth, int screenHeight, VolumeDataset &volume);
	void Raycast(GLuint currTexture3D, TransferFunction &transferFunction, GLuint shaderProgramID, Camera &camera);
};

#endif

