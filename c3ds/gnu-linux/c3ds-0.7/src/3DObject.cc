//
// File: 3DObject.h
// Created by: <Andrea Ingegneri>
//

#include "3DObject.h"

C3DObject::C3DObject()
{

}

C3DObject::~C3DObject()
{
	vertices.clear();
	texturePoints.clear();
	faces.clear();
}

void C3DObject::UseTrasform()
{
	int vnum = vertices.size();

	for (int i = 0; i < vnum; i++)
	{
		vertices[i] = vertices[i] * transform;
	}
}
