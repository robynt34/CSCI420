/*
CSCI 420 Computer Graphics, USC
Robyn To 1310457641
Assignment 1: Height Fields
C++ starter code

Student username: rto
*/

#include <iostream>																																																																																	// Robyn To 1310467641
#include <cstring>
#include "openGLHeader.h"
#include "glutHeader.h"

#include "imageIO.h"
#include "openGLMatrix.h"
#include "basicPipelineProgram.h"

#ifdef WIN32
#ifdef _DEBUG
#pragma comment(lib, "glew32d.lib")
#else
#pragma comment(lib, "glew32.lib")
#endif
#endif

#ifdef WIN32
char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
char shaderBasePath[1024] = "../openGLHelper-starterCode";
#endif

using namespace std;
static GLfloat spin = 0.0;
static GLfloat zStudent = 3 + (1310457641 / 10000000000);
OpenGLMatrix *matrix;
float positions[3][3] = { { -1.0, -1.0, -1.0 }, { 1.0, -1.0, -1.0 }, { 1.0, 1.0, -1.0 } };
float colors[3][3] = { { 0.0, 0.0, 1.0 }, { 1.0, 0.0, -1.0 }, { 0.0, 1.0, 1.0 } };
GLuint buffer;
GLfloat theta[3] = { 0.0, 0.0, 0.0 };BasicPipelineProgram *pipelineProgram;
void spinDisplay(void)
{
	spin = spin + 2.0;
	if (spin > 360.0)
	{
		spin = spin - 360.0;
	}
	glutPostRedisplay();
}

void reshape(int w, int h)
{
	GLfloat aspect = (GLfloat)w / (GLfloat)h;
	glViewport(0, 0, w, h);
	matrix->SetMatrixMode(OpenGLMatrix::Projection);
	matrix->LoadIdentity();
	matrix->Ortho(-2.0, 2.0, -2.0 / aspect, 2.0 / aspect, 0.0, 10.0);
	matrix->SetMatrixMode(OpenGLMatrix::ModelView);
}

void bindProgram()
{
	// bind our buffer, so that glVertexAttribPointer refers
	// to the correct buffer
	pipelineProgram->Init("");
	GLuint program = pipelineProgram->GetProgramHandle();
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	GLuint loc = glGetAttribLocation(program, "position");
	glEnableVertexAttribArray(loc);
	const void * offset = (const void*)0;
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, offset);
	GLuint loc2 = glGetAttribLocation(program, "color");
	glEnableVertexAttribArray(loc2);
	const void * offset2 = (const void*) sizeof(positions);
	glVertexAttribPointer(loc2, 4, GL_FLOAT, GL_FALSE, 0, offset2);
	// write projection and modelview matrix to shader
	// next lexture…
}

void renderQuad()
{
	GLint first = 0; 
	GLsizei numberOfVertices = 3;
	glDrawArrays(GL_TRIANGLES, first, numberOfVertices);
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT |
		GL_DEPTH_BUFFER_BIT);
	matrix->LoadIdentity();
	matrix->LookAt(0, 0, 0, 0, 0, -1, 0, 1, 0); // default camera
	matrix->Rotate(theta[0], 1.0, 0.0, 0.0);
	matrix->Rotate(theta[1], 0.0, 1.0, 0.0);
	matrix->Rotate(theta[2], 0.0, 0.0, 1.0);
	bindProgram();
	renderQuad();
	glutSwapBuffers();
}

void mouse(int button, int state, int x, int y)
{
	switch (button) {
		case GLUT_LEFT_BUTTON:
			if (state == GLUT_DOWN)
				glutIdleFunc(spinDisplay);
			break;
		case GLUT_RIGHT_BUTTON:
			if (state == GLUT_DOWN)
				glutIdleFunc(NULL);
			break;
		default:
			break;
	}
}

void initPipelineProgram()
{
	// initialize shader pipeline program (shader lecture)
	pipelineProgram->Init("shaders/basic.vertexShader.glsl");
}

void initVBO()
{
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(colors),
		NULL, GL_STATIC_DRAW); // init buffer’s size, but don’t assign any
	// data to it
	// upload position data
	glBufferSubData(GL_ARRAY_BUFFER, 0,
		sizeof(positions), positions);
	// upload color data
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions),
		sizeof(colors), colors);
}

void init()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_DEPTH_TEST);
	matrix = new OpenGLMatrix();
	initVBO();
	initPipelineProgram();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	// double buffering for smooth animation
	glutInitDisplayMode(GLUT_DOUBLE |
		GLUT_DEPTH |
		GLUT_RGBA);
	 // window creation and callbacks here (next slide)
	glutInitWindowSize(800, 800);
	glutCreateWindow("quad");
	glewInit();
		glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutIdleFunc(spinDisplay);
	glutMouseFunc(mouse);
		init(); // our custom initialization
	glutMainLoop();
	return(0);
}


