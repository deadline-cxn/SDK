//
// File: Window.h
// Created by: <Andrea Ingegneri>
//

#ifndef _CWINDOW_H_
#define _CWINDOW_H_

class CWindow
{
public:
	CWindow(string Title, int PosX, int PosY, int DimX, int DimY);
	virtual ~CWindow() {}
	string	GetTitle() const {return mTitle;}
	int		GetDimX() const {return mDimX;}
	int		GetDimY() const {return mDimY;}
	int		GetPosX() const {return mPosX;}
	int		GetPosY() const {return mPosY;}
	virtual void Init();
	virtual	void Reshape(int DimX, int DimY);
	virtual void Display();
	virtual void Idle();
	virtual void Keyboard(unsigned char key, int x, int y);
private:
	string mTitle;
	int	mPosX, mPosY;
	int mDimX, mDimY;
};

#endif	//_CWINDOW_H_

