// Draft for a demo of billboarding
// Cleaned up version 2019

// MS Windows needs GLEW or glee.
#ifdef __APPLE__
	#include <OpenGL/gl3.h>
	#include "MicroGlut.h"
	// linking hint for Lightweight IDE
	//uses framework Cocoa
#endif
#include "GL_utilities.h"
#include "VectorUtils3.h"
#include "loadobj.h"
#include "LoadTGA.h"

mat4 projectionMatrix;

Model *quad, *floormodel;
GLuint grasstex;
GLuint hailtex;
GLuint lir_vpt, lir_vpl, lir_avpt, lir_avpl;

// Reference to shader programs
GLuint texShader;


// A quad
GLfloat vertices[] = {	-0.5,-0.5,0.0,
						0.5,-0.5,0.0,
						0.5,0.5,0.0,
						-0.5,0.5,0.0};
GLfloat texcoord[] = {	0.0f, 1.0f,
						1.0f, 1.0f,
						1.0f, 0.0f,
						0.0f, 0.0f};
GLuint indices[] = {0,3,2, 0,2,1};

// Another quad
GLfloat vertices2[] = {	-20.5,0.0,-20.5,
						20.5,0.0,-20.5,
						20.5,0.0,20.5,
						-20.5,0.0,20.5};
GLfloat texcoord2[] = {	100.0f, 100.0f,
						0.0f, 100.0f,
						0.0f, 0.0f,
						100.0f, 0.0f};
GLuint indices2[] = {	0,3,2, 0,2,1};

void init(void)
{
	// GL inits
	glClearColor(0.2,0.2,0.5,0);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	printError("GL inits");

	projectionMatrix = frustum(-0.1, 0.1, -0.1, 0.1, 0.2, 300.0);

	// Load and compile shader
	texShader = loadShaders("textured.vert", "textured.frag");
	printError("init shader");

	// Upload geometry to the GPU:
	quad = LoadDataToModel(vertices, NULL, texcoord, NULL,
			indices, 4, 6);
	floormodel = LoadDataToModel(vertices2, NULL, texcoord2, NULL,
			indices, 4, 6);

// Important! The shader we upload to must be active!

	glUseProgram(texShader);
	glUniformMatrix4fv(glGetUniformLocation(texShader, "projectionMatrix"), 1, GL_TRUE, projectionMatrix.m);
	glUniform1i(glGetUniformLocation(texShader, "tex"), 0); // Texture unit 0

	LoadTGATextureSimple("Ingemar-256-vpt.tga", &lir_vpt);
	LoadTGATextureSimple("Ingemar-256-vpl.tga", &lir_vpl);
	LoadTGATextureSimple("Ingemar-256-avpt.tga", &lir_avpt);
	LoadTGATextureSimple("Ingemar-256-avpl.tga", &lir_avpl);

	LoadTGATextureSimple("hail-texture.tga", &hailtex);
	LoadTGATextureSimple("grass.tga", &grasstex);
	glBindTexture(GL_TEXTURE_2D, grasstex);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_S,	GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,	GL_TEXTURE_WRAP_T,	GL_REPEAT);

	printError("init arrays");
}

GLfloat a = 0.0;
vec3 campos = {0, 0.5, 4};
vec3 forward = {0, 0, -4};
vec3 up = {0, 1, 0};

void display(void)
{
	int i, j;

	printError("pre display");

	// clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat4 worldToView, m; // m1, m2, m, tr, scale;

	if (glutKeyIsDown('a'))
		forward = MultMat3Vec3(mat4tomat3(Ry(0.03)), forward);
	if (glutKeyIsDown('d'))
		forward = MultMat3Vec3(mat4tomat3(Ry(-0.03)), forward);
	if (glutKeyIsDown('w'))
		campos = VectorAdd(campos, ScalarMult(forward, 0.01));
	if (glutKeyIsDown('s'))
		campos = VectorSub(campos, ScalarMult(forward, 0.01));
	if (glutKeyIsDown('q'))
	{
		vec3 side = CrossProduct(forward, SetVector(0,1,0));
		campos = VectorSub(campos, ScalarMult(side, 0.01));
	}
	if (glutKeyIsDown('e'))
	{
		vec3 side = CrossProduct(forward, SetVector(0,1,0));
		campos = VectorAdd(campos, ScalarMult(side, 0.01));
	}

	// Move up/down
	if (glutKeyIsDown('z'))
		campos = VectorAdd(campos, ScalarMult(SetVector(0,1,0), 0.01));
	if (glutKeyIsDown('c'))
		campos = VectorSub(campos, ScalarMult(SetVector(0,1,0), 0.01));

	// NOTE: Looking up and down is done by making a side vector and rotation around arbitrary axis!
	if (glutKeyIsDown('+'))
	{
		vec3 side = CrossProduct(forward, SetVector(0,1,0));
		mat4 m = ArbRotate(side, 0.05);
		forward = MultMat3Vec3(mat4tomat3(m), forward);
	}
	if (glutKeyIsDown('-'))
	{
		vec3 side = CrossProduct(forward, SetVector(0,1,0));
		mat4 m = ArbRotate(side, -0.05);
		forward = MultMat3Vec3(mat4tomat3(m), forward);
	}

	worldToView = lookAtv(campos, VectorAdd(campos, forward), up);

	a += 0.1;

	glBindTexture(GL_TEXTURE_2D, grasstex);
	// Floor
	glUseProgram(texShader);
	m = worldToView;
	glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);
	DrawModel(floormodel, texShader, "inPosition", NULL, "inTexCoord");


	glBindTexture(GL_TEXTURE_2D, hailtex);
	// Billboard
	glUseProgram(texShader);
	m = Mult(worldToView, Mult(T(-1, 0.5, 0), Mult(Ry(-a),Rz(M_PI/8))));
	// Modify m!
	// View plane oriented billboard: Zap rotation!
	m = T(m.m[3], m.m[7], m.m[11]);
	glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);
	DrawModel(quad, texShader, "inPosition", NULL, "inTexCoord");

	glBindTexture(GL_TEXTURE_2D, lir_avpt);
	// Billboard
	// Axial viewpoint
	glUseProgram(texShader);
	vec3 billloc = {1, 0.5, 0};
	vec3 pos2bill = VectorSub(campos, billloc);
	pos2bill.y = 0; // Axial!
	pos2bill = Normalize(pos2bill);
	m = IdentityMatrix();
