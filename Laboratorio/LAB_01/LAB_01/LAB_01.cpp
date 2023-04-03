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

float ArrayControlPointCatmull[MaxNumPts][2];
float CurveArrayCatmull[MaxNumPts * 10][2];

// attiva/disattiva DeCastejau
bool decast = false;
// attiva/disattiva punti controllo Catmull
bool catmullPoint = false;
// attiva/disattiva curva Catmull-Romm
bool catmullCurve = false;


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
	case 'd':
		if (decast == false) {
			decast = true;
		}
		else {
			decast = false;
		}
		glutPostRedisplay();
		break;
	case 'C':
		if (catmullPoint == false) {
			catmullPoint = true;
		}
		else {
			catmullPoint = false;
		}
		glutPostRedisplay();
		break;
	case 'c':
		if (catmullCurve == false) {
			catmullCurve = true;
		}
		else {
			catmullCurve = false;
		}
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

void deCasteljau(float t, float* result, float Buffer_ArrayPoints[][2], int Buffer_NumPts) {

	float CurveArray_bezier[MaxNumPts][MaxNumPts][2];
	for (int y = 0; y < Buffer_NumPts; y++) {
		CurveArray_bezier[0][y][0] = Buffer_ArrayPoints[y][0];
		CurveArray_bezier[0][y][1] = Buffer_ArrayPoints[y][1];
	}

	for (int i = 1; i < Buffer_NumPts; i++) {
		for (int j = 0; j < Buffer_NumPts - i; j++) {
			CurveArray_bezier[i][j][0] = ((1 - t) * CurveArray_bezier[i - 1][j][0]) + (t * CurveArray_bezier[i - 1][j + 1][0]);
			CurveArray_bezier[i][j][1] = ((1 - t) * CurveArray_bezier[i - 1][j][1]) + (t * CurveArray_bezier[i - 1][j + 1][1]);
		}

	}
	result[0] = CurveArray_bezier[Buffer_NumPts - 1][0][0];
	result[1] = CurveArray_bezier[Buffer_NumPts - 1][0][1];
}

// dato un punto, il suo precedente e il suo successivo, questa funzione calcola il P- e il P+
void calcolaPointPlusAndMinus(float pointPre[2], float point[2], float pointPost[2], float pointMinus[2], float pointPlus[2], int i) {
	float mX = (pointPost[0] - pointPre[0]) / 2.0;
	float mY = (pointPost[1] - pointPre[1]) / 2.0;
	float pointPlusX = point[0] + (mX / 3.0);
	float pointPlusY = point[1] + (mY / 3.0);
	float pointMinusX = point[0] - (mX / 3.0);
	float pointMinusY = point[1] - (mY / 3.0);
	pointMinus[0] = pointMinusX;
	pointMinus[1] = pointMinusY;
	pointPlus[0] = pointPlusX;
	pointPlus[1] = pointPlusY;

	std::cout << i << "^ vertice " << std::endl;
	std::cout << "point x: " << point[0] << std::endl;
	std::cout << "point y: " << point[1] << std::endl;

	std::cout << "PrevPoint x: " << pointMinus[0] << std::endl;
	std::cout << "PrevPoint y: " << pointMinus[1] << std::endl;

	std::cout << "NextPoint x: " << pointPlus[0] << std::endl;
	std::cout << "NextPoint y: " << pointPlus[1] << std::endl;
	std::cout << "\n" << std::endl;
	
}

