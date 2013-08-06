//
// File: AppFrame.h
// Created by: <Andrea Ingegneri>
//

#ifndef _CAPPFRAME_H_
#define _CAPPFRAME_H_

#include "Window.h"
#include "3DModel.h"

class CAppFrame : public CWindow
{
public:
	CAppFrame(string Name, int PosX, int PosY, int DimX, int DimY);
	virtual ~CAppFrame() {}
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

#endif	//_CAPPFRAME_H_

