#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include "Shader.h"
#include <vector>
#include "GLM.h"

enum ShaderType {FrameVarianceShader, NV_Shader, GradientShader};

class ShaderManager
{
public:
	static void Init();
	static GLuint UseShader(ShaderType shaderType);
    ShaderManager();

private:
	static std::vector<Shader> shaders;

	static void LoadShaders();
	static void CompileShaders();

};

#endif
