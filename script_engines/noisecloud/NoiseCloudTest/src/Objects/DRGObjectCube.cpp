// DRGObjectCube.cpp - Created with DR&E AppWizard 1.0B9
// ------------------------------------------------------------------------------------
// Copyright © 1999 Intel Corporation
// All Rights Reserved
// 
// Permission is granted to use, copy, distribute and prepare derivative works of this 
// software for any purpose and without fee, provided, that the above copyright notice
// and this statement appear in all copies.  Intel makes no representations about the
// suitability of this software for any purpose.  This software is provided "AS IS." 
//
// Intel specifically disclaims all warranties, express or implied, and all liability,
// including consequential and other indirect damages, for the use of this software, 
// including liability for infringement of any proprietary rights, and including the 
// warranties of merchantability and fitness for a particular purpose.  Intel does not
// assume any responsibility for any errors which may appear in this software nor any
// responsibility to update it.
// ------------------------------------------------------------------------------------
//
//  PURPOSE:
//    
// DRGObjectCube.cpp: definition of hte the Cube Object, derived from the DRGObject base class.
// This class is used to contain a cube object, it's vertices, matrices, etc.
// The class stores two copies of the vertices, an array of D3DVerts, and a Vertex
// buffer used for rendering. At the time this class was authored, it was unclear
// whether the array was needed in case of a lost vertex buffer (DX7 beta docs unclear)
// so there may be some memory savings to be made there.
//
// ------------------------------------------------------------------------------------
//
// Author: Kim Pallister
// 
////////////////////////////////////////////////////////////////////////////////////////

#include "..\DRGShell.h"


// -----------------------------------------------------------------------
// DRGObject::DRGObject()
//
// Initialize the necesary member vars.
// -----------------------------------------------------------------------
CDRGObjectCube::CDRGObjectCube()
{
	
	// Pointer to sysmem copy of verts
	m_pVertices = new D3DVERTEX[24];
	m_pIndices = new WORD[36];
	m_iNumVertices = 24;	
	m_iNumIndices = 36;	

}

// -----------------------------------------------------------------------
// DRGObject::~DRGObject()
//
// El Destructor
// -----------------------------------------------------------------------
CDRGObjectCube::~CDRGObjectCube()
{
	
	// Get rid of the VB
	if (m_pIndexedVertexBuffer)
			SAFE_RELEASE( m_pIndexedVertexBuffer );

	m_pWrapper = NULL;

	//pointer to siblings and children, if this object is part of a hierarchy (this is still TBD)
	m_pNext = NULL;

	if (m_pFirstChild)
		delete m_pFirstChild;	

}

// -----------------------------------------------------------------------
// HRESULT CDRGObject::Init( *bunch o stuff* )
//	
//  This function is called to set up all the member vars with specific 
//  values and/or to feed in all the verts from a specific source known
//	to the app. 
//
//  derived versions of this class will call this member function to 
//  init all the verts for procedural objects, or ojbects coming from files.

// -----------------------------------------------------------------------

HRESULT CDRGObjectCube::Init(	CDRGWrapper *pDRGWrapper,
					DRGVector	pos				,
					DRGVector	scale			,
					DRGVector	rotationaxis	,
					float		rotationangle	,
					DRGVector	vel				,
					DRGVector	scalevel		,
					float		rotationalvel	,
					DWORD		dwFlags			 )
{
	m_vRotationAxis			= rotationaxis;
	m_fRotationAngle = rotationangle;
	m_vScale			= scale;
	m_vPosition				= pos;	
	
	// current velocity, rotational velocity, scale velociy
	m_fRotationalVelocity	= rotationalvel;
	m_vScalingVelocity		= scalevel;
	m_vVelocity				= vel;	

	// vertex and index counters
	//m_iNumVertices = 24;	//6 faces * 4 per face
	//m_iNumIndices = 36;		//6 faces * 2 triangles * 3 indices per triangle

	//ptr to the wrapper
	m_pWrapper = pDRGWrapper;


	//if we are being fed a bunch of verts, then we need to set them up
	if (SetupVertices() != S_OK)
	{
		OutputDebugString("unable to set up verts");
		return -1;
	}
		
	if (SetupIndices() != S_OK)
	{
		OutputDebugString("unable to set up indices");
		return -1;
	}

	if (SetupIndexedVertexBuffer() != S_OK)
	{
		OutputDebugString("unable to set up VB");
		return -1;
	}

	return S_OK;
}
	

