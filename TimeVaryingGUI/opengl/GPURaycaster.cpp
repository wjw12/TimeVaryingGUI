#include "GPURaycaster.h"

GPURaycaster::GPURaycaster(int screenWidth, int screenHeight, VolumeDataset &volume)
{
	// Variables which will be used in shader
	maxRaySteps = 1000;
	rayStepSize = 0.005f;
	gradientStepSize = 0.005f;
	lightPosition = glm::vec3(-0.0f, -5.0f, 5.0f);
    volumeDataset = volume;
    f = QOpenGLContext::currentContext()->functions();
}




// Messy but just inputs all uniforms to shader and raycasts using the passed in 3D texture and transfer function
void GPURaycaster::Raycast(GLuint currTexture3D, GLuint lastTexture3D, GLuint TF_textureUnit, GLuint shaderProgramID, Camera &camera)
{
	int uniformLoc;

	glm::mat4 model_mat = glm::mat4(1.0f);

    uniformLoc = f->glGetUniformLocation (shaderProgramID, "proj");
    f->glUniformMatrix4fv (uniformLoc, 1, GL_FALSE, &camera.projMat[0][0]);

    uniformLoc = f->glGetUniformLocation (shaderProgramID, "view");
    f->glUniformMatrix4fv (uniformLoc, 1, GL_FALSE, &camera.viewMat[0][0]);

    uniformLoc = f->glGetUniformLocation (shaderProgramID, "model");
    f->glUniformMatrix4fv (uniformLoc, 1, GL_FALSE, &model_mat[0][0]);

    f->glActiveTexture (GL_TEXTURE0);
    uniformLoc = f->glGetUniformLocation(shaderProgramID,"volume");
    f->glUniform1i(uniformLoc,0);
    f->glBindTexture (GL_TEXTURE_3D, currTexture3D);

    f->glActiveTexture (GL_TEXTURE1);
    uniformLoc = f->glGetUniformLocation(shaderProgramID,"tempvolume");
    f->glUniform1i(uniformLoc,1);
    f->glBindTexture (GL_TEXTURE_3D, lastTexture3D);

    uniformLoc = f->glGetUniformLocation(shaderProgramID,"camPos");
    f->glUniform3f(uniformLoc, camera.position.x, camera.position.y, camera.position.z);

    uniformLoc = f->glGetUniformLocation(shaderProgramID,"maxRaySteps");
    f->glUniform1i(uniformLoc, maxRaySteps);

    uniformLoc = f->glGetUniformLocation(shaderProgramID,"rayStepSize");
    f->glUniform1f(uniformLoc, rayStepSize);

    uniformLoc = f->glGetUniformLocation(shaderProgramID,"gradientStepSize");
    f->glUniform1f(uniformLoc, gradientStepSize);


    uniformLoc = f->glGetUniformLocation(shaderProgramID,"lightPosition");
    f->glUniform3f(uniformLoc, lightPosition.x, lightPosition.y, lightPosition.z);

    f->glActiveTexture (GL_TEXTURE2);
    uniformLoc = f->glGetUniformLocation(shaderProgramID,"tfTexture2d");
    f->glUniform1i(uniformLoc, 2);
    f->glBindTexture(GL_TEXTURE_2D, TF_textureUnit);




	// Final render is the front faces of a cube which can then be raycasted into
    glBegin(GL_QUADS);

    float x, y;
    int a[3], temp;
    a[0] = volumeDataset.xRes;
    a[1] = volumeDataset.yRes;
    a[2] = volumeDataset.zRes;

    for (int i=0; i<3; i++) {
        for (int j=i+1; j<3; j++)
            if (a[j] < a[i]) {temp = a[j]; a[j] = a[i]; a[i] = temp;}
    }

    // a[2] is max resolution
    x = (float) a[0] / a[2];
    y = (float) a[1] / a[2];

    /*
	// Front Face
    glVertex3f(-1.0f, -1.0f,  1.0f);
    glVertex3f( 1.0f, -1.0f,  1.0f);
    glVertex3f( 1.0f,  1.0f,  1.0f);
    glVertex3f(-1.0f,  1.0f,  1.0f);
 
	// Back Face
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f,  1.0f, -1.0f);
    glVertex3f( 1.0f,  1.0f, -1.0f);
    glVertex3f( 1.0f, -1.0f, -1.0f);
 
	// Top Face
    glVertex3f(-1.0f,  1.0f, -1.0f);
    glVertex3f(-1.0f,  1.0f,  1.0f);
    glVertex3f( 1.0f,  1.0f,  1.0f);
    glVertex3f( 1.0f,  1.0f, -1.0f);
	
	// Bottom Face
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f( 1.0f, -1.0f, -1.0f);
    glVertex3f( 1.0f, -1.0f,  1.0f);
    glVertex3f(-1.0f, -1.0f,  1.0f);
 
	// Right face
    glVertex3f( 1.0f, -1.0f, -1.0f);
    glVertex3f( 1.0f,  1.0f, -1.0f);
    glVertex3f( 1.0f,  1.0f,  1.0f);
    glVertex3f( 1.0f, -1.0f,  1.0f);
 
	// Left Face
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, -1.0f,  1.0f);
    glVertex3f(-1.0f,  1.0f,  1.0f);
    glVertex3f(-1.0f,  1.0f, -1.0f);
    */

    // Front Face
    glVertex3f(-x, -y,  1.0f);
    glVertex3f( x, -y,  1.0f);
    glVertex3f( x,  y,  1.0f);
    glVertex3f(-x,  y,  1.0f);

    // Back Face
    glVertex3f(-x, -y, -1.0f);
    glVertex3f(-x,  y, -1.0f);
    glVertex3f( x,  y, -1.0f);
    glVertex3f( x, -y, -1.0f);

    // Top Face
    glVertex3f(-x,  y, -1.0f);
    glVertex3f(-x,  y,  1.0f);
    glVertex3f( x,  y,  1.0f);
    glVertex3f( x,  y, -1.0f);

    // Bottom Face
    glVertex3f(-x, -y, -1.0f);
    glVertex3f( x, -y, -1.0f);
    glVertex3f( x, -y,  1.0f);
    glVertex3f(-x, -y,  1.0f);

    // Right face
    glVertex3f( x, -y, -1.0f);
    glVertex3f( x,  y, -1.0f);
    glVertex3f( x,  y,  1.0f);
    glVertex3f( x, -y,  1.0f);

    // Left Face
    glVertex3f(-x, -y, -1.0f);
    glVertex3f(-x, -y,  1.0f);
    glVertex3f(-x,  y,  1.0f);
    glVertex3f(-x,  y, -1.0f);

    glEnd();

    f->glBindTexture(GL_TEXTURE_3D, 0);
}
