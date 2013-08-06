//
// File: 3DObject.h
// Created by: <Andrea Ingegneri>
//

#ifndef _C3DOBJECT_H_
#define _C3DOBJECT_H_

#include <vector>
using namespace std;

#include "VECTOR.h"
#include "MATRIX.h"
#include "3DFace.h"

class C3DObject
{
public:
	C3DObject();
	~C3DObject();
	void	UseTrasform();
	vector	<VECTOR> vertices;
	vector	<VECTOR> texturePoints;
	vector	<C3DFace> faces;
	MATRIX	transform;
	float	translate[3];
};

#endif	//_C3DOBJECT_H_

