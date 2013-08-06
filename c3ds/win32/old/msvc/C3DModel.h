/************************************
 * 3ds C++ File Loader				*
 * Author: Andrea Ingegneri			*
 * Date: 16/12/2001					*
 * Distribution: LGPL (www.gnu.org) *
 ************************************/

class C3DFace
{
public:
	C3DFace(int a, int b, int c) : A(a), B(b), C(c) {}
	int A, B, C;
};

class C3DCamera
{
public:
	C3DCamera(VECTOR Position, VECTOR Target, float Bank, float Lens) : position(Position),
																		target(Target),
																		bank(Bank),
																		lens(Lens) {}
	VECTOR	position;
	VECTOR	target;
	float	bank, lens;
};

class C3DObject
{
public:
	C3DObject() {}
	~C3DObject();
	void	UseTrasform();
	vector	<VECTOR> vertices;
	vector	<VECTOR> texturePoints;
	vector	<C3DFace> faces;
	MATRIX	transform;
	float	translate[3];
};


void C3DObject::UseTrasform()
{
	int vnum = vertices.size();

	for (int i = 0; i < vnum; i++)
	{
		vertices[i] = vertices[i] * transform;
	}
}

C3DObject::~C3DObject()
{
	vertices.clear();
	texturePoints.clear();
	faces.clear();
}

class C3DModel : public C3dsFileLoader
{
public:
	enum	{NoObjects = -1};
	C3DModel() : C3dsFileLoader(), mObjectsCount(NoObjects), mStillOpen(false) {}
	bool	NewObject();
	bool	EndObject();
	bool	IsObjectWriting() const {return mStillOpen;}
	bool	AddVertex(float x, float y, float z);
	bool	AddFace(int A, int B, int C);
	bool	AddTexturePoint(float u, float v);
	bool	AddCamera(C3DCamera Camera);
	bool	SetMatrix(MATRIX Mtx);
	bool	SetTranslation(float x, float y, float z);
	int		GetObjNumber() const {return (mObjectsCount + 1);}
	C3DObject *GetObject(int ObjectNumber) const {return mObjects[ObjectNumber];}
private:
	int		mObjectsCount;	// Objects counter
	bool	mStillOpen;		// If true an object creating session was opened but still not closed
	vector	<C3DObject*> mObjects;
	vector	<C3DCamera> mCameras;
protected:
	void		User3dVert(float x, float y, float z);
	void		User3dFace(unsigned short A, unsigned short B, unsigned short C, unsigned short Flags);
	void		UserCamera(float px, float py, float pz,
								   float tx, float ty, float tz,
								   float Bank, float Lens);
	void		UserMapVertex(float U, float V);
	void		UserTransformMatrix(const Transform3dsMatrix &Transform, const Translate3dsMatrix &Translate);
	void		UserChunkObj();
	void		UserEndOfFile();
};

bool C3DModel::NewObject()
{
	if (mStillOpen == true)
		return false;			// Previous session is still incomplete!
	else mStillOpen = true;

	if(mObjectsCount == NoObjects)
		mObjectsCount = 0;
	else mObjectsCount++;

	C3DObject	*objectPtr;
	objectPtr = new C3DObject();

	mObjects.push_back(objectPtr);

	return true;
}

bool C3DModel::EndObject()
{
	if (mStillOpen == false)
		return false;			// No active session
	else mStillOpen = false;
	return true;
}

bool C3DModel::AddVertex(float x, float y, float z)
{
	if (mStillOpen == false)
		return false;			// No active session

	mObjects[mObjectsCount]->vertices.push_back(VECTOR(x, y, z));

	return true;
}

bool C3DModel::AddFace(int A, int B, int C)
{
	if (mStillOpen == false)
		return false;			// No active session

	mObjects[mObjectsCount]->faces.push_back(C3DFace(A, B, C));

	return true;
}

bool C3DModel::AddTexturePoint(float u, float v)
{
	if (mStillOpen == false)
		return false;			// No active session

	mObjects[mObjectsCount]->texturePoints.push_back(VECTOR(u, v, 0));
	return true;
}

bool C3DModel::AddCamera(C3DCamera Camera)
{
	mCameras.push_back(Camera);
	return true;
}

bool C3DModel::SetMatrix(MATRIX Mtx)
{
	if (mStillOpen == false)
		return false;			// No active session

	mObjects[mObjectsCount]->transform = Mtx;
	return true;
}

bool C3DModel::SetTranslation(float x, float y, float z)
{
	if (mStillOpen == false)
		return false;			// No active session

	mObjects[mObjectsCount]->translate[0] = x;
	mObjects[mObjectsCount]->translate[1] = y;
	mObjects[mObjectsCount]->translate[2] = z;
	return true;
}

void C3DModel::User3dFace(unsigned short A, unsigned short B, unsigned short C, 
						  unsigned short Flags)
{
	AddFace((int) A, (int) B, (int) C);
}

void C3DModel::User3dVert(float x, float y, float z)
{
	AddVertex(x, y, z);
}

void C3DModel::UserMapVertex(float U, float V)
{
	AddTexturePoint(U, V);
}

void C3DModel::UserCamera(float px, float py, float pz, float tx, float ty, float tz, float Bank, float Lens)
{
	AddCamera(C3DCamera(VECTOR(px, py, pz), VECTOR(tx, ty, tz), Bank, Lens));
}

void C3DModel::UserChunkObj()
{
	if(IsObjectWriting() == true)
		EndObject();
	NewObject();
}

void C3DModel::UserTransformMatrix(const Transform3dsMatrix &Transform, 
								   const Translate3dsMatrix &Translate)
{
	SetTranslation(Translate._11, Translate._12, Translate._13);
	VECTOR	c1(Transform._11, Transform._21, Transform._31);
	VECTOR	c2(Transform._12, Transform._22, Transform._32);
	VECTOR	c3(Transform._13, Transform._23, Transform._33);
	MATRIX	trans(c1, c2, c3);
	SetMatrix(trans);
}

void C3DModel::UserEndOfFile()
{
	if(IsObjectWriting() == true)
		EndObject();
}
