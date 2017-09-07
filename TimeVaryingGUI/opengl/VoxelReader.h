#ifndef VOXEL_READER_H
#define VOXEL_READER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <vector>
#include <Windows.h>
#include <QOpenGLFunctions>
#include <QOpenGLContext>

struct VolumeProperties
{
	GLubyte* bufferAddress;
	int timesteps;
	float timePerFrame;
	int numDims;
	int xRes, yRes, zRes;
	std::string rawFilePath;
	int bytesPerElement;
	bool littleEndian;
	std::string elementType;
    std::vector<std::string> dataFiles;

	VolumeProperties();
};

class VoxelReader
{
public:
	std::vector<std::string> files;

	void LoadVolume(std::string folderPath, std::string headerFile, VolumeProperties &properties);
	void CopyFileToBuffer(std::string fileName, long int &numBytesInBufferFilled, VolumeProperties &properties);

private:
	void ReadMHD(std::string folderPath, std::string headerFile, VolumeProperties &properties);
	void ReadRaw(VolumeProperties &properties);
	
};


#endif
