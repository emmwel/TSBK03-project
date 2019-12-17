
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
#define W 1000
#define H 1000

// define PI for physics
//#define M_PI 3.1415926535897932384626433832795

void onTimer(int value);

// particle amounts, pixel size
int numParticles = 16384;
float maxLifetime = 50.0;
float pixelSize;
bool firstTexture = true;

// model
float planeWidth = 580;

// Globals
FBOstruct *positionTex1, *positionTex2, *velocityTex1, *velocityTex2, *depthBuffer;
GLuint minShader = 0,
       updatePosShader = 0,
		   updateVelShader = 0,
		   renderShader = 0,
		   phongShader = 0,
		   texShader = 0,
		   depthShader = 0,
			 emptyShader = 0;

// Depth camera
vec3 forwardDepth = {0, -1, 0};
vec3 camDepth = {0, 100, 0};
float camFarClip = 105;
float camNearClip = 1.0;
vec3 point = {0, 0, 0};
vec3 upDepth = {0, 0, -1};

// Scene camera
vec3 forward= {0, 0, -1};
vec3 cam = {0, 75, 200};
vec3 up = {0, 1, 0};

// Shadow map camera (ligt source)
// vec3 camShadow = {100.0, 100.0, 100.0};
// float camShadowNearClip = 1.0;
// float camShadowFarClip = 1000.0;

//colors
vec4 black = {0.1, 0.1, 0.1, 1.0};
vec4 gray = {0.9, 0.9, 0.9, 1.0};
vec4 blue = {0.3, 0.5, 0.9, 1.0};
vec4 green = {0.5, 0.8, 0.4, 1.0};

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

Model *squareModel, *hailModel, *planeModel, *sphere, *houseFoundation, *houseWindows, *houseMetal, *houseGround;

// Matrices for rendering
// mat4 projectionMatrixOrthoShadow;
mat4 projectionMatrixPerspective;
mat4 projectionMatrixOrthographic;
mat4 viewMatrix;
mat4 modelToWorldMatrix;
mat4 worldToView;
mat4 worldToViewDepth;
mat4 m;
mat4 m_house;

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
	//fprintf(stderr, "%f\n", deltaT);
    currentTime = getElapsedTime();
    glutTimerFunc(20, &onTimer, value);
}

