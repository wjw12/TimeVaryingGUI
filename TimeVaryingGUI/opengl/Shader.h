#ifndef SHADERS_H
#define SHADERS_H

#define GL_GLEXT_PROTOTYPES 1

#include <string>
#include <fstream>
#include <iostream>
#include <QOpenGLFunctions>

using namespace std;

class Shader
{
public:
    Shader() {f = QOpenGLContext::currentContext()->functions();}
	char pVS[10000];
	char pFS[10000];
	GLuint ID;

    QOpenGLFunctions *f;

	void Load(string vName, string fName);
	void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType);
	GLuint CompileShaders();
};

#endif
