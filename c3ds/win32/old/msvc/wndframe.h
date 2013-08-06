/************************************
 * A C++ Glut Wrapper				*
 * Author: Andrea Ingegneri			*
 * Date: 16/12/2001					*
 * Distribution: LGPL (www.gnu.org) *
 ************************************/

// A Glut Wrapper v0.1

class CWindow
{
public:
	CWindow(string Title, int PosX, int PosY, int DimX, int DimY);
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

CWindow::CWindow(string Title, int PosX, int PosY, int DimX, int DimY) : 
				 mTitle(Title), mPosX(PosX), mPosY(PosY), mDimX(DimX), mDimY(DimY)
{
}

void CWindow::Init()
{
 glShadeModel(GL_SMOOTH);     
 glClearColor(0.0f, 0.0f, 0.0f, 0.5f);  
 glClearDepth(1.0f);         
 glEnable(GL_DEPTH_TEST);    
 glDepthFunc(GL_LEQUAL);      
 glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
}

void CWindow::Reshape(int DimX, int DimY)
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

void CWindow::Display()
{
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
 glLoadIdentity();        
 glTranslatef(-1.5f,0.0f,-6.0f);     
 glFlush();
}

void CWindow::Idle()
{
	Display();
}

void CWindow::Keyboard(unsigned char key, int x, int y)
{
}

CWindow *gViewPorts[1];

class CViewControl
{
public:
	CViewControl(string ApplicationName);
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

CViewControl::CViewControl(string ApplicationName) : mCount(0), 
													 mApplicationName(ApplicationName)
{
   char *Argv[1];
   Argv[0] = (char *) mApplicationName.c_str();
   int Argc = 1;

   // Initialize GLUT

   glutInit(&Argc, Argv);
}

void CViewControl::Reshape(int Width, int Height)
{
	int hWnd = glutGetWindow();
	gViewPorts[hWnd]->Reshape(Width, Height);
}

void CViewControl::Display()
{
	int hWnd = glutGetWindow();
	gViewPorts[hWnd]->Display();
}

void CViewControl::Idle()
{
	int hWnd = glutGetWindow();
	gViewPorts[hWnd]->Idle();
}

void CViewControl::Keyboard(unsigned char key, int x, int y)
{
	int hWnd = glutGetWindow();
	gViewPorts[hWnd]->Keyboard(key, x, y);
}

bool CViewControl::AppendWindow(CWindow *Window)
{
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowPosition(Window->GetPosX(), Window->GetPosY());
	int hWnd = glutCreateWindow(Window->GetTitle().c_str());
	gViewPorts[hWnd] = Window;
	glutSetWindow(hWnd);
	Window->Init();
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutIdleFunc(Idle);
	glutKeyboardFunc(Keyboard);
	glutReshapeWindow(Window->GetDimX(), Window->GetDimY());
	mCount++;
	mHandles.push_back(hWnd);
	return true;
}

bool CViewControl::Start()
{
	glutMainLoop();
	return true;
}

bool CViewControl::RedrawAll()
{
	int nHandles = mHandles.size();
	if (nHandles == 0)
		return false;

	for (int i = 0; i < nHandles; i++)
	{
		int hWnd = mHandles[i];
		glutSetWindow(hWnd);
		glutReshapeWindow(gViewPorts[i]->GetDimX(), gViewPorts[i]->GetDimY());
	}
	return true;
}
