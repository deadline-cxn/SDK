//
// File: 3DModel.h
// Created by: <Andrea Ingegneri>
//

#ifndef _C3DMODEL_H_
#define _C3DMODEL_H_

#include <vector>
using namespace std;

#include "VECTOR.h"
#include "MATRIX.h"
#include "3DCamera.h"
#include "3dsFileLoader.h"

class C3DObject;

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

#endif	//_C3DMODEL_H_

