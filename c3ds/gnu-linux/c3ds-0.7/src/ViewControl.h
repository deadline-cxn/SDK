//
// File: ViewControl.h
// Created by: <Andrea Ingegneri>
//

#ifndef _CVIEWCONTROL_H_
#define _CVIEWCONTROL_H_

class CWindow;

class CViewControl
{
public:
	CViewControl(string ApplicationName);
	virtual ~CViewControl() {}
	static	void		Reshape(int Width, int Height);
	static	void		Display();
	static	void		Idle();
	static	void		Keyboard(unsigned char key, int x, int y);
	virtual	bool		AppendWindow(CWindow *Window);
	virtual	bool		Start();
	virtual	bool		RedrawAll();
private:
	int					mCount;							// Number of windows
	const string		mApplicationName;
	vector		<int>	mHandles;
};

#endif	//_CVIEWCONTROL_H_

