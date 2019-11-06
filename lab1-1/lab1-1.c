// Lab 1-1, multi-pass rendering with FBOs and HDR.
// Revision for 2013, simpler light, start with rendering on quad in final stage.
// Switched to low-res Stanford Bunny for more details.
// No HDR is implemented to begin with. That is your task.

// You can compile like this:
// gcc lab1-1.c ../common/*.c -lGL -o lab1-1 -I../common
// Not working! -DGL_GLEXT_PROTOTYPES and more are missing. Use the makefile.

// 2018: No zpr, trackball code added in the main program.

#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#ifdef __APPLE__
// Mac
	#define GL_SILENCE_DEPRECATION
	#include <OpenGL/gl3.h>
	#include "MicroGlut.h"
	// uses framework Cocoa
#else
	#ifdef WIN32
// MS
		#include <windows.h>
		#include <stdio.h>
		#include <GL/glew.h>
		#include <GL/glut.h>
	#else
// Linux
		#include <stdio.h>
		#include <GL/gl.h>
		#include "MicroGlut.h"
//		#include <GL/glut.h>
	#endif
#endif

#include "VectorUtils3.h"
#include "GL_utilities.h"
#include "loadobj.h"

// initial width and heights
#define W 512
#define H 512

#define NUM_LIGHTS 4

void OnTimer(int value);

mat4 projectionMatrix;
mat4 viewMatrix, modelToWorldMatrix;


GLfloat square[] = {
							-1,-1,0,
							-1,1, 0,
							1,1, 0,
							1,-1, 0};
GLfloat squareTexCoord[] = {
							 0, 0,
							 0, 1,
							 1, 1,
							 1, 0};
GLuint squareIndices[] = {0, 1, 2, 0, 2, 3};

Model* squareModel;

// -------------- IMPORTED STRUCS / FUNCTIONS -----------------
// Particle struct
struct particle {
	vec3 pos;
	vec3 vel;
};

typedef struct
{
  Model* model;
  GLuint textureId;
} ModelTexturePair;

//----------------------Globals-------------------------------------------------
Model *model1, *sphere;
FBOstruct *fbo1, *fbo2, *fbo3, *posfbo;
GLuint phongshader = 0,
			 plaintextureshader = 0,
			 trunktextureshader = 0,
			 filter_xtextureshader = 0,
			 filter_ytextureshader = 0,
			 combiningshader = 0;

//-------------------------------------------------------------------------------------

void init(void)
{
	dumpInfo();  // shader info

	// GL inits
	glClearColor(0.1, 0.1, 0.3, 0);
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	printError("GL inits");

	// Load and compile shaders
	plaintextureshader = loadShaders("plaintextureshader.vert", "plaintextureshader.frag");  // puts texture on teapot
	phongshader = loadShaders("phong.vert", "phong.frag");  // renders with light (used for initial renderin of teapot)
	trunktextureshader = loadShaders("plaintextureshader.vert", "trunkering.frag");
	filter_xtextureshader = loadShaders("plaintextureshader.vert", "filtering_x.frag");
	filter_ytextureshader = loadShaders("plaintextureshader.vert", "filtering_y.frag");
	combiningshader = loadShaders("plaintextureshader.vert", "combining.frag");

	printError("init shader");

	fbo1 = initFBO(W, H, 0); // picture
	fbo2 = initFBO(W, H, 0); // trunk img
	fbo3 = initFBO(W, H, 0); // filtrering img
	posfbo = initFBO(2, 1, 0); //positions


	// load the model
//	model1 = LoadModelPlus("teapot.obj");
	model1 = LoadModelPlus("stanford-bunny.obj");
	sphere = LoadModelPlus("sphere.obj");

	squareModel = LoadDataToModel(
			square, NULL, squareTexCoord, NULL,
			squareIndices, 4, 6);

	vec3 cam = SetVector(0, 5, 15);
	vec3 point = SetVector(0, 1, 0);
	vec3 up = {0, 1, 0};
	viewMatrix = lookAtv(cam, point, up);
	modelToWorldMatrix = IdentityMatrix();

	glutTimerFunc(5, &OnTimer, 0);

//	zprInit(&viewMatrix, cam, point);
}

