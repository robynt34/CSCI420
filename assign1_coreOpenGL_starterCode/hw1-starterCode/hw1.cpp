/*
CSCI 420 Computer Graphics, USC
Assignment 1: Height Fields
C++ starter code

Student username: rto
Robyn To

*/

#include <iostream>
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

int mousePos[2]; // x,y coordinate of the mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

// state of the world
float landRotate[3] = { 0.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };

int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework I";
int jpgCounter = 0;

ImageIO * heightmapImage;

// My variables
OpenGLMatrix *matrix;
BasicPipelineProgram *pipelineProgram;
GLuint program;
GLuint drawMode = GL_POINTS;
GLuint glCount;

// Point vars
float *pointArray;
GLuint pointArraySize;
GLuint pointVBO;
GLuint pointVAO;

// Wireframe vars
float *wireframeArray;
GLuint wireframeArraySize;
GLuint wireframeVBO;
GLuint wireframeVAO;

// Triangel vars
float *triangleArray;
GLuint triangleArraySize;
GLuint triangleVBO;
GLuint triangleVAO;

// Extra photo
ImageIO * funnyPhoto;

// write a screenshot to the specified filename
void saveScreenshot(const char * filename)
{
	unsigned char * screenshotData = new unsigned char[windowWidth * windowHeight * 3];
	glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

	ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

	if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
		cout << "File " << filename << " saved successfully." << endl;
	else cout << "Failed to save file " << filename << '.' << endl;

	delete[] screenshotData;
}

// Populate all arrays (point/wireframe/triangle)
void popArrays()
{
	int width = heightmapImage->getWidth();
	int height = heightmapImage->getHeight();

	// Init points array
	pointArraySize = height * width * 3;
	glCount = pointArraySize; // count is for points by default
	pointArray = new float[pointArraySize];

	float maxHeight = 9.0;
	float centeringNum = height / 2;
	int index = 0;

	// Populate points array
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			float imageHeight = heightmapImage->getPixel(i, j, 0) / maxHeight;
			pointArray[index] = i - centeringNum;
			index++;
			pointArray[index] = imageHeight;
			index++;
			pointArray[index] = -j + centeringNum;
			index++;
		}
	}

	//Create array for Wireframe	
	float mapDivision = (height - 1)*(width - 1)*2*3; // total array size /3
	wireframeArraySize = mapDivision * 3;
	wireframeArray = new float[wireframeArraySize];

	int x = 0, y = 0;
	bool diagFlag= false, horizontalFlag = false;

	// Populate wireframe array
	for (int i = 0; i < mapDivision*3; i += 6)
	{
		if (i < mapDivision)
		{
			// Vertical lines
			// First point
			float heightOfPixel = heightmapImage->getPixel(x, y, 0) / maxHeight;
			wireframeArray[i] = x - centeringNum;
			wireframeArray[i + 1] = heightOfPixel;
			wireframeArray[i + 2] = -y + centeringNum;

			// Second point
			heightOfPixel = heightmapImage->getPixel(x, (y + 1), 0) / maxHeight;
			wireframeArray[i + 3] = x - centeringNum;
			wireframeArray[i + 4] = heightOfPixel;
			wireframeArray[i + 5] = -(y + 1) + centeringNum;
			x++;

			// Reset x, move up one y
			if (x >= width - 1)
			{
				x = 0;
				y++;
			}
		}
		else if (i >= mapDivision && i < (mapDivision*2))
		{
			// Horizontal lines
			if (!horizontalFlag)
			{
				x = 0, y = 0;
				horizontalFlag = true;
			}

			// First point
			float heightOfPixel = heightmapImage->getPixel(x, y, 0) / maxHeight;
			wireframeArray[i] = x - centeringNum;
			wireframeArray[i + 1] = heightOfPixel;
			wireframeArray[i + 2] = -y + centeringNum;
			x++;

			// Second point
			heightOfPixel = heightmapImage->getPixel(x, y, 0) / maxHeight;
			wireframeArray[i + 3] = x - centeringNum;
			wireframeArray[i + 4] = heightOfPixel;
			wireframeArray[i + 5] = -y + centeringNum;

			// Reset x, move up one y
			if (x >= width - 1){
				x = 0;
				y++;
			}
		}
		else if (i >= (mapDivision * 2) && i < (mapDivision * 3))
		{
			// Diag lines
			if (!diagFlag)
			{
				x = 0, y = 0;
				diagFlag = true;
			}
			// First point
			float heightOfPixel = heightmapImage->getPixel(x, y, 0) / maxHeight;
			wireframeArray[i] = x - centeringNum;
			wireframeArray[i + 1] = heightOfPixel;
			wireframeArray[i + 2] = -y + centeringNum;
			x++;

			// Second point
			heightOfPixel = heightmapImage->getPixel(x, (y + 1), 0) / maxHeight;
			wireframeArray[i + 3] = x - centeringNum;
			wireframeArray[i + 4] = heightOfPixel;
			wireframeArray[i + 5] = -(y + 1) + centeringNum;

			// Reset x, move up one y
			if (x >= width - 1){
				x = 0;
				y++;
			}
		}
	}

	// Create array for triangles	
	triangleArraySize = ((height-2) *width* 2) + (width);
	triangleArraySize *= 3;
	triangleArraySize += 6;
	triangleArray = new float[triangleArraySize];

	x = 0, y = 0;
	// Populate triangles array
	for (int k = 0; k < triangleArraySize; k += 6){
		if (y % 2 == 0){
			// Even row
			// First point
			float heightOfPixel = heightmapImage->getPixel(x, y, 0) / maxHeight;
			triangleArray[k] = x - centeringNum;
			triangleArray[k + 1] = heightOfPixel;		
			triangleArray[k + 2] = -y + centeringNum;

			// Second point
			heightOfPixel = heightmapImage->getPixel(x, y + 1, 0) / maxHeight;
			triangleArray[k + 3] = x - centeringNum;
			triangleArray[k + 4] = heightOfPixel;
			triangleArray[k + 5] = -(y + 1) + centeringNum;

			x++;
			if (x == width){
				x--;
				y++;
			}
		}
		else{
			// Odd row
			// First point
			float heightOfPixel = heightmapImage->getPixel(x, y + 1, 0) / maxHeight;
			triangleArray[k] = x - centeringNum;
			triangleArray[k + 1] = heightOfPixel;
			triangleArray[k + 2] = -(y + 1) + centeringNum;

			// Second point
			heightOfPixel = heightmapImage->getPixel(x - 1, y, 0) / maxHeight;
			triangleArray[k + 3] = (x - 1) - centeringNum;
			triangleArray[k + 4] = heightOfPixel;
			triangleArray[k + 5] = -y + centeringNum;

			x--;
			if (x == 0){
				y++;
			}
		}
	}
}

