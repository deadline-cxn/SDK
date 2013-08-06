/************************************
 * 3ds C++ File Loader				*
 * Author: Andrea Ingegneri			*
 * Date: 16/12/2001					*
 * Distribution: LGPL (www.gnu.org) *
 ************************************/


char gVersion[] = "(c) Andrea Ingegneri 2001";

class C3dsFileFormat
{
public:
	C3dsFileFormat() {}
protected:
#pragma pack(2)
	typedef struct _chunk3ds
	{
		unsigned short	Flag;
		long			Size;
	} chunk3ds;
#pragma pack()

typedef struct _Transform3dsMatrix
{
	float _11, _12, _13;
	float _21, _22, _23;
	float _31, _32, _33;
} Transform3dsMatrix;

typedef struct _Translate3dsMatrix
{
	float _11, _12, _13;
} Translate3dsMatrix;

class ViewPortLayout
{
public:
	ViewPortLayout(unsigned short Style, short Active, short Unknow1, short Swap, 
				   short Unknow2, short Swap_prior, short Swap_view) : style(Style),
																	   active(Active),
																	   unknow1(Unknow1),
																	   swap(Swap),
																	   unknow2(Unknow2),
																	   swap_prior(Swap_prior),
																	   swap_view(Swap_view) {}
	unsigned short	style;
    short			active;
	short			unknow1;
    short			swap;
	short			unknow2;
    short			swap_prior;
    short			swap_view;
};

enum {CameraNameSize = 11};

class ViewPortData
{
public:
	ViewPortData(short Flags, short Axis_lockout, short Win_x, short Win_y, short Win_w, short Win_h,
				 short Win_view, float Zoom, float Worldcenter_x, float Worldcenter_y, float Worldcenter_z,
				 float Horiz_ang, float Vert_ang, char *CameraName) : flags(Flags), axis_lockout(Axis_lockout),
																	  win_x(Win_x), win_y(Win_y),
																	  win_w(Win_w), win_h(Win_h),
																	  win_view(Win_view), zoom(Zoom),
																	  worldcenter_x(Worldcenter_x),
																	  worldcenter_y(Worldcenter_y),
																	  worldcenter_z(Worldcenter_z),
																	  horiz_ang(Horiz_ang), vert_ang(Vert_ang),
																	  camera_name(CameraName) {}
	short	flags, axis_lockout;
	short	win_x, win_y, win_w, win_h, win_view;
	float	zoom; 
	float	worldcenter_x, worldcenter_y, worldcenter_z;
	float	horiz_ang, vert_ang;
	string	camera_name;
};

enum {
	CHUNK_VERSION	= 0x0002,
    CHUNK_RGBF      = 0x0010,
    CHUNK_RGBB      = 0x0011,

	CHUNK_PERCENTW	= 0x0030,
	CHUNK_PERCENTF	= 0x0031,

    CHUNK_PRJ       = 0xC23D,
    CHUNK_MLI       = 0x3DAA,

    CHUNK_MAIN      = 0x4D4D,
        CHUNK_OBJMESH   = 0x3D3D,
			CHUNK_ONEUNIT	= 0x0100,
            CHUNK_BKGCOLOR  = 0x1200,
            CHUNK_AMBCOLOR  = 0x2100,
			CHUNK_DEFAULT_VIEW = 0x3000,
				CHUNK_VIEW_TOP = 0x3010,
				CHUNK_VIEW_BOTTOM = 0x3020,
				CHUNK_VIEW_LEFT = 0x3030,
				CHUNK_VIEW_RIGHT = 0x3040,
				CHUNK_VIEW_FRONT = 0x3050,
				CHUNK_VIEW_BACK = 0x3060,
				CHUNK_VIEW_USER = 0x3070,
				CHUNK_VIEW_CAMERA = 0x3080,
            CHUNK_OBJBLOCK  = 0x4000,
                CHUNK_TRIMESH   = 0x4100,
                    CHUNK_VERTLIST  = 0x4110,
                    CHUNK_VERTFLAGS = 0x4111,
                    CHUNK_FACELIST  = 0x4120,
                    CHUNK_FACEMAT   = 0x4130,
                    CHUNK_MAPLIST   = 0x4140,
                    CHUNK_SMOOLIST  = 0x4150,
                    CHUNK_TRMATRIX  = 0x4160,
                    CHUNK_MESHCOLOR = 0x4165,
                    CHUNK_TXTINFO   = 0x4170,
                CHUNK_LIGHT     = 0x4600,
                    CHUNK_SPOTLIGHT = 0x4610,
                CHUNK_CAMERA    = 0x4700,
                CHUNK_HIERARCHY = 0x4F00,

