//
// File: 3DFace.h
// Created by: <Andrea Ingegneri>
//

#ifndef _C3DFACE_H_
#define _C3DFACE_H_

class C3DFace
{
public:
	C3DFace();
	C3DFace(int a, int b, int c) : A(a), B(b), C(c) {}
	int A, B, C;
};

#endif	//_C3DFACE_H_