void OnTimer(int value)
{
	glutPostRedisplay();
	glutTimerFunc(5, &OnTimer, value);
}

void runfilter(GLuint shader, FBOstruct *in1, FBOstruct *in2, FBOstruct *out) {

  glUseProgram(shader);

  // Many of these things would be more efficiently done once and for all
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glUniform1i(glGetUniformLocation(shader, "texUnit"), 0);
  glUniform1i(glGetUniformLocation(shader, "texUnit2"), 1);


  useFBO(out, in1, in2);
  DrawModel(squareModel, shader, "in_Position", NULL, "in_TexCoord");
}

//-------------------------------callback functions------------------------------------------
void display(void)
{
	mat4 vm2;

	// This function is called whenever it is time to render
	//  a new frame; due to the idle()-function below, this
	//  function will get called several times per second

	// render to fbo1!
	useFBO(0L, posfbo, 0L);

	// Clear framebuffer & zbuffer
	glClearColor(0.1, 0.1, 0.3, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Activate shader program
	glUseProgram(phongshader);

	vm2 = Mult(viewMatrix, modelToWorldMatrix);
	// Scale and place bunny since it is too small
	vm2 = Mult(vm2, T(0, -8.5, 0));
	vm2 = Mult(vm2, S(80,80,80));

	glUniformMatrix4fv(glGetUniformLocation(phongshader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniformMatrix4fv(glGetUniformLocation(phongshader, "modelviewMatrix"), 1, GL_TRUE, vm2.m);
//	glUniform3fv(glGetUniformLocation(phongshader, "camPos"), 1, &cam.x);
	glUniform1i(glGetUniformLocation(phongshader, "texUnit"), 0);

	// Enable Z-buffering
	glEnable(GL_DEPTH_TEST);
	// Enable backface culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);


	DrawModelInstanced(sphere, phongshader, "in_Position", "in_Normal", NULL, 2);

	// Done rendering the FBO! Set up for rendering on screen, using the result as texture!

//	glFlush(); // Can cause flickering on some systems. Can also be necessary to make drawing complete.
	// runfilter(trunktextureshader, fbo1, 0L, fbo2);
	//
	// int count = 0;
	//
	// while (count < 10) {
	// 	// filter first x, then y
	// 	runfilter(filter_xtextureshader, fbo2, 0L, fbo3);
	// 	runfilter(filter_ytextureshader, fbo3, 0L, fbo2);
	// 	++count;
	// }
	//
	// runfilter(combiningshader, fbo2, fbo1, 0L);

	glutSwapBuffers();
}

void reshape(GLsizei w, GLsizei h)
{
	glViewport(0, 0, w, h);
	GLfloat ratio = (GLfloat) w / (GLfloat) h;
	projectionMatrix = perspective(90, ratio, 1.0, 1000);
}


// This function is called whenever the computer is idle
// As soon as the machine is idle, ask GLUT to trigger rendering of a new
// frame
void idle()
{
  glutPostRedisplay();
}

// Trackball

int prevx = 0, prevy = 0;

void mouseUpDown(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		prevx = x;
		prevy = y;
	}
}

void mouseDragged(int x, int y)
{
	vec3 p;
	mat4 m;

	// This is a simple and IMHO really nice trackball system:

	// Use the movement direction to create an orthogonal rotation axis

	p.y = x - prevx;
	p.x = -(prevy - y);
	p.z = 0;

	// Create a rotation around this axis and premultiply it on the model-to-world matrix
	// Limited to fixed camera! Will be wrong if the camera is moved!

	m = ArbRotate(p, sqrt(p.x*p.x + p.y*p.y) / 50.0); // Rotation in view coordinates
	modelToWorldMatrix = Mult(m, modelToWorldMatrix);

	prevx = x;
	prevy = y;

	glutPostRedisplay();
}


//-----------------------------main-----------------------------------------------
int main(int argc, char *argv[])
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(W, H);

	glutInitContextVersion(3, 2);
	glutCreateWindow ("Render to texture with FBO");
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouseUpDown);
	glutMotionFunc(mouseDragged);
	glutIdleFunc(idle);

	init();
	glutMainLoop();
	exit(0);
}
