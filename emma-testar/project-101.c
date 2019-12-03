
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
int numParticles = 1000;
float pixelSize;
int whichTexture = 1;

// Globals
FBOstruct *positionTex1, *positionTex2, *velocityTex1, *velocityTex2;
GLuint minShader = 0,
       updatePosShader = 0,
		   updateVelShader = 0,
		   renderShader = 0,
		   phongShader = 0,
			 texShader = 0;
GLfloat a = 0.0;
vec3 forward = {0, 0, -4};
vec3 cam = {0, 1, 10};
vec3 point = {0, 1, 0};
vec3 up = {0, 1, 0};

// texture
GLuint hailtex;

// Billboard to draw texture on
GLfloat squareVertices[] = {
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

Model *squareModel, *hailModel, *planeModel;//, *sphere;

// matrices for rendering
mat4 projectionMatrix, viewMatrix, modelToWorldMatrix;

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

	// load sphere
	// sphere = LoadModelPlus("sphere.obj");

  squareModel = LoadDataToModel(
		squareVertices, NULL, squareTexCoord, NULL,
		squareIndices, 4, 6);
	hailModel = LoadDataToModel(
		quadVertices, NULL, quadTexcoords, NULL,
		quadIndices, 4, 6);

	planeModel = LoadModelPlus("plane.obj");

	// initialize matrices

	viewMatrix = lookAtv(cam, point, up);
	modelToWorldMatrix = IdentityMatrix();

	//glutTimerFunc(20, &onTimer, 0);

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
  // clear the screen
  glClearColor(0.1, 0.1, 0.3, 0); // sets to blue color
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// fix matrices
	mat4 worldToView, vm2, m;
	vm2 = Mult(viewMatrix, modelToWorldMatrix);
	vm2 = Mult(vm2, T(0, -8.5, 0));
	vm2 = Mult(vm2, S(80,80,80));

	worldToView = lookAtv(cam, VectorAdd(cam, forward), up);
	a += 0.1;
	m = Mult(worldToView, Mult(T(-1, 0.5, 0), Mult(Ry(-a),Rz(M_PI/8))));
	m = T(m.m[3], m.m[7], m.m[11]);


	// // Update particles
	// if (whichTexture == 1) {
	// 	// --------- Run physics calculations ---------
	// 	// runShader(updatePosShader, positionTex1, velocityTex1, positionTex2);
	// 	// runShader(updateVelShader, positionTex2, velocityTex1, velocityTex2);
	//
	// 	// --------- Render result ---------
	// 	useFBO(0L, positionTex2, 0L);
	//
	// 	// Clear framebuffer & zbuffer
	// 	glClearColor(0.1, 0.1, 0.3, 0);
	// 	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//
	// 	// render to screen
	// 	glBindTexture(GL_TEXTURE_2D, hailtex);
  //   glUseProgram(texShader);
	// 	glUniformMatrix4fv(glGetUniformLocation(texShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	// 	glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);
	// 	glUniform1i(glGetUniformLocation(texShader, "texPositionsUnit"), 0);
	// 	glUniform1i(glGetUniformLocation(texShader, "texLookUnit"), 1);
	// 	glUniform1f(glGetUniformLocation(texShader, "pixelSize"), pixelSize);
	//
	// 	// Enable Z-buffering
	// 	glEnable(GL_DEPTH_TEST);
	// 	// Enable backface culling
	// 	glEnable(GL_CULL_FACE);
	// 	glCullFace(GL_BACK);
	//
	// 	DrawModelInstanced(hailModel, texShader, "in_Position", NULL, "in_TexCoord", numParticles);
	//
	// 	glUseProgram(phongShader);
	// 	glUniformMatrix4fv(glGetUniformLocation(phongShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	// 	glUniformMatrix4fv(glGetUniformLocation(phongShader, "modelviewMatrix"), 1, GL_TRUE, vm2.m);
	//
	// 	DrawModel(planeModel, phongShader, "in_Position", "in_Normal", NULL);
	//
	// 	whichTexture = 2;
	// }
	// else {
	// 	// runShader(updatePosShader, positionTex2, velocityTex2, positionTex1);
	// 	// runShader(updateVelShader, positionTex1, velocityTex2, velocityTex1);
	//
	// 	// fbo to render from
	// 	useFBO(0L, positionTex1, 0L);
	//
	// 	// Clear framebuffer & zbuffer
	// 	glClearColor(0.1, 0.1, 0.3, 0);
	// 	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//
	// 	// render to screen
	// 	glBindTexture(GL_TEXTURE_2D, hailtex);
	// 	glUseProgram(texShader);
	// 	glUniformMatrix4fv(glGetUniformLocation(texShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	// 	glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);
	// 	glUniform1i(glGetUniformLocation(texShader, "texPositionsUnit"), 0);
	// 	glUniform1i(glGetUniformLocation(texShader, "texLookUnit"), 1);
	// 	glUniform1f(glGetUniformLocation(texShader, "pixelSize"), pixelSize);
	//
	// 	// Enable Z-buffering
	// 	glEnable(GL_DEPTH_TEST);
	// 	// Enable backface culling
	// 	glEnable(GL_CULL_FACE);
	// 	glCullFace(GL_BACK);
	//
  //   DrawModelInstanced(hailModel, texShader, "in_Position", NULL, "in_TexCoord", numParticles);
	//
	// 	glUseProgram(phongShader);
	// 	glUniformMatrix4fv(glGetUniformLocation(phongShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	// 	glUniformMatrix4fv(glGetUniformLocation(phongShader, "modelviewMatrix"), 1, GL_TRUE, vm2.m);
	//
	// 	DrawModel(planeModel, phongShader, "in_Position", "in_Normal", NULL);
	//
	// 	whichTexture = 1;
	// }

	// BILLBIARD HAIL
	// a += 0.1;
	// worldToView = lookAtv(cam, VectorAdd(cam, forward), up);
	glBindTexture(GL_TEXTURE_2D, hailtex);
	// Billboard
	glUseProgram(texShader);
	// m = Mult(worldToView, Mult(T(-1, 0.5, 0), Mult(Ry(-a),Rz(M_PI/8))));
	// Modify m!
	// View plane oriented billboard: Zap rotation!
	// m = T(m.m[3], m.m[7], m.m[11]);
	glUniformMatrix4fv(glGetUniformLocation(texShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);
	DrawModel(hailModel, texShader, "in_Position", NULL, "in_TexCoord");

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
	glutMouseFunc(mouseUpDown);
	glutMotionFunc(mouseDragged);
	glutIdleFunc(idle);
  glutTimerFunc(20, &onTimer, 0);

  // initalize
  init();
  glutMainLoop();
  exit(0);

}
