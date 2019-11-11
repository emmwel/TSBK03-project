
// Line to compile on mac
// gcc project-101.c ../common/*.c ../common/Mac/MicroGlut.m -o project-101 -framework OpenGL -framework Cocoa -I../common/Mac -I../common -Wno-deprecated-declarations

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sys/time.h>

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

// particle amounts, pixel size
int numParticles = 3;
float pixelSize;
int whichTexture = 1;

// Globals
FBOstruct *positionTex1, *positionTex2, *velocityTex1, *velocityTex2;
GLuint minShader = 0,
       updatePosShader = 0,
	   updateVelShader = 0,
	   renderShader = 0;


// Billboard to draw texture on
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

Model *squareModel, *sphere;

// matrices for rendering
mat4 projectionMatrix;
mat4 viewMatrix, modelToWorldMatrix;

// Time to integrate in shader
GLfloat deltaT, currentTime;

void onTimer(int value);

static double startTime = 0;

void resetElapsedTime()
{
  struct timeval timeVal;
  gettimeofday(&timeVal, 0);
  startTime = (double) timeVal.tv_sec + (double) timeVal.tv_usec * 0.000001;
}

float getElapsedTime()
{
  struct timeval timeVal;
  gettimeofday(&timeVal, 0);
  double currentTime = (double) timeVal.tv_sec
    + (double) timeVal.tv_usec * 0.000001;

  return currentTime - startTime;
}

void onTimer(int value)
{
    glutPostRedisplay();
    deltaT = getElapsedTime() - currentTime;
    currentTime = getElapsedTime();
    glutTimerFunc(20, &onTimer, value);
}

void init(void)
{
    // Reference to shader program:
    GLuint program;

	dumpInfo();

    // GL inits
    glClearColor(1.0,1.0,1.0,0); // sets to white color
    glEnable(GL_DEPTH_TEST);

	// initialize pixelSize
	pixelSize = 1.0f / numParticles;

    // Load and compile shaders
    minShader = loadShaders("minimal.vert", "minimal.frag");
    updatePosShader = loadShaders("minimal.vert", "updatePos.frag");
	updateVelShader = loadShaders("minimal.vert", "updateVel.frag");
	renderShader = loadShaders("render.vert", "render.frag");
    printError("init shader");

	// initialize texture FBOs for simulation
    positionTex1 = initNoiseFBO(numParticles, 1, 0); // start positions
    positionTex2 = initZeroFBO(numParticles, 1, 0);
    velocityTex1 = initZeroFBO(numParticles, 1, 0);
    velocityTex2 = initZeroFBO(numParticles, 1, 0);

	// load sphere
	sphere = LoadModelPlus("sphere.obj");

    squareModel = LoadDataToModel(
			square, NULL, squareTexCoord, NULL,
			squareIndices, 4, 6);

	// initialize matrices
	vec3 cam = SetVector(0, 5, 15);
	vec3 point = SetVector(0, 1, 0);
	vec3 up = {0, 1, 0};
	viewMatrix = lookAtv(cam, point, up);
	modelToWorldMatrix = IdentityMatrix();

	resetElapsedTime();

}

void runShader(GLuint shader, FBOstruct *in1, FBOstruct *in2, FBOstruct *out) {

  glUseProgram(shader);

  // Many of these things would be more efficiently done once and for all
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  glUniform1i(glGetUniformLocation(shader, "texUnitPosition"), 0);
  glUniform1i(glGetUniformLocation(shader, "texUnitVelocity"), 1);
  glUniform1f(glGetUniformLocation(shader, "deltaTime"), deltaT);
  glUniform1f(glGetUniformLocation(shader, "pixelSize"), pixelSize);


  useFBO(out, in1, in2);
  DrawModel(squareModel, shader, "in_Position", NULL, "in_TexCoord");
}

void display(void)
{
    useFBO(0L, positionTex1, 0L);

    // clear the screen
    glClearColor(1.0,1.0,1.0,0); // sets to white color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// fix matrices
	mat4 vm2;
	vm2 = Mult(viewMatrix, modelToWorldMatrix);
	// Scale and place bunny since it is too small
	vm2 = Mult(vm2, T(0, -8.5, 0));
	vm2 = Mult(vm2, S(80,80,80));

	// update particles
	if (whichTexture == 1) {
		//fprintf(stderr, "%f \n",deltaT);
		runShader(updatePosShader, positionTex1, velocityTex1, positionTex2);
		runShader(updateVelShader, positionTex2, velocityTex1, velocityTex2);

		// render to screen
	    glUseProgram(renderShader);
		glUniformMatrix4fv(glGetUniformLocation(renderShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
		glUniformMatrix4fv(glGetUniformLocation(renderShader, "modelviewMatrix"), 1, GL_TRUE, vm2.m);
	    glUniform1i(glGetUniformLocation(renderShader, "texUnit"), 0);

		useFBO(0L, positionTex2, 0L);
		DrawModelInstanced(sphere, renderShader, "in_Position", "in_Normal", "in_TexCoord", numParticles);

		whichTexture = 2;
	}
	else {
		//fprintf(stderr, "%i \n",2);
		runShader(updatePosShader, positionTex2, velocityTex2, positionTex1);
		runShader(updateVelShader, positionTex1, velocityTex2, velocityTex1);

		// render to screen
	    glUseProgram(renderShader);
		glUniformMatrix4fv(glGetUniformLocation(renderShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
		glUniformMatrix4fv(glGetUniformLocation(renderShader, "modelviewMatrix"), 1, GL_TRUE, vm2.m);
	    glUniform1i(glGetUniformLocation(renderShader, "texUnit"), 0);

		useFBO(0L, positionTex1, 0L);
	    DrawModelInstanced(sphere, renderShader, "in_Position", "in_Normal", "in_TexCoord", numParticles);

		whichTexture = 1;
	}


    glFlush();
    glutSwapBuffers();
}

int main(int argc, char *argv[])
{
    // initialize GLUT
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(W, H);
    glutInitContextVersion(3, 2); // modern OpenGL
    glutCreateWindow ("Hail simulation");

    // display window and update every 20 ms
    glutDisplayFunc(display);
    glutTimerFunc(20, &onTimer, 0);

    // initalize
    init ();
    glutMainLoop();
    exit(0);
}