        CHUNK_VIEWPORT_LAYOUT_OLD	= 0x7000,
			CHUNK_VIEWPORT_DATA_OLD	= 0x7010,
				CHUNK_VIEWPORT_SIZE = 0x7020,
					CHUNK_NETWORK_VIEW = 0X7030,

        CHUNK_VIEWPORT_LAYOUT	= 0x7001,
			CHUNK_VIEWPORT_DATA	= 0x7011,
			CHUNK_VIEWPORT_DATA3 = 0x7012,

		CHUNK_MATERIAL  = 0xAFFF,
            CHUNK_MATNAME   = 0xA000,
            CHUNK_AMBIENT   = 0xA010,
            CHUNK_DIFFUSE   = 0xA020,
            CHUNK_SPECULAR  = 0xA030,
            CHUNK_TEXTURE   = 0xA200,
            CHUNK_BUMPMAP   = 0xA230,
            CHUNK_MAPFILE   = 0xA300,
        CHUNK_KEYFRAMER = 0xB000,
            CHUNK_AMBIENTKEY    = 0xB001,
            CHUNK_TRACKINFO = 0xB002,
                CHUNK_TRACKOBJNAME  = 0xB010,
                CHUNK_TRACKPIVOT    = 0xB013,
                CHUNK_TRACKPOS      = 0xB020,
                CHUNK_TRACKROTATE   = 0xB021,
                CHUNK_TRACKSCALE    = 0xB022,
                CHUNK_TRACKMORPH    = 0xB026,
                CHUNK_TRACKHIDE     = 0xB029,
                CHUNK_OBJNUMBER     = 0xB030,
            CHUNK_TRACKCAMERA = 0xB003,
                CHUNK_TRACKFOV  = 0xB023,
                CHUNK_TRACKROLL = 0xB024,
            CHUNK_TRACKCAMTGT = 0xB004,
            CHUNK_TRACKLIGHT  = 0xB005,
            CHUNK_TRACKLIGTGT = 0xB006,
            CHUNK_TRACKSPOTL  = 0xB007,
            CHUNK_FRAMES    = 0xB008,
	};
};

class C3dsFileLoader : public C3dsFileFormat
{
public:
// Constructor
	C3dsFileLoader() : C3dsFileFormat(), PC(0), mFileSize(0) {}
// Methods
	void		ProcessFile(string FileName);
private:
	void		movePC(long forward);
	bool		ParseChunks();
	bool		ReadBlock(char *dest, long size);
	float		ReadRealNumber();
	bool		SkipChunk(chunk3ds &chunk);
	void		FileSize();
	bool		isOk();
	void		ReadASCIIZ(char *dest);
	void		ReadRGBF();
	void		ReadRGBB();
	void		ReadObjBlock();
	void		ReadVertList();
	void		ReadFaceList();
	void		ReadFaceMat();
	void		ReadMapList();
	void		ReadSmoothList();
	void		ReadTrMatrix();
	void		ReadLight();
	void		ReadSpotLight();
	void		ReadCamera();
	void		ReadMatName();
	void		ReadMapFile();
	void		ReadFrames();
	void		ReadTrackObjName();
	void		ReadTrackPos();
	void		ReadTrackRot();
	void		ReadTrackScale();
	void		ReadObjNumber();
	void		ReadObjChunk();
	void		ReadOneUnit();
	void		ReadViewPortData();
	void		ReadViewUser();
	void		ReadViewTop();
	void		ReadViewBottom();
	void		ReadViewLeft();
	void		ReadViewRight();
	void		ReadViewFront();
	void		ReadViewBack();
	void		ReadViewCamera();
	void		ReadViewPortLayout();
	void		ReadViewPortSize();
	void		EndReading();
	
