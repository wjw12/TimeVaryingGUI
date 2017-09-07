#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include "Shader.h"
#include <vector>
#include "GLM.h"

enum ShaderType {	TFShader  , ProcessShader };

class ShaderManager
{
public:
	static void Init();
	static GLuint UseShader(ShaderType shaderType);

private:
	static std::vector<Shader> shaders;

	static void LoadShaders();
	static void CompileShaders();

	ShaderManager();
};

#endif