// Init VBOs and VAOs
void initVO()
{
	//Init Point VBO
	glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * pointArraySize, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * pointArraySize, pointArray);

	//Init Point VAO
	glBindVertexArray(pointVAO);
	glBindBuffer(GL_ARRAY_BUFFER, pointVAO);
	GLuint loc = glGetAttribLocation(program, "position");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

	// Init wireframe VBO
	glBindBuffer(GL_ARRAY_BUFFER, wireframeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * wireframeArraySize, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * wireframeArraySize, wireframeArray);

	// Init wireframe VAO
	glBindVertexArray(wireframeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, wireframeVBO);
	loc = glGetAttribLocation(program, "position");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);

	// Init triangle VBO
	glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * triangleArraySize, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * triangleArraySize, triangleArray);

	// Init triangle VAO
	glBindVertexArray(triangleVAO);
	glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
	loc = glGetAttribLocation(program, "position");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
}

void bindProgram()
{
	GLint h_modelViewMatrix = glGetUniformLocation(program, "modelViewMatrix");
	float m[16];
	matrix->SetMatrixMode(OpenGLMatrix::ModelView);
	matrix->GetMatrix(m);
	glUniformMatrix4fv(h_modelViewMatrix, 1, GL_FALSE, m);

	GLint h_projectionMatrix = glGetUniformLocation(program, "projectionMatrix");
	float p[16];
	matrix->SetMatrixMode(OpenGLMatrix::Projection);
	matrix->GetMatrix(p);
	glUniformMatrix4fv(h_projectionMatrix, 1, GL_FALSE, p);
}

