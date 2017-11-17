/*
*
* Demonstrates how to load and display an Wavefront OBJ file.
* Using triangles and normals as static object. No texture mapping.
*
* OBJ files must be triangulated!!!
* Non triangulated objects wont work!
* You can use Blender to triangulate
*
*/

#include <windows.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <GL/glut.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <Windows.h>
#include <MMSystem.h>

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#define KEY_ESCAPE 27

using namespace std;

/************************************************************************
Window
************************************************************************/

typedef struct {
	int width;
	int height;
	char* title;

	float field_of_view_angle;
	float z_near;
	float z_far;
} glutWindow;



/***************************************************************************
OBJ Loading
***************************************************************************/

class Model_OBJ
{
public:
	Model_OBJ();
	float* Model_OBJ::calculateNormal(float* coord1, float* coord2, float* coord3);
	int Model_OBJ::Load(char *filename);	// Loads the model
	void Model_OBJ::Draw();					// Draws the model on the screen
	void Model_OBJ::Release();				// Release the model

	float* normals;							// Stores the normals
	float* Faces_Triangles;					// Stores the triangles
	float* vertexBuffer;					// Stores the points which make the object
	long TotalConnectedPoints;				// Stores the total number of connected verteces
	long TotalConnectedTriangles;			// Stores the total number of connected triangles

};


#define POINTS_PER_VERTEX 3
#define TOTAL_FLOATS_IN_TRIANGLE 9
using namespace std;

Model_OBJ::Model_OBJ()
{
	this->TotalConnectedTriangles = 0;
	this->TotalConnectedPoints = 0;
}

float* Model_OBJ::calculateNormal(float *coord1, float *coord2, float *coord3)
{
	/* calculate Vector1 and Vector2 */
	float va[3], vb[3], vr[3], val;
	va[0] = coord1[0] - coord2[0];
	va[1] = coord1[1] - coord2[1];
	va[2] = coord1[2] - coord2[2];

	vb[0] = coord1[0] - coord3[0];
	vb[1] = coord1[1] - coord3[1];
	vb[2] = coord1[2] - coord3[2];

	/* cross product */
	vr[0] = va[1] * vb[2] - vb[1] * va[2];
	vr[1] = vb[0] * va[2] - va[0] * vb[2];
	vr[2] = va[0] * vb[1] - vb[0] * va[1];

	/* normalization factor */
	val = sqrt(vr[0] * vr[0] + vr[1] * vr[1] + vr[2] * vr[2]);

	float norm[3];
	norm[0] = vr[0] / val;
	norm[1] = vr[1] / val;
	norm[2] = vr[2] / val;


	return norm;
}