void costruzioneArrayCatmull() {
	// dato un array di punti di controllo calcolo i pointMinus e pointPlus e creo ArrayPuntiControlloCatmull
	//ricopio i punti inseriti dall'utente in un nuovo array a cui aggiungo un punto iniziale uguale al primo inserito dall'utente e un punto finale uguale all'ultimo inserito dall'utente
	int numeroPuntiArrayEsteso = NumPts + 2;
	//-------------------------------------------------------------------------------------
	// crea un array uguale a quello dei PointArray con in più un punto all'inizio e uno alla fine uguali rispettivamente al primo e all'ultimo
	float arrayPuntiUtenteEsteso[MaxNumPts][2];
	arrayPuntiUtenteEsteso[0][0] = PointArray[0][0];
	arrayPuntiUtenteEsteso[0][1] = PointArray[0][1];
	arrayPuntiUtenteEsteso[numeroPuntiArrayEsteso - 1][0] = PointArray[NumPts - 1][0];
	arrayPuntiUtenteEsteso[numeroPuntiArrayEsteso - 1][1] = PointArray[NumPts - 1][1];
	for (int i = 0; i < NumPts; i++) {
		arrayPuntiUtenteEsteso[i + 1][0] = PointArray[i][0];
		arrayPuntiUtenteEsteso[i + 1][1] = PointArray[i][1];
	}
	//-------------------------------------------------------------------------------------
	int tmp_debug = 0;
	int contatore = 0; //contatore che tiene traccia del PointArrayCatmull
	// per ogni punto del arrayPuntiUtenteEsteso (a cui è stato aggiunto sia il punto iniziale che finale) viene calcolato il P- e il P+ come PointMinus e PointPlus, quindi vengono aggiunti PointMinus, Point e PointPlus al PointArrayCatmull. Nel caso del primo punto del arrayPuntiUtenteEsteso non viene aggiunto il PointMinus mentre nel caso dell'ultimo punto del arrayPuntiUtenteEsteso non viene aggiunto il PointPlus
	for (int i = 1; i < numeroPuntiArrayEsteso - 1; i++) {
		float pointPre[2];
		pointPre[0] = arrayPuntiUtenteEsteso[i - 1][0];
		pointPre[1] = arrayPuntiUtenteEsteso[i - 1][1];
		float pointPoint[2];
		pointPoint[0] = arrayPuntiUtenteEsteso[i][0];
		pointPoint[1] = arrayPuntiUtenteEsteso[i][1];
		float pointPost[2];
		pointPost[0] = arrayPuntiUtenteEsteso[i + 1][0];
		pointPost[1] = arrayPuntiUtenteEsteso[i + 1][1];
		float pointMinus[2];
		float pointPlus[2];
		calcolaPointPlusAndMinus(pointPre, pointPoint, pointPost, pointMinus, pointPlus, i);
		//aggiungo il minus
		if (i - 1 > 0) { // se è il primo non aggiungo il plus
			ArrayControlPointCatmull[contatore][0] = pointMinus[0];
			ArrayControlPointCatmull[contatore][1] = pointMinus[1];
			contatore++;
		}
		// aggiungo il point
		ArrayControlPointCatmull[contatore][0] = arrayPuntiUtenteEsteso[i][0];
		ArrayControlPointCatmull[contatore][1] = arrayPuntiUtenteEsteso[i][1];
		contatore++;
		if (i < numeroPuntiArrayEsteso - 2) { // se è l'ultimo non aggiungo il plus
			// aggiungo il plus
			ArrayControlPointCatmull[contatore][0] = pointPlus[0];
			ArrayControlPointCatmull[contatore][1] = pointPlus[1];
			contatore++;
		}

	}
	for (int j = 0; j < 10; j++) {
		std::cout << j + 1 << "^ punto x:" << ArrayControlPointCatmull[j][0] << " y:" << ArrayControlPointCatmull[j][1] << std::endl;
	}
	std::cout << "######################################" << std::endl;

}

void drawScene(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	if (NumPts > 1 && decast) {
		// Draw curve
		// TODO
		float result[2];
		for (int i = 0; i <= 100; i++) {
			deCasteljau((GLfloat)i / 100, result, PointArray, NumPts);
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
	if (NumPts > 1 && catmullPoint) {
		//std::cout << "Calcolo il pointArray per Catmull Rom" << std::endl;
		costruzioneArrayCatmull();
		glBindVertexArray(VAO_2);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_2);
		glBufferData(GL_ARRAY_BUFFER, sizeof(ArrayControlPointCatmull), &ArrayControlPointCatmull[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glPointSize(10.0);
		glDrawArrays(GL_POINTS, 0, NumPts * 3 - 2);
	}
	if (NumPts > 1 && catmullCurve) {
		//std::cout << "Calcolo la spline Catmull Rom" << std::endl;
		costruzioneArrayCatmull();
		int n_segmento = 0;
		for (int j = 0; j < (NumPts * 3) - 2; j = j + 3) {
			float arrayDeCast[4][2];
			arrayDeCast[0][0] = ArrayControlPointCatmull[j][0];
			arrayDeCast[0][1] = ArrayControlPointCatmull[j][1];
			arrayDeCast[1][0] = ArrayControlPointCatmull[j + 1][0];
			arrayDeCast[1][1] = ArrayControlPointCatmull[j + 1][1];
			arrayDeCast[2][0] = ArrayControlPointCatmull[j + 2][0];
			arrayDeCast[2][1] = ArrayControlPointCatmull[j + 2][1];
			arrayDeCast[3][0] = ArrayControlPointCatmull[j + 3][0];
			arrayDeCast[3][1] = ArrayControlPointCatmull[j + 3][1];
			float result[2];
			for (int i = 0; i <= 20; i++) {
				deCasteljau((GLfloat)i / 20, result, arrayDeCast, 4);
				CurveArrayCatmull[i + (20 * n_segmento)][0] = result[0];
				CurveArrayCatmull[i + (20 * n_segmento)][1] = result[1];
			}
			n_segmento++;
		}
		glBindVertexArray(VAO_2);
		glBindBuffer(GL_ARRAY_BUFFER, VBO_2);
		glBufferData(GL_ARRAY_BUFFER, sizeof(CurveArrayCatmull), &CurveArrayCatmull[0], GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glLineWidth(0.5);
		glDrawArrays(GL_LINE_STRIP, 0, (NumPts - 1) * 20);
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