void displayFunc()
{
	// render some stuff...
	// Set up camera
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	matrix->SetMatrixMode(OpenGLMatrix::ModelView);
	matrix->LoadIdentity();
	matrix->LookAt(0, 130, 0, 0, 0, 0, 0, 0, 1);

	matrix->Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
	matrix->Rotate(landRotate[0], 1.0f, 0.0f, 0.0f);
	matrix->Rotate(landRotate[1], 0.0f, 1.0f, 0.0f);
	matrix->Rotate(landRotate[2], 0.0f, 0.0f, 1.0f);
	matrix->Scale(landScale[0], landScale[1], landScale[2]);

	bindProgram();
	initVO();

	// Draw points
	if (drawMode == GL_POINTS)
	{
		glCount = pointArraySize;
		glBindVertexArray(pointVAO);
	}
	else if (drawMode == GL_LINES)
	{
		glCount = wireframeArraySize;
		glBindVertexArray(wireframeVAO);
	}
	else if (drawMode == GL_TRIANGLE_STRIP)
	{
		glCount = triangleArraySize;
		glBindVertexArray(triangleVAO);
	}
	glDrawArrays(drawMode, 0, glCount/3);

	glBindVertexArray(0);
	glutSwapBuffers();
}

void idleFunc()
{
	// do some stuff... 

	// for example, here, you can save the screenshots to disk (to make the animation)

	// make the screen update 
	glutPostRedisplay();
}

void reshapeFunc(int w, int h)
{
	glViewport(0, 0, w, h);

	// setup perspective matrix...
	GLfloat aspect = (GLfloat)w / (GLfloat)h;
	
	matrix->SetMatrixMode(OpenGLMatrix::Projection);
	matrix->LoadIdentity();
	matrix->Perspective(100.0, aspect, 0.01, 1000.0);
	//matrix->Ortho(-2.0, 2.0, -2.0 / aspect, 2.0 / aspect, 0.0, 10.0);

	matrix->SetMatrixMode(OpenGLMatrix::ModelView);
}

void mouseMotionDragFunc(int x, int y)
{
	// mouse has moved and one of the mouse buttons is pressed (dragging)

	// the change in mouse position since the last invocation of this function
	int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };

	switch (controlState)
	{
		// translate the landscape
	case TRANSLATE:
		if (leftMouseButton)
		{
			// control x,y translation via the left mouse button
			landTranslate[0] += mousePosDelta[0] * 0.01f;
			landTranslate[1] -= mousePosDelta[1] * 0.01f;
		}
		if (middleMouseButton)
		{
			// control z translation via the middle mouse button
			landTranslate[2] += mousePosDelta[1] * 0.01f;
		}
		break;

		// rotate the landscape
	case ROTATE:
		if (leftMouseButton)
		{
			// control x,y rotation via the left mouse button
			landRotate[0] += mousePosDelta[1];
			landRotate[1] += mousePosDelta[0];
		}
		if (middleMouseButton)
		{
			// control z rotation via the middle mouse button
			landRotate[2] += mousePosDelta[1];
		}
		break;

		// scale the landscape
	case SCALE:
		if (leftMouseButton)
		{
			// control x,y scaling via the left mouse button
			landScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
			landScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
		}
		if (middleMouseButton)
		{
			// control z scaling via the middle mouse button
			landScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
		}
		break;
	}

	// store the new mouse position
	mousePos[0] = x;
	mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
	// mouse has moved
	// store the new mouse position
	mousePos[0] = x;
	mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
	// a mouse button has has been pressed or depressed

	// keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		leftMouseButton = (state == GLUT_DOWN);
		break;

	case GLUT_MIDDLE_BUTTON:
		middleMouseButton = (state == GLUT_DOWN);
		break;

	case GLUT_RIGHT_BUTTON:
		rightMouseButton = (state == GLUT_DOWN);
		break;
	}

	// keep track of whether CTRL and SHIFT keys are pressed
	switch (glutGetModifiers())
	{
	case GLUT_ACTIVE_CTRL:
		controlState = TRANSLATE;
		break;

	case GLUT_ACTIVE_SHIFT:
		controlState = SCALE;
		break;

		// if CTRL and SHIFT are not pressed, we are in rotate mode
	default:
		controlState = ROTATE;
		break;
	}

	// store the new mouse position
	mousePos[0] = x;
	mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27: // ESC key
		exit(0); // exit the program
		break;

	// Move Camera
	//move right
	case 'a':
		landTranslate[0] -= 1.0f;
		break;
		//move left
	case 'd':
		landTranslate[0] += 1.0f;
		break;
		//zoom in
	case 'q':
		landTranslate[1] -= 1.0f;
		break;
		//zoom out
	case 'e':
		landTranslate[1] += 1.0f;
		break;
	//move down
	case 'w':
		landTranslate[2] -= 1.0f;
		break;
	//move up
	case 's':
		landTranslate[2] += 1.0f;
		break;

	case ' ':
		cout << "You pressed the spacebar." << endl;
		break;

	case 'x':
		// take a screenshot
		saveScreenshot("screenshot.jpg");
		break;
	case '1':
		drawMode = GL_POINTS;
		break;
	case '2':
		drawMode = GL_LINES;
		break;
	case '3':
		drawMode = GL_TRIANGLE_STRIP;
		break;
	case '4':
		landRotate[1] += 1;
		break;
	case '6':
		landRotate[1] -= 1;
		break;
	case '8':
		landRotate[0] += 1;
		break;
	case '5':
		landRotate[0] -= 1;
		break;
	case 'p':
		//funny photo
		heightmapImage = funnyPhoto;
		popArrays();
		break;
	}
}