int Model_OBJ::Load(char* filename)
{
	string line;
	ifstream objFile(filename);
	if (objFile.is_open())													// If obj file is open, continue
	{
		objFile.seekg(0, ios::end);										// Go to end of the file, 
		long fileSize = objFile.tellg();									// get file size
		objFile.seekg(0, ios::beg);										// we'll use this to register memory for our 3d model
		float* normalBuffer;
		vertexBuffer = (float*)malloc(fileSize);							// Allocate memory for the verteces
		normalBuffer = (float*)malloc(fileSize);							// Allocate memory for the verteces
		Faces_Triangles = (float*)malloc(fileSize * sizeof(float));			// Allocate memory for the triangles
		normals = (float*)malloc(fileSize * sizeof(float));					// Allocate memory for the normals

		int triangle_index = 0;												// Set triangle index to zero
		int normal_index = 0;												// Set normal index to zero
		int i = 0;
		//for (int i = 0; i < 10; i++)
		while (!objFile.eof())											// Start reading file data
		{
			getline(objFile, line);											// Get line from file
			if (line.c_str()[0] == 'v'&&line.c_str()[1] == 'n')										// The first character is a v: on this line is a vertex stored.
			{
				line[0] = ' ';												// Set first character to 0. This will allow us to use sscanf
				line[1] = ' ';
				//cout << "VN " << line << endl;
				sscanf_s(line.c_str(), "%f %f %f ",							// Read floats from the line: v X Y Z
					&normalBuffer[TotalConnectedPoints],
					&normalBuffer[TotalConnectedPoints + 1],
					&normalBuffer[TotalConnectedPoints + 2]);

				//TotalConnectedPoints += POINTS_PER_VERTEX;					// Add 3 to the total connected points
			}
			else if (line.c_str()[0] == 'v')										// The first character is a v: on this line is a vertex stored.
			{
				line[0] = ' ';												// Set first character to 0. This will allow us to use sscanf
																			//cout << "V " << line << endl;
				sscanf_s(line.c_str(), "%f %f %f ",							// Read floats from the line: v X Y Z
					&vertexBuffer[TotalConnectedPoints],
					&vertexBuffer[TotalConnectedPoints + 1],
					&vertexBuffer[TotalConnectedPoints + 2]);

				TotalConnectedPoints += POINTS_PER_VERTEX;					// Add 3 to the total connected points
			}
			else if (line.c_str()[0] == 'f')										// The first character is an 'f': on this line is a point stored
			{
				//cout << "F " << line << endl;
				line[0] = ' ';												// Set first character to 0. This will allow us to use sscanf

				int vertexNumber[4] = { 0, 0, 0 };
				int normalNumber[4] = { 0,0,0 };
				sscanf_s(line.c_str(), "%i%i%i%i%i%i",						// Read integers from the line:  f 1 2 3
					&vertexNumber[0], &normalNumber[0],									// First point of our triangle. This is an 
					&vertexNumber[1], &normalNumber[1],									// pointer to our vertexBuffer list
					&vertexNumber[2], &normalNumber[2]);										// each point represents an X,Y,Z.

				vertexNumber[0] -= 1;										// OBJ file starts counting from 1
				vertexNumber[1] -= 1;										// OBJ file starts counting from 1
				vertexNumber[2] -= 1;										// OBJ file starts counting from 1
				normalNumber[0] -= 1;
				normalNumber[1] -= 1;
				normalNumber[2] -= 1;

				/********************************************************************
				* Create triangles (f 1 2 3) from points: (v X Y Z) (v X Y Z) (v X Y Z).
				* The vertexBuffer contains all verteces
				* The triangles will be created using the verteces we read previously
				*/

				int tCounter = 0;
				for (int i = 0; i < POINTS_PER_VERTEX; i++)
				{
					Faces_Triangles[triangle_index + tCounter] = vertexBuffer[3 * vertexNumber[i]];
					Faces_Triangles[triangle_index + tCounter + 1] = vertexBuffer[3 * vertexNumber[i] + 1];
					Faces_Triangles[triangle_index + tCounter + 2] = vertexBuffer[3 * vertexNumber[i] + 2];
					tCounter += POINTS_PER_VERTEX;
				}

				/*********************************************************************
				* Calculate all normals, used for lighting
				*/
				float coord1[3] = { Faces_Triangles[triangle_index], Faces_Triangles[triangle_index + 1],Faces_Triangles[triangle_index + 2] };
				float coord2[3] = { Faces_Triangles[triangle_index + 3],Faces_Triangles[triangle_index + 4],Faces_Triangles[triangle_index + 5] };
				float coord3[3] = { Faces_Triangles[triangle_index + 6],Faces_Triangles[triangle_index + 7],Faces_Triangles[triangle_index + 8] };
				float *norm = this->calculateNormal(coord1, coord2, coord3);

				tCounter = 0;
				for (int i = 0; i < POINTS_PER_VERTEX; i++)
				{
					normals[normal_index + tCounter] = norm[0];
					normals[normal_index + tCounter + 1] = norm[1];
					normals[normal_index + tCounter + 2] = norm[2];
					tCounter += POINTS_PER_VERTEX;
				}
				//render VN mode
				/*for (int i = 0; i < POINTS_PER_VERTEX; i++)
				{
				normals[normal_index + tCounter] = normalBuffer[3 * normalNumber[i]];
				normals[normal_index + tCounter + 1] = normalBuffer[3 * normalNumber[i] + 1];
				normals[normal_index + tCounter + 2] = normalBuffer[3 * normalNumber[i] + 2];
				tCounter += POINTS_PER_VERTEX;
				}*/

				triangle_index += TOTAL_FLOATS_IN_TRIANGLE;
				normal_index += TOTAL_FLOATS_IN_TRIANGLE;
				TotalConnectedTriangles += TOTAL_FLOATS_IN_TRIANGLE;
			}
		}
		objFile.close();														// Close OBJ file
	}
	else
	{
		cout << "Unable to open file";
	}
	return 0;
}

