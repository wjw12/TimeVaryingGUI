#ifndef OPENGL_RENDERER_H
#define OPENGL_RENDERER_H

#define GL_GLEXT_PROTOTYPES 1

#include "VolumeDataset.h"
#include "ShaderManager.h"
#include "GPURaycaster.h"
#include <QOpenGLFunctions>
#include <QOpenGLExtraFunctions>


class OpenGLRenderer
{
public:
    GPURaycaster *raycaster;
	GLuint currTexture3D;
	GLuint lastTexture3D;
	int textureSize;
    unsigned int TF_textureUnit;

    QOpenGLExtraFunctions *f;

    OpenGLRenderer(int screenWidth, int screenHeight, VolumeDataset &volume, Camera &camera, unsigned int TF_Unit);
	void UpdateTexture(int currentTimestep, VolumeDataset &volume);
	GLuint GenerateTexture(VolumeDataset &volume);
    void Draw(VolumeDataset &volume, Camera &camera, ShaderType shaderType);

};

#endif