void initPipelineProgram()
{
	pipelineProgram = new BasicPipelineProgram();
	pipelineProgram->Init("../openGLHelper-starterCode");
	pipelineProgram->Bind();
	program = pipelineProgram->GetProgramHandle();
}

void initScene(int argc, char *argv[])
{

	// load the image from a jpeg disk file to main memory
	heightmapImage = new ImageIO();
	if (heightmapImage->loadJPEG(argv[1]) != ImageIO::OK)
	{
		cout << "Error reading image " << argv[1] << "." << endl;
		exit(EXIT_FAILURE);
	}

	funnyPhoto = new ImageIO();
	if (funnyPhoto->loadJPEG("./heightmap/please.jpg") != ImageIO::OK)
	{
		cout << "could not open Robyn's extra image...delete lines in initscene" << endl;
		exit(EXIT_FAILURE);
	}

	// do additional initialization here...
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);

	matrix = new OpenGLMatrix();
	initPipelineProgram();
	popArrays();

	// Generate all buffers
	glGenBuffers(1, &pointVBO);
	glGenVertexArrays(1, &pointVAO);
	glGenBuffers(1, &wireframeVBO);
	glGenVertexArrays(1, &wireframeVAO);
	glGenBuffers(1, &triangleVBO);
	glGenVertexArrays(1, &triangleVAO);
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cout << "The arguments are incorrect." << endl;
		cout << "usage: ./hw1 <heightmap file>" << endl;
		exit(EXIT_FAILURE);
	}

	cout << "Initializing GLUT..." << endl;
	glutInit(&argc, argv);

	cout << "Initializing OpenGL..." << endl;

#ifdef __APPLE__
	glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
#else
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
#endif

	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(0, 0);
	glutCreateWindow(windowTitle);

	cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
	cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

	// tells glut to use a particular display function to redraw 
	glutDisplayFunc(displayFunc);
	// perform animation inside idleFunc
	glutIdleFunc(idleFunc);
	// callback for mouse drags
	glutMotionFunc(mouseMotionDragFunc);
	// callback for idle mouse movement
	glutPassiveMotionFunc(mouseMotionFunc);
	// callback for mouse button changes
	glutMouseFunc(mouseButtonFunc);
	// callback for resizing the window
	glutReshapeFunc(reshapeFunc);
	// callback for pressing the keys on the keyboard
	glutKeyboardFunc(keyboardFunc);

	// init glew
#ifdef __APPLE__
	// nothing is needed on Apple
#else
	// Windows, Linux
	GLint result = glewInit();
	if (result != GLEW_OK)
	{
		cout << "error: " << glewGetErrorString(result) << endl;
		exit(EXIT_FAILURE);
	}
#endif

	// do initialization
	initScene(argc, argv);

	// sink forever into the glut loop
	glutMainLoop();
}