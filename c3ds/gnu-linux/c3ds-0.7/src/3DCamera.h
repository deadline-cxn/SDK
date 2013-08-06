//
// File: 3DCamera.h
// Created by: <Andrea Ingegneri>
//

#ifndef _C3DCAMERA_H_
#define _C3DCAMERA_H_

#include "VECTOR.h"

class C3DCamera
{
public:
	C3DCamera();
	C3DCamera(VECTOR Position, VECTOR Target, float Bank, float Lens) : position(Position),
																		target(Target),
																		bank(Bank),
																		lens(Lens) {}
	VECTOR	position;
	VECTOR	target;
	float	bank, lens;
};

#endif	//_C3DCAMERA_H_