	ifstream	mInputFile;
protected:
	long		PC;		// File Pointer
	long		mFileSize;
// Inheritable functions for polymorphism
	virtual void		User3dVert(float x, float y, float z);
	virtual void		User3dFace(unsigned short A, unsigned short B, unsigned short C, unsigned short Flags);
	virtual void		UserCamera(float px, float py, float pz,
								   float tx, float ty, float tz,
								   float Bank, float Lens);
	virtual void		UserFaceMaterial(string Name, int Number);
	virtual	void		UserMapVertex(float U, float V);
	virtual void		UserTransformMatrix(const Transform3dsMatrix &Transform, const Translate3dsMatrix &Translate);
	virtual void		UserLight(float x, float y, float z);
	virtual void		UserSpotLight(float x, float y, float z, float HotSpot, float FallOff);
	virtual void		UserMatName(const string Name);
	virtual void		UserMapFile(const string FileName);
	virtual void		UserFrames(long Start, long End);
	virtual void		UserTrackObjectName(const string Name, int Key1, int Key2, int Key3);
	virtual void		UserTrackPos(int Frame, long Flags, float x, float y, float z);
	virtual void		UserTrackRot(int Frame, long Flags, float DegX, float DegY, float DegZ);
	virtual void		UserTrackScale(int Frame, long Flags, float ScaleX, float ScaleY, float ScaleZ);
	virtual void		UserObjNumber(int ObjNumber);
	virtual void		UserObjName(string Name);
	virtual void		UserChunkObj();
	virtual void		UserOneUnit(float Unit);
	virtual void		UserViewPortLayout(ViewPortLayout &Layout);
	virtual void		UserViewPortSize(int PosX, int PosY, int Width, int Height);
	virtual void		UserViewPortData(ViewPortData &Data);
	virtual	void		UserViewUser(float x, float y, float z, float Width, float XYangle, float YZangle, float BackAngle);
	virtual void		UserViewCamera(string CameraName);
	virtual void		UserEndOfFile();
};

void C3dsFileLoader::UserObjName(string Name)
{
	cout << "Object Name: " << Name << endl;
}

void C3dsFileLoader::User3dVert(float x, float y, float z)
{
	cout << "X: " << x << " Y: " << y << " Z:" << z << endl;
}

void C3dsFileLoader::User3dFace(unsigned short A, unsigned short B, unsigned short C, unsigned short Flags)
{
	cout << "A: " << A << " B: " << B << " C:" << C << endl;
	cout << " Flags: " << Flags << endl;
}

void C3dsFileLoader::UserCamera(float px, float py, float pz,
								float tx, float ty, float tz,
								float Bank, float Lens)
{
	cout << "Position: X:" << px << "Y: " << py << "Z: " << pz << endl;
	cout << "Target: X:" << tx << "Y: " << ty << "Z: " << tz << endl;
	cout << "Bank: " << Bank << endl;
	cout << "Lens: " << Lens << endl;
}

void C3dsFileLoader::UserFaceMaterial(string Name, int Number)
{
	cout << "Name: " << Name << endl;
	cout << "Face Number: " << Number << endl;
}

void C3dsFileLoader::UserMapVertex(float U, float V)
{
	cout << "U: " << U << " V: " << V << endl;
}

void C3dsFileLoader::UserTransformMatrix(const Transform3dsMatrix &Transform, const Translate3dsMatrix &Translate)
{
	cout << "Transformation Matrix:" << endl;
	cout << Transform._11 << " " << Transform._12 << " " << Transform._13 << " " << endl;
	cout << Transform._21 << " " << Transform._22 << " " << Transform._23 << " " << endl;
	cout << Transform._31 << " " << Transform._32 << " " << Transform._33 << " " << endl;
	cout << "Translation Matrix:" << endl;
	cout << Translate._11 << " " << Translate._12 << " " << Translate._13 << " " << endl;
}

void C3dsFileLoader::UserLight(float x, float y, float z)
{
	cout << "X: " << x << " Y: " << y << " Z: " << z << endl;
}

void C3dsFileLoader::UserSpotLight(float x, float y, float z, float HotSpot, float FallOff)
{
	cout << "X: " << x << " Y: " << y << " Z: <" << z << endl;
	cout << "HotSpot: " << HotSpot << endl;
	cout << "FallOff: " << FallOff << endl;
}

void C3dsFileLoader::UserMatName(const string Name)
{
	cout << "Material Name: " << Name << endl;
}

void C3dsFileLoader::UserMapFile(const string FileName)
{
	cout << "Texture FileName: " << FileName << endl;
}

void C3dsFileLoader::UserFrames(long Start, long End)
{
	cout << "Start: " << Start << " End: " << End << endl;
}

