
#include <GLTools.h>            // OpenGL toolkit
#include <GLFrame.h>
#include <glFrustum.h>

#define FREEGLUT_STATIC
#include "glut.h"            // Windows FreeGlut equivalent



///////////////////////////////////////////////////////////////////////////////
GLuint shader;
int MVPMatrixLocation;
M3DMatrix44f MVPMatrix;

float rotation = 0;
GLFrame glFrame; // widok
GLFrustum frustum;
///////////////////////////////////////////////////////////////////////////////



void SetUpFrame(GLFrame &frame, const M3DVector3f origin, const M3DVector3f forward, const M3DVector3f up)
{
    frame.SetOrigin(origin);
    frame.SetForwardVector(forward);
    M3DVector3f side, oUp;
    m3dCrossProduct3(side, forward, up);
    m3dCrossProduct3(oUp, side, forward);
    frame.SetUpVector(oUp);
    frame.Normalize();
}
;

void LookAt(GLFrame &frame, const M3DVector3f eye, const M3DVector3f at, const M3DVector3f up)
{
    M3DVector3f forward;
    m3dSubtractVectors3(forward, at, eye);
    SetUpFrame(frame, eye, forward, up);
}


///////////////////////////////////////////////////////////////////////////////
// Window has changed size, or has just been created. In either case, we need
// to use the window dimensions to set the viewport and the projection matrix.
void ChangeSize(int w, int h) 
{
	if (h == 0)
		h = 1;
    glViewport(0, 0, w, h);

	frustum.SetPerspective(45, w/h, 0.001, 1000);
}


///////////////////////////////////////////////////////////////////////////////
// This function does any needed initialization on the rendering context.
// This is the first opportunity to do any OpenGL related tasks.
void SetupRC() 
{
    glClearColor(0.25f, 0.25f, 0.25f, 1.0f);

    shader = gltLoadShaderPairWithAttributes("shader.vp", "shader.fp", 2, GLT_ATTRIBUTE_VERTEX, "vVertex", GLT_ATTRIBUTE_COLOR, "vColor");

    fprintf(stdout, "GLT_ATTRIBUTE_VERTEX : %d\nGLT_ATTRIBUTE_COLOR : %d \n", GLT_ATTRIBUTE_VERTEX, GLT_ATTRIBUTE_COLOR);

	MVPMatrixLocation = glGetUniformLocation(shader,"MVPMatrix");

	if(MVPMatrixLocation == -1)
		fprintf(stderr, "uniform MVPMatrix could not be found\n");

	glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE,MVPMatrix);
	glEnable(GL_DEPTH_TEST);
}



///////////////////////////////////////////////////////////////////////////////
// Called to draw scene
void RenderScene(void) 
{
// definiujemy zmienne ...
	rotation += 0.001;
	M3DVector3f vector1, vector2, vector3;
	M3DMatrix44f camera, rot, rot2, scale, translate;

// ... i macierze przekszta³ceñ
	M3DMatrix44f macierz0, macierz1, macierz2;

// ustawiamy scenê, czyli ... 
	m3dLoadVector3(vector1, 2, 0, -1);  // ... sk¹d patrzymy
	m3dLoadVector3(vector2, 0, 0, 0);   // ... punkt, na który patrzymy
	m3dLoadVector3(vector3, 0, 1, 0);   // ... wektor k¹ta obrotu kamery, wskazuj¹cy górê ekranu

	LookAt(glFrame, vector1, vector2, vector3); // tworzona macierz widoku, czyli sk³adamy elementy macierzy do kupy


	m3dScaleMatrix44(scale, 0.6, 0.6, 0.6); // tworzymy macierz skali

	
	m3dRotationMatrix44(rot, rotation*1/3.1415, 0, 1, 0); // tworzy macierz przekszta³ceñ - obrotu wokó³ osi y
	m3dRotationMatrix44(rot2, 43*3.1415/180, 1, 0, 0); // to samo tylko dla osi x, pod k¹tem 30 stopni

	glFrame.GetCameraMatrix(camera); //  macierz jest przypis. do zmiennej


// sk³adnie macierzy przekszta³ceñ do jednej (MVPMatrix)
	m3dMatrixMultiply44(macierz0, rot2, rot); 
	m3dMatrixMultiply44(macierz1, scale, macierz0);
	m3dMatrixMultiply44(macierz2, camera, macierz1);
	m3dMatrixMultiply44(MVPMatrix, frustum.GetProjectionMatrix(), macierz2);

	glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, MVPMatrix); // przekazuje macie¿ przeksz³ta³c. do shadera


 // Clear the window with current clearing color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

 // abrakadabra, shader dzia³a!
    glUseProgram(shader);



// rysujemy trójk¹ciki :)

glBegin(GL_TRIANGLES);
    glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.0, 1.0, 0.0);
		glVertex3f(0, 1, 0);
		glVertex3f(-1, 0, 1);
		glVertex3f(1, 0, 1);

    glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0, 1.0, 0.0);
		glVertex3f(0, 1, 0);
		glVertex3f(1, 0, -1);
		glVertex3f(-1, 0, -1);

    glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.0, 0.0, 1.0);
		glVertex3f(0, 1, 0);
		glVertex3f(-1, 0, 1);
		glVertex3f(-1, 0, -1);
		
    glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0, 0.0, 0.0);
		glVertex3f(0, 1, 0);
		glVertex3f(1, 0, -1);
		glVertex3f(1, 0, 1);
glEnd();

// ... i kwadracik ;)
glBegin(GL_QUADS);
	glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.0, 1.0, 1.0);
		glVertex3f(-1, 0, -1);
		glVertex3f(1, 0, -1);
		glVertex3f(1, 0, 1);
		glVertex3f(-1, 0, 1);
glEnd();
	

// Perform the buffer swap to display back buffer
    glutSwapBuffers();
}


///////////////////////////////////////////////////////////////////////////////
// Main entry point for GLUT based programs
int main(int argc, char* argv[]) 
{	
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Piramidka");
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
    
// this function will perform animation
    glutIdleFunc(RenderScene);

    GLenum err = glewInit();
    if (GLEW_OK != err) 
	{
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return 1;
    }

    SetupRC();
	
    glutMainLoop();

    return 0;
}
