/*
 * Lab-01_students.c
 *
 *     This program draws straight lines connecting dots placed with mouse clicks.
 *
 * Usage:
 *   Left click to place a control point.
 *		Maximum number of control points allowed is currently set at 64.
 *	 Press "f" to remove the first control point
 *	 Press "l" to remove the last control point.
 *	 Press escape to exit.
 */


#include <iostream>
#include "ShaderMaker.h"
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>

 // Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

static unsigned int programId;

unsigned int VAO;
unsigned int VBO;

unsigned int VAO_2;
unsigned int VBO_2;

using namespace glm;

#define MaxNumPts 300
float PointArray[MaxNumPts][2];
float CurveArray[MaxNumPts][2];

int NumPts = 0;

bool dragging = false;
float bound = 0.05;

// Window size in pixels
int		width = 500;
int		height = 500;

/* Prototypes */
void addNewPoint(float x, float y);
int main(int argc, char** argv);
void removeFirstPoint();
void removeLastPoint();


void myKeyboardFunc(unsigned char key, int x, int y)
{
	switch (key) {
	case 'f':
		removeFirstPoint();
		glutPostRedisplay();
		break;
	case 'l':
		removeLastPoint();
		glutPostRedisplay();
		break;
	case 27:			// Escape key
		exit(0);
		break;
	}
}
void removeFirstPoint() {
	int i;
	if (NumPts > 0) {
		// Remove the first point, slide the rest down
		NumPts--;
		for (i = 0; i < NumPts; i++) {
			PointArray[i][0] = PointArray[i + 1][0];
			PointArray[i][1] = PointArray[i + 1][1];
		}
	}
}
void resizeWindow(int w, int h)
{
	height = (h > 1) ? h : 2;
	width = (w > 1) ? w : 2;
	gluOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f);
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
}

// Left button presses place a new control point.
void myMouseFunc(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		float xPos = -1.0f + ((float)x) * 2 / ((float)(width));
		float yPos = -1.0f + ((float)(height - y)) * 2 / ((float)(height));

		bool moved = false;
		for (int i = 0; i < NumPts; i++) {
			if ((xPos - PointArray[i][0] < bound && xPos - PointArray[i][0] >  -bound) &&
				(yPos - PointArray[i][1] < bound && yPos - PointArray[i][1] >  -bound))
			{
				moved = true;
			}
		}
		if (moved == false)
			addNewPoint(xPos, yPos);
		glutPostRedisplay();

	}
}

void myPassiveMotionFunc(int x, int y) {
	if (glutGetModifiers() == GLUT_LEFT_BUTTON) dragging = true;
	else dragging = false;
}

void myMotionFunc(int x, int y) {
	if (dragging) {
		float xPos = -1.0f + ((float)x) * 2 / ((float)(width));
		float yPos = -1.0f + ((float)(height - y)) * 2 / ((float)(height));

		for (int i = 0; i < NumPts; i++) {
			if ((xPos - PointArray[i][0] < bound && xPos - PointArray[i][0] >  -bound) &&
				(yPos - PointArray[i][1] < bound && yPos - PointArray[i][1] >  -bound))
			{
				//moves the point without creating a new one
				PointArray[i][0] = xPos;
				PointArray[i][1] = yPos;
				glutPostRedisplay();
				return;
			}
		}
	}
}

// Add a new point to the end of the list.  
// Remove the first point in the list if too many points.
void removeLastPoint() {
	if (NumPts > 0) {
		NumPts--;
	}
}

// Add a new point to the end of the list.  
// Remove the first point in the list if too many points.
void addNewPoint(float x, float y) {
	if (NumPts >= MaxNumPts) {
		removeFirstPoint();
	}
	PointArray[NumPts][0] = x;
	PointArray[NumPts][1] = y;
	NumPts++;
}
void initShader(void)
{
	GLenum ErrorCheckValue = glGetError();

	char* vertexShader = (char*)"vertexShader.glsl";
	char* fragmentShader = (char*)"fragmentShader.glsl";

	programId = ShaderMaker::createProgram(vertexShader, fragmentShader);
	glUseProgram(programId);

}


void init(void)
{
	// VAO for control polygon
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// VAO for curve
	glGenVertexArrays(1, &VAO_2);
	glBindVertexArray(VAO_2);
	glGenBuffers(1, &VBO_2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO_2);

	// Background color
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glViewport(0, 0, 500, 500);
}

void de_Casteljau(float t, float* result) {

	float CurveArray_bezier[MaxNumPts][MaxNumPts][2];
	for (int y = 0; y < NumPts; y++) {
		CurveArray_bezier[0][y][0] = PointArray[y][0];
		CurveArray_bezier[0][y][1] = PointArray[y][1];
	}

	for (int i = 1; i < NumPts; i++) {
		for (int j = 0; j < NumPts - i; j++) {
			CurveArray_bezier[i][j][0] = ((1 - t) * CurveArray_bezier[i - 1][j][0]) + (t * CurveArray_bezier[i - 1][j + 1][0]);
			CurveArray_bezier[i][j][1] = ((1 - t) * CurveArray_bezier[i - 1][j][1]) + (t * CurveArray_bezier[i - 1][j + 1][1]);
		}

	}
	result[0] = CurveArray_bezier[NumPts - 1][0][0];
	result[1] = CurveArray_bezier[NumPts - 1][0][1];
}

void drawScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	if (NumPts > 1) {
		// Draw curve
		// TODO
		float result[2];
		for (int i = 0; i <= 100; i++) {
			de_Casteljau((GLfloat)i / 100, result);
			CurveArray[i][0] = result[0];
			CurveArray[i][1] = result[1];
		}


		glBindVertexArray(VAO_2);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_2);
		glBufferData(GL_ARRAY_BUFFER, sizeof(CurveArray), &CurveArray[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glLineWidth(1.0);
		glDrawArrays(GL_LINE_STRIP, 0, 100);

		glBindVertexArray(0);
	}
	// Draw control polygon
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(PointArray), &PointArray[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// Draw the control points CP
	glPointSize(6.0);
	glDrawArrays(GL_POINTS, 0, NumPts);
	// Draw the line segments between CP
	glLineWidth(2.0);
	glDrawArrays(GL_LINE_STRIP, 0, NumPts);
	glBindVertexArray(0);
	glutSwapBuffers();
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(width, height);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Draw curves 2D");

	glutDisplayFunc(drawScene);
	glutReshapeFunc(resizeWindow);
	glutKeyboardFunc(myKeyboardFunc);
	glutMouseFunc(myMouseFunc);
	glutMotionFunc(myMotionFunc);
	glutPassiveMotionFunc(myPassiveMotionFunc);

	glewExperimental = GL_TRUE;
	glewInit();

	initShader();
	init();

	glutMainLoop();
}