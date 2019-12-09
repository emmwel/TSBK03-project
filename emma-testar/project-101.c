
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
#include "LoadTGA.h"

// initial width and heights
#define W 800
#define H 800

void onTimer(int value);

// particle amounts, pixel size
int numParticles = 10000;
float pixelSize;
bool firstTexture = true;

// Globals
FBOstruct *positionTex1, *positionTex2, *velocityTex1, *velocityTex2, *depthBuffer;
GLuint minShader = 0,
       updatePosShader = 0,
		   updateVelShader = 0,
		   renderShader = 0,
		   phongShader = 0,
		   texShader = 0;
vec3 forward = {0, 0, -1};
vec3 cam = {0, 15, 75};
vec3 point = {0, 1, 0};
vec3 up = {0, 1, 0};

// texture
GLuint hailtex;

// Billboard to draw texture on
GLfloat squareVertices[] = {
							-1.0,-1.0,0,
							-1.0,1.0, 0,
							1.0,1.0, 0,
							1.0,-1.0, 0};
GLfloat squareTexCoord[] = {
							 0, 0,
							 0, 1,
							 1, 1,
							 1, 0};
GLuint squareIndices[] = {0, 1, 2, 0, 2, 3};

// A quad
GLfloat quadVertices[] = {	-0.5,-0.5,0.0,
						0.5,-0.5,0.0,
						0.5,0.5,0.0,
						-0.5,0.5,0.0};
GLfloat quadTexcoords[] = {	0.0f, 1.0f,
						1.0f, 1.0f,
						1.0f, 0.0f,
						0.0f, 0.0f};
GLuint quadIndices[] = {	0,3,2, 0,2,1};

Model *squareModel, *hailModel, *planeModel, *sphere;

// matrices for rendering
mat4 projectionMatrix, viewMatrix, modelToWorldMatrix, worldToView, m, m_plane;

// Time to integrate in shader
GLfloat deltaT, currentTime;

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
	dumpInfo();

  	// GL inits
	glClearColor(0.1, 0.1, 0.3, 0);
	glClearDepth(1.0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	printError("GL inits");

	// initialize pixelSize
	pixelSize = 1.0f / numParticles;

  // Load and compile shaders
	minShader = loadShaders("minimal.vert", "minimal.frag");
	updatePosShader = loadShaders("minimal.vert", "updatePos.frag");
	updateVelShader = loadShaders("minimal.vert", "updateVel.frag");
	renderShader = loadShaders("render.vert", "render.frag");
	phongShader = loadShaders("phong.vert", "phong.frag");
	texShader = loadShaders("textured.vert", "textured.frag");
	printError("init shader");

	//textures
	LoadTGATextureSimple("hail-texture.tga", &hailtex);

	// initialize texture FBOs for simulation
  	positionTex1 = initPositionsFBO(numParticles, 1, 0); // start positions
  	positionTex2 = initZeroFBO(numParticles, 1, 0);
  	velocityTex1 = initVelocityFBO(numParticles, 1, 0);
  	velocityTex2 = initZeroFBO(numParticles, 1, 0);

	// Load models
	squareModel = LoadDataToModel(
		squareVertices, NULL, squareTexCoord, NULL,
		squareIndices, 4, 6);
	hailModel = LoadDataToModel(
		quadVertices, NULL, quadTexcoords, NULL,
		quadIndices, 4, 6);
	planeModel = LoadModelPlus("plane-and-objects.obj");

	// initialize matrices
	viewMatrix = lookAtv(cam, point, up);
	modelToWorldMatrix = IdentityMatrix();

	resetElapsedTime();
}

void runShader(GLuint shader, FBOstruct *in1, FBOstruct *in2, FBOstruct *out) {

  glUseProgram(shader);

  // Many of these things would be more efficiently done once and for all
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  useFBO(out, in1, in2);

  glUniform1i(glGetUniformLocation(shader, "texUnitPosition"), 0);
  glUniform1i(glGetUniformLocation(shader, "texUnitVelocity"), 1);
  glUniform1f(glGetUniformLocation(shader, "deltaTime"), deltaT);
  glUniform1f(glGetUniformLocation(shader, "pixelSize"), pixelSize);

  DrawModel(squareModel, shader, "in_Position", NULL, "in_TexCoord");
}

void display(void)
{
  // clear the screen
  glClearColor(0.1, 0.1, 0.3, 0); // sets background to blue color
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// fix matrices
	worldToView = lookAtv(cam, VectorAdd(cam, forward), up);
	m = Mult(worldToView, modelToWorldMatrix);
	m = Mult(worldToView, Mult(T(-1, 0.5, 0), IdentityMatrix()));
	m = T(m.m[3], m.m[7], m.m[11]);
	m_plane = Mult(m, S(10.0, 10.0, 10.0));

	// Update particles
	if (firstTexture == 1) {
		// --------- Run physics calculations ---------
		runShader(updatePosShader, positionTex1, velocityTex1, positionTex2);
		runShader(updateVelShader, positionTex2, velocityTex1, velocityTex2);

		// Use position texture
		useFBO(0L, positionTex2, 0L);
	}
	else {
		// --------- Run physics calculations ---------
		runShader(updatePosShader, positionTex2, velocityTex2, positionTex1);
		runShader(updateVelShader, positionTex1, velocityTex2, velocityTex1);

		// Use position texture
		useFBO(0L, positionTex1, 0L);
	}
	// Switch which position texture to render from
	firstTexture = !firstTexture;

	// Bind hailstone appearance texture
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, hailtex);

	// Activate shader
	glUseProgram(texShader);
	glEnable(GL_DEPTH_TEST);

	// Upload variables to shader
	glUniformMatrix4fv(glGetUniformLocation(texShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);
	glUniform1i(glGetUniformLocation(texShader, "texPositionsUnit"), 0);
	glUniform1i(glGetUniformLocation(texShader, "texLookUnit"), 1);
	glUniform1f(glGetUniformLocation(texShader, "pixelSize"), pixelSize);
	DrawModelInstanced(hailModel, texShader, "in_Position", NULL, "in_TexCoord", numParticles);

	// Render plane
	glUseProgram(phongShader);
	glUniformMatrix4fv(glGetUniformLocation(phongShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniformMatrix4fv(glGetUniformLocation(phongShader, "modelviewMatrix"), 1, GL_TRUE, m_plane.m);

	DrawModel(planeModel, phongShader, "in_Position", "in_Normal", NULL);

	printError("display");
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

int main(int argc, char *argv[])
{
  //initialize GLUT
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(W, H);
  glutInitContextVersion(3, 2); // modern OpenGL
  glutCreateWindow ("Hail simulation");

  // display window and update every 20 ms
  glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	// glutMouseFunc(mouseUpDown);
	// glutMotionFunc(mouseDragged);
	glutIdleFunc(idle);
  glutTimerFunc(20, &onTimer, 0);

  // initalize
  init();
  glutMainLoop();
  exit(0);

}
