// DRGObjectSphere.cpp - Created with DR&E AppWizard 1.0B9
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
// DRGObjectSphere.cpp: definition of hte the Sphere Object, derived from the DRGObject base class.
// This class is used to contain a Sphere object, it's vertices, matrices, etc.
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
CDRGObjectSphere::CDRGObjectSphere()
{
	;
}

// -----------------------------------------------------------------------
// DRGObject::~DRGObject()
//
// El Destructor
// -----------------------------------------------------------------------
CDRGObjectSphere::~CDRGObjectSphere()
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

HRESULT CDRGObjectSphere::Init(	CDRGWrapper *pDRGWrapper,
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
	m_fRotationAngle = rotationangle;
	m_vScale			= scale;
	m_vPosition				= pos;	
	
	// current velocity, rotational velocity, scale velociy
	m_fRotationalVelocity	= rotationalvel;
	m_vScalingVelocity		= scalevel;
	m_vVelocity				= vel;	

	// vertex and index counters
	m_iNumDiv = iDiv;
	m_fRadius = radius;

	m_iNumVertices = ((m_iNumDiv) * 2) + ((m_iNumDiv + 1) * (m_iNumDiv-1));
	
	m_iNumIndices = 6 * (m_iNumDiv + (m_iNumDiv * (m_iNumDiv-2)));

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
	
	
HRESULT CDRGObjectSphere::SetupVertices( )
{
	
	float pi = 3.14159265358979323846f;
	// theta is the current latitudinal angle
	float theta = pi / 2.0f;
	// phi is the current longitudinal angle
	float phi = 0.0f;
	
	// delta values for the angles depending
	// on the number of divisions
	float dTheta = pi / m_iNumDiv;
	float dPhi = 2.0f * pi / m_iNumDiv;

	int curVert = 0;
	float x, y, z;

	// Initialize the vertices at the top of the sphere
	for(int i=0; i<m_iNumDiv; i++)
	{
		
		// We need multiple vertices at the top position in order
		// to correctly map a texture to the sphere
		m_pVertices[curVert].x = m_pVertices[curVert].z = m_pVertices[curVert].nx 
			                 = m_pVertices[curVert].nz = 0.0f;
		m_pVertices[curVert].y = m_fRadius;
		m_pVertices[curVert].ny = 1.0f;
		m_pVertices[curVert].tu = (float)i / m_iNumDiv;
		m_pVertices[curVert++].tv = 0.0f;
	}

	
	// Intialize the vertices that make up the middle section
	for(i = 1; i<m_iNumDiv; i++)
	{
		theta += dTheta;	// Longitude angle
		phi = 0.0f;			// Latitude angle
		
		
		// Iterates m_iNumDiv+1 times because we need an extra vertex at
		// each latitude to map a texture completely around the sphere
		for(int j=0; j<m_iNumDiv+1; j++)
		{
			x = (float)cos(theta) * (float)cos(phi);
			y = (float)sin(theta);
			z = (float)cos(theta) * (float)sin(phi);
			m_pVertices[curVert++] = D3DVERTEX(m_fRadius * D3DVECTOR(x, y, z), D3DVECTOR(x, y, z),
							                (float)j/m_iNumDiv, (float)i/m_iNumDiv);

			phi += dPhi;
		}
	
	}


	// Initialize the bottom section vertices
	for(i = 0; i<m_iNumDiv; i++)
	{
		m_pVertices[curVert].x = m_pVertices[curVert].z = m_pVertices[curVert].nx
			                 = m_pVertices[curVert].nz = 0.0f;
		m_pVertices[curVert].y = -m_fRadius;
		m_pVertices[curVert].ny = -1.0f;
		m_pVertices[curVert].tu = (float)i / m_iNumDiv;
		m_pVertices[curVert++].tv = 1.0f;
	}
		
	return S_OK;
}

	// Set up indexed vertex indices
	// for using DrawIndexedPrimitive(VB/Strided)
HRESULT CDRGObjectSphere::SetupIndices( )
{

	int iCurLat, iCurLong;
				
	int curVert = 0;

	// Top section
	for(iCurLat=0; iCurLat<m_iNumDiv; iCurLat++)
	{

		m_pIndices[curVert++] = iCurLat;
		
		// Skip past the vertices at the top of the sphere (+m_iNumDiv)
		// and also skip to the next vertice (+1)
		m_pIndices[curVert++] = m_iNumDiv + (iCurLat + 1);

		// Skip past the vertices at the top of the sphere (+m_iNumDiv)
		m_pIndices[curVert++] = m_iNumDiv + iCurLat;	
	}

	// Middle section
	for(iCurLat=1; iCurLat<m_iNumDiv-1; iCurLat++)
	{
		for(iCurLong=0; iCurLong<m_iNumDiv; iCurLong++)
		{
			
			// There are m_iNumDiv vertices at each latitude so we need to 
			// multiply by that amount to skip to a specific latitude
			// *----* <- Each quadrilateral space is composed of 2 triangles
		    // | \ 2|    Triangle 1 is initialized after Triangle 2
			// |1  \|    
			// *----*    

			// first triangle of the quad area
			m_pIndices[curVert++] = (iCurLat * m_iNumDiv) + (iCurLat-1) + iCurLong;
			m_pIndices[curVert++] = ((iCurLat + 1) * m_iNumDiv) + iCurLat + iCurLong + 1;
			m_pIndices[curVert++] = ((iCurLat + 1) * m_iNumDiv) + iCurLat + iCurLong;

			// second triangle of the quad area
			m_pIndices[curVert++] = (iCurLat * m_iNumDiv) + (iCurLat-1) + iCurLong;
			m_pIndices[curVert++] = (iCurLat * m_iNumDiv) + (iCurLat-1) + iCurLong + 1;
			m_pIndices[curVert++] = ((iCurLat + 1) * m_iNumDiv) + iCurLat + iCurLong + 1;
		}
	}

	// Bottom section
	for(iCurLat = 0; iCurLat<m_iNumDiv; iCurLat++)
	{
		// Go to the second to last latitude (last is the bottom-most point)
		m_pIndices[curVert++] = ((m_iNumDiv * m_iNumDiv) - 2) + iCurLat;
		m_pIndices[curVert++] = ((m_iNumDiv * m_iNumDiv)-2) + (iCurLat + 1);	
		m_pIndices[curVert++] = ((m_iNumDiv * (m_iNumDiv + 1)) - 1) + iCurLat;
	}
	
	return S_OK;
}
