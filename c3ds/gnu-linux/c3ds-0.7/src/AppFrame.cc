//
// File: AppFrame.h
// Created by: <Andrea Ingegneri>
//

#include <iostream>
#include <vector>
#include <string>
using namespace std;

#include <GL/glut.h>

#include "3DObject.h"
#include "Window.h"
#include "AppFrame.h"

CAppFrame::CAppFrame(string Name, int PosX, int PosY, int DimX, int DimY) : 
					 CWindow(Name, PosX, PosY, DimX, DimY), Degree(0), mScale(100.0f)
{
	ModelloPtr = new C3DModel();
	ModelloPtr->ProcessFile("tpot.3ds");

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
	if (DimY==0)
	{
		DimY=1;
	}

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
			float col = ((float) face) / (float) (nfaces*2);
			glColor3f(col+0.49f, col/2.0f, (col/1.2f)+0.49f);
			glBegin(GL_TRIANGLES);
				glVertex3f( ObjectPtr->vertices[A].x/mScale, ObjectPtr->vertices[A].y/mScale, ObjectPtr->vertices[A].z/mScale);     // Top
				glVertex3f( ObjectPtr->vertices[B].x/mScale, ObjectPtr->vertices[B].y/mScale, ObjectPtr->vertices[B].z/mScale);     // Top
				glVertex3f( ObjectPtr->vertices[C].x/mScale, ObjectPtr->vertices[C].y/mScale, ObjectPtr->vertices[C].z/mScale);     // Top
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