void Model_OBJ::Release()
{
	free(this->Faces_Triangles);
	free(this->normals);
	free(this->vertexBuffer);
}

void Model_OBJ::Draw()
{
	glEnableClientState(GL_VERTEX_ARRAY);						// Enable vertex arrays
	glEnableClientState(GL_NORMAL_ARRAY);						// Enable normal arrays
	glVertexPointer(3, GL_FLOAT, 0, Faces_Triangles);				// Vertex Pointer to triangle array
	glNormalPointer(GL_FLOAT, 0, normals);						// Normal pointer to normal array
	glDrawArrays(GL_TRIANGLES, 0, TotalConnectedTriangles);		// Draw the triangles
	glDisableClientState(GL_VERTEX_ARRAY);						// Disable vertex arrays
	glDisableClientState(GL_NORMAL_ARRAY);						// Disable normal arrays
}

/***************************************************************************
* Program code
***************************************************************************/

//OBJ MODEL
Model_OBJ obj;
Model_OBJ obj2;
Model_OBJ obj3;
Model_OBJ obj4;
Model_OBJ obj5;
Model_OBJ obj6;
Model_OBJ obj7;
Model_OBJ obj8;
Model_OBJ obj9;
Model_OBJ obj10;
Model_OBJ obj11;
Model_OBJ obj12;
Model_OBJ obj13;
Model_OBJ obj14;
Model_OBJ obj15;
Model_OBJ container;

//BOARD POSITION & ROTATION
vector<int> board = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0 };
float g_rotation = 0.0f;
float g_rotation2 = 10.0f;
float g_zoom = 1.0f;