//	m = Ry(atan2(pos2bill.z, -pos2bill.x)+3.14/2);
	m.m[0] = pos2bill.z;
	m.m[2] = pos2bill.x;
	m.m[8] = -pos2bill.x;
	m.m[10] = pos2bill.z;
//	m = Mult(worldToView, Mult(T(1, 1, 0), Mult(Ry(-a),Rz(M_PI/8))));
	m = Mult(worldToView, Mult(T(1, 0.5, 0), m));
	glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);
	DrawModel(quad, texShader, "inPosition", NULL, "inTexCoord");

	glBindTexture(GL_TEXTURE_2D, lir_avpl);
	// Billboard
	// Axial view plane
	// Project forward vector on XZ
	vec3 forwardxz = {-forward.x, 0, -forward.z};
	forwardxz = Normalize(forwardxz);
	m = IdentityMatrix();
//	m = Ry(atan2(pos2bill.z, -pos2bill.x)+3.14/2);
	m.m[0] = forwardxz.z;
	m.m[2] = forwardxz.x;
	m.m[8] = -forwardxz.x;
	m.m[10] = -forwardxz.z;
	glUseProgram(texShader);
//	m = Mult(worldToView, Mult(T(-1, 1, 1), Mult(Ry(-a),Rz(M_PI/8))));
	m = Mult(worldToView, Mult(T(-1, 0.5, 1), m));
	glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);
	DrawModel(quad, texShader, "inPosition", NULL, "inTexCoord");

	// Billboard
	// Viewpoint!
	glBindTexture(GL_TEXTURE_2D, lir_vpt);
//	vec3 bw = {-forward.x, 0, -forward.z};
	vec3 billlocvp = {1, 0.5, 1};
	vec3 bw = VectorSub(campos, billlocvp);
	bw = Normalize(bw);
	vec3 rv = Normalize(CrossProduct(SetVector(0,1,0), bw));
	vec3 up = CrossProduct(bw, rv);
	m = IdentityMatrix();
	m.m[0] = rv.x;
	m.m[1] = rv.y;
	m.m[2] = rv.z;
	m.m[4] = up.x;
	m.m[5] = up.y;
	m.m[6] = up.z;
	m.m[8] = bw.x;
	m.m[9] = bw.y;
	m.m[10] = bw.z;
	m = Transpose(m);
	glUseProgram(texShader);
	m = Mult(worldToView, Mult(T(1, 0.5, 1), m));
	glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);
	DrawModel(quad, texShader, "inPosition", NULL, "inTexCoord");

	// Double world oriented
	glUseProgram(texShader);
	m = Mult(worldToView, Mult(T(5, 1, 0), Mult(Ry(-a),Rz(M_PI/8))));
	glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);
	DrawModel(quad, texShader, "inPosition", NULL, "inTexCoord");
	m = Mult(m, Ry(3.14/2));
	glUniformMatrix4fv(glGetUniformLocation(texShader, "modelviewMatrix"), 1, GL_TRUE, m.m);
	DrawModel(quad, texShader, "inPosition", NULL, "inTexCoord");

	printError("display");

	glutSwapBuffers();
}

void keys(unsigned char key, int x, int y)
{
}

void onTimer(int value)
{
    glutPostRedisplay();
    //deltaT = getElapsedTime() - currentTime;
    //currentTime = getElapsedTime();
    glutTimerFunc(20, &onTimer, value);
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitContextVersion(3, 2);
	glutCreateWindow ("GL3 billboarding example");
	//glutRepeatingTimerFunc(20);
	glutTimerFunc(20, &onTimer, 0);
	glutDisplayFunc(display);
	glutKeyboardFunc(keys);
	init ();
	glutMainLoop();
}
