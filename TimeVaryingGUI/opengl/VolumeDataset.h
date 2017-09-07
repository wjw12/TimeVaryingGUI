#ifndef VOLUME_DATASET_H
#define VOLUME_DATASET_H

#include "VoxelReader.h"
#include <string>
#include "GLM.h"
#include "Camera.h"
#include <QOpenGLFunctions>


enum VolumeType {TimeVarying, Static};


class VolumeDataset
{
public:
	GLubyte *memblock3D;

	int timesteps;
	float timePerFrame;
	int numDims;
	int xRes, yRes, zRes;
	int bytesPerElement;
	bool littleEndian;
	std::string elementType;
    VolumeType volumeType;

    std::vector<std::string> dataFiles;
    bool hasData;

	VoxelReader voxelReader;

    void Init(std::string pathToHeader, std::string headerFileName);
	void ReverseEndianness();
    void Clean();
};



#endif
