#define STB_IMAGE_IMPLEMENTATION
#include "TransferFunction.h"
#include "stb_image.h"

void TransferFunction::Init(const char *filename, VolumeDataset &volume_)
{
	// Want the transfer functions to have 256 possible values
	colorTable.resize(256);
	LoadXML(filename);

	colors.resize(numIntensities);

	//Generate the 1D texture
	glGenTextures(1, &tfTexture);
	glBindTexture(GL_TEXTURE_1D, tfTexture);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_FLOAT, 0); 
	glBindTexture(GL_TEXTURE_1D, 0);
	
	LoadLookup(colorTable);

	// generate 2D texture array
	
	glGenTextures(1, &styleTexture);
	glBindTexture(GL_TEXTURE_2D_ARRAY, styleTexture);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, 800, 800, 3, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, 800, 800, 3, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	bool s = LoadStyleTextures();
	if(!s) {
		std::cerr << "Load Texture failed!";
	}
	
}

bool TransferFunction::LoadStyleTextures()
{
	std::string folder("../../styles/");
	std::string filenames[3] = {"brush.png", "brush.png", "brush.png"};

	int width, height, xx;
	unsigned char* data;

	glBindTexture(GL_TEXTURE_2D_ARRAY, styleTexture);

	for (int i=0; i<3; i++){
		std::string filename = folder + filenames[i];

		data = stbi_load(filename.c_str(), &width, &height, &xx, 0);

		if(!data) std::cout << "Image load error";

		if (xx == 3)
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, GL_RGB, GL_UNSIGNED_BYTE, data);
		else if (xx == 4)
			glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);

		stbi_image_free(data);
	}

	return true;

}



// Reads XML file transfer function and copies it into the tables
void TransferFunction::LoadXML(const char *filename)
{
	tinyxml2::XMLDocument doc;
	//	tinyxml2::XMLError r = doc.LoadFile("D:/Project at TCD/code/transferfuncs/CT-Knee2.tfi");
	//	tinyxml2::XMLError r = doc.LoadFile("D:/Project at TCD/code/transferfuncs/tooth.tfi");
	tinyxml2::XMLError r = doc.LoadFile("C:/Users/GV2/Desktop/wjw/code/transferfuncs/nucleon.tfi");
	//	tinyxml2::XMLError r = doc.LoadFile("../../transferfuncs/VisMale_spectrum_4_balance_1000.tfi");
	//	tinyxml2::XMLError r = doc.LoadFile("../../transferfuncs/BlueSmoke.tfi");

	if (r != tinyxml2::XML_NO_ERROR)
		std::cout << "failed to open transfunc xml file" << std::endl;

	tinyxml2::XMLElement* transFuncIntensity = doc.FirstChildElement("VoreenData")->FirstChildElement("TransFuncIntensity");

	tinyxml2::XMLElement* key = doc.FirstChildElement("VoreenData")->FirstChildElement("TransFuncIntensity")->FirstChildElement("Keys")->FirstChildElement("key");

	while (key)
	{
		float intensity = atof(key->FirstChildElement("intensity")->Attribute("value"));
		intensities.push_back(intensity);

		int r = atoi(key->FirstChildElement("colorL")->Attribute("r"));
		int g = atoi(key->FirstChildElement("colorL")->Attribute("g"));
		int b = atoi(key->FirstChildElement("colorL")->Attribute("b"));
		int a = atoi(key->FirstChildElement("colorL")->Attribute("a"));

		colors.push_back(glm::vec4(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f));

		key = key->NextSiblingElement();
	}

	numIntensities = intensities.size();
}

// Because file only specifies a number of control points, a full color table must be made from these control points by linearly interpolating
void TransferFunction::LoadLookup(std::vector<glm::vec4> &colorTable)
{
	glm::vec4 previousColor(0.0f);
	float previousIntensity = 0.0f;
	int next = 0;

	for (int i=0; i<256; i++)
	{
		float currentIntensity = (float)i / (float)255;

		while (next < numIntensities && currentIntensity > intensities[next])
		{
			previousIntensity = intensities[next];
			previousColor = colors[next];
			next++;
		}

		if (next < numIntensities)
			colorTable[i] = LERPColor(previousColor, colors[next], previousIntensity, intensities[next], currentIntensity);
		else
			colorTable[i] = LERPColor(previousColor, glm::vec4(0.0f), previousIntensity, 1.0f, currentIntensity);
	}
	
	int j = 2;
	for (int i=255; i > 0; i--) {
		if (colorTable[i].a == 0 && colorTable[i-1].a > 0) {
			seperate[j] = (float) i / (float) 255;
			j--;
		}
		if (j < 0) break;
	}

	CopyToTex(colorTable);
}

// Copy final table to GPU memory
void TransferFunction::CopyToTex(std::vector<glm::vec4> &data)
{
	glBindTexture(GL_TEXTURE_1D, tfTexture);
	glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 256, GL_RGBA, GL_FLOAT, &data[0]);
	glBindTexture(GL_TEXTURE_1D, 0);
}


// Linearly interpolate between two control points
glm::vec4 TransferFunction::LERPColor(glm::vec4 firstColor, glm::vec4 secondColor, float firstIntensity, float secondIntensity, float currentIntensity)
{
	float difference = secondIntensity - firstIntensity;

	if (difference > 0.0001f)
	{
		float fraction = (currentIntensity - firstIntensity) / difference;

		return firstColor + ((secondColor - firstColor) * fraction);
	}
	else
		return firstColor;
}
