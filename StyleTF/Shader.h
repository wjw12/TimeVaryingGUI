#ifndef SHADERS_H
#define SHADERS_H

#include <GL/glew.h>
#include <GL/freeglut.h>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

class Shader
{
public:
	char pVS[10000];
	char pFS[10000];
	GLuint ID;

	void Load(string vName, string fName);
	void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType);
	GLuint CompileShaders();
};

#endif