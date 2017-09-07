#include "VolumeDataset.h"

// Initializes volume by calling voxel reader and copying in values from header
void VolumeDataset::Init(std::string pathToHeader, std::string headerFileName)
{
    VolumeProperties properties;
    voxelReader.LoadVolume(pathToHeader, headerFileName, properties);

	memblock3D = properties.bufferAddress;
	timesteps = properties.timesteps;
	timePerFrame = properties.timePerFrame;
	numDims = properties.numDims;
	xRes = properties.xRes;
	yRes = properties.yRes;
	zRes = properties.zRes;
	bytesPerElement = properties.bytesPerElement;
	littleEndian = properties.littleEndian;
	elementType = properties.elementType;
    dataFiles = properties.dataFiles;

    if (timesteps > 1) {volumeType = TimeVarying; hasData = false;}
    else {volumeType = Static; hasData = true;}
    std::cout << "Volume dataset init done";

}

void VolumeDataset::Clean() {
    delete[] memblock3D;
    voxelReader = VoxelReader();
    hasData = false;
}

// Only needed if you want to analyse the volume on the CPU and the endianness is in the wrong order
void VolumeDataset::ReverseEndianness()
{
	std::vector<GLubyte> bytes;
	bytes.resize(xRes * yRes * zRes * bytesPerElement);

	for (int i=0; i<bytes.size(); i+=bytesPerElement)
	{
		for (int j=0; j<bytesPerElement; j++)
		{
			bytes[i+j] = memblock3D[i+(bytesPerElement-(1+j))];
		}
	}

	memcpy(memblock3D, &bytes[0], xRes * yRes * zRes * bytesPerElement);
}