void C3dsFileLoader::UserTrackObjectName(string Name, int Key1, int Key2, int Key3)
{
	cout << "Track Object Name: " << Name << endl;
	cout << "First Key: " << Key1 << endl;
	cout << "Second Key: " << Key2 << endl;
	cout << "Third Key: " << Key3 << endl;
}

void C3dsFileLoader::UserTrackPos(int Frame, long Flags, float x, float y, float z)
{
	cout << "Frame Number: " << Frame << endl;
	cout << "Flags: " << Flags << endl;
	cout << "Position - X: " << x << " Y: " << y << " Z: " << z << endl;
}

void C3dsFileLoader::UserTrackRot(int Frame, long Flags, float DegX, float DegY, float DegZ)
{
	cout << "Frame Number: " << Frame << endl;
	cout << "Flags: " << Flags << endl;
	cout << "Rotation - X: " << DegX << " Y: " << DegY << " Z: " << DegZ << endl;
}

void C3dsFileLoader::UserTrackScale(int Frame, long Flags, float ScaleX, float ScaleY, float ScaleZ)
{
	cout << "Frame Number: " << Frame << endl;
	cout << "Flags: " << Flags << endl;
	cout << "Scaling - X: " << ScaleX << " Y: " << ScaleY << " Z: " << ScaleZ << endl;
}

void C3dsFileLoader::UserObjNumber(int ObjNumber)
{
	cout << "Object Number: " << ObjNumber << endl;
}

void C3dsFileLoader::UserChunkObj()
{
}

void C3dsFileLoader::UserEndOfFile()
{
	cout << "EndOfFile" << endl;
}

void C3dsFileLoader::UserOneUnit(float Unit)
{
	cout << "One Unit is: " << Unit << endl;
}

void C3dsFileLoader::UserViewPortLayout(ViewPortLayout &Layout)
{
	cout << "Style: " << Layout.style << endl;
	cout << "Active: " << Layout.active << endl;
	cout << "Unknow1: " << Layout.unknow1 << endl;
	cout << "Swap: " << Layout.swap<< endl;
	cout << "Unknow2: " << Layout.unknow2 << endl;
	cout << "Swap Prior: " << Layout.swap_prior << endl;
	cout << "Swap View: " << Layout.swap_view << endl;
}

void C3dsFileLoader::UserViewPortSize(int PosX, int PosY, int Width, int Height)
{
	cout << "Position X: " << PosX << endl;
	cout << "Position Y: " << PosY << endl;
	cout << "Width: " << PosY << endl;
	cout << "Height: " << PosY << endl;
}

void C3dsFileLoader::UserViewPortData(ViewPortData &Data)
{
	cout << "Flags: " << Data.flags << endl;
	cout << "Axis Lockout: " << Data.axis_lockout << endl;
	cout << "Window Position - X: " << Data.win_x << " Y: " << Data.win_y << endl;
	cout << "Window Size - Width: " << Data.win_w << " Height: " << Data.win_h << endl;
	cout << "Window View: " << Data.win_view << endl;
	cout << "Zoom factor: " << Data.zoom << endl;
	cout << "World Origin - X: " << Data.worldcenter_x << " Y: " << Data.worldcenter_y << " Z: " << Data.worldcenter_z << endl;
	cout << "Horizontal Angle: " << Data.horiz_ang << endl;
	cout << "Vertical Angle: " << Data.vert_ang << endl;
	cout << "Camera Name: " << Data.camera_name << endl;	
}

void C3dsFileLoader::UserViewUser(float x, float y, float z, float Width, float XYangle, float YZangle, float BackAngle)
{
	cout << "Point - X: " << x << " Y: " << y << " Z: " << z << endl;
	cout << "Width: " << Width << endl;
	cout << "Horizontal view Angle from OXY Plane: " << XYangle << endl;
	cout << "Vertical view Angle from OYZ Plane: " << YZangle << endl;
	cout << "BankAngle of View Rotation: " << BackAngle << endl;
}

void C3dsFileLoader::UserViewCamera(string CameraName)
{
	cout << "Default Camera Name: " << CameraName << endl;
}

void C3dsFileLoader::movePC(long forward)
{
	PC += forward;
	mInputFile.seekg(PC);
}

void C3dsFileLoader::ReadRGBF()
{
    float c[3];
    if (!ReadBlock((char *) c, sizeof(c))) 
		return;
}

