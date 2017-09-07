#ifndef TRANSFER_FUNCTION_H
#define TRANSFER_FUNCTION_H

#include "tinyxml2.h"
#include <vector>
#include "GLM.h"
#include <iostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "VolumeDataset.h"
#include "FreeImage.h"

#define tfEPSILON glm::epsilon<float>()

class TransferFunction
{
public:
	std::vector<glm::vec4> colors;
	std::vector<float> intensities;

	float seperate[3]; // seperate different style texture
	
	int numIntensities;
	GLuint tfTexture;
	GLuint styleTexture;
	GLuint tfTexture2d;

	std::vector<glm::vec4> colorTable;

	void Init(const char *filename, VolumeDataset &volume_);
	void LoadXML(const char *filename);
	void LoadLookup(std::vector<glm::vec4> &colorTable);
	void CopyToTex(std::vector<glm::vec4> &data);

	bool LoadStyleTextures(void);
	bool Load2DLookup(void);

	glm::vec4 LERPColor(glm::vec4 firstColor, glm::vec4 secondColor, float firstIntensity, float secondIntensity, float currentIntensity);
};

#endif