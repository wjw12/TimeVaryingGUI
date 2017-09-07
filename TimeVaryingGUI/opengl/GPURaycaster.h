#ifndef GPU_RAYCASTER_H
#define GPU_RAYCASTER_H

#define GL_GLEXT_PROTOTYPES 1

#include "Camera.h"
#include "GLM.h"
#include <vector>
#include "VolumeDataset.h"
#include <string>
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>

class GPURaycaster
{
public:
	int maxRaySteps;
	float rayStepSize, gradientStepSize;
	glm::vec3 lightPosition;
    QOpenGLFunctions *f;

	GPURaycaster(int screenWidth, int screenHeight, VolumeDataset &volume);
    void Raycast(GLuint currTexture3D, GLuint lastTexture3D, GLuint TF_textureUnit, GLuint shaderProgramID, Camera &camera);

private:
    VolumeDataset volumeDataset;
};

#endif