void C3dsFileLoader::ReadRGBB()
{
	unsigned char c[3];
	if(!ReadBlock((char *) c, sizeof(c)))
		return;
}

void C3dsFileLoader::ReadObjBlock()
{
	char ObjName[255];
	ReadASCIIZ(ObjName);
	UserObjName(string(ObjName));
}

void C3dsFileLoader::ReadCamera()
{
    float c[8];
    if (!ReadBlock( (char *) c, sizeof(c)))
		return;
	UserCamera(c[0], c[1], c[2], c[3], c[4], c[5], c[6], c[7]);
}

void C3dsFileLoader::ReadFaceMat()
{
    short n, nf;
	char MaterialName[255];

    ReadASCIIZ(MaterialName);

    if (!ReadBlock((char *) &n, sizeof(n))) 
		return;
    while (n-- > 0) 
	{	// Material number assigned to any face
		// Numero di material assegnato ad ogni faccia
        if (!ReadBlock((char*) &nf, sizeof(nf)))
			return;
		UserFaceMaterial(string(MaterialName), (int) nf);
    }
}

void C3dsFileLoader::ReadFrames()
{
    long c[2];
    if (!ReadBlock((char*) c, sizeof(c)))
		return;
	UserFrames(c[0], c[1]);
}

void C3dsFileLoader::ReadLight()
{
    float c[3];
    if (!ReadBlock( (char*) c, sizeof(c))) 
		return;
	UserLight(c[0], c[1], c[2]);
}

void C3dsFileLoader::ReadMapFile()
{
	char fileName[255];
	ReadASCIIZ(fileName);
	UserMapFile(string(fileName));
}

void C3dsFileLoader::ReadMapList()
{
    short nv;
    float c[2];

    if (!ReadBlock((char *) &nv, sizeof(nv))) 
		return;
    while (nv-- > 0) 
	{
        if (!ReadBlock((char *) c, sizeof(c)))
			return;
		UserMapVertex(c[0], c[1]);
    }
}

void C3dsFileLoader::ReadMatName()
{
	char	matName[255];

	ReadASCIIZ(matName);
	UserMatName(string(matName));
}

void C3dsFileLoader::ReadObjNumber()
{
    signed short n;

    if (!ReadBlock((char *) &n, sizeof(n)))
		return;
	UserObjNumber((int) n);
}

void C3dsFileLoader::ReadSpotLight()
{
    float c[5];
    if (!ReadBlock( (char*) c, sizeof(c)))
		return;
	UserSpotLight(c[0], c[1], c[2], c[3], c[4]);
}

void C3dsFileLoader::ReadTrackObjName()
{
    unsigned short w[3];
	char trackName[255];

	// Read ASCIIZ name
	ReadASCIIZ(trackName);
    if (!ReadBlock((char*) w, sizeof(w))) 
		return;
	UserTrackObjectName(string(trackName), w[0], w[1], w[2]);
}

void C3dsFileLoader::ReadTrackPos()
{
    short	n, nf;
    float	pos[3];
    long	flags;


	mInputFile.seekg(10, ios::cur);
	PC += 10;
    if(!ReadBlock( (char*) &n, sizeof(n))) 
		return;
	mInputFile.seekg(2, ios::cur);
	PC += 2;
    while (n-- > 0) 
	{
        int i;
        float dat;
        if (!ReadBlock( (char *) &nf, sizeof(nf))) 
			return;
        if (!ReadBlock((char *) &flags, sizeof(flags))) 
			return;
        for (i = 0; i < 32; i++)
            if (flags & (1 << i))
                if (!ReadBlock( (char *) &dat, sizeof(dat))) 
					return;
        if (!ReadBlock( (char *) pos, sizeof(pos)))
			return;
		UserTrackPos(nf, flags, pos[0], pos[1], pos[2]);
    }
}

void C3dsFileLoader::ReadTrackRot()
{
    unsigned short	n, nf;
    float			pos[4];
    long			flags;

	mInputFile.seekg(10, ios::cur);
	PC += 10;
    if(!ReadBlock((char *) &n, sizeof(n)))
		return;
	mInputFile.seekg(2, ios::cur);
	PC += 2;
    while (n-- > 0) 
	{
        int i;
        float dat;
        if (!ReadBlock( (char*) &nf, sizeof(nf)))
			return;
        if (!ReadBlock( (char*) &flags, sizeof(flags))) 
			return;
        for (i = 0; i < 32; i++)
            if (flags & (1 << i))
                if (!ReadBlock((char*) &dat, sizeof(dat))) 
					return;
		if (!ReadBlock((char *) pos, sizeof(pos))) 
			return;
		UserTrackRot(nf, flags, pos[0], pos[1], pos[2]);
    }
}

