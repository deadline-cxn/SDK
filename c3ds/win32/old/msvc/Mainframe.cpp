/************************************
 * A C++ Glut Wrapper				*
 * Author: Andrea Ingegneri			*
 * Date: 16/12/2001					*
 * Distribution: LGPL (www.gnu.org) *
 ************************************/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>
using namespace std;

#include <GL/glut.h>

#include "c3ds.h"
#include "wndframe.h"
#include "VECTOR.h"
#include "MATRIX.h"
#include "C3DModel.h"

class CAppFrame : public CWindow
{
public:
	CAppFrame(string Name, int PosX, int PosY, int DimX, int DimY);
	void	Display(void);
	void	Reshape(int DimX, int DimY);
	void	Idle(void);
	void	Init(void);
	void	Keyboard(unsigned char key, int x, int y);
// Application frames
	C3DModel	*ModelloPtr;
private:
	float		mScale;
	float		Degree;
};

CAppFrame::CAppFrame(string Name, int PosX, int PosY, int DimX, int DimY) :
					 CWindow(Name, PosX, PosY, DimX, DimY), Degree(0), mScale(100.0f)
{
	ModelloPtr = new C3DModel();
	ModelloPtr->ProcessFile("tpot.3ds");
	cout << "File Loaded" << endl;
	int nobj = ModelloPtr->GetObjNumber();

	for (int i = 0; i < nobj; i++)
	{
		C3DObject *ObjectPtr;
		ObjectPtr = ModelloPtr->GetObject(i);
		ObjectPtr->UseTrasform();
	}

}

void CAppFrame::Reshape(int DimX, int DimY)
{
	if (DimY==0)	DimY=1;

	glViewport(0, 0, DimX, DimY);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f,(GLfloat)DimX/(GLfloat)DimY,0.1f,100.0f);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void CAppFrame::Display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	glTranslatef(0.0f,0.0f,-6.0f);

	glRotatef(Degree, 1, 1, 2);

	int nobj = ModelloPtr->GetObjNumber();

	for (int i = 0; i < nobj; i++)
	{
		C3DObject *ObjectPtr;
		ObjectPtr = ModelloPtr->GetObject(i);
		int nfaces = ObjectPtr->faces.size();
		for(int face = 0; face < nfaces; face++)
		{
			C3DFace *FacePtr;
			FacePtr = &ObjectPtr->faces[face];
			int A, B, C;
			A = FacePtr->A;
			B = FacePtr->B;
			C = FacePtr->C;
			glBegin(GL_LINES);
				glVertex3f( ObjectPtr->vertices[A].x/mScale, ObjectPtr->vertices[A].y/mScale, ObjectPtr->vertices[A].z/mScale);
				glVertex3f( ObjectPtr->vertices[B].x/mScale, ObjectPtr->vertices[B].y/mScale, ObjectPtr->vertices[B].z/mScale);
			glEnd();
			glBegin(GL_LINES);
				glVertex3f( ObjectPtr->vertices[B].x/mScale, ObjectPtr->vertices[B].y/mScale, ObjectPtr->vertices[B].z/mScale);
				glVertex3f( ObjectPtr->vertices[C].x/mScale, ObjectPtr->vertices[C].y/mScale, ObjectPtr->vertices[C].z/mScale);
			glEnd();
			glBegin(GL_LINES);
				glVertex3f( ObjectPtr->vertices[C].x/mScale, ObjectPtr->vertices[C].y/mScale, ObjectPtr->vertices[C].z/mScale);
				glVertex3f( ObjectPtr->vertices[A].x/mScale, ObjectPtr->vertices[A].y/mScale, ObjectPtr->vertices[A].z/mScale);
			glEnd();
		}
	}

	glutSwapBuffers();
}

void CAppFrame::Idle()
{
	Degree += 0.25f;
	if (Degree >= 360)
		Degree = 0;
	Display();
}

void CAppFrame::Init()
{
	glShadeModel(GL_SMOOTH);
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void CAppFrame::Keyboard(unsigned char key, int x, int y)
{
	if (key == '-')
		mScale -= 1.0f;
	if (key == '+')
		mScale += 1.0f;
}

void main()
{
	CViewControl Control("C3dsView");
	CAppFrame Finestra2("C3ds", 10, 10, 400, 400);
	Control.AppendWindow(&Finestra2);
	Control.Start();
}
