// DRGObjectCone.cpp - Created with DR&E AppWizard 1.0B9
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
// DRGObjectCone.cpp: definition of  the Cone Object, derived from the DRGObject base class.
// This class is used to contain a Cone object, it's vertices, matrices, etc.
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
CDRGObjectCone::CDRGObjectCone()
{
	;
}

// -----------------------------------------------------------------------
// DRGObject::~DRGObject()
//
// El Destructor
// -----------------------------------------------------------------------
CDRGObjectCone::~CDRGObjectCone()
{
	
	// Get rid of the VB
	if (m_pIndexedVertexBuffer)
			SAFE_RELEASE( m_pIndexedVertexBuffer );

	m_pWrapper = NULL;

	if (m_pNext)
		delete m_pNext;
	
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

HRESULT CDRGObjectCone::Init(	CDRGWrapper *pDRGWrapper,
					float		halfheight		,					
					float		radius			,
					int			iDiv			,					
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
	m_fRotationAngle		= rotationangle;
	m_vScale				= scale;
	m_vPosition				= pos;	
	
	// current velocity, rotational velocity, scale velociy
	m_fRotationalVelocity	= rotationalvel;
	m_vScalingVelocity		= scalevel;
	m_vVelocity				= vel;	

	// vertex and index counters
	if (iDiv < 3)
		iDiv = 3;
	m_iNumDiv = iDiv;
	m_fRadius = radius;
	m_fHalfHeight = halfheight;

	m_iNumVertices = (m_iNumDiv) * 3;	
	m_iNumIndices = (3 * m_iNumDiv) + ((m_iNumDiv -2)*3);


	//alloc mem for the verts
	m_pVertices = new D3DVERTEX[m_iNumVertices];
	m_pIndices = new WORD[m_iNumIndices];
	
	

	//ptr to the wrapper
	m_pWrapper = pDRGWrapper;


	//if we are being fed a bunch of verts, then we need to set them up
	if (SetupVertices() != S_OK)
	{
		OutputDebugString("unable to set up verts");
		return E_FAIL;
	}
		
	if (SetupIndices() != S_OK)
	{
		OutputDebugString("unable to set up indices");
		return E_FAIL;
	}

	if (SetupIndexedVertexBuffer() != S_OK)
	{
		OutputDebugString("unable to set up VB");
		return E_FAIL;
	}
	
	return S_OK;
}
	
	
HRESULT CDRGObjectCone::SetupVertices( )
{
	
	float pi = 3.14159265358979323846f;
	// theta is the current latitudinal angle
	float theta = pi / 2.0f;
	
	// delta values for the angles depending
	// on the number of divisions
	float dTheta = 2.0f*pi / m_iNumDiv;

	int curVert = 0;
	float x, y, z;

	// Initialize the vertices at the top of the Cone
	for(int i=0; i<m_iNumDiv; i++)
	{
		theta += dTheta;
		x = (float)cos(theta);
		y = (float)sin(theta);
			
		
		// We need multiple vertices at the top position in order
		// to correctly map a texture to the Cone
		m_pVertices[curVert].x = 0;
		m_pVertices[curVert].y = m_fHalfHeight;
		m_pVertices[curVert].z = 0;

		m_pVertices[curVert].nx = x;
		m_pVertices[curVert].ny = 0;
		m_pVertices[curVert].nz = y; //This isn't quit correct. Should depends on radius (upward tilt). to be fixed later

		m_pVertices[curVert].tu = (float)i / m_iNumDiv;
		m_pVertices[curVert++].tv = 0.0f;
	}

	//reset this
	 theta = pi / 2.0f;

	
	// Intialize the vertices that make up the cone base
	for(i = 0; i<m_iNumDiv; i++)
	{
		theta += dTheta;
				
		x = (float)cos(theta);// * (float)cos(phi);
		y = 0.0f - m_fHalfHeight;
		z = (float)sin(theta);
	
		D3DVECTOR temp = m_fRadius * D3DVECTOR(x,0,z);
		temp.y = y;

		m_pVertices[curVert] = 
			D3DVERTEX(temp, D3DVECTOR(x, y, z), (float)i/m_iNumDiv, 1.0f);

		m_pVertices[m_iNumDiv + curVert++] = 
			D3DVERTEX(temp, D3DVECTOR(0, -1, 0), (float)i/m_iNumDiv, 1.0f);

	}

	return S_OK;
}

	// Set up indexed vertex indices
	// for using DrawIndexedPrimitive(VB/Strided)
HRESULT CDRGObjectCone::SetupIndices( )
{

	int i;
	
	int curVert = 0;

	// Top section
	for(i=0; i<m_iNumDiv; i++)
	{

		m_pIndices[curVert++] = i;
		
		// Skip past the vertices at the top of the Cone (+m_iNumDiv)
		// and also skip to the next vertice (+1)
		m_pIndices[curVert++] = i+m_iNumDiv+1;

		m_pIndices[curVert++] = i+m_iNumDiv;	

		
	}
	
	m_pIndices[curVert-2] = m_iNumDiv;	
	
	for(i=0; i<(m_iNumDiv-2); i++)
	{

		m_pIndices[curVert++] = 2* m_iNumDiv;
		
		// Skip past the vertices at the top of the Cone (+m_iNumDiv)
		// and also skip to the next vertice (+1)
		m_pIndices[curVert++] = i+2*m_iNumDiv+1;

		m_pIndices[curVert++] = i+2*m_iNumDiv+2;	

		
	}
	
	return S_OK;
}
