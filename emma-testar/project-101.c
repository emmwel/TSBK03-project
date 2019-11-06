
// Line to compile on mac
// gcc project-101.c ../common/*.c ../common/Mac/MicroGlut.m -o project-101 -framework OpenGL -framework Cocoa -I../common/Mac -I../common -Wno-deprecated-declarations

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

// Globals
FBOstruct *fbo1;
GLuint min_shader = 0;

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

Model* squareModel;

void init(void)
{
    // Reference to shader program:
    GLuint program;

	dumpInfo();

    // GL inits
    glClearColor(1.0,1.0,1.0,0); // sets to white color
    glEnable(GL_DEPTH_TEST);

    // Load and compile shader
    min_shader = loadShaders("minimal.vert", "minimal.frag"); // In our GL_utilities
    printError("init shader");

    fbo1 = initNoiseFBO(W, H, 0); // picture

    squareModel = LoadDataToModel(
			square, NULL, squareTexCoord, NULL,
			squareIndices, 4, 6);

}

void display(void)
{
    useFBO(0L, fbo1, 0L);

    // clear the screen
    glClearColor(1.0,1.0,1.0,0); // sets to white color
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Activate shader program
    glUseProgram(min_shader);
    glUniform1i(glGetUniformLocation(min_shader, "texUnit"), 0);

    DrawModel(squareModel, min_shader, "in_Position", NULL, "in_TexCoord");

    glFlush();
    glutSwapBuffers();
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(W, H);

    glutInitContextVersion(3, 2);
    glutCreateWindow ("GL3 white triangle example");
    glutDisplayFunc(display);
    init ();
    glutMainLoop();
}