void C3dsFileLoader::ReadTrackScale()
{
    unsigned short n, nf;
    float pos[3];
	long flags;

	mInputFile.seekg(10, ios::cur);
	PC += 10;
    if(!ReadBlock((char *) &n, sizeof(n)))
		return;
	mInputFile.seekg(2, ios::cur);
	PC += 2;
    while (n-- > 0) 
	{
        int i;
        float dat;
        if (!ReadBlock((char*) &nf, sizeof(nf)))
			return;
        if (!ReadBlock((char*) &flags, sizeof(flags)))
			return;
        for (i = 0; i < 32; i++)
            if (flags & (1 << i))
                if (!ReadBlock( (char *) &dat, sizeof(dat)))
					return;
        if (!ReadBlock((char *) pos, sizeof(pos)))
			return;
		UserTrackScale(nf, flags, pos[0], pos[1], pos[2]);
    }
}

void C3dsFileLoader::ReadTrMatrix()
{
    float rot[9];
    float trans[3];
    if (!ReadBlock((char *) rot, sizeof(rot)))
		return;
    if (!ReadBlock( (char *) trans, sizeof(trans)))
		return;
	Transform3dsMatrix Transform;
	Translate3dsMatrix Translate;
	Transform._11 = rot[0]; Transform._12 = rot[1]; Transform._13 = rot[2];
	Transform._21 = rot[3]; Transform._22 = rot[4]; Transform._23 = rot[5];
	Transform._31 = rot[6]; Transform._32 = rot[7]; Transform._33 = rot[8];
	Translate._11 = trans[0]; Translate._12 = trans[1]; Translate._13 = trans[2];
	UserTransformMatrix(Transform, Translate);
}

void C3dsFileLoader::ReadVertList()
{
    short nv;
    float c[3];

    if (!ReadBlock((char *) &nv, 2))
		return;
    while (nv-- > 0) 
	{
        if (!ReadBlock((char *) c, sizeof(c)))
			return;
		User3dVert(c[0], c[1], c[2]);
    }
}

void C3dsFileLoader::ReadFaceList()
{
    unsigned short nv;
    unsigned short c[4];

    if (!ReadBlock((char *) &nv, sizeof(nv)))
		return;
    while (nv-- > 0)
	{
        if (!ReadBlock((char *) c, sizeof(c)))
			return;
		User3dFace(c[0], c[1], c[2], c[3]);
    }
}

void C3dsFileLoader::ReadObjChunk()
{
	UserChunkObj();
}

void C3dsFileLoader::EndReading()
{
	UserEndOfFile();
}

void C3dsFileLoader::ReadOneUnit()
{
	float OneUnit;
    if (!ReadBlock((char *) &OneUnit, sizeof(OneUnit)))
		return;
	UserOneUnit(OneUnit);
}

void C3dsFileLoader::ReadViewPortLayout()
{
    unsigned short	style;
    short			active, unknow1, swap, unknow2, swap_prior, swap_view;

	if (!ReadBlock((char *) &style, sizeof(style)))
		return;
    if (!ReadBlock((char *) &active, sizeof(active)))
		return;
    if (!ReadBlock((char *) &unknow1, sizeof(unknow1)))
		return;
    if (!ReadBlock((char *) &swap, sizeof(swap)))
		return;
    if (!ReadBlock((char *) &unknow2, sizeof(unknow2)))
		return;
    if (!ReadBlock((char *) &swap_prior, sizeof(swap_prior)))
		return;
    if (!ReadBlock((char *) &swap_view, sizeof(swap_view)))
		return;
	UserViewPortLayout(ViewPortLayout(style, active, unknow1, swap, unknow2, swap_prior, swap_view));
}

void C3dsFileLoader::ReadViewPortSize()
{
	short PosX, PosY, Width, Height; 
	if (!ReadBlock((char *) &PosX, sizeof(PosX)))
		return;
	if (!ReadBlock((char *) &PosY, sizeof(PosY)))
		return;
	if (!ReadBlock((char *) &Width, sizeof(Width)))
		return;
	if (!ReadBlock((char *) &Height, sizeof(Height)))
		return;
	UserViewPortSize(PosX, PosY, Width, Height);
}

