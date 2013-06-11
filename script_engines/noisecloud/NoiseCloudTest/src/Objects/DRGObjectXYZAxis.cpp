// DRGObjectXYZAxis.cpp - Created with DR&E AppWizard 1.0B9
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
// DRGObjectXYZAxis.cpp: definition of hte the XYZAxis Object, derived from the DRGObject base class.
// This class is used to contain a XYZAxis object, it's vertices, matrices, etc.
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
CDRGObjectXYZAxis::CDRGObjectXYZAxis()
{
	// Rest of stuff comes from the parents constructor
	
	// Pointer to sysmem copy of verts
	m_pVertices = new D3DVERTEX[22];
	m_pIndices = new WORD[22];
	m_iNumVertices = 22;	
	m_iNumIndices = 22;	

	m_d3dPrimType = D3DPT_LINELIST; 

	
}

// -----------------------------------------------------------------------
// DRGObject::~DRGObject()
//
// El Destructor
// -----------------------------------------------------------------------
CDRGObjectXYZAxis::~CDRGObjectXYZAxis()
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

HRESULT CDRGObjectXYZAxis::Init(	CDRGWrapper *pDRGWrapper,
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
	
	//primitive type is different for this object than the rest of them
	m_d3dPrimType = D3DPT_LINELIST;
	//m_dwVertexFlags = D3DFVF_VERTEX;
	
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
	
int GetTriangleCount()
{
	return 0;
}

	
HRESULT CDRGObjectXYZAxis::SetupVertices( )
{	
	m_pVertices[0].x = 0.0f;	m_pVertices[0].y = 0.0f;	m_pVertices[0].z = 0.0f; //line along x axis
	m_pVertices[1].x = 1.0f;	m_pVertices[1].y = 0.0f;	m_pVertices[1].z = 0.0f;
	m_pVertices[2].x = 0.0f;	m_pVertices[2].y = 0.0f;	m_pVertices[2].z = 0.0f; //line along y axis
	m_pVertices[3].x = 0.0f;	m_pVertices[3].y = 1.0f;	m_pVertices[3].z = 0.0f;
	m_pVertices[4].x = 0.0f;	m_pVertices[4].y = 0.0f;	m_pVertices[4].z = 0.0f; //line along z axis
	m_pVertices[5].x = 0.0f;	m_pVertices[5].y = 0.0f;	m_pVertices[5].z = 1.0f;
	//letter x
	m_pVertices[6].x = 1.1f;	m_pVertices[6].y = 0.0f;	m_pVertices[6].z = 0.0f;
	m_pVertices[7].x = 1.2f;	m_pVertices[7].y = 0.2f;	m_pVertices[7].z = 0.0f;	
	m_pVertices[8].x = 1.2f;	m_pVertices[8].y = 0.0f;	m_pVertices[8].z = 0.0f;
	m_pVertices[9].x = 1.1f;	m_pVertices[9].y = 0.2f;	m_pVertices[9].z = 0.0f;	
	//letter Y
	m_pVertices[10].x = 0.0f;	m_pVertices[10].y = 1.1f;	m_pVertices[10].z = 0.0f;	
	m_pVertices[11].x = 0.0f;	m_pVertices[11].y = 1.2f;	m_pVertices[11].z = 0.0f;	
	m_pVertices[12].x = 0.0f;	m_pVertices[12].y = 1.2f; 	m_pVertices[12].z = 0.0f;	
	m_pVertices[13].x = 0.1f;	m_pVertices[13].y = 1.3f;	m_pVertices[13].z = 0.0f;
	m_pVertices[14].x = 0.0f;	m_pVertices[14].y = 1.2f;	m_pVertices[14].z = 0.0f;
	m_pVertices[15].x = -0.1f;	m_pVertices[15].y = 1.3f;	m_pVertices[15].z = 0.0f;
	//letter z
	m_pVertices[16].x = 0.1f;	m_pVertices[16].y = 0.2f;	m_pVertices[16].z = 1.1f;
	m_pVertices[17].x = 0.2f;	m_pVertices[17].y = 0.2f;	m_pVertices[17].z = 1.1f;
	m_pVertices[18].x = 0.2f;	m_pVertices[18].y = 0.2f;	m_pVertices[18].z = 1.1f;
	m_pVertices[19].x = 0.1f;	m_pVertices[19].y = 0.0f;	m_pVertices[19].z = 1.1f;
	m_pVertices[20].x = 0.1f;	m_pVertices[20].y = 0.0f;	m_pVertices[20].z = 1.1f;
	m_pVertices[21].x = 0.2f;	m_pVertices[21].y = 0.0f;	m_pVertices[21].z = 1.1f;

	// common elements
	for (int i=0;i<11;i++)
	{
		m_pVertices[i].nx = 1.0f;	m_pVertices[i].ny = 1.0f;	m_pVertices[i].nz = 1.0f;
		m_pVertices[i+1].nx = 1.0f;	m_pVertices[i+1].ny = 1.0f;	m_pVertices[i+1].nz = 1.0f;
		m_pVertices[i].tu = 0.0f;
		m_pVertices[i].tv = 0.0f;	
		m_pVertices[i+1].tu = 1.0f;
		m_pVertices[i+1].tv = 1.0f;	
	}
		
	return S_OK;	
}

	// Set up indexed vertex indices
	// for using DrawIndexedPrimitive(VB/Strided)
HRESULT CDRGObjectXYZAxis::SetupIndices( )
{
	for (int i=0; i<22; i++)
	{
		m_pIndices[i] = i;
	}

	return S_OK;
}

// -----------------------------------------------------------------------
// HRESULT DRGObject::Render( DRGMatrix& drgMxMatrix, bool bRenderChilren = true )
//
//  This draws the object to the screen using DrawIndexedPrimitiveVB. 
// -----------------------------------------------------------------------
HRESULT CDRGObjectXYZAxis::Render( DRGMatrix& drgMxWorld, bool bRenderChildren)
{
	HRESULT hr;
	LPDIRECTDRAWSURFACE7	lpOldTexture;

	// Set the world matrix to the local matrix
	//m_MxLocalToWorld = m_MxLocalToWorld * drgMxWorld;
	m_MxLocalToWorld =  drgMxWorld * m_MxLocalToParent ;

	//if the object has children, and we want to render them, now is the time
	if (bRenderChildren)
	{
		CDRGObject	*pTempObject = m_pFirstChild;
		while (pTempObject != NULL)
		{
			hr |= pTempObject->Render(m_MxLocalToWorld, true);
			pTempObject = pTempObject->m_pNext;
		}
	}
	
	if (m_pWrapper && m_pWrapper->m_pD3DDevice7 && m_bVisible)
	{
		m_pWrapper->m_pD3DDevice7->SetTransform( D3DTRANSFORMSTATE_WORLD, &m_MxLocalToWorld.m_Mx );
		m_pWrapper->m_pD3DDevice7->GetTexture(0,&lpOldTexture);
		m_pWrapper->m_pD3DDevice7->SetTexture(0,NULL);
		m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);


		// Draw the object
		// the first 0 is for where to start in the VB. For now, we are assuming one VB per object
		hr = m_pWrapper->m_pD3DDevice7->DrawIndexedPrimitiveVB( m_d3dPrimType, 
																		 m_pIndexedVertexBuffer,
																		 0, 
																		 m_iNumVertices,
																		 m_pIndices, 
																		 m_iNumIndices, 
																		 0 );
		
		m_pWrapper->m_pD3DDevice7->SetTexture(0, lpOldTexture);
		m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_LIGHTING, TRUE);		
	}
	m_pWrapper->m_pD3DDevice7->SetTransform( D3DTRANSFORMSTATE_WORLD, &drgMxWorld.m_Mx );
		
	return S_OK;
}