//TILES POSITION
float tx[16] = { 0,0,10,20,30,0,10,20,30,0,10,20,30,0,10,20 };
float ty[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
float tz[16] = { 0,0,0,0,0,10,10,10,10,20,20,20,20,30,30,30 };

//FINISH BOARD
vector<int> fboard = { 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0 };

//FINISH TILES
float fx[16] = { 0,0,10,20,30,0,10,20,30,0,10,20,30,0,10,20 };
float fy[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
float fz[16] = { 0,0,0,0,0,10,10,10,10,20,20,20,20,30,30,30 };

glutWindow win;
int i;

//TILE POSTION TO MOVE
int movePos;

//START ?
bool isStart = false;

//POSITION OF ZERO
int posZero = 15;

//NOT VALID FUNCTION
int notvalid1[4] = { 0,12,0,3 };
int notvalid2[4] = { 1,13,4,7 };
int notvalid3[4] = { 2,14,8,11 };
int notvalid4[4] = { 3,15,12,15 };

//LIGHTING ENABLER
bool light = true;

/*int notvalid1[4] = { 0,2,0,1 };
int notvalid2[4] = { 1,3,2,3 };*/

void handleResize(int w, int h) {
	//CONVERT COORDINATES TO PIXEL VALUES
	glViewport(0, 0, w, h);

	//CAMERA PERSPECTIVE
	glMatrixMode(GL_PROJECTION);

	//RESET CAMERA
	glLoadIdentity();
	gluPerspective(45.0, (double)w / (double)h, win.z_near, win.z_far);
}

bool isFinish() {
	bool finish = false;
	for (int i = 0; i < 16; i++) {
		if (board[i] == fboard[i]) {
			finish = true;
		}
		else {
			finish = false;
			return false;
		}
	}
	return true;
}

//MOVE BLANK
void moveBoard(int d) {
	//BLANK UP
	if (d == 0)
		i = posZero - 4;
	//BLANK DOWN
	if (d == 1)
		i = posZero + 4;
	//BLANK LEFT
	if (d == 2)
		i = posZero - 1;
	//BLANK RIGHT
	if (d == 3)
		i = posZero + 1;

	movePos = board[i];

	//SWAP POSITION BETWEEN EMPTY AND TILE
	swap(board[posZero], board[i]);
	posZero = i;

	//DEBUGING
	/*for (int i : board)
	cout << i << " ";
	cout << endl;*/
}



void handleKeypress(unsigned char key, int x, int y)
{
	switch (key) {
	case KEY_ESCAPE:
		exit(0);
		break;
		//REVERSE DIRECTION IN ORDER TO USE MOVE BLANKS METHOD

		//MOVE BLANK RIGHT
	case 'a':
		if (isStart) {
			if (!(posZero == notvalid1[3] || posZero == notvalid2[3] || posZero == notvalid3[3] || posZero == notvalid4[3])) {
				moveBoard(3);
				tx[movePos] -= 10;
			}
		}
		break;

		//MOVE BLANK DOWN
	case 'w':
		if (isStart) {
			if (!(posZero == notvalid1[1] || posZero == notvalid2[1] || posZero == notvalid3[1] || posZero == notvalid4[1])) {
				moveBoard(1);
				tz[movePos] -= 10;
			}
		}
		break;

		//MOVE BLANK LEFT
	case 'd':
		if (isStart) {
			if (!(posZero == notvalid1[2] || posZero == notvalid2[2] || posZero == notvalid3[2] || posZero == notvalid4[2])) {
				moveBoard(2);
				tx[movePos] += 10;
			}
		}
		break;

		//MOVE BLANK UP
	case 's':
		if (isStart) {
			if (!(posZero == notvalid1[0] || posZero == notvalid2[0] || posZero == notvalid3[0] || posZero == notvalid4[0])) {
				moveBoard(0);
				tz[movePos] += 10;
			}
		}

		break;

		//ROTATE Y AXIS CW
	case 'e':
		g_rotation -= 10;
		break;

		//ROTATE Y AXIS CCW
	case 'r':
		g_rotation += 10;
		break;

		//ROTATE X AXIS CW
	case 't':
		g_rotation2 += 10;
		break;

		//ROTATE X AXIS CCW
	case 'y':
		g_rotation2 -= 10;
		break;

		//ZOOM IN
	case 'z':
		g_zoom += 10.0f;
		break;

		//ZOOM OUT
	case 'x':
		g_zoom -= 10.0f;
		break;

		//RESET VIEW
	case 'f':
		g_zoom = 0.0f;
		g_rotation = 0.0f;
		g_rotation2 = 10.0f;
		break;

		//LIGHTING
	case 'l':
		if (light)
			glDisable(GL_LIGHTING);
		else
			glEnable(GL_LIGHTING);
		light = !light;
		break;

		//SOLVE
	case 'g':
		isStart = false;
		for (int i = 1; i <= 16; i++) {
			tx[i] = fx[i];
			ty[i] = fy[i];
			tz[i] = fz[i];
		}
		board = fboard;
		posZero = 15;

		break;

		//SHUFFLE
	case 'q':
		isStart = true;
		srand(time(NULL));
		for (int i = 0; i < 1000; i++) {
			int tran = rand() % 4;
			if (!(posZero == notvalid1[tran] || posZero == notvalid2[tran] || posZero == notvalid3[tran] || posZero == notvalid4[tran])) {
				moveBoard(tran);
				if (tran == 0) tz[movePos] += 10;
				else if (tran == 1) tz[movePos] -= 10;
				else if (tran == 2) tx[movePos] += 10;
				else tx[movePos] -= 10;
			}
		}
		break;

	default:
		break;
	}
}

void initialize()
{
	//MAKE 3D DRAWING WORK	
	glEnable(GL_DEPTH_TEST);

	//BACKGROUND COLOR
	glClearColor(0.0f, 1.0f, 1.0f, 0.3f);

	//LIGHTNING ARRAYS WITH ENABLER

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glEnable(GL_LIGHT2);

	GLfloat amb_light[] = { 0.7, 0.7, 0.7, 1.0 };

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, amb_light);

	//Add positioned light
	GLfloat lightColor0[] = { 0.5f, 0.5f, 0.5f, 1.0f }; //Color (0.5, 0.5, 0.5)
	GLfloat lightPos0[] = { 4.0f, 0.0f, 8.0f, -100.0f }; //Positioned at (4, 0, 8)
	glLightfv(GL_LIGHT1, GL_DIFFUSE, lightColor0);
	glLightfv(GL_LIGHT1, GL_POSITION, lightPos0);

	//Add directed light
	GLfloat lightColor1[] = { 0.5f, 0.2f, 0.2f, 1.0f }; //Color (0.5, 0.2, 0.2)
														//Coming from the direction (-1, 0.5, 0.5)
	GLfloat lightPos1[] = { -1.0f, 0.5f, 0.5f, -100.0f };
	glLightfv(GL_LIGHT2, GL_DIFFUSE, lightColor1);
	glLightfv(GL_LIGHT2, GL_POSITION, lightPos1);

	//SMOOTH SHADING
	glShadeModel(GL_SMOOTH);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

	//ENABLE COLORING
	glEnable(GL_COLOR_MATERIAL);

}

void drawScene()
{
	//CLEAR INFORMATION OF LAST DRAW
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//SWITCH TO DRAWING PERSPECTIVE
	glMatrixMode(GL_MODELVIEW);

	//RESET DRAWING PERSPECTIVE
	glLoadIdentity();

	//ZOOM IN/OUT BY KEY : Z (IN), KEY : X (OUT)
	glTranslatef(0, 0, g_zoom);

	//SET INITIAL POSITION
	glTranslatef(-15.0f, 0.0f, -100.0f);
	gluLookAt(0, 1, 4, 0, 0, 0, 0, 1, 0);
	//glRotatef(g_rotation, 0, 1, 0);

	//ROTATE BY Y AXIS KEY : R
	glTranslatef(15.0f, 0.0f, 15.0f);
	glRotatef(g_rotation, 0, 1, 0);
	glTranslatef(-15.0f, 0.0f, -15.0f);

	//ROTATE BY X AXIS KEY : T
	glTranslatef(0.0f, 0.0f, 15.0f);
	glRotatef(g_rotation2, 1, 0, 0);
	glTranslatef(0.0f, 0.0f, -15.0f);

	/*
	TRANSLATE BY EACH POSITION OF BLOCK i IN tx[i] ty[i] tz[i]
	ROTATE IN X-AXIS 90 DEGREE CCW
	SCALE 4 TIMES
	*/

	string x = "15 PUZZLE";
	for (int i = 0; i < x.length(); i++) {
		glPushMatrix();
		glTranslatef(-25 + i * 10, 10, -100);
		glScalef(0.1, 0.1, 0.1);
		glColor3f(0, 0, 0);
		glutStrokeCharacter(GLUT_STROKE_ROMAN, x[i]);
		glPopMatrix();
	}

	x = "BY CU CPR";
	for (int i = 0; i < x.length(); i++) {
		glPushMatrix();
		glTranslatef(40 + i * 3, 5, -100);
		glScalef(0.02, 0.02, 0.02);
		glColor3f(0, 0, 0);
		glutStrokeCharacter(GLUT_STROKE_ROMAN, x[i]);
		glPopMatrix();
	}

	x = "PRESS Q TO START";
	if (!isStart) {
		for (int i = 0; i < x.length(); i++) {
			glPushMatrix();
			glTranslatef(-25 + i * 5, -10, -100);
			glScalef(0.05, 0.05, 0.05);
			glColor3f(0, 0, 0);
			glutStrokeCharacter(GLUT_STROKE_ROMAN, x[i]);
			glPopMatrix();
		}
	}

	//BLOCK 1 Draw
	glPushMatrix();
	glColor3f(0.6f, 0.0f, 0.0f);
	glTranslatef(tx[1], ty[1], tz[1]);
	glRotatef(-90.0f, 1, 0, 0);
	glScalef(4.0f, 4.0f, 4.0f);
	obj.Draw();
	glPopMatrix();

	//BLOCK 2 Draw
	glPushMatrix();
	glColor3f(0.7f, 0.0f, 0.0f);
	glTranslatef(tx[2], ty[2], tz[2]);
	glRotatef(-90.0f, 1, 0, 0);
	glScalef(4.0f, 4.0f, 4.0f);
	obj2.Draw();
	glPopMatrix();

	//BLOCK 3 Draw
	glPushMatrix();
	glColor3f(0.8f, 0.0f, 0.0f);
	glTranslatef(tx[3], ty[3], tz[3]);
	glRotatef(-90.0f, 1, 0, 0);
	glScalef(4.0f, 4.0f, 4.0f);
	obj3.Draw();
	glPopMatrix();

	//BLOCK 4 Draw
	glPushMatrix();
	glColor3f(1.0f, 0.0f, 0.0f);
	glTranslatef(tx[4], ty[4], tz[4]);
	glRotatef(-90.0f, 1, 0, 0);
	glScalef(4.0f, 4.0f, 4.0f);
	obj4.Draw();
	glPopMatrix();

	//BLOCK 5 Draw
	glPushMatrix();
	glColor3f(0.6f, 0.6f, 0.0f);
	glTranslatef(tx[5], ty[5], tz[5]);
	glRotatef(-90.0f, 1, 0, 0);
	glScalef(4.0f, 4.0f, 4.0f);
	obj5.Draw();
	glPopMatrix();

	//BLOCK 6 Draw
	glPushMatrix();
	glColor3f(0.7f, 0.7f, 0.0f);
	glTranslatef(tx[6], ty[6], tz[6]);
	glRotatef(-90.0f, 1, 0, 0);
	glScalef(4.0f, 4.0f, 4.0f);
	obj6.Draw();
	glPopMatrix();

	//BLOCK 7 Draw
	glPushMatrix();
	glColor3f(0.8f, 0.8f, 0.0f);
	glTranslatef(tx[7], ty[7], tz[7]);
	glRotatef(-90.0f, 1, 0, 0);
	glScalef(4.0f, 4.0f, 4.0f);
	obj7.Draw();
	glPopMatrix();

	//BLOCK 8 Draw
	glPushMatrix();
	glColor3f(1.0f, 1.0f, 0.0f);
	glTranslatef(tx[8], ty[8], tz[8]);
	glRotatef(-90.0f, 1, 0, 0);
	glScalef(4.0f, 4.0f, 4.0f);
	obj8.Draw();
	glPopMatrix();

	//BLOCK 9 Draw
	glPushMatrix();
	glColor3f(1.0f, 0.6f, 0.0f);
	glTranslatef(tx[9], ty[9], tz[9]);
	glRotatef(-90.0f, 1, 0, 0);
	glScalef(4.0f, 4.0f, 4.0f);
	obj9.Draw();
	glPopMatrix();

	//BLOCK 10 Draw
	glPushMatrix();
	glColor3f(1.0f, 0.5f, 0.0f);
	glTranslatef(tx[10], ty[10], tz[10]);
	glRotatef(-90.0f, 1, 0, 0);
	glScalef(4.0f, 4.0f, 4.0f);
	obj10.Draw();
	glPopMatrix();

	//BLOCK 11 Draw
	glPushMatrix();
	glColor3f(1.0f, 0.4f, 0.0f);
	glTranslatef(tx[11], ty[11], tz[11]);
	glRotatef(-90.0f, 1, 0, 0);
	glScalef(4.0f, 4.0f, 4.0f);
	obj11.Draw();
	glPopMatrix();

	//BLOCK 12 Draw
	glPushMatrix();
	glColor3f(1.0f, 0.3f, 0.0f);
	glTranslatef(tx[12], ty[12], tz[12]);
	glRotatef(-90.0f, 1, 0, 0);
	glScalef(4.0f, 4.0f, 4.0f);
	obj12.Draw();
	glPopMatrix();

	//BLOCK 13 Draw
	glPushMatrix();
	glColor3f(0.0f, 0.6f, 0.0f);
	glTranslatef(tx[13], ty[13], tz[13]);
	glRotatef(-90.0f, 1, 0, 0);
	glScalef(4.0f, 4.0f, 4.0f);
	obj13.Draw();
	glPopMatrix();

	//BLOCK 14 Draw
	glPushMatrix();
	glColor3f(0.0f, 0.8f, 0.0f);
	glTranslatef(tx[14], ty[14], tz[14]);
	glRotatef(-90.0f, 1, 0, 0);
	glScalef(4.0f, 4.0f, 4.0f);
	obj14.Draw();
	glPopMatrix();

	//BLOCK 15 Draw
	glPushMatrix();
	glColor3f(0.0f, 1.0f, 0.0f);
	glTranslatef(tx[15], ty[15], tz[15]);
	glRotatef(-90.0f, 1, 0, 0);
	glScalef(4.0f, 4.0f, 4.0f);
	obj15.Draw();
	glPopMatrix();

	//CONTAINER Draw
	glPushMatrix();
	glTranslatef(11.5f, -6.5f, 15.0f);
	glColor3f(0.54f, 0.27f, 0.07f);
	glScalef(4.0f, 4.0f, 4.0f);
	container.Draw();
	glPopMatrix();

	glutSwapBuffers();

	if (isFinish() && isStart) {
		isStart = false;
		PlaySound(TEXT("./sound/aa.wav"), NULL, SND_SYNC);
		PlaySound(TEXT("./sound/a.wav"), NULL, SND_ASYNC);
	}
}

int main(int argc, char **argv)
{
	// set window values
	win.width = 640;
	win.height = 480;
	win.title = "CUCPRCG OpenGL/GLUT OBJ Loader.";
	win.field_of_view_angle = 45;
	win.z_near = 1.0f;
	win.z_far = 500.0f;

	// initialize and run program
	glutInit(&argc, argv);                                      // GLUT initialization
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);  // Display Mode
	glutInitWindowSize(win.width, win.height);					// set window size
	glutCreateWindow(win.title);								// create Window
	initialize();
	obj.Load("./obj/block1.obj");
	obj2.Load("./obj/block2.obj");
	obj3.Load("./obj/block3.obj");
	obj4.Load("./obj/block4.obj");
	obj5.Load("./obj/block5.obj");
	obj6.Load("./obj/block6.obj");
	obj7.Load("./obj/block7.obj");
	obj8.Load("./obj/block8.obj");
	obj9.Load("./obj/block9.obj");
	obj10.Load("./obj/block10.obj");
	obj11.Load("./obj/block11.obj");
	obj12.Load("./obj/block12.obj");
	obj13.Load("./obj/block13.obj");
	obj14.Load("./obj/block14.obj");
	obj15.Load("./obj/block15.obj");
	container.Load("./obj/container.obj");

	PlaySound(TEXT("./sound/a.wav"), NULL, SND_ASYNC);

	string startText = "Press Q to start\nPress E | R to rotate Y axis cw/ccw\nPress T | Y to rotate X axis cw/ccw\nPress F to reset view\nPress G to Reset board\nPress Z | X to zoom in/out\n";
	cout << startText << endl;

	glutDisplayFunc(drawScene);									// register Display Function
	glutIdleFunc(drawScene);									// register Idle Function
	glutKeyboardFunc(handleKeypress);							// register Keyboard Handler
	glutReshapeFunc(handleResize);

	glutMainLoop();												// run GLUT mainloop
	return 0;
}