void renderDepth() {
	// ---------------------- Render depth -------------------------------
	useFBO(depthBuffer, 0L, 0L);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(emptyShader);
	glEnable(GL_DEPTH_TEST);
	glUniformMatrix4fv(glGetUniformLocation(emptyShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrixOrthographic.m);
	glUniformMatrix4fv(glGetUniformLocation(emptyShader, "modelviewMatrix"), 1, GL_TRUE, m_house.m);
	DrawModel(houseFoundation, emptyShader, "in_Position", NULL, NULL);
	DrawModel(houseMetal, emptyShader, "in_Position", NULL, NULL);
	DrawModel(houseWindows, emptyShader, "in_Position", NULL, NULL);
	DrawModel(houseGround, emptyShader, "in_Position", NULL, NULL);
}

void init(void)
{
	dumpInfo();

	// GL inits
	glClearColor(0.5, 0.5, 0.5, 1.0);
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
	depthShader = loadShaders("depth.vert", "depth.frag");
	emptyShader = loadShaders("empty.vert", "empty.frag");
	printError("init shader");

	//textures
	LoadTGATextureSimple("hail-small.tga", &hailtex);

	// Initialize texture FBOs for simulation
	positionTex1 = initPositionsFBO(numParticles, 1, 0, planeWidth); // start positions
	positionTex2 = initZeroFBO(numParticles, 1, 0);
	velocityTex1 = initVelocityFBO(numParticles, 1, 0);
	velocityTex2 = initZeroFBO(numParticles, 1, 0);

	// create FBO which saves depth
	depthBuffer = initFBO2(W, H, 0, 2);

	// Load models
	squareModel = LoadDataToModel(
		squareVertices, NULL, squareTexCoord, NULL,
		squareIndices, 4, 6);
	hailModel = LoadDataToModel(
		quadVertices, NULL, quadTexcoords, NULL,
		quadIndices, 4, 6);
	// planeModel = LoadModelPlus("plane-and-objects.obj");
	houseFoundation = LoadModelPlus("house_foundation2.obj");
	houseMetal = LoadModelPlus("house_metal_pieces.obj");
	houseWindows = LoadModelPlus("house_glass_pieces.obj");
	houseGround = LoadModelPlus("house_ground.obj");

	// initialize matrices
	viewMatrix = lookAtv(cam, point, up);
	modelToWorldMatrix = IdentityMatrix();

	// Fix matrices
	mat4 worldToViewDepth = lookAtv(camDepth, VectorAdd(camDepth, forwardDepth), upDepth);
	m = Mult(worldToViewDepth, modelToWorldMatrix);
	m = Mult(worldToViewDepth, Mult(T(-1, 0.5, 0), IdentityMatrix()));
	m = T(m.m[3], m.m[7], m.m[11]);
	m_house = Mult(worldToViewDepth, S(10.0, 10.0, 10.0));

	// Cameras
	glViewport(0, 0, W, H);
	GLfloat ratio = (GLfloat) W / (GLfloat) H;
	projectionMatrixPerspective = perspective(90, ratio, 1.0, 1000.0);
	projectionMatrixOrthographic = ortho(-planeWidth/2, planeWidth/2, -planeWidth/2, planeWidth/2, camNearClip, camFarClip);

	renderDepth();

	worldToView = lookAtv(cam, VectorAdd(cam, forward), up);
	m = Mult(worldToView, modelToWorldMatrix);
	m = Mult(worldToView, Mult(T(-1, 0.5, 0), IdentityMatrix()));
	m = T(m.m[3], m.m[7], m.m[11]);
	m_house = Mult(worldToView, S(10.0, 10.0, 10.0));

	resetElapsedTime();
}

void runPosShader(GLuint shader, FBOstruct *in1, FBOstruct *in2, FBOstruct *out) {

  glUseProgram(shader);

  // Many of these things would be more efficiently done once and for all
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  useFBO(out, in1, in2);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glUniform1i(glGetUniformLocation(shader, "texUnitPosition"), 0);
  glUniform1i(glGetUniformLocation(shader, "texUnitVelocity"), 1);
  glUniform1f(glGetUniformLocation(shader, "deltaTime"), deltaT);
  glUniform1f(glGetUniformLocation(shader, "pixelSize"), pixelSize);
  glUniform1f(glGetUniformLocation(shader, "maxLifetime"), maxLifetime);

  DrawModel(squareModel, shader, "in_Position", NULL, "in_TexCoord");
}

void runVelShader(GLuint shader, FBOstruct *in1, FBOstruct *in2, FBOstruct *out) {

  glUseProgram(shader);

  // Many of these things would be more efficiently done once and for all
  glDisable(GL_CULL_FACE);
  glDisable(GL_DEPTH_TEST);
  useFBO(out, in1, in2);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, depthBuffer->depth);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // positions and velocity update variables
  glUniform1i(glGetUniformLocation(shader, "texUnitPosition"), 0);
  glUniform1i(glGetUniformLocation(shader, "texUnitVelocity"), 1);
  glUniform1f(glGetUniformLocation(shader, "deltaTime"), deltaT);
  glUniform1f(glGetUniformLocation(shader, "pixelSize"), pixelSize);

  // depth buffer variables
  glUniform1i(glGetUniformLocation(shader, "texUnitDepth"), 2);
  glUniformMatrix4fv(glGetUniformLocation(shader, "orthogonalProjectionMatrix"), 1, GL_TRUE, projectionMatrixOrthographic.m);
  glUniformMatrix4fv(glGetUniformLocation(shader, "worldToView"), 1, GL_TRUE, worldToViewDepth.m);
  glUniform1f(glGetUniformLocation(shader, "zFar"), camFarClip);
  glUniform1f(glGetUniformLocation(shader, "zNear"), camNearClip);
  glUniform1f(glGetUniformLocation(shader, "camHeight"), camDepth.y);
  glUniform1f(glGetUniformLocation(shader, "planeWidth"), planeWidth);

  // particle defs for air resistance
  float radius = 0.005; // in meters
  float splitArea =  radius * radius * M_PI;
  // particles modeled as spheres in air
  float airDragCoefficient = 0.47;
  // air density
  float airDensity = 1.2;

  // mass in kg
  float mass = 0.005;

  // forces variables
  glUniform1f(glGetUniformLocation(shader, "splitArea"), splitArea);
  glUniform1f(glGetUniformLocation(shader, "airDragCoefficient"), airDragCoefficient);
  glUniform1f(glGetUniformLocation(shader, "airDensity"), airDensity);
  glUniform1f(glGetUniformLocation(shader, "mass"), mass);

  // lifetime
  glUniform1f(glGetUniformLocation(shader, "maxLifetime"), maxLifetime);

  DrawModel(squareModel, shader, "in_Position", NULL, "in_TexCoord");
}