void C3dsFileLoader::ReadViewPortData()
{
	short flags, axis_lockout;
    short win_x, win_y, win_w, win_h, win_view;
	float zoom; 
	float worldcenter_x, worldcenter_y, worldcenter_z;
	float horiz_ang, vert_ang;
	char camera_name[12];

	if (!ReadBlock((char *) &flags, sizeof(flags)))
		return;
	if (!ReadBlock((char *) &axis_lockout, sizeof(axis_lockout)))
		return;
	if (!ReadBlock((char *) &win_x, sizeof(win_x)))
		return;
	if (!ReadBlock((char *) &win_y, sizeof(win_y)))
		return;
	if (!ReadBlock((char *) &win_w, sizeof(win_w)))
		return;
	if (!ReadBlock((char *) &win_h, sizeof(win_h)))
		return;
	if (!ReadBlock((char *) &win_view, sizeof(win_view)))
		return;
	if (!ReadBlock((char *) &zoom, sizeof(zoom)))
		return;
	if (!ReadBlock((char *) &worldcenter_x, sizeof(worldcenter_x)))
		return;
	if (!ReadBlock((char *) &worldcenter_y, sizeof(worldcenter_y)))
		return;
	if (!ReadBlock((char *) &worldcenter_z, sizeof(worldcenter_z)))
		return;
	if (!ReadBlock((char *) &horiz_ang, sizeof(horiz_ang)))
		return;
	if (!ReadBlock((char *) &vert_ang, sizeof(vert_ang)))
		return;
	if (!ReadBlock((char *) camera_name, CameraNameSize))
		return;
	UserViewPortData(ViewPortData(flags, axis_lockout, win_x, win_y, win_w, win_h,
								  win_view, zoom, worldcenter_x, worldcenter_y,
								  worldcenter_z, horiz_ang, vert_ang, camera_name));
}

void C3dsFileLoader::ReadViewUser()
{
	float c[3];
	float Width, XYangle, YZangle, BackAngle;

	if (!ReadBlock((char *) c, sizeof(c)))
		return;
	if (!ReadBlock((char *) &Width, sizeof(Width)))
		return;
	if (!ReadBlock((char *) &XYangle, sizeof(XYangle)))
		return;
	if (!ReadBlock((char *) &YZangle, sizeof(YZangle)))
		return;
	if (!ReadBlock((char *) &BackAngle, sizeof(BackAngle)))
		return;
	UserViewUser(c[0], c[1], c[2], Width, XYangle, YZangle, BackAngle);
}

void C3dsFileLoader::ReadViewCamera()
{
	char camera_name[12];

	if (!ReadBlock((char *) camera_name, CameraNameSize))
		return;
	UserViewCamera(string(camera_name));

}