HRESULT CDRGObjectCube::SetupVertices( )
{
	////////////
	//back plane Z = -1 (ok)
	m_pVertices[0].x = -1.0f; m_pVertices[0].y = 1.0f; m_pVertices[0].z = -1.0f;
	m_pVertices[1].x =  1.0f; m_pVertices[1].y = 1.0f; m_pVertices[1].z = -1.0f;
	m_pVertices[2].x = -1.0f; m_pVertices[2].y = -1.0f; m_pVertices[2].z = -1.0f;
	m_pVertices[3].x =  1.0f; m_pVertices[3].y = -1.0f; m_pVertices[3].z = -1.0f;

	//back plane normals
	m_pVertices[0].nx = 0.0f; m_pVertices[0].ny = 0.0f; m_pVertices[0].nz = -1.0f;
	m_pVertices[1].nx = 0.0f; m_pVertices[1].ny = 0.0f; m_pVertices[1].nz = -1.0f;
	m_pVertices[2].nx = 0.0f; m_pVertices[2].ny = 0.0f; m_pVertices[2].nz = -1.0f;
	m_pVertices[3].nx = 0.0f; m_pVertices[3].ny = 0.0f; m_pVertices[3].nz = -1.0f;

	//back plane UV's
	m_pVertices[0].tu = 0.0f; m_pVertices[0].tv = 0.0f; 
	m_pVertices[1].tu = 1.0f; m_pVertices[1].tv = 0.0f; 
	m_pVertices[2].tu = 0.0f; m_pVertices[2].tv = 1.0f; 
	m_pVertices[3].tu = 1.0f; m_pVertices[3].tv = 1.0f; 

	////////////
	// left plane (x = -1)
	m_pVertices[4].x = -1.0f; m_pVertices[4].y = 1.0f; m_pVertices[4].z =1.0f;// 1.0f;
	m_pVertices[5].x = -1.0f; m_pVertices[5].y = 1.0f; m_pVertices[5].z =-1.0f;// -1.0f;
	m_pVertices[6].x = -1.0f; m_pVertices[6].y = -1.0f; m_pVertices[6].z =1.0f;// 1.0f;
	m_pVertices[7].x = -1.0f; m_pVertices[7].y = -1.0f; m_pVertices[7].z =-1.0f;// -1.0f;

	//left plane normals
	m_pVertices[4].nx = -1.0f; m_pVertices[4].ny = 0.0f; m_pVertices[4].nz = 0.0f;
	m_pVertices[5].nx = -1.0f; m_pVertices[5].ny = 0.0f; m_pVertices[5].nz = 0.0f;
	m_pVertices[6].nx = -1.0f; m_pVertices[6].ny = 0.0f; m_pVertices[6].nz = 0.0f;
	m_pVertices[7].nx = -1.0f; m_pVertices[7].ny = 0.0f; m_pVertices[7].nz = 0.0f;

	//left plane UV's
	m_pVertices[4].tu = 0.0f; m_pVertices[4].tv = 0.0f; 
	m_pVertices[5].tu = 1.0f; m_pVertices[5].tv = 0.0f; 
	m_pVertices[6].tu = 0.0f; m_pVertices[6].tv = 1.0f; 
	m_pVertices[7].tu = 1.0f; m_pVertices[7].tv = 1.0f; 

	////////////
	// front plane
	m_pVertices[8].x = 1.0f; m_pVertices[8].y = 1.0f; m_pVertices[8].z = 1.0f;
	m_pVertices[9].x = -1.0f; m_pVertices[9].y = 1.0f; m_pVertices[9].z = 1.0f;
	m_pVertices[10].x = 1.0f; m_pVertices[10].y = -1.0f; m_pVertices[10].z = 1.0f;
	m_pVertices[11].x = -1.0f; m_pVertices[11].y = -1.0f; m_pVertices[11].z = 1.0f;

	//front plane normals
	m_pVertices[8].nx = 0.0f; m_pVertices[8].ny = 0.0f; m_pVertices[8].nz = 1.0f;
	m_pVertices[9].nx = 0.0f; m_pVertices[9].ny = 0.0f; m_pVertices[9].nz = 1.0f;
	m_pVertices[10].nx = 0.0f; m_pVertices[10].ny = 0.0f; m_pVertices[10].nz = 1.0f;
	m_pVertices[11].nx = 0.0f; m_pVertices[11].ny = 0.0f; m_pVertices[11].nz = 1.0f;

	//front plane UV's
	m_pVertices[8].tu = 0.0f; m_pVertices[8].tv = 0.0f; 
	m_pVertices[9].tu = 1.0f; m_pVertices[9].tv = 0.0f; 
	m_pVertices[10].tu = 0.0f; m_pVertices[10].tv = 1.0f; 
	m_pVertices[11].tu = 1.0f; m_pVertices[11].tv = 1.0f; 


	////////////
	// right plane
	m_pVertices[12].x = 1.0f; m_pVertices[12].y = 1.0f; m_pVertices[12].z = -1.0f;
	m_pVertices[13].x = 1.0f; m_pVertices[13].y = 1.0f; m_pVertices[13].z = 1.0f;
	m_pVertices[14].x = 1.0f; m_pVertices[14].y = -1.0f; m_pVertices[14].z = -1.0f;
	m_pVertices[15].x = 1.0f; m_pVertices[15].y = -1.0f; m_pVertices[15].z =1.0f;

	//right plane normals
	m_pVertices[12].nx = 0.0f; m_pVertices[12].ny = 0.0f; m_pVertices[12].nz = 1.0f;
	m_pVertices[13].nx = 0.0f; m_pVertices[13].ny = 0.0f; m_pVertices[13].nz = 1.0f;
	m_pVertices[14].nx = 0.0f; m_pVertices[14].ny = 0.0f; m_pVertices[14].nz = 1.0f;
	m_pVertices[15].nx = 0.0f; m_pVertices[15].ny = 0.0f; m_pVertices[15].nz = 1.0f;

	//right plane UV's
	m_pVertices[12].tu = 0.0f; m_pVertices[12].tv = 0.0f; 
	m_pVertices[13].tu = 1.0f; m_pVertices[13].tv = 0.0f; 
	m_pVertices[14].tu = 0.0f; m_pVertices[14].tv = 1.0f; 
	m_pVertices[15].tu = 1.0f; m_pVertices[15].tv = 1.0f; 

	////////////
	// top plane
	m_pVertices[16].x = -1.0f; m_pVertices[16].y = 1.0f; m_pVertices[16].z = 1.0f;
	m_pVertices[17].x = 1.0f; m_pVertices[17].y = 1.0f; m_pVertices[17].z = 1.0f;
	m_pVertices[18].x = -1.0f; m_pVertices[18].y = 1.0f; m_pVertices[18].z = -1.0f;
	m_pVertices[19].x = 1.0f; m_pVertices[19].y = 1.0f; m_pVertices[19].z = -1.0f;

	//right plane normals
	m_pVertices[16].nx = 0.0f; m_pVertices[16].ny = 1.0f; m_pVertices[16].nz = 0.0f;
	m_pVertices[17].nx = 0.0f; m_pVertices[17].ny = 1.0f; m_pVertices[17].nz = 0.0f;
	m_pVertices[18].nx = 0.0f; m_pVertices[18].ny = 1.0f; m_pVertices[18].nz = 0.0f;
	m_pVertices[19].nx = 0.0f; m_pVertices[19].ny = 1.0f; m_pVertices[19].nz = 0.0f;

	//right plane UV's
	m_pVertices[16].tu = 0.0f; m_pVertices[16].tv = 0.0f; 
	m_pVertices[17].tu = 1.0f; m_pVertices[17].tv = 0.0f; 
	m_pVertices[18].tu = 0.0f; m_pVertices[18].tv = 1.0f; 
	m_pVertices[19].tu = 1.0f; m_pVertices[19].tv = 1.0f; 

	////////////
	// bottom plane
	m_pVertices[20].x = -1.0f; m_pVertices[20].y = -1.0f; m_pVertices[20].z = -1.0f;
	m_pVertices[21].x = 1.0f; m_pVertices[21].y = -1.0f; m_pVertices[21].z = -1.0f;
	m_pVertices[22].x = -1.0f; m_pVertices[22].y = -1.0f; m_pVertices[22].z = 1.0f;
	m_pVertices[23].x = 1.0f; m_pVertices[23].y = -1.0f; m_pVertices[23].z = 1.0f;

	//bottom plane normals
	m_pVertices[20].nx = 0.0f; m_pVertices[20].ny = -1.0f; m_pVertices[20].nz = 0.0f;
	m_pVertices[21].nx = 0.0f; m_pVertices[21].ny = -1.0f; m_pVertices[21].nz = 0.0f;
	m_pVertices[22].nx = 0.0f; m_pVertices[22].ny = -1.0f; m_pVertices[22].nz = 0.0f;
	m_pVertices[23].nx = 0.0f; m_pVertices[23].ny = -1.0f; m_pVertices[23].nz = 0.0f;

	//bottom plane UV's
	m_pVertices[20].tu = 0.0f; m_pVertices[20].tv = 0.0f; 
	m_pVertices[21].tu = 1.0f; m_pVertices[21].tv = 0.0f; 
	m_pVertices[22].tu = 0.0f; m_pVertices[22].tv = 1.0f; 
	m_pVertices[23].tu = 1.0f; m_pVertices[23].tv = 1.0f; 

	return S_OK;
}

	// Set up indexed vertex indices
	// for using DrawIndexedPrimitive(VB/Strided)
HRESULT CDRGObjectCube::SetupIndices( )
{

	for (int i=0;i<6;i++)
	{
		int j = i*6;
		int k = i*4;

		m_pIndices[j+0] = k+0; m_pIndices[j+1] = k+1; m_pIndices[j+2] = k+2;
		m_pIndices[j+3] = k+2; m_pIndices[j+4] = k+1; m_pIndices[j+5] = k+3;

	}

	return S_OK;
}
