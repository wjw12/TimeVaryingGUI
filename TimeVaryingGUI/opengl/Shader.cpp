#include "Shader.h"
#include <QOpenGLFunctions>

using namespace std;

//Reads in shader text files
// vertex shader and fragment shader
void Shader::Load(string vName, string fName)
{
	ifstream readFileV;
	readFileV.open(vName.c_str());
	int i = 0;

	if (readFileV.is_open())
	{
		while(readFileV.good())
		{
			pVS[i] = readFileV.get();
			if (!readFileV.eof())
			{
				i++;
			}
			pVS[i] = 0;
		}
		readFileV.close();
	} else 
	{
		cout << "Unable to open vert shader file: " << vName.c_str() << endl;
	}
	ifstream readFileF;
	readFileF.open(fName.c_str());
	i = 0;

	if (readFileF.is_open())
	{
		while(readFileF.good())
		{
			pFS[i] = readFileF.get();
			if (!readFileF.eof())
			{
				i++;
			}
			pFS[i] = 0;
		}
		readFileF.close();
	} else 
	{
		cout << "Unable to open frag shader file: " << fName.c_str() << endl;
	}
}

// Shader Functions- click on + to expand
//#pragma region SHADER_FUNCTIONS
// create a shader object and compile the shader
void Shader::AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
    GLuint ShaderObj = f->glCreateShader(ShaderType);

    if (ShaderObj == 0) 
	{
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(0);
    }
	// Bind the source code to the shader, this happens before compilation
    f->glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderText, NULL);
	// compile the shader and check for errors
    f->glCompileShader(ShaderObj);
    GLint success;
	// check for shader related errors using glGetShaderiv
    f->glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar InfoLog[1024];
        f->glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
		system("pause");
        exit(1);
    }
	// Attach the compiled shader object to the program object
    f->glAttachShader(ShaderProgram, ShaderObj);
}

GLuint Shader::CompileShaders()
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
    ID = f->glCreateProgram();
    if (ID == 0) 
	{
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }

	// Create two shader objects, one for the vertex, and one for the fragment shader
    AddShader(ID, pVS, GL_VERTEX_SHADER);
    AddShader(ID, pFS, GL_FRAGMENT_SHADER);
	

    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };


	// After compiling all shader objects and attaching them to the program, we can finally link it
    f->glLinkProgram(ID);
	// check for program related errors using glGetProgramiv
    f->glGetProgramiv(ID, GL_LINK_STATUS, &Success);
	if (Success == 0) 
	{
        f->glGetProgramInfoLog(ID, sizeof(ErrorLog), NULL, ErrorLog);
		fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
    f->glValidateProgram(ID);

	// check for program related errors using glGetProgramiv
    f->glGetProgramiv(ID, GL_VALIDATE_STATUS, &Success);
    if (!Success) 
	{
        f->glGetProgramInfoLog(ID, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        //exit(1);
    }

	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	return ID;
}
// #pragma endregion SHADER_FUNCTIONS

	