bool C3dsFileLoader::ParseChunks()
{
	if(!mInputFile.is_open())
		return false;
	if(mInputFile.eof())
		return false;

	chunk3ds	chunk;

//	getchar();

	if(!ReadBlock((char*) &chunk, sizeof(chunk3ds)))
		return false;
	switch(chunk.Flag)
	{

	case CHUNK_RGBF:
		cout << "Chunk RGBF detected" << endl;
		ReadRGBF();
		ParseChunks();
		break;
	case CHUNK_RGBB:
		cout << "Chunk RGBB detected" << endl;
		ReadRGBB();
		ParseChunks();
		break;
	case CHUNK_MAIN:
		cout << "Chunk Main detected" << endl;
		ParseChunks();
		break;
	case CHUNK_OBJMESH:
		cout << "Chunk Object detected" << endl;
		ReadObjChunk();
		ParseChunks();
		break;
	case CHUNK_OBJBLOCK:
		cout << "Chunk Object Block detected" << endl;
		ReadObjBlock();
		cout << endl;
		ParseChunks();
		break;
	case CHUNK_TRIMESH:
		cout << "Chunk Trimesh detected" << endl;
		ParseChunks();
		break;
	case CHUNK_VERTLIST:
		cout << "Chunk VertList detected at " << (PC - sizeof(chunk3ds)) << endl;
		ReadVertList();
		ParseChunks();
		break;
	case CHUNK_FACELIST:
		cout << "Chunk FaceList detected" << endl;
		ReadFaceList();
		ParseChunks();
		break;
	case CHUNK_FACEMAT:
		cout << "Chunk FaceMat detected" << endl;
		ReadFaceMat();
		ParseChunks();
		break;
	case CHUNK_MAPLIST:
		cout << "Chunk MapList detected" << endl;
		ReadMapList();
		ParseChunks();
		break;
	case CHUNK_SMOOLIST:
		cout << "Chunk SmoothList detected" << endl;
//		ReadSmoothList(); // Still unsupported
		SkipChunk(chunk);
		ParseChunks();
		break;
	case CHUNK_TRMATRIX:
		cout << "Chunk TRMatrix detected" << endl;
		ReadTrMatrix();
		ParseChunks();
		break;
	case CHUNK_LIGHT:
		cout << "Chunk Light detected" << endl;
		ReadLight();
		ParseChunks();
		break;
	case CHUNK_SPOTLIGHT:
		cout << "Chunk SpotLight detected" << endl;
		ReadSpotLight();
		ParseChunks();
		break;
	case CHUNK_CAMERA:
		cout << "Chunk Camera detected" << endl;
		ReadCamera();
		ParseChunks();
		break;
	case CHUNK_MATERIAL:
		cout << "Chunk Material detected" << endl;
		ParseChunks();
		break;
	case CHUNK_MATNAME:
		cout << "Chunk MatName detected" << endl;
		ReadMatName();
		ParseChunks();
		break;
	case CHUNK_MAPFILE:
		cout << "Chunk MapFile detected" << endl;
		ReadMapFile();
		ParseChunks();
		break;
	case CHUNK_KEYFRAMER:
		cout << "Chunk Keyframer detected" << endl;
		ParseChunks();
		break;
	case CHUNK_TRACKINFO:
		cout << "Chunk TrackInfo detected" << endl;
		ParseChunks();
		break;
	case CHUNK_TRACKOBJNAME:
		cout << "Chunk TrackObjName detected" << endl;
		ReadTrackObjName();
		ParseChunks();
		break;
	case CHUNK_TRACKPOS:
		cout << "Chunk TrackPos detected" << endl;
		ReadTrackPos();
		ParseChunks();
		break;
	case CHUNK_TRACKROTATE:
		cout << "Chunk TrackRotate detected" << endl;
		ReadTrackRot();
		ParseChunks();
		break;
	case CHUNK_TRACKSCALE:
		cout << "Chunk TrackScale detected" << endl;
		ReadTrackScale();
		ParseChunks();
		break;
	case CHUNK_OBJNUMBER:
		cout << "Chunk ObjNumber detected" << endl;
		ReadObjNumber();
		ParseChunks();
		break;
	default:
		cout << "Unknown Chunk at " << mInputFile.tellg() << " offset" << endl;
		SkipChunk(chunk);
		ParseChunks();
		break;
	}
	return true;
}

bool C3dsFileLoader::SkipChunk(chunk3ds &chunk)
{
	PC += (chunk.Size - sizeof(chunk3ds));
	mInputFile.seekg(PC);
	if(mInputFile.eof())
		return false;
	return true;
}

void C3dsFileLoader::ProcessFile(string FileName)
{
	mInputFile.open(FileName.c_str(), ios::binary);
	FileSize();
	if(mInputFile.is_open() == false)
		return;
	ParseChunks();
	EndReading();
}

bool C3dsFileLoader::ReadBlock(char *dest, long size)
{
	PC = mInputFile.tellg();
    if(!mInputFile.read(dest, size) && mInputFile.eof())
        return false;
	PC += size;
	mInputFile.seekg(PC);
	return true;
}

bool C3dsFileLoader::isOk()
{
	if(!mInputFile.is_open())
		return false;
	if(mInputFile.eof())
		return false;
	return true;
}

void C3dsFileLoader::FileSize()
{
	// get length of file:
	mInputFile.seekg (0, ios::end);
	mFileSize = mInputFile.tellg();
	mInputFile.seekg (0, ios::beg);
}

void C3dsFileLoader::ReadASCIIZ(char *dest)
{
	char c;
	long Count = 0;
	do 
	{
		c = mInputFile.get();       // get character from file
		*dest++ = c;
		Count++;
	} while (c != 0);     // loop while extraction from file is possible
	PC+= Count;
	mInputFile.seekg(PC);
}
