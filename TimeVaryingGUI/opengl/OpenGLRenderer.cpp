#include "OpenGLRenderer.h"

OpenGLRenderer::OpenGLRenderer(int screenWidth, int screenHeight, VolumeDataset &volume, Camera &camera, unsigned int TF_Unit)
{
    f = QOpenGLContext::currentContext()->extraFunctions();
    if (!f) {
        std::cout << "Could not obtain OpenGL functions";
        exit(1);
    }

    int bufferSize = volume.xRes * volume.yRes * volume.zRes * volume.bytesPerElement * 2;

    if (!volume.hasData) {
        try {
            volume.memblock3D = new GLubyte[bufferSize];
            volume.hasData = true;
            std::cout << "Allocated buffer size = " << bufferSize << std::endl;
        } catch (const std::bad_alloc& e) {
            std::cout << "Allocation failed: " << e.what() << '\n';
            exit(1);
        }
    }

    //std::cout << "to gen texture";
	currTexture3D = GenerateTexture(volume);
	lastTexture3D = GenerateTexture(volume);

    raycaster = new GPURaycaster(screenWidth, screenHeight, volume);

    TF_textureUnit = TF_Unit;



}


// Updates the texture by copying the block corresponding to the current timestep to GPU memory
void OpenGLRenderer::UpdateTexture(int currentTimestep, VolumeDataset &volume)
{

    std::cout<< "Timestep: " << currentTimestep << std::endl;
    if(volume.volumeType == TimeVarying) {
        int prevtime;
        if (currentTimestep == 0) prevtime = 0;
        else prevtime = currentTimestep - 1;

        int textureSize = volume.xRes * volume.yRes * volume.zRes * volume.bytesPerElement; // one time step

        // read file
        std::ifstream myFile (volume.dataFiles[prevtime], std::ios::in|std::ios::binary);
        if (myFile.is_open()) {
            myFile.read((char*)volume.memblock3D, textureSize);
        }
        else std::cout << "Failed to open file at timestep " << prevtime;
        myFile.close();
        myFile.open(volume.dataFiles[currentTimestep], std::ios::in|std::ios::binary);
        if (myFile.is_open()) {
            myFile.read((char*)volume.memblock3D+textureSize, textureSize);
        }
        else std::cout << "Failed to open file at timestep " << currentTimestep;
        myFile.close();

        glBindTexture(GL_TEXTURE_3D, lastTexture3D);

        if (!volume.littleEndian)
            glPixelStoref(GL_UNPACK_SWAP_BYTES, true);


        if (volume.elementType == "MET_UCHAR")
            f->glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, volume.xRes, volume.yRes, volume.zRes, 0,  GL_RED, GL_UNSIGNED_BYTE, volume.memblock3D);

        else if (volume.elementType == "SHORT")
            f->glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, volume.xRes, volume.yRes, volume.zRes, 0, GL_RED, GL_UNSIGNED_SHORT, volume.memblock3D);

        else if (volume.elementType == "FLOAT")
            f->glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, volume.xRes, volume.yRes, volume.zRes, 0, GL_RED, GL_FLOAT, volume.memblock3D);

        glPixelStoref(GL_UNPACK_SWAP_BYTES, false);

        // volume data at present
        glBindTexture(GL_TEXTURE_3D, currTexture3D);

        if (!volume.littleEndian)
            glPixelStoref(GL_UNPACK_SWAP_BYTES, true);

        if (volume.elementType == "MET_UCHAR")
            f->glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, volume.xRes, volume.yRes, volume.zRes, 0,  GL_RED, GL_UNSIGNED_BYTE, volume.memblock3D + textureSize);

        else if (volume.elementType == "SHORT")
            f->glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, volume.xRes, volume.yRes, volume.zRes, 0, GL_RED, GL_UNSIGNED_SHORT, volume.memblock3D + textureSize);

        else if (volume.elementType == "FLOAT")
            f->glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, volume.xRes, volume.yRes, volume.zRes, 0, GL_RED, GL_FLOAT, volume.memblock3D + textureSize);

        glPixelStoref(GL_UNPACK_SWAP_BYTES, false);
    }
    else {
        // static data

        glBindTexture(GL_TEXTURE_3D, currTexture3D);

        if (!volume.littleEndian)
            glPixelStoref(GL_UNPACK_SWAP_BYTES, true);

        if (volume.elementType == "MET_UCHAR")
            f->glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, volume.xRes, volume.yRes, volume.zRes, 0,  GL_RED, GL_UNSIGNED_BYTE, volume.memblock3D);

        else if (volume.elementType == "SHORT")
            f->glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, volume.xRes, volume.yRes, volume.zRes, 0, GL_RED, GL_UNSIGNED_SHORT, volume.memblock3D);

        else if (volume.elementType == "FLOAT")
            f->glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, volume.xRes, volume.yRes, volume.zRes, 0, GL_RED, GL_FLOAT, volume.memblock3D);

        glPixelStoref(GL_UNPACK_SWAP_BYTES, false);
    }

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
        f->glTexImage3D(GL_TEXTURE_3D, 0, GL_R8, volume.xRes, volume.yRes, volume.zRes, 0,  GL_RED, GL_UNSIGNED_BYTE, volume.memblock3D);

	else if (volume.elementType == "SHORT")
        f->glTexImage3D(GL_TEXTURE_3D, 0, GL_R16F, volume.xRes, volume.yRes, volume.zRes, 0, GL_RED, GL_UNSIGNED_SHORT, volume.memblock3D);

	else if (volume.elementType == "FLOAT")
        f->glTexImage3D(GL_TEXTURE_3D, 0, GL_R32F, volume.xRes, volume.yRes, volume.zRes, 0, GL_RED, GL_FLOAT, volume.memblock3D);

	glPixelStoref(GL_UNPACK_SWAP_BYTES, false);

	GLenum err = glGetError();
	
	glBindTexture(GL_TEXTURE_3D, 0);

	return tex;
}


void OpenGLRenderer::Draw(VolumeDataset &volume, Camera &camera, ShaderType shaderType)
{
	glEnable(GL_DEPTH_TEST); 
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // necessary, don't forget!

    GLuint shaderProgramID = ShaderManager::UseShader(shaderType);
    raycaster->Raycast(currTexture3D, lastTexture3D, TF_textureUnit, shaderProgramID, camera);
}