void display(void)
{
	// Update particles
	if (firstTexture == 1) {
		// --------- Run physics calculations ---------
		runPosShader(updatePosShader, positionTex1, velocityTex1, positionTex2);
		runVelShader(updateVelShader, positionTex2, velocityTex1, velocityTex2);

		// Use position texture
		useFBO(0L, positionTex2, 0L);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	else {
		// --------- Run physics calculations ---------
		runPosShader(updatePosShader, positionTex2, velocityTex2, positionTex1);
		runVelShader(updateVelShader, positionTex1, velocityTex2, velocityTex1);

		// Use position texture
		useFBO(0L, positionTex1, 0L);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}
	// Switch which position texture to render from
	firstTexture = !firstTexture;

	// Bind hailstone appearance texture
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, hailtex);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Activate shader
	glUseProgram(texShader);
	glEnable(GL_DEPTH_TEST);

	// Upload variables to shader
	glUniformMatrix4fv(glGetUniformLocation(texShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrixPerspective.m);
	glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);
	glUniform1i(glGetUniformLocation(texShader, "texPositionsUnit"), 0);
	glUniform1i(glGetUniformLocation(texShader, "texLookUnit"), 1);
	glUniform1f(glGetUniformLocation(texShader, "pixelSize"), pixelSize);
	DrawModelInstanced(hailModel, texShader, "in_Position", NULL, "in_TexCoord", numParticles);

	// Render plane
	glUseProgram(phongShader);
	glEnable(GL_DEPTH_TEST);
	glUniformMatrix4fv(glGetUniformLocation(phongShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrixPerspective.m);
	glUniformMatrix4fv(glGetUniformLocation(phongShader, "modelviewMatrix"), 1, GL_TRUE, m_house.m);
	glUniform4f(glGetUniformLocation(phongShader, "surface_color"), gray.x, gray.y, gray.z, gray.w);
	DrawModel(houseFoundation, phongShader, "in_Position", "in_Normal", NULL);
	glUniform4f(glGetUniformLocation(phongShader, "surface_color"), black.x, black.y, black.z, black.w);
	DrawModel(houseMetal, phongShader, "in_Position", "in_Normal", NULL);
	glUniform4f(glGetUniformLocation(phongShader, "surface_color"), blue.x, blue.y, blue.z, blue.w);
	DrawModel(houseWindows, phongShader, "in_Position", "in_Normal", NULL);
	glUniform4f(glGetUniformLocation(phongShader, "surface_color"), green.x, green.y, green.z, green.w);
	DrawModel(houseGround, phongShader, "in_Position", "in_Normal", NULL);

	// // Draw the depth buffer to screen
	// useFBO(0L, 0L, 0L);
	// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, depthBuffer->depth);
	//
	// glUseProgram(depthShader);
	// glUniform1i(glGetUniformLocation(depthShader, "texUnit"), 0);
	// DrawModel(squareModel, depthShader, "in_Position", NULL, "in_TexCoord");

	printError("display");
  glutSwapBuffers();
}

void reshape(GLsizei w, GLsizei h)
{
	glViewport(0, 0, w, h);
	GLfloat ratio = (GLfloat) w / (GLfloat) h;
	projectionMatrixPerspective = perspective(90, ratio, 1.0, 1000.0);
	projectionMatrixOrthographic = ortho(-planeWidth/2, planeWidth/2, -planeWidth/2, planeWidth/2, camNearClip, camFarClip);
	// projectionMatrixOrthoShadow = ortho(-planeWidth/2, planeWidth/2, -planeWidth/2, planeWidth/2, camShadowNearClip, camShadowFarClip);
}

// Necessary
void idle()
{
  glutPostRedisplay();
}

// Trackball
int prevx = 0, prevy = 0;

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
  glutIdleFunc(idle);
  glutTimerFunc(20, &onTimer, 0);

  // initalize
  init();
  glutMainLoop();
  exit(0);

}
