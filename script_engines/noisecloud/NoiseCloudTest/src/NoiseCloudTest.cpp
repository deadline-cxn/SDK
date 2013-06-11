// NoiseCloudTest.h - Created with DR&E AppWizard 1.0B9
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
// NoiseCloudTest.h: implementation the NoiseCloudTest class.
// This class is the "blank" class created by the DRG 3D Appwizard, where you will put your stuff
//
// ------------------------------------------------------------------------------------
//
// Author: Kim Pallister - Intel Developer Relations & Engineering Group
//
////////////////////////////////////////////////////////////////////////////////////////

#include "DRGShell.h"

#define	 TEXTURESIZE	256

// -----------------------------------------------------------------------
// CNoiseCloudTest::CNoiseCloudTest() 
// The NoiseCloudTest class constructor initializes some of it's member vars (which
// are instances of the DRGMatrix class
// -----------------------------------------------------------------------
CNoiseCloudTest::CNoiseCloudTest()
{
	//init general scene mgr vars
	//////////////////////////////
	m_pWrapper = NULL;

	m_MxWorld.SetIdentity();
	
	//set the camera at 0,0,-5; the target at 0,0,0 and the up vector at 0,1,0
	m_MxView.SetView( DRGVector( 0, 0.2f, -1 ), DRGVector( 0, 0, 0 ), DRGVector( 0, 1, 0 ) );
	
	//set the projection matrix to 1.02 field of view (60 degrees in radians), 1.33 aspect ratio 
	// (640x480), //the near clip plane to 0.2 and the far to clip plane 100
	m_MxProjection.SetProjection( 1.02f, 1.333f, 0.2f, 100.0f );

	m_pDefaultMaterial = NULL;
	m_bWireframe = false;
	
	// demo specific vars
	//////////////////////////////
	m_bAccelerateTime = true;
	m_iCurrentDemoShader = -1;
	
	m_fCloudCover = 0.34f; //0.0 means all cloud, while 0.25 means zero cloud
	m_fCloudDarkeningFactor = 0.2f;
	m_bDarkenClouds = false;

	m_dwSkyColor = 0xff3333dd; // dark blue
	m_dwFogColor = 0xffbcd7e7; // light blue

	m_vSunDirection = DRGVector(10000.0f, 20000.0f, -30000.0f);

	m_fUpdateInterval[0] = 7.0f;	//intervals at which noise octaves are updated (in seconds)
	m_fUpdateInterval[1] = 5.0f;
	m_fUpdateInterval[2] = 4.5f;
	m_fUpdateInterval[3] = 3.0f;

	m_fElapsedTime[0] = 0.0f;
	m_fElapsedTime[1] = 0.0f;
	m_fElapsedTime[2] = 0.0f;
	m_fElapsedTime[3] = 0.0f;

	m_fTotalElapsedTime = 0.0f;

	int  m_iNumSmoothingSteps = 2;

	//init all these ptrs to null. We'll set them up in a function later
	m_pShader0_DemoOctave0 = NULL;
	m_pShader1_DemoOctave1 = NULL;
	m_pShader2_DemoOctave2 = NULL;
	m_pShader3_DemoOctave3 = NULL;
	m_pShader4_InterpolateOctaves	= NULL;
	m_pShader5_CompileOctaves		= NULL;
	m_pShader6_DemoCompiledOctaves	= NULL;
	m_pShader7_CompiledOctavesSquared	= NULL;
	m_pShader8_FactorMinusNoiseSquared	= NULL;

	m_pShader9_GrayFactorMinusNoiseSquared = NULL;
	m_pShader10_BlendCloudsWithFB	= NULL;

	m_pShader11_BlendDarkenedCloudsWithFB	= NULL;

	m_pShader30_Ground		= NULL;
	m_pShader31_SunGlow		= NULL; 
	m_pShader32_SunGlare	= NULL; 

	// same goes for the ptrs to objects
	m_pSampleRootObj	= NULL;
	m_pSampleXYZAxisObj	= NULL;
	m_pSampleFloorPlaneObj	= NULL;
	m_pSkyObj	= NULL;
	m_pTerrainObj = NULL;

	// same goes for the ptrs to all the texture objects
	for (int oct = 0;oct<NUMOCTAVES;oct++)
	{
		m_pBasicNoiseTexture[oct] = NULL;				
		m_pBasicNoiseTextureVidmem[oct] = NULL;				
		m_pOnceSmoothedNoiseTexture[oct] = NULL;
		m_pTwiceSmoothedNoiseTexture[oct] = NULL;
		m_pPreviousSmoothedNoiseTexture[oct] = NULL;
		m_pCurrentInterpolatedNoiseTexture[oct] = NULL;
	}
	m_pCurrentCompiledOctavesNoiseTexture = NULL;					//for time t1, t2, to sum the four octaves
		
	m_pCompiledOctavesSquaredTexture = NULL;
	m_pFactorMinusCompiledNoiseTexture = NULL;
	m_pLightFactorMinusCompiledNoiseTexture = NULL;

	m_pGlareTexture = NULL;
	m_pGlowTexture = NULL;

	m_pGroundTexture = NULL;

	//this is a boring function to setup a bunch of TLVert arrays to hold
	// quads we'll use for the various rendering to textures
	InitTLVerts();

}

// -----------------------------------------------------------------------
// CNoiseCloudTest::~CNoiseCloudTest()
// destructor does nothing for now.
// -----------------------------------------------------------------------
CNoiseCloudTest::~CNoiseCloudTest()
{
	;//nothing needed here for now	
	
}

// -----------------------------------------------------------------------
// HRESULT CNoiseCloudTest::InitWorld( CDRGWrapper *pWrapper )
//
// -----------------------------------------------------------------------
HRESULT CNoiseCloudTest::InitWorld( CDRGWrapper *pWrapper )
{
	//
	// Make local copy of the pointer to the wrapper class
	//
	m_pWrapper = pWrapper;
	HRESULT hr;
	
	m_fCloudSpeed[0] = 0.006f ;
	m_fCloudSpeed[1] = 0.009f ;
	m_fCloudSpeed[2] = 0.013f ;
	m_fCloudSpeed[3] = 0.017f ;

	//
	// Create texture objects (the surfaces themselves get created later).
	//

	// sun glare (lens flare) texture	
	m_pGlareTexture = new CDRGTexture;
	if( m_pGlareTexture->CreateTexture( CBMPLoader( CFileStream( "Images\\glare.bmp" ) ),
						  (DWORD)0, (DWORD)0) == S_OK )
		OutputDebugString("Texture Object created. Surface to be created later /n");

	//sun glow (sky saturation) texture
	m_pGlowTexture = new CDRGTexture;
	if( m_pGlowTexture->CreateTexture( CBMPLoader( CFileStream( "Images\\glow.bmp" ) ),									    
						  (DWORD)0, (DWORD)0/*DRGTEXTR_32BITSPERPIXEL*/) == S_OK )
		OutputDebugString("Texture Object created. Surface to be created later /n");

	//All of the textures for the noise. These first ones are arrays of texture objects, where
	// the only difference between the different elements in the array is the size (256, 128, 64, 32)
	int size = TEXTURESIZE/(int)pow(2,NUMOCTAVES-1); //32;
	for (int oct = 0;oct<NUMOCTAVES;oct++)
	{			
		m_pBasicNoiseTexture[oct] = new CDRGTexture;
		hr = m_pBasicNoiseTexture[oct]->CreateTexture(m_pWrapper,size,size,16,(DWORD)0,DRGTEXTR_FORCEINSYSMEM/*DRGTEXTR_FORCEINAGP*/, NULL);

		m_pBasicNoiseTextureVidmem[oct] = new CDRGTexture;
		hr = m_pBasicNoiseTextureVidmem[oct]->CreateTexture(m_pWrapper,size,size,16,(DWORD)0,DWORD(0), NULL);

		m_pTwiceSmoothedNoiseTexture[oct] = new CDRGTexture;
		hr = m_pTwiceSmoothedNoiseTexture[oct]->CreateTexture(m_pWrapper,size,size,16,(DWORD)0,DRGTEXTR_RENDERTARGET /*| DRGTEXTR_32BITSPERPIXEL*/, NULL);


		m_pOnceSmoothedNoiseTexture[oct] = new CDRGTexture;
		hr = m_pOnceSmoothedNoiseTexture[oct]->CreateTexture(m_pWrapper,size,size,16,(DWORD)0,DRGTEXTR_RENDERTARGET /*| DRGTEXTR_32BITSPERPIXEL*/, NULL);

		m_pPreviousSmoothedNoiseTexture[oct] = new CDRGTexture;
		hr = m_pPreviousSmoothedNoiseTexture[oct]->CreateTexture(m_pWrapper,size,size,16,(DWORD)0,(DWORD)0 /*| DRGTEXTR_32BITSPERPIXEL*/, NULL);

		m_pCurrentInterpolatedNoiseTexture[oct] = new CDRGTexture;
		hr = m_pCurrentInterpolatedNoiseTexture[oct]->CreateTexture(m_pWrapper,size,size,16,(DWORD)0,DRGTEXTR_RENDERTARGET | DRGTEXTR_32BITSPERPIXEL, NULL);

		size *= 2;
	}
	
	m_pCurrentCompiledOctavesNoiseTexture = new CDRGTexture;					
	m_pCurrentCompiledOctavesNoiseTexture->CreateTexture(m_pWrapper,TEXTURESIZE,TEXTURESIZE,16,(DWORD)0,DRGTEXTR_RENDERTARGET | DRGTEXTR_32BITSPERPIXEL , NULL);
	
	m_pCompiledOctavesSquaredTexture = new CDRGTexture;					
	m_pCompiledOctavesSquaredTexture->CreateTexture(m_pWrapper,TEXTURESIZE,TEXTURESIZE,16,(DWORD)0,DRGTEXTR_RENDERTARGET | DRGTEXTR_32BITSPERPIXEL, NULL);
	
	m_pFactorMinusCompiledNoiseTexture = new CDRGTexture;					
	m_pFactorMinusCompiledNoiseTexture->CreateTexture(m_pWrapper,TEXTURESIZE,TEXTURESIZE,16,(DWORD)0,DRGTEXTR_RENDERTARGET | DRGTEXTR_32BITSPERPIXEL, NULL);

	m_pLightFactorMinusCompiledNoiseTexture = new CDRGTexture;					
	m_pLightFactorMinusCompiledNoiseTexture->CreateTexture(m_pWrapper,TEXTURESIZE,TEXTURESIZE,16,(DWORD)0,DRGTEXTR_RENDERTARGET | DRGTEXTR_32BITSPERPIXEL, NULL);

	
	m_pGroundTexture = new CDRGTexture;
	if( m_pGroundTexture->CreateTexture( CBMPLoader( CFileStream( "Images\\ground.bmp" ) ),									    
						  (DWORD)0, (DWORD)0/*DRGTEXTR_32BITSPERPIXEL*/) == S_OK )
		OutputDebugString("Texture Object created. Surface to be created later /n");


	//
	// create geometry (and dummy objects) in object hierarchy
	//

	m_pSampleRootObj		= new CDRGObject;
	m_pSampleRootObj->Init(pWrapper, NULL, NULL, 0, 0,						 
					DRGVector(0.0f,0.0f,0.0f), DRGVector(1.0f), DRGVector(0.0f,1.0f,0.0f), 0.0f,
					DRGVector(0.0f),  DRGVector(0.0f),  0.0f,
					0x0);

	
	m_pSampleXYZAxisObj		= new CDRGObjectXYZAxis;
	m_pSampleXYZAxisObj->Init(pWrapper,
					DRGVector(0.0f,0.0f,0.0f), DRGVector(0.1f), DRGVector(0.0f,1.0f,0.0f), 0.0f,
					DRGVector(0.0f), DRGVector(0.0f), 0.0f,
					0x0);
	m_pSampleRootObj->AddChild(m_pSampleXYZAxisObj);
	m_pSampleXYZAxisObj->m_bVisible = false;

	m_pSampleFloorPlaneObj	= new CDRGObjectPlane;
	m_pSampleFloorPlaneObj->Init(pWrapper, 1, 1.0f,
					DRGVector(0.0f,-1.0f,0.0f), DRGVector(1.0f), DRGVector(0.0f,1.0f,0.0f), 0.0f,
					DRGVector(0.0f), DRGVector(0.0f), 0.0f,
					0x0);
	m_pSampleRootObj->AddChild(m_pSampleFloorPlaneObj);

	m_pSampleFloorPlaneObj->m_bVisible = false;	

	//float height = 10.0f;
	float height = 12.5f;
	float htile = 3.0f;
	float vtile = 3.0f;
	float radius = 500.0f; //that's about all it can handle without changing the z

	m_pSkyObj	= new CDRGObjectSkyPlane;
	m_pSkyObj->Init(pWrapper, 20, radius,radius+height, htile, vtile, 
					DRGVector(0.0f,height,0.0f), DRGVector(1.0f), DRGVector(0.0f,1.0f,0.0f), 0.0f,
					DRGVector(0.0f), DRGVector(0.0f), 0.0f,
					0x0);
	m_pSampleRootObj->AddChild(m_pSkyObj); //may want to not add it later

	m_pTerrainObj	= new CDRGObject;
	m_pTerrainObj->Init(pWrapper,"images/terrain.x", DRGVector(2.0f,-2.0f,0.0f), DRGVector(0.6f,0.25f,0.5f), DRGVector(1.0f,0.0f,0.0f), -1.57f,
					DRGVector(0.0f), DRGVector(0.0f), 0.0f,
					0x0);
	m_pSampleRootObj->AddChild(m_pTerrainObj);

	//now we call target changed, which sets up device dependant items, like the texture DDsurfaces, etc
	TargetChanged();
	
	return S_OK;
}

// -----------------------------------------------------------------------
// HRESULT CNoiseCloudTest::DestroyWorld( void )
//
// -----------------------------------------------------------------------
HRESULT CNoiseCloudTest::DestroyWorld( void )
{
	return S_OK;
}

// -----------------------------------------------------------------------
// HRESULT CNoiseCloudTest::UpdateWorld( DWORD dwTime, int InputFlag )
//
// -----------------------------------------------------------------------
HRESULT CNoiseCloudTest::UpdateWorld()
{

	//Call the Update method of the object at the root of our hierarchy,
	// specifying to propogate the update down the hierarchy. This will
	// update all the objects positions/orientations according to their
	// velocities. By specifying the frame time, our animation speed is
	// independant of frame rate
	if(m_pSampleRootObj)
		m_pSampleRootObj->Update(m_pWrapper->m_fFrameTime, true);

	//update the texcoords to make the different octaves move at different speeds:
	for (int oct = 0;oct<NUMOCTAVES;oct++)
	{
		for (int v = 0;v<6; v++)
		{
			//we could probably scroll these in different directions for a slightly different effect
			m_pCompositRectsVerts[oct][v].tu += m_fCloudSpeed[oct];			
		}
	}

	//we update the octaves of noise at different intervals, thus the intervals being stored in an array
	for (int i=0;i<NUMOCTAVES;i++) //-1 to skip the 4th octave
	{
		//if its time to update the noise
		if(m_fElapsedTime[i] >= m_fUpdateInterval[i])
		{			
			m_fElapsedTime[i] = 0.0f;
			
			m_pPreviousSmoothedNoiseTexture[i]->ptexTexture->Blt(NULL, m_pTwiceSmoothedNoiseTexture[i]->ptexTexture,NULL,DDBLT_ASYNC ,NULL);
			
			InitBasicNoiseTexture(m_fTotalElapsedTime,i);
			SmoothNoiseTexture(i);			
		} 
	}

	//These steps are done at every frame
	///////////////////////////////

	//For each octave, come up with a texture that is a blend between the last and next update, according
	// to the current time
	InterpolateOctaves();

	//Take the four textures from InterpolateOctaves(), and compile them to one texture
	CompileFinalNoiseTexture();

	//update times (we have a counter for each octave because we reset them at different times
	for (i=0;i<NUMOCTAVES;i++)
	{
		m_fElapsedTime[i] += m_pWrapper->m_fFrameTime;
	}
	m_fTotalElapsedTime += m_pWrapper->m_fFrameTime;

	return S_OK;
}


////////////////////////////////////////////////////////////////
// 
// This just updates a single octave with salt and pepper noise.
// 
////////////////////////////////////////////////////////////////
HRESULT CNoiseCloudTest::InitBasicNoiseTexture( float time, int oct )
{
	
	int iTime = (int)time;
	
	//lock the textures in AGP and copy the data over;
	DDSURFACEDESC2 ddsd;
	ZeroMemory( &ddsd, sizeof(DDSURFACEDESC2) );
	ddsd.dwSize                 = sizeof(DDSURFACEDESC2);
	HRESULT hr;

	hr = m_pBasicNoiseTexture[oct]->ptexTexture->Lock(NULL,&ddsd,DDLOCK_WAIT, NULL);

	//calculate noise, plus all the stuff to accomodate various texture pixel
	// formats (cut and pasted from DXFrame)
	////////////////////////////////////////////////////////////////////
	
	//calculate shift increments (clever shift stuff courtesy of the MS DX dudes)
	DWORD dwRMask = ddsd.ddpfPixelFormat.dwRBitMask;
    DWORD dwGMask = ddsd.ddpfPixelFormat.dwGBitMask;
    DWORD dwBMask = ddsd.ddpfPixelFormat.dwBBitMask;
    DWORD dwAMask = ddsd.ddpfPixelFormat.dwRGBAlphaBitMask;

	DWORD dwRShiftL = 8, dwRShiftR = 0;
    DWORD dwGShiftL = 8, dwGShiftR = 0;
    DWORD dwBShiftL = 8, dwBShiftR = 0;
    DWORD dwAShiftL = 8, dwAShiftR = 0;

    DWORD dwMask;
    for( dwMask=dwRMask; dwMask && !(dwMask&0x1); dwMask>>=1 ) dwRShiftR++;
    for( ; dwMask; dwMask>>=1 ) dwRShiftL--;

    for( dwMask=dwGMask; dwMask && !(dwMask&0x1); dwMask>>=1 ) dwGShiftR++;
    for( ; dwMask; dwMask>>=1 ) dwGShiftL--;

    for( dwMask=dwBMask; dwMask && !(dwMask&0x1); dwMask>>=1 ) dwBShiftR++;
    for( ; dwMask; dwMask>>=1 ) dwBShiftL--;

    for( dwMask=dwAMask; dwMask && !(dwMask&0x1); dwMask>>=1 ) dwAShiftR++;
    for( ; dwMask; dwMask>>=1 ) dwAShiftL--;

	for( DWORD y=0; y<ddsd.dwHeight; y++ )
	{
		WORD*  p16 =  (WORD*)((BYTE*)ddsd.lpSurface + y*ddsd.lPitch);
		DWORD* p32 = (DWORD*)((BYTE*)ddsd.lpSurface + y*ddsd.lPitch);
       
		for( DWORD x=0; x<ddsd.dwWidth; x++ )
		{
			//Get the color info from the bitmap
			unsigned char intensity = (char)((1.0f+Noise3D(x,y,iTime)) * 128);
			DWORD r = intensity;
			DWORD g = intensity; 
			DWORD b = intensity;
			DWORD a = 0x000000ff;

			DWORD dr = ((r>>(dwRShiftL))<<dwRShiftR)&dwRMask;
            DWORD dg = ((g>>(dwGShiftL))<<dwGShiftR)&dwGMask;
            DWORD db = ((b>>(dwBShiftL))<<dwBShiftR)&dwBMask;
            DWORD da = ((a>>(dwAShiftL))<<dwAShiftR)&dwAMask;

			if( 32 == ddsd.ddpfPixelFormat.dwRGBBitCount )
                p32[x] = (DWORD)(dr+dg+db+da);
            else
                p16[x] = (WORD)(dr+dg+db+da);			
		}//end for x=width
		p16++;
		p32++;
	} //end for y=height

	hr = m_pBasicNoiseTexture[oct]->ptexTexture->Unlock(NULL);

	return hr;
}

////////////////////////////////////////////////////////////////
// 
// This blits the salt and pepper noise texture (in sysmem) to a 
// copy in vidmem, then renders it to another texture to smooth it
// (between 0 and 2 times, depending on app settings)
// 
////////////////////////////////////////////////////////////////
HRESULT CNoiseCloudTest::SmoothNoiseTexture(int oct)
{
	HRESULT hr;
	if (m_iNumSmoothingSteps > 0)
	{
		hr = m_pBasicNoiseTextureVidmem[oct]->ptexTexture->Blt(NULL, 
				m_pBasicNoiseTexture[oct]->ptexTexture,NULL,(DWORD)0,NULL);
		D3DVIEWPORT7	VPTemp, VPOld;
		m_pWrapper->m_pD3DDevice7->GetViewport(&VPOld);
		memcpy(&VPTemp,&VPOld,sizeof(D3DVIEWPORT7));
		VPTemp.dwWidth = m_pTwiceSmoothedNoiseTexture[oct]->m_ddsd.dwWidth;
		VPTemp.dwHeight = m_pTwiceSmoothedNoiseTexture[oct]->m_ddsd.dwHeight;


		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState( D3DRENDERSTATE_LIGHTING, FALSE);

		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_COLOROP, D3DTOP_DISABLE); 
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);


		if (m_iNumSmoothingSteps > 1) //if equal to two,
		{			
			m_pWrapper->m_pD3DDevice7->SetRenderTarget(m_pOnceSmoothedNoiseTexture[oct]->ptexTexture, 0);

			//Set the right size VP
			m_pWrapper->m_pD3DDevice7->SetViewport(&VPTemp);

			//Clear the texture surface.
			m_pWrapper->m_pD3DDevice7->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0 );

			//render the 4 octaves, using filtering to smooth them out
			m_pWrapper->m_pD3DDevice7->BeginScene();
	
			hr = m_pWrapper->m_pD3DDevice7->SetTexture(0,m_pBasicNoiseTextureVidmem[oct]->ptexTexture);
			hr = m_pWrapper->m_pD3DDevice7->SetTexture(1,NULL);
	
			hr = m_pWrapper->m_pD3DDevice7->DrawPrimitive(D3DPT_TRIANGLELIST,
		  										   D3DFVF_TLVERTEX,
												   m_pSmoothingRectsVerts[oct],
												   6,
												   NULL);			

			m_pWrapper->m_pD3DDevice7->EndScene();

			//second stage of smoothing
			hr = m_pWrapper->m_pD3DDevice7->SetRenderTarget(m_pTwiceSmoothedNoiseTexture[oct]->ptexTexture, 0);
			hr = m_pWrapper->m_pD3DDevice7->SetTexture(0,m_pOnceSmoothedNoiseTexture[oct]->ptexTexture);

			m_pWrapper->m_pD3DDevice7->BeginScene();		
	
			hr = m_pWrapper->m_pD3DDevice7->DrawPrimitive(D3DPT_TRIANGLELIST,
		  										   D3DFVF_TLVERTEX,
												   m_pSmoothingRectsVerts[oct],
												   6,
												   NULL);			

			m_pWrapper->m_pD3DDevice7->EndScene();
			
		}
		else //only one smoothing step
		{
			m_pWrapper->m_pD3DDevice7->SetRenderTarget(m_pTwiceSmoothedNoiseTexture[oct]->ptexTexture, 0);
			
			//Set the right size VP
			m_pWrapper->m_pD3DDevice7->SetViewport(&VPTemp);

			//Clear the texture surface.
			m_pWrapper->m_pD3DDevice7->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0 );

			//render the 4 octaves, using filtering to smooth them out
			m_pWrapper->m_pD3DDevice7->BeginScene();

			hr = m_pWrapper->m_pD3DDevice7->SetTexture(0,m_pBasicNoiseTextureVidmem[oct]->ptexTexture);
			hr = m_pWrapper->m_pD3DDevice7->SetTexture(1,NULL);
			hr = m_pWrapper->m_pD3DDevice7->DrawPrimitive(D3DPT_TRIANGLELIST,
		  										   D3DFVF_TLVERTEX,
												   m_pSmoothingRectsVerts[oct],
												   6,
												   NULL);			

			m_pWrapper->m_pD3DDevice7->EndScene();
		}
		//srt back to back buffer
		m_pWrapper->m_pD3DDevice7->SetRenderTarget(m_pWrapper->m_pBackBuffer, 0);
		m_pWrapper->m_pD3DDevice7->SetViewport(&VPOld);
	}
	else //if not smoothing, so just blit
	{
		hr = m_pTwiceSmoothedNoiseTexture[oct]->ptexTexture->Blt(NULL, 
								m_pBasicNoiseTexture[oct]->ptexTexture,NULL,(DWORD)0,NULL);		
	}

	return S_OK;
}

///////////////////////////////////////////////////////////////////////
//
// this function interpolates bewtween the last update to the smoothed
// octaves, and the one that was just done in the previous funtion.
//
///////////////////////////////////////////////////////////////////////
HRESULT CNoiseCloudTest::InterpolateOctaves()
{
	HRESULT hr;
	
	D3DVIEWPORT7	VPTemp, VPOld;
	m_pWrapper->m_pD3DDevice7->GetViewport(&VPOld);
	memcpy(&VPTemp,&VPOld,sizeof(D3DVIEWPORT7));

	for (int oct=0;oct<NUMOCTAVES;oct++)
	{
		//figure out what our interpolant value is
		float temp = 1.0f - m_fElapsedTime[oct] / m_fUpdateInterval[oct];
		DWORD color = D3DRGB(temp,temp,temp);
		m_pSmoothingRectsVerts[oct][0].dcColor = color;
		m_pSmoothingRectsVerts[oct][1].dcColor = color;
		m_pSmoothingRectsVerts[oct][2].dcColor = color;
		m_pSmoothingRectsVerts[oct][3].dcColor = color;
		m_pSmoothingRectsVerts[oct][4].dcColor = color;
		m_pSmoothingRectsVerts[oct][5].dcColor = color;
		

		VPTemp.dwWidth = m_pCurrentInterpolatedNoiseTexture[oct]->m_ddsd.dwWidth;
		VPTemp.dwHeight = m_pCurrentInterpolatedNoiseTexture[oct]->m_ddsd.dwHeight;

		m_pWrapper->m_pD3DDevice7->SetRenderTarget(m_pCurrentInterpolatedNoiseTexture[oct]->ptexTexture, 0);

		//Set the right size VP
		m_pWrapper->m_pD3DDevice7->SetViewport(&VPTemp);

		//Clear the texture surface.
		m_pWrapper->m_pD3DDevice7->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0 );

		//render the current octave
		m_pWrapper->m_pD3DDevice7->BeginScene();

		m_pWrapper->m_pD3DDevice7->ApplyStateBlock(m_pShader4_InterpolateOctaves->m_dwStateBlock);
		
		hr = m_pWrapper->m_pD3DDevice7->SetTexture(0,m_pPreviousSmoothedNoiseTexture[oct]->ptexTexture);

		hr = m_pWrapper->m_pD3DDevice7->DrawPrimitive(D3DPT_TRIANGLELIST,
													   D3DFVF_TLVERTEX,
													   m_pSmoothingRectsVerts[oct],
														6,
														NULL);

		m_pWrapper->m_pD3DDevice7->ApplyStateBlock(m_pShader4_InterpolateOctaves->m_pNext->m_dwStateBlock);

		hr = m_pWrapper->m_pD3DDevice7->SetTexture(0,m_pTwiceSmoothedNoiseTexture[oct]->ptexTexture);

		hr = m_pWrapper->m_pD3DDevice7->DrawPrimitive(D3DPT_TRIANGLELIST,
													   D3DFVF_TLVERTEX,
													   m_pSmoothingRectsVerts[oct],
													   6,
													   NULL);

		m_pWrapper->m_pD3DDevice7->EndScene();
	}
	//srt back to back buffer
	m_pWrapper->m_pD3DDevice7->SetRenderTarget(m_pWrapper->m_pBackBuffer, 0);
	m_pWrapper->m_pD3DDevice7->SetViewport(&VPOld);
	
	return S_OK;
}

	
////////////////////////////////////////////////////////////////////////
// This function does several things:
//
// 1. Compile the four octaves: Takes the four textures we created by 
// interpolating in the last function, and blends them together as:
//
//		1/2 Octave0 + 1/4 Octave1 + 1/8 Octave2 + 1/16 Octave3
//
// 2. Takes that texture, and squares it
// 3. subtracts that from some cloudcover value to derive a cloudtexture
// 4. subracts the same thing from a different value, to derive a slightly
//    more clamped version to use for cloud darkening
//////////////////////////////////////////////////////////////////////////
HRESULT CNoiseCloudTest::CompileFinalNoiseTexture()
{
	HRESULT hr;
	
	D3DVIEWPORT7	VPTemp, VPOld;
	m_pWrapper->m_pD3DDevice7->GetViewport(&VPOld);
	memcpy(&VPTemp,&VPOld,sizeof(D3DVIEWPORT7));
	VPTemp.dwWidth = m_pCurrentCompiledOctavesNoiseTexture->m_ddsd.dwWidth;
	VPTemp.dwHeight = m_pCurrentCompiledOctavesNoiseTexture->m_ddsd.dwHeight;

	m_pWrapper->m_pD3DDevice7->SetRenderTarget(m_pCurrentCompiledOctavesNoiseTexture->ptexTexture, 0);

	//Set the right size VP
	m_pWrapper->m_pD3DDevice7->SetViewport(&VPTemp);

	//Clear the texture surface.
	m_pWrapper->m_pD3DDevice7->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0 );

	//render the 4 octaves
	m_pWrapper->m_pD3DDevice7->BeginScene();
	
	m_pWrapper->m_pD3DDevice7->ApplyStateBlock(m_pShader5_CompileOctaves->m_dwStateBlock);
	
	// This will compile the 4 interpolated textures from above. Since the verts have a diffuse color of 0.5
	// we end up with :
	// 
	//		0.5 *(Octave0 + (0.5 * Octave1+ (0.5 * Octave2+ (0.5 * Octave3))))
	//
	//				OR:
	//
	//		1/2 Octave0 + 1/4 Octave1 + 1/8 Octave2 + 1/16 Octave3
	//
	// (Actually, since the series isn't expanded out forever, and since those numbers totaled to 0.937, I compensated by making
	//  the vertex color 0.5/0.937, or 0.53. You could probably tweak that up a little to saturate it if so desired.

	for (int oct=NUMOCTAVES-1; oct>=0;oct--) //not updating the smallest octave here
	{
		hr = m_pWrapper->m_pD3DDevice7->SetTexture(0,m_pCurrentInterpolatedNoiseTexture[oct]->ptexTexture);
	
		hr = m_pWrapper->m_pD3DDevice7->DrawPrimitive(D3DPT_TRIANGLELIST,
													   D3DFVF_TLVERTEX,
													   m_pCompositRectsVerts[oct],
													   6,
													   NULL);
	}

	m_pWrapper->m_pD3DDevice7->EndScene();

	/////////////////////////////
	//DO cloudcover squared
	///////////////////////////////


	m_pWrapper->m_pD3DDevice7->SetRenderTarget(m_pCompiledOctavesSquaredTexture->ptexTexture, 0);

	// WARNING: I AM ASSUMING THE VP IS THE SAME SIZE (as the previous step)
	//m_pWrapper->m_pD3DDevice7->SetViewport(&VPTemp);

	m_pWrapper->m_pD3DDevice7->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0 );

	m_pWrapper->m_pD3DDevice7->BeginScene();

	m_pWrapper->m_pD3DDevice7->ApplyStateBlock(m_pShader7_CompiledOctavesSquared->m_dwStateBlock);

	hr = m_pWrapper->m_pD3DDevice7->DrawPrimitive(D3DPT_TRIANGLELIST,
												   D3DFVF_TLVERTEX,
												   m_pFinalCompositeRectVerts,
												   6,
												   NULL);


	m_pWrapper->m_pD3DDevice7->EndScene();

	/////////////////////////////
	//DO (cloudcover) MINUS NOISE
	///////////////////////////////
	m_pWrapper->m_pD3DDevice7->SetRenderTarget(m_pFactorMinusCompiledNoiseTexture->ptexTexture, 0);

	// WARNING: ASSUMES THE VP IS THE SAME SIZE
	//m_pWrapper->m_pD3DDevice7->SetViewport(&VPTemp);

	m_pWrapper->m_pD3DDevice7->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0 );

	m_pWrapper->m_pD3DDevice7->BeginScene();

	m_pWrapper->m_pD3DDevice7->ApplyStateBlock(m_pShader8_FactorMinusNoiseSquared->m_dwStateBlock);
	for (int i=0;i<6;i++)
		m_pFinalCompositeRectVerts[i].dcColor = D3DRGB(m_fCloudCover,m_fCloudCover,m_fCloudCover);

	hr = m_pWrapper->m_pD3DDevice7->DrawPrimitive(D3DPT_TRIANGLELIST,
												   D3DFVF_TLVERTEX,
												   m_pFinalCompositeRectVerts,
												   6,
												   NULL);

	for ( i=0;i<6;i++)
		m_pFinalCompositeRectVerts[i].dcColor = D3DRGB(1.0,1.0,1.0f);


	m_pWrapper->m_pD3DDevice7->EndScene();

	/////////////////////////////
	//DO (lightfactor) MINUS NOISE
	///////////////////////////////
	if (m_bDarkenClouds)
	{
		m_pWrapper->m_pD3DDevice7->SetRenderTarget(m_pLightFactorMinusCompiledNoiseTexture->ptexTexture, 0);

		// WARNING: ASSUMES THE VP IS THE SAME SIZE AS IN THE LAST STEP
		//m_pWrapper->m_pD3DDevice7->SetViewport(&VPTemp);

		m_pWrapper->m_pD3DDevice7->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xff000000, 1.0f, 0 );

		m_pWrapper->m_pD3DDevice7->BeginScene();

		m_pWrapper->m_pD3DDevice7->ApplyStateBlock(m_pShader8_FactorMinusNoiseSquared->m_dwStateBlock);
	
		float darkcolor = m_fCloudCover + m_fCloudDarkeningFactor;
		if (darkcolor>1.0f)
			darkcolor = 1.0f;

		for (i=0;i<6;i++)
			m_pFinalCompositeRectVerts[i].dcColor = D3DRGB(darkcolor, darkcolor, darkcolor);

		hr = m_pWrapper->m_pD3DDevice7->DrawPrimitive(D3DPT_TRIANGLELIST,
												   D3DFVF_TLVERTEX,
												   m_pFinalCompositeRectVerts,
												   6,
												   NULL);

		for ( i=0;i<6;i++)
			m_pFinalCompositeRectVerts[i].dcColor = D3DRGB(1.0,1.0,1.0f);

		m_pWrapper->m_pD3DDevice7->EndScene();
	} //end if darken clouds 

	///////////////////////////
	//srt back to back buffer
	m_pWrapper->m_pD3DDevice7->SetRenderTarget(m_pWrapper->m_pBackBuffer, 0);
	m_pWrapper->m_pD3DDevice7->SetViewport(&VPOld);
	
	return S_OK;
}

// -----------------------------------------------------------------------
// HRESULT CNoiseCloudTest::RenderWorld( void )
//
// -----------------------------------------------------------------------
HRESULT CNoiseCloudTest::RenderWorld( void )
{
	HRESULT hr;

	//Just for debugging
	if (m_bWireframe)
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_WIREFRAME);

	//First, we'll clear the background to the sky color, 
	D3DRECT rcClear;
	
	rcClear.x1 = 0;
	rcClear.y1 = 0;
	rcClear.x2 = m_pWrapper->m_dwRenderWidth;
	rcClear.y2 = m_pWrapper->m_dwRenderHeight;

	hr = m_pWrapper->m_pD3DDevice7->Clear( 1, &rcClear, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
							m_dwSkyColor, 
							1.0f,	/*z value*/
							0		/*stencil value*/
							);

	
	// Now we need to add a glow around the sun. which will represent the scattering of sunlight you see around the sun.
	
	DRGVector vSunInScreenSpace = Normalize(m_vSunDirection * m_MxView);

	if (vSunInScreenSpace.z > 0.0f)
	{
	
		float size = m_pWrapper->m_dwRenderWidth * 0.1f;
		
		float xpos = m_pWrapper->m_dwRenderWidth*0.5f  + m_pWrapper->m_dwRenderWidth * vSunInScreenSpace.x;
		float ypos = m_pWrapper->m_dwRenderHeight*0.5f - m_pWrapper->m_dwRenderHeight * (vSunInScreenSpace.y/1.33f); //aspect ratio
	
		m_pSunglowRectVerts[0].sx = xpos - size;	m_pSunglowRectVerts[0].sy = ypos - size;	
		m_pSunglowRectVerts[1].sx = xpos + size;	m_pSunglowRectVerts[1].sy = ypos + size;
		m_pSunglowRectVerts[2].sx = xpos - size;	m_pSunglowRectVerts[2].sy = ypos + size;	
		m_pSunglowRectVerts[3].sx = xpos - size;	m_pSunglowRectVerts[3].sy = ypos - size;	
		m_pSunglowRectVerts[4].sx = xpos + size;	m_pSunglowRectVerts[4].sy = ypos - size;	
		m_pSunglowRectVerts[5].sx = xpos + size;	m_pSunglowRectVerts[5].sy = ypos + size;	

		hr = m_pWrapper->m_pD3DDevice7->ApplyStateBlock(m_pShader31_SunGlow->m_dwStateBlock);
		
					
		hr = m_pWrapper->m_pD3DDevice7->DrawPrimitive(D3DPT_TRIANGLELIST,
													   D3DFVF_TLVERTEX,
													   m_pSunglowRectVerts,												   
													   6,
													   NULL);
	}

	//////
	// Now that we've drawn the sky background color and added the glow, 
	// we can enable vertex fog (for haze), We want to switch on fog when 
	// drawing the world, but off when draw all the SRT stuff to generate 
	// the clouds between frames
	/////
	hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_FOGENABLE, TRUE);


	
	// Draw the ground, sky, etc
	if(m_pSampleRootObj)
		hr = m_pSampleRootObj->Render(m_MxWorld,true);


	// Switch fog off until the next frame
	hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_FOGENABLE, FALSE);
	
	//now draw the lens flare
	if (vSunInScreenSpace.z > 0.0f) // no need to draw the lens flare if the sun is behind us
	{
		float size = m_pWrapper->m_dwRenderWidth * 0.15f;

		float xpos = m_pWrapper->m_dwRenderWidth*0.5f  + m_pWrapper->m_dwRenderWidth * vSunInScreenSpace.x;
		float ypos = m_pWrapper->m_dwRenderHeight*0.5f - m_pWrapper->m_dwRenderHeight * (vSunInScreenSpace.y/1.33f);
		
		float sunu = 0.5f + m_vSunDirection.x / Magnitude(m_vSunDirection);
		float sunv = 0.5f + (/*1.0f - */m_vSunDirection.y / Magnitude(m_vSunDirection));

		//the above U and V would be enough if we were mapping the cloud texture once across the whole sky. However, I know that were are tiling it
		// across the sky (3 time in this case), so we need to do a little more math.
		sunu *= 3.0f; //tile factor used on sky
		sunv *= 3.0f; //tile factor used on sky

		sunu = sunu - (float)((int)sunu);
		sunv = sunv - (float)((int)sunv);

			
		m_pSunglareRectVerts[0].sx = xpos - size;	m_pSunglareRectVerts[0].sy = ypos - size;	
		m_pSunglareRectVerts[1].sx = xpos + size;	m_pSunglareRectVerts[1].sy = ypos + size;
		m_pSunglareRectVerts[2].sx = xpos - size;	m_pSunglareRectVerts[2].sy = ypos + size;	
		m_pSunglareRectVerts[3].sx = xpos - size;	m_pSunglareRectVerts[3].sy = ypos - size;	
		m_pSunglareRectVerts[4].sx = xpos + size;	m_pSunglareRectVerts[4].sy = ypos - size;	
		m_pSunglareRectVerts[5].sx = xpos + size;	m_pSunglareRectVerts[5].sy = ypos + size;	
		
		m_pSunglareRectVerts[0].tu = sunu;	m_pSunglareRectVerts[0].tv = sunv;	
		m_pSunglareRectVerts[1].tu = sunu;	m_pSunglareRectVerts[1].tv = sunv;
		m_pSunglareRectVerts[2].tu = sunu;	m_pSunglareRectVerts[2].tv = sunv;	
		m_pSunglareRectVerts[3].tu = sunu;	m_pSunglareRectVerts[3].tv = sunv;	
		m_pSunglareRectVerts[4].tu = sunu;	m_pSunglareRectVerts[4].tv = sunv;	
		m_pSunglareRectVerts[5].tu = sunu;	m_pSunglareRectVerts[5].tv = sunv;	
	
		
		hr = m_pWrapper->m_pD3DDevice7->ApplyStateBlock(m_pShader32_SunGlare->m_dwStateBlock);
		
			
		hr = m_pWrapper->m_pD3DDevice7->DrawPrimitive(D3DPT_TRIANGLELIST,
													   (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_SPECULAR  | D3DFVF_TEX2),
													   m_pSunglareRectVerts,												   
													   6,
													   NULL);	

		//I know my shader messed this up, so I'm resetting it here
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_TEXCOORDINDEX, 0);
		
	}


	// If we were in wireframe, I want to hand it back to the wrapper in solid fill, so that the text will still get drawn right
	if (m_bWireframe)
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_FILLMODE, D3DFILL_SOLID);

	return S_OK;
}


// -----------------------------------------------------------------------
// HRESULT CNoiseCloudTest::TargetChanged( void )
//
// -----------------------------------------------------------------------
HRESULT CNoiseCloudTest::TargetChanged( void )
{
	HRESULT hr;

	m_fTotalElapsedTime = 0.0f;

	//
	// This indicates that the devices of the DRGWrapper class have changed.
	// If you have anything that depends on the current rendering device, you'll
	// have to re-initialize it here
	//
	m_pWrapper->m_pD3DDevice7->SetRenderState( D3DRENDERSTATE_DITHERENABLE, TRUE );

	m_MxProjection.SetProjection( 1.57f, 
		(float)m_pWrapper->m_dwRenderWidth/(float)m_pWrapper->m_dwRenderHeight,
		0.2f, 100.0f );

	m_pWrapper->m_pD3DDevice7->SetTransform( D3DTRANSFORMSTATE_WORLD, &m_MxWorld.m_Mx );
	m_pWrapper->m_pD3DDevice7->SetTransform( D3DTRANSFORMSTATE_VIEW, &m_MxView.m_Mx );
	m_pWrapper->m_pD3DDevice7->SetTransform( D3DTRANSFORMSTATE_PROJECTION, &m_MxProjection.m_Mx );

	
	hr = SetupDeviceDependantItems();

	hr = SetupShaders();

	return hr;
}

HRESULT CNoiseCloudTest::SetupDeviceDependantItems(void)
{
	HRESULT hr;

	//This can be placed back in (along with the one after all the Restore()'s to 
	// check how much vidmem is used
	/*
	char	msg[255];
	DDSCAPS2 DDSCaps;  
	DWORD    dwTotal;   
	DWORD    dwFree;
	
	DDSCaps.dwCaps = 0;
	DDSCaps.dwCaps2 = 0;
	DDSCaps.dwCaps3 = 0;
	DDSCaps.dwCaps4 = 0;
    
	m_pWrapper->m_pDD7->GetAvailableVidMem(&DDSCaps, &dwTotal, &dwFree );
	sprintf(msg,"Total vidmem %d. Mem available before texture creation %d", dwTotal, dwFree);
	MessageBox( NULL, msg, WINDOWNAME, MB_ICONERROR|MB_OK );
	*/
	
	//Restore textures
	for (int oct = 0;oct<NUMOCTAVES;oct++)
	{			
		if(m_pBasicNoiseTexture[oct])
			m_pBasicNoiseTexture[oct]->Restore(m_pWrapper->m_pD3DDevice7);

		if(m_pBasicNoiseTextureVidmem[oct])
			m_pBasicNoiseTextureVidmem[oct]->Restore(m_pWrapper->m_pD3DDevice7);

		if(m_pTwiceSmoothedNoiseTexture[oct])
			m_pTwiceSmoothedNoiseTexture[oct]->Restore(m_pWrapper->m_pD3DDevice7);

		if(m_pOnceSmoothedNoiseTexture[oct])
			m_pOnceSmoothedNoiseTexture[oct]->Restore(m_pWrapper->m_pD3DDevice7);

		if(m_pPreviousSmoothedNoiseTexture[oct])
			m_pPreviousSmoothedNoiseTexture[oct]->Restore(m_pWrapper->m_pD3DDevice7);

		if(m_pCurrentInterpolatedNoiseTexture[oct])
			m_pCurrentInterpolatedNoiseTexture[oct]->Restore(m_pWrapper->m_pD3DDevice7);
	}

	if(m_pCurrentCompiledOctavesNoiseTexture)
		m_pCurrentCompiledOctavesNoiseTexture->Restore(m_pWrapper->m_pD3DDevice7);		
	
	if (m_pCompiledOctavesSquaredTexture)
		m_pCompiledOctavesSquaredTexture->Restore(m_pWrapper->m_pD3DDevice7);

	if (m_pFactorMinusCompiledNoiseTexture)
		m_pFactorMinusCompiledNoiseTexture->Restore(m_pWrapper->m_pD3DDevice7);

	if (m_pLightFactorMinusCompiledNoiseTexture)
		m_pLightFactorMinusCompiledNoiseTexture->Restore(m_pWrapper->m_pD3DDevice7);

	//managed textures
	if (m_pGlareTexture)
		m_pGlareTexture->Restore(m_pWrapper->m_pD3DDevice7);

	if (m_pGlowTexture)
		m_pGlowTexture->Restore(m_pWrapper->m_pD3DDevice7);

	if (m_pGroundTexture)
		m_pGroundTexture->Restore(m_pWrapper->m_pD3DDevice7);

	/*
	m_pWrapper->m_pDD7->GetAvailableVidMem(&DDSCaps, &dwTotal, &dwFree );
	sprintf(msg,"Total vidmem %d. Mem available after texture creation %d", dwTotal, dwFree);
	MessageBox( NULL, msg, WINDOWNAME, MB_ICONERROR|MB_OK );
	*/


	//This rebuild's all the VBs. Necesary, since we may have switched from a non-T&L 
	// device to one with T&L or vice versa
	if(m_pSampleRootObj)
		hr = m_pSampleRootObj->TargetChanged(m_pWrapper, true);

	//create a default material
	if (m_pDefaultMaterial)
		delete m_pDefaultMaterial;
	m_pDefaultMaterial = new D3DMATERIAL7;
	ZeroMemory( m_pDefaultMaterial, sizeof(D3DMATERIAL7) );
  
	m_pDefaultMaterial->ambient.r = 0.2f;    m_pDefaultMaterial->ambient.g = 0.2f;    m_pDefaultMaterial->ambient.b = 0.2f;
	m_pDefaultMaterial->diffuse.r = 1.0f;    m_pDefaultMaterial->diffuse.g = 1.0f;    m_pDefaultMaterial->diffuse.b = 1.0f;
	m_pDefaultMaterial->specular.r = 0.0f;    m_pDefaultMaterial->specular.g = 0.0f;    m_pDefaultMaterial->specular.b = 0.0f;

	if( FAILED( hr = m_pWrapper->m_pD3DDevice7->SetMaterial(m_pDefaultMaterial) ) )
		return hr;

	// Set up a default light
    D3DLIGHT7 light;
    ZeroMemory( &light, sizeof(light) );
    light.dltType       = D3DLIGHT_DIRECTIONAL;
    light.dcvDiffuse.r  = 1.0f;
    light.dcvDiffuse.g  = 1.0f;
    light.dcvDiffuse.b  = 1.0f;
	light.dcvAmbient.r  = 0.0f;
    light.dcvAmbient.g  = 0.0f;
    light.dcvAmbient.b  = 0.0f;
    
    light.dvRange       = D3DLIGHT_RANGE_MAX;
    
    light.dvDirection    = D3DVECTOR( 1.0f, 2.0f, -3.0f);

    // Set the light
    if( FAILED(m_pWrapper->m_pD3DDevice7->SetLight( 0, &light )))
		return hr;

	hr = m_pWrapper->m_pD3DDevice7->LightEnable( 0, TRUE );
	
    hr = m_pWrapper->m_pD3DDevice7->SetRenderState( D3DRENDERSTATE_LIGHTING, TRUE );
	
	hr = m_pWrapper->m_pD3DDevice7->SetRenderState( D3DRENDERSTATE_AMBIENT, 0xff333333 ) ;
	
	hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_COLORVERTEX, TRUE); 
	
	hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState( 0, D3DTSS_MINFILTER, D3DTFN_LINEAR );
	hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState( 0, D3DTSS_MAGFILTER, D3DTFN_LINEAR );

	hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState( 1, D3DTSS_MINFILTER, D3DTFN_LINEAR );
	hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState( 1, D3DTSS_MAGFILTER, D3DTFN_LINEAR );

	hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState( 2, D3DTSS_MINFILTER, D3DTFN_LINEAR );
	hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState( 2, D3DTSS_MAGFILTER, D3DTFN_LINEAR );

	float fFogStart = 25.0f;
	float fFogEnd = 65.0f;

	hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_FOGCOLOR, m_dwFogColor);
	hr = m_pWrapper->m_pD3DDevice7->SetRenderState( D3DRENDERSTATE_FOGSTART, *((LPDWORD) (&fFogStart)) );
	hr = m_pWrapper->m_pD3DDevice7->SetRenderState( D3DRENDERSTATE_FOGEND, *((LPDWORD) (&fFogEnd)) );
	hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_FOGVERTEXMODE, D3DFOG_LINEAR);  


	return S_OK;
}

///////////////
// SetupShaders
//
// This is where I set up all the stateblocks we'll be using. Some are just for demo 
// purposes, to demo the various stages of the process, while the rest are either part of
// the procedural cloud process, or are used on other objects in the scene.
//
///////////////
HRESULT CNoiseCloudTest::SetupShaders()
{
	HRESULT hr;

	DWORD numpasses;

	hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_ALPHAOP, D3DTOP_SELECTARG1); 
	hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_ALPHAARG1, D3DTA_TEXTURE); 
	hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_ALPHAARG2, D3DTA_DIFFUSE); 

	hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_ALPHAOP, D3DTOP_SELECTARG1); 
	hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_ALPHAARG1, D3DTA_TEXTURE); 
	hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_ALPHAARG2, D3DTA_CURRENT); 

	hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(2,D3DTSS_ALPHAOP, D3DTOP_SELECTARG1); 
	hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(2,D3DTSS_ALPHAARG1, D3DTA_TEXTURE); 
	hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(2,D3DTSS_ALPHAARG2, D3DTA_CURRENT); 

	////////////////////////////////////////////////////////////
	// For demo purposes, a shader to demo octave 0 of the noise
	////////////////////////////////////////////////////////////
	m_pShader0_DemoOctave0	= new CDRGShader("Noise Octave 0");
	hr = m_pWrapper->m_pD3DDevice7->BeginStateBlock();
		hr = m_pWrapper->m_pD3DDevice7->SetTexture(0,m_pCurrentInterpolatedNoiseTexture[0]->ptexTexture);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE); 
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG2, D3DTA_DIFFUSE); 
	
		hr = m_pWrapper->m_pD3DDevice7->SetTexture(1,NULL);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_COLOROP, D3DTOP_DISABLE); 
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_COLORARG1, D3DTA_TEXTURE); 
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_COLORARG2, D3DTA_DIFFUSE); 
	
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);

	hr = m_pWrapper->m_pD3DDevice7->EndStateBlock(&m_pShader0_DemoOctave0->m_dwStateBlock);

	hr = m_pWrapper->m_pD3DDevice7->ValidateDevice(&numpasses);
	if (hr != D3D_OK)
	{
		OutputDebugString("WARNING: SHADER 0 DIDN'T VALIDATE!\n");
		CheckResult(hr);
	}
	////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////
	// For demo purposes, a shader to display octave 1 of the noise
	////////////////////////////////////////////////////////////
	m_pShader1_DemoOctave1	= new CDRGShader("Noise Octave 1");
	hr = m_pWrapper->m_pD3DDevice7->BeginStateBlock();
		hr = m_pWrapper->m_pD3DDevice7->SetTexture(0,m_pCurrentInterpolatedNoiseTexture[1]->ptexTexture);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE); 

		hr = m_pWrapper->m_pD3DDevice7->SetTexture(1,NULL);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_COLOROP, D3DTOP_DISABLE); 
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_ALPHAOP, D3DTOP_DISABLE); 

	
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);

	hr = m_pWrapper->m_pD3DDevice7->EndStateBlock(&m_pShader1_DemoOctave1->m_dwStateBlock);

	hr = m_pWrapper->m_pD3DDevice7->ValidateDevice(&numpasses);
	if (hr != D3D_OK)
	{
		OutputDebugString("WARNING: SHADER 1 DIDN'T VALIDATE!\n");
		CheckResult(hr);
	}
	////////////////////////////////////////////////////////////
	
	////////////////////////////////////////////////////////////
	// For demo purposes, a shader to display octave 2 of the noise
	////////////////////////////////////////////////////////////
	m_pShader2_DemoOctave2	= new CDRGShader("Noise Octave 2");
	hr = m_pWrapper->m_pD3DDevice7->BeginStateBlock();
		hr = m_pWrapper->m_pD3DDevice7->SetTexture(0,m_pCurrentInterpolatedNoiseTexture[2]->ptexTexture);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE); 

		hr = m_pWrapper->m_pD3DDevice7->SetTexture(1,NULL);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_COLOROP, D3DTOP_DISABLE); 
			
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);

		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);

	hr = m_pWrapper->m_pD3DDevice7->EndStateBlock(&m_pShader2_DemoOctave2->m_dwStateBlock);

	hr = m_pWrapper->m_pD3DDevice7->ValidateDevice(&numpasses);
	if (hr != D3D_OK)
	{
		OutputDebugString("WARNING: SHADER 2 DIDN'T VALIDATE!\n");
		CheckResult(hr);
	}
	////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////
	// For demo purposes, a shader to display octave 3 of the noise
	////////////////////////////////////////////////////////////
	m_pShader3_DemoOctave3	= new CDRGShader("Noise Octave 3");
	hr = m_pWrapper->m_pD3DDevice7->BeginStateBlock();
		hr = m_pWrapper->m_pD3DDevice7->SetTexture(0,m_pCurrentInterpolatedNoiseTexture[3]->ptexTexture);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE); 
	
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_COLOROP, D3DTOP_DISABLE); 
		hr = m_pWrapper->m_pD3DDevice7->SetTexture(1,NULL);
		
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);

	hr = m_pWrapper->m_pD3DDevice7->EndStateBlock(&m_pShader3_DemoOctave3->m_dwStateBlock);

	hr = m_pWrapper->m_pD3DDevice7->ValidateDevice(&numpasses);
	if (hr != D3D_OK)
	{
		OutputDebugString("WARNING: SHADER 3 DIDN'T VALIDATE!\n");
		CheckResult(hr);
	}
	////////////////////////////////////////////////////////////
	
	////////////////////////////////////////////////////////////
	// This shader is used to interpolate between each of the
	// octaves at time0 and time1, using two passes, one pass 
	// modulating with diffuse color, one with inv diffuse. The 
	// diffuse color is modified according to the time between 
	// updates. This gives:
	//
	//     Color = ColorT0*Diffuse + ColorT1*InvDiffuse
	//
	// BECAUSE WE USE THIS SHADER WITH SEVERAL TEXTURES, WE WILL 
	// NOT SET THE TEXTURES HERE, BUT INLINE AT RENDER TIME INSTEAD
	////////////////////////////////////////////////////////////
	m_pShader4_InterpolateOctaves	= new CDRGShader("Shader4 - 1st pass of interpolation shader");
	CDRGShader	*tempshader = new CDRGShader("Shader4 - 2nd pass of interpolation shader");

	hr = m_pWrapper->m_pD3DDevice7->BeginStateBlock();

		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_MODULATE);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		
		hr = m_pWrapper->m_pD3DDevice7->SetTexture(1,NULL);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_COLOROP, D3DTOP_DISABLE); 
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
		
	hr = m_pWrapper->m_pD3DDevice7->EndStateBlock(&m_pShader4_InterpolateOctaves->m_dwStateBlock);
	
	hr = m_pWrapper->m_pD3DDevice7->BeginStateBlock();
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_MODULATE);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG2, D3DTA_DIFFUSE | D3DTA_COMPLEMENT);
		
		hr = m_pWrapper->m_pD3DDevice7->SetTexture(1,NULL);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_COLOROP, D3DTOP_DISABLE); 
		
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_ONE);

		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);

	hr = m_pWrapper->m_pD3DDevice7->EndStateBlock(&tempshader->m_dwStateBlock);
	
	hr = m_pShader4_InterpolateOctaves->AddChild(tempshader);

	hr = m_pWrapper->m_pD3DDevice7->ValidateDevice(&numpasses);
	if (hr != D3D_OK)
	{
		OutputDebugString("WARNING: SHADER 4  DIDN'T VALIDATE!\n");
		CheckResult(hr);
	}
	////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////
	// This shader is used to accumulate the four interpolated 
	// noise octaves. The vertices have a color of 0.5, and the 
	// multi-pass blend mode gives:
	//
	// Color = FrameBuffer*0.5 + Color[Octave];
	//
	// When rendered multi-pass from the highest frequency octave
	// to the lowest, we end up with:
	//
	// Color = 1/2 Octave0 + 1/4 Octave1 + 1/8 Octave2 + 1/16 Octave3 + ....  
	//
	////////////////////////////////////////////////////////////
	m_pShader5_CompileOctaves	= new CDRGShader("Shader5 - compilation of 4 octaves");
	
	hr = m_pWrapper->m_pD3DDevice7->BeginStateBlock();
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_SRCBLEND , D3DBLEND_ONE );
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_DESTBLEND ,D3DBLEND_SRCCOLOR );//should be factor of 0.5	
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_MODULATE); 
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE); 
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG2, D3DTA_DIFFUSE); 
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_COLOROP, D3DTOP_DISABLE); 
		hr = m_pWrapper->m_pD3DDevice7->SetTexture(1,NULL);

		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);

	hr = m_pWrapper->m_pD3DDevice7->EndStateBlock(&m_pShader5_CompileOctaves->m_dwStateBlock);
	
	hr = m_pWrapper->m_pD3DDevice7->ValidateDevice(&numpasses);
	if (hr != D3D_OK)
	{
		OutputDebugString("WARNING: SHADER 5 DIDN'T VALIDATE!\n");
		CheckResult(hr);
	}
	////////////////////////////////////////////////////////////
	

	////////////////////////////////////////////////////////////
	// For demo purposes, a shader to demo the four compiled 
	// octaves:
	////////////////////////////////////////////////////////////
	m_pShader6_DemoCompiledOctaves	= new CDRGShader("Shader 6 - Demo Composite Texture");
	hr = m_pWrapper->m_pD3DDevice7->BeginStateBlock();

		hr = m_pWrapper->m_pD3DDevice7->SetTexture(0,m_pCurrentCompiledOctavesNoiseTexture->ptexTexture);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		
		hr = m_pWrapper->m_pD3DDevice7->SetTexture(1,NULL);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_COLOROP, D3DTOP_DISABLE); 

		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);

		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
	
	hr = m_pWrapper->m_pD3DDevice7->EndStateBlock(&m_pShader6_DemoCompiledOctaves->m_dwStateBlock);

	hr = m_pWrapper->m_pD3DDevice7->ValidateDevice(&numpasses);
	if (hr != D3D_OK)
	{
		OutputDebugString("WARNING: SHADER 6 DIDN'T VALIDATE!\n");
		CheckResult(hr);
	}
	////////////////////////////////////////////////////////////


	////////////////////////////////////////////////////////////
	// This shader is used to modulate the composited texture with 
	// itself, to square it
	////////////////////////////////////////////////////////////
	m_pShader7_CompiledOctavesSquared	= new CDRGShader("Shader 7 - Composite texture, squared (optionally cubed)");
	hr = m_pWrapper->m_pD3DDevice7->BeginStateBlock();

		hr = m_pWrapper->m_pD3DDevice7->SetTexture(0,m_pCurrentCompiledOctavesNoiseTexture->ptexTexture);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG2, D3DTA_DIFFUSE);

		hr = m_pWrapper->m_pD3DDevice7->SetTexture(1,m_pCurrentCompiledOctavesNoiseTexture->ptexTexture);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_COLOROP, D3DTOP_MODULATE/*4X*/);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_COLORARG1, D3DTA_TEXTURE);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_COLORARG2, D3DTA_CURRENT);

		hr = m_pWrapper->m_pD3DDevice7->SetTexture(2,NULL);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(2,D3DTSS_COLOROP, D3DTOP_DISABLE); 
		
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
	
	hr = m_pWrapper->m_pD3DDevice7->EndStateBlock(&m_pShader7_CompiledOctavesSquared->m_dwStateBlock);

	hr = m_pWrapper->m_pD3DDevice7->ValidateDevice(&numpasses);
	if (hr != D3D_OK)
	{
		OutputDebugString("WARNING: SHADER 7 DIDN'T VALIDATE!\n");
		CheckResult(hr);
	}
	////////////////////////////////////////////////////////////
	
	////////////////////////////////////////////////////////////
	// This shader is used to make our final cloud texture. It's 
	// some color of gray (which affects where clamping occurs, thus
	// the amount of cloud cover), minus the composited noise 
	// texture squared (remember, we squared it in the last shader)
	////////////////////////////////////////////////////////////
	m_pShader8_FactorMinusNoiseSquared	= new CDRGShader("Test shader 8 - gray minus Final composite squared");
	hr = m_pWrapper->m_pD3DDevice7->BeginStateBlock();

		hr = m_pWrapper->m_pD3DDevice7->SetTexture(0,m_pCompiledOctavesSquaredTexture->ptexTexture); //Texture not used anyway
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_SELECTARG2);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG2, D3DTA_DIFFUSE);

		hr = m_pWrapper->m_pD3DDevice7->SetTexture(1,m_pCompiledOctavesSquaredTexture->ptexTexture);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_COLOROP, D3DTOP_SUBTRACT);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_COLORARG1, D3DTA_TEXTURE);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_COLORARG2, D3DTA_CURRENT);

		hr = m_pWrapper->m_pD3DDevice7->SetTexture(2,NULL);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(2,D3DTSS_COLOROP, D3DTOP_DISABLE); 

		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_SRCCOLOR);
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR);

		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);

	hr = m_pWrapper->m_pD3DDevice7->EndStateBlock(&m_pShader8_FactorMinusNoiseSquared->m_dwStateBlock);

	hr = m_pWrapper->m_pD3DDevice7->ValidateDevice(&numpasses);
	if (hr != D3D_OK)
	{
		OutputDebugString("WARNING: SHADER 8 DIDN'T VALIDATE!\n");
		CheckResult(hr);
	}
	////////////////////////////////////////////////////////////



	////////////////////////////////////////////////////////////
	// This shader is used to take the final cloud texture generated
	// in the last stage, and blend it with the frame buffer. (The 
	// reason we didn't enable the blend it in the last shader is that we will re-use the
	// texture from shader 6 for things like masking out the sun
	////////////////////////////////////////////////////////////
	//
	// Try this: two passes. First pass is to modulate the sky with the inv of the clouds. Darken it.
	//
	m_pShader10_BlendCloudsWithFB	= new CDRGShader("Test shader 10 - Final cloud texture, blended with the FB.");
	hr = m_pWrapper->m_pD3DDevice7->BeginStateBlock();


		hr = m_pWrapper->m_pD3DDevice7->SetRenderState( D3DRENDERSTATE_LIGHTING, FALSE );		
	
		hr = m_pWrapper->m_pD3DDevice7->SetTexture(0,m_pFactorMinusCompiledNoiseTexture->ptexTexture);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_MODULATE);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		
		
		hr = m_pWrapper->m_pD3DDevice7->SetTexture(1,NULL);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_COLOROP, D3DTOP_DISABLE);
		
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR);
		
	hr = m_pWrapper->m_pD3DDevice7->EndStateBlock(&m_pShader10_BlendCloudsWithFB->m_dwStateBlock);
	

	hr = m_pWrapper->m_pD3DDevice7->ValidateDevice(&numpasses);
	if (hr != D3D_OK)
	{
		OutputDebugString("WARNING: SHADER 10 DIDN'T VALIDATE!\n");
		CheckResult(hr);
	}
	////////////////////////////////////////////////////////////
	
	m_pSkyObj->SetShader(m_pShader10_BlendCloudsWithFB);

	////////////////////////////////////////////////////////////
	// This shader is used to take the final cloud texture generated
	// in the last stage, and blend it with the frame buffer. (The 
	// reason we didn't enable the blend it in the last shader is that we will re-use the
	// texture from shader 6 for things like masking out the sun
	////////////////////////////////////////////////////////////
	//
	// Try this: two passes. First pass is to modulate the sky with the inv of the clouds. Darken it.
	//
	m_pShader11_BlendDarkenedCloudsWithFB	= new CDRGShader("Test shader 11 - Final cloud texture, blended with the FB.");
	hr = m_pWrapper->m_pD3DDevice7->BeginStateBlock();

		hr = m_pWrapper->m_pD3DDevice7->SetRenderState( D3DRENDERSTATE_LIGHTING, FALSE );		
	
	    hr = m_pWrapper->m_pD3DDevice7->SetTexture(0,m_pFactorMinusCompiledNoiseTexture->ptexTexture);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_MODULATE);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		
		
		hr = m_pWrapper->m_pD3DDevice7->SetTexture(1,NULL);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_COLOROP, D3DTOP_DISABLE);
		
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ONE);
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_INVSRCCOLOR);

	hr = m_pWrapper->m_pD3DDevice7->EndStateBlock(&m_pShader11_BlendDarkenedCloudsWithFB->m_dwStateBlock);

	CDRGShader	*tempshader2 = new CDRGShader("Shader11b - 2nd pass of sky shader, to darken clouds");
	hr = m_pWrapper->m_pD3DDevice7->BeginStateBlock();
		hr = m_pWrapper->m_pD3DDevice7->SetTexture(0,m_pLightFactorMinusCompiledNoiseTexture->ptexTexture);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_MODULATE);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE | D3DTA_COMPLEMENT);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	
		hr = m_pWrapper->m_pD3DDevice7->SetTexture(1,NULL);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_COLOROP, D3DTOP_DISABLE); 
	
			
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_SRCBLEND, D3DBLEND_ZERO);
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_DESTBLEND, D3DBLEND_SRCCOLOR);
		
	hr = m_pWrapper->m_pD3DDevice7->EndStateBlock(&tempshader2->m_dwStateBlock);
	hr = m_pShader11_BlendDarkenedCloudsWithFB->AddChild(tempshader2);


	hr = m_pWrapper->m_pD3DDevice7->ValidateDevice(&numpasses);
	if (hr != D3D_OK)
	{
		OutputDebugString("WARNING: SHADER 11 DIDN'T VALIDATE!\n");
		CheckResult(hr);
	}
	////////////////////////////////////////////////////////////
	
	
	////////////////////////////////////////////////////////////
	// A simple shader for our terrain
	////////////////////////////////////////////////////////////
	m_pShader30_Ground	= new CDRGShader("ground shader");
	hr = m_pWrapper->m_pD3DDevice7->BeginStateBlock();


		hr = m_pWrapper->m_pD3DDevice7->SetTexture(0,m_pGroundTexture->ptexTexture);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	
		hr = m_pWrapper->m_pD3DDevice7->SetTexture(1,m_pFactorMinusCompiledNoiseTexture->ptexTexture);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_COLOROP, D3DTOP_MODULATE);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_COLORARG1, D3DTA_TEXTURE | D3DTA_COMPLEMENT);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_COLORARG2, D3DTA_CURRENT);
		
		hr = m_pWrapper->m_pD3DDevice7->SetTexture(2,NULL);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(2,D3DTSS_COLOROP, D3DTOP_DISABLE); 

		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);

	hr = m_pWrapper->m_pD3DDevice7->EndStateBlock(&m_pShader30_Ground->m_dwStateBlock);

	hr = m_pWrapper->m_pD3DDevice7->ValidateDevice(&numpasses);
	if (hr != D3D_OK)
	{
		OutputDebugString("WARNING: SHADER 30 DIDN'T VALIDATE!\n");
		CheckResult(hr);
	}

	////////////////////////////////////////////////////////////

	m_pTerrainObj->SetShader(m_pShader30_Ground);

	////////////////////////////////////////////////////////////
	// A simple shader for the glow around the sun
	////////////////////////////////////////////////////////////
	m_pShader31_SunGlow	= new CDRGShader("Sunglow shader");
	hr = m_pWrapper->m_pD3DDevice7->BeginStateBlock();

		hr = m_pWrapper->m_pD3DDevice7->SetTexture(0,m_pGlowTexture->ptexTexture);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG2, D3DTA_DIFFUSE);
				
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_COLOROP, D3DTOP_DISABLE); 
		hr = m_pWrapper->m_pD3DDevice7->SetTexture(1,NULL);

		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_SRCBLEND , D3DBLEND_SRCCOLOR );
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_DESTBLEND , D3DBLEND_ONE );

				
	hr = m_pWrapper->m_pD3DDevice7->EndStateBlock(&m_pShader31_SunGlow->m_dwStateBlock);

	hr = m_pWrapper->m_pD3DDevice7->ValidateDevice(&numpasses);
	if (hr != D3D_OK)
	{
		OutputDebugString("WARNING: SHADER 31 DIDN'T VALIDATE!\n");
		CheckResult(hr);
	}

	////////////////////////////////////////////////////////////

	////////////////////////////////////////////////////////////
	// A simple shader for the 'glare' from the sun. 
	////////////////////////////////////////////////////////////
	m_pShader32_SunGlare	= new CDRGShader("Sunglare shader");
	hr = m_pWrapper->m_pD3DDevice7->BeginStateBlock();


		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_SRCBLEND , D3DBLEND_SRCCOLOR );
		hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_DESTBLEND , D3DBLEND_ONE );
		
		hr = m_pWrapper->m_pD3DDevice7->SetTexture(0,m_pFactorMinusCompiledNoiseTexture->ptexTexture);
		hr = m_pWrapper->m_pD3DDevice7->SetTexture(1,m_pGlareTexture->ptexTexture);
		
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_TEXCOORDINDEX, 0);
		
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG1, D3DTA_TEXTURE | D3DTA_COMPLEMENT);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(0,D3DTSS_COLORARG2, D3DTA_DIFFUSE);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_TEXCOORDINDEX, 1);
		
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_COLOROP, D3DTOP_MODULATE); 
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_COLORARG1, D3DTA_TEXTURE);
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(1,D3DTSS_COLORARG2, D3DTA_CURRENT);
		
		hr = m_pWrapper->m_pD3DDevice7->SetTextureStageState(2,D3DTSS_COLOROP, D3DTOP_DISABLE); 
		hr = m_pWrapper->m_pD3DDevice7->SetTexture(2,NULL);

		//hr = m_pWrapper->m_pD3DDevice7->SetRenderState(D3DRENDERSTATE_ALPHATESTENABLE, FALSE);
						
	hr = m_pWrapper->m_pD3DDevice7->EndStateBlock(&m_pShader32_SunGlare->m_dwStateBlock);

	hr = m_pWrapper->m_pD3DDevice7->ValidateDevice(&numpasses);
	if (hr != D3D_OK)
	{
		OutputDebugString("WARNING: SHADER 32 DIDN'T VALIDATE!\n");
		CheckResult(hr);
	}

	////////////////////////////////////////////////////////////
		
	return S_OK;

}




// -----------------------------------------------------------------------
// void CNoiseCloudTest::KeyUp(WPARAM wParam)
//
//   Keyboard keypress handler. 
// -----------------------------------------------------------------------
void CNoiseCloudTest::KeyDown(WPARAM wParam)
{
	//we need a variable to modify our rotation speed with frame rate	
	float scaler = 1.0f;

	//if exiting, the wrapper could be invalid, but we don't care, since we will exit anyway
	if (wParam == 'X') 
		return;

	if(m_pWrapper) //we could get into this loop after the wrapper has been invalidates
	{
		if (m_pWrapper->m_fFrameTime >0.0f)
			scaler = m_pWrapper->m_fFrameTime;  
	}
		
	switch (wParam) 
	{
	case VK_UP:
		m_MxView.Rotate( m_MxView.m_Mx._11, m_MxView.m_Mx._21, m_MxView.m_Mx._31, -3.14159f*scaler );
		break;
	
	case VK_DOWN:
		m_MxView.Rotate( m_MxView.m_Mx._11, m_MxView.m_Mx._21, m_MxView.m_Mx._31, 3.14159f*scaler );
		break;
	
	case VK_LEFT:
		m_MxView.Rotate( m_MxView.m_Mx._12, m_MxView.m_Mx._22, m_MxView.m_Mx._32, -3.14159f*scaler );
		break;
	
	case VK_RIGHT:	
		m_MxView.Rotate( m_MxView.m_Mx._12, m_MxView.m_Mx._22, m_MxView.m_Mx._32, 3.14159f*scaler );
		break;
			
	case 'W':
		m_bWireframe = !m_bWireframe;		
		break;
	case 'Z':
		m_pSampleXYZAxisObj->m_bVisible = !m_pSampleXYZAxisObj->m_bVisible;
		break;

	case '1':
		if (m_iCurrentDemoShader != 1)
		{
			m_iCurrentDemoShader = 1;
			m_pSampleFloorPlaneObj->m_bVisible = true;			
			m_pSampleFloorPlaneObj->SetShader(m_pShader0_DemoOctave0);
		}
		else
		{	
			m_pSampleFloorPlaneObj->m_bVisible = false;	
			m_iCurrentDemoShader = -1; 	
		}
		break;
	case '2':
		if (m_iCurrentDemoShader != 2)
		{
			m_iCurrentDemoShader = 2;
			m_pSampleFloorPlaneObj->m_bVisible = true;			
			m_pSampleFloorPlaneObj->SetShader(m_pShader1_DemoOctave1);
		}
		else
		{	
			m_pSampleFloorPlaneObj->m_bVisible = false;	
			m_iCurrentDemoShader = -1; 	
		}					
		break;
	case '3':
		if (m_iCurrentDemoShader != 3)
		{
			m_iCurrentDemoShader = 3;
			m_pSampleFloorPlaneObj->m_bVisible = true;			
			m_pSampleFloorPlaneObj->SetShader(m_pShader2_DemoOctave2);
		}
		else
		{	
			m_pSampleFloorPlaneObj->m_bVisible = false;	
			m_iCurrentDemoShader = -1; 	
		}					
		break;
	case '4':
		if (m_iCurrentDemoShader != 4)
		{
			m_iCurrentDemoShader = 4;
			m_pSampleFloorPlaneObj->m_bVisible = true;			
			m_pSampleFloorPlaneObj->SetShader(m_pShader3_DemoOctave3);
		}
		else
		{	
			m_pSampleFloorPlaneObj->m_bVisible = false;	
			m_iCurrentDemoShader = -1; 	
		}
		break;
	case '5':
		if (m_iCurrentDemoShader != 5)
		{
			m_iCurrentDemoShader = 5;
			m_pSampleFloorPlaneObj->m_bVisible = true;			
			m_pSampleFloorPlaneObj->SetShader(m_pShader6_DemoCompiledOctaves);
		}
		else
		{	
			m_pSampleFloorPlaneObj->m_bVisible = false;	
			m_iCurrentDemoShader = -1; 	
		}
		break;
	case '6':
		if (m_iCurrentDemoShader != 6)
		{
			m_iCurrentDemoShader = 6;
			m_pSampleFloorPlaneObj->m_bVisible = true;			
			m_pSampleFloorPlaneObj->SetShader(m_pShader7_CompiledOctavesSquared);
		}
		else
		{	
			m_pSampleFloorPlaneObj->m_bVisible = false;	
			m_iCurrentDemoShader = -1; 	
		}	
		break;
		
	case '7':
		if (m_iCurrentDemoShader != 7)
		{
			m_iCurrentDemoShader = 7;
			m_pSampleFloorPlaneObj->m_bVisible = true;			
			m_pSampleFloorPlaneObj->SetShader(m_pShader11_BlendDarkenedCloudsWithFB);
		}
		else
		{	
			m_pSampleFloorPlaneObj->m_bVisible = false;	
			m_iCurrentDemoShader = -1; 	
		}
		break;
	
	case 'R':
		if (m_fCloudCover>0.02f)
			m_fCloudCover-=0.02f;
		break;
	case 'T':
		if (m_fCloudCover<0.98f)
			m_fCloudCover+=0.02f;			
		break;	
	case 'S':
		m_iNumSmoothingSteps += 1;
		if (m_iNumSmoothingSteps == 3)
			m_iNumSmoothingSteps = 0;
		break;
	case 'D':
		m_bDarkenClouds = !m_bDarkenClouds;
		if (m_bDarkenClouds)
			m_pSkyObj->SetShader(m_pShader11_BlendDarkenedCloudsWithFB);
		else
			m_pSkyObj->SetShader(m_pShader10_BlendCloudsWithFB);
		break;
	case 'A':
		m_bAccelerateTime = !m_bAccelerateTime;
		if (m_bAccelerateTime)
		{
			m_fCloudSpeed[0] = 0.006f ;
			m_fCloudSpeed[1] = 0.007f ;
			m_fCloudSpeed[2] = 0.013f ;
			m_fCloudSpeed[3] = 0.019f ;
		}
		else
		{
			m_fCloudSpeed[0] = 0.0006f ;
			m_fCloudSpeed[1] = 0.0007f ;
			m_fCloudSpeed[2] = 0.0013f ;
			m_fCloudSpeed[3] = 0.0019f ;
		}
		break;		
	}

	if (m_pWrapper)
	{
		m_pWrapper->m_pD3DDevice7->SetTransform( D3DTRANSFORMSTATE_WORLD, &m_MxWorld.m_Mx );
		m_pWrapper->m_pD3DDevice7->SetTransform( D3DTRANSFORMSTATE_VIEW, &m_MxView.m_Mx );
	}

	
	return;
}

void CNoiseCloudTest::KeyUp(WPARAM wParam)
{		
	switch (wParam) 
	{
		//add code here to handle key releases, if you need them (as in remmed out example)
	case VK_UP:
		//OutputDebugString("The up arrow was released;");			
		break;
	}
	
	return;
}


void CNoiseCloudTest::MouseLeftUp(WPARAM wParam, LPARAM lParam)
{	
	return;
}

void CNoiseCloudTest::MouseLeftDown(WPARAM wParam, LPARAM lParam)
{		
	return;
}

void CNoiseCloudTest::MouseRightUp(WPARAM wParam, LPARAM lParam)
{	
	return;
}

void CNoiseCloudTest::MouseRightDown(WPARAM wParam, LPARAM lParam)
{		
	return;
}

void CNoiseCloudTest::MouseMove(WPARAM wParam, LPARAM lParam)
{
	if ( wParam & MK_LBUTTON )	// Left button down plus mouse move == rotate
	{
		//we need a variable to modify our rotation speed with frame rate	
		float scaler = 1.0f;
		if(m_pWrapper) //we could get into this loop after the wrapper has been invalidates
		{
			scaler = 2.0f / max(m_pWrapper->m_dwRenderWidth, m_pWrapper->m_dwRenderHeight);  
		}

		//we need to compare the last position to the current position
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		
		if (x - m_iMousePrevX)
		{
			m_MxView.Rotate( m_MxView.m_Mx._12, m_MxView.m_Mx._22, m_MxView.m_Mx._32, 3.14159f*scaler*(float)(x - m_iMousePrevX) );
		}
			
		if (y - m_iMousePrevY)
		{
			m_MxView.Rotate( m_MxView.m_Mx._11, m_MxView.m_Mx._21, m_MxView.m_Mx._31, 3.14159f*scaler*(float)(y - m_iMousePrevY) );
		}

		m_pWrapper->m_pD3DDevice7->SetTransform( D3DTRANSFORMSTATE_WORLD, &m_MxWorld.m_Mx );
		m_pWrapper->m_pD3DDevice7->SetTransform( D3DTRANSFORMSTATE_VIEW, &m_MxView.m_Mx );

	
		m_iMousePrevX = x;
		m_iMousePrevY = y;

	}
	else if ( wParam & MK_RBUTTON )	// Right button down plus mouse move == Zoom camera in/out
	{
		//we need a variable to modify our rotation speed with frame rate	
		float scaler = 1.0f;
		if(m_pWrapper) //we could get into this loop after the wrapper has been invalidates
		{
			scaler = 5.0f / max(m_pWrapper->m_dwRenderWidth, m_pWrapper->m_dwRenderHeight);  
		}

		//we need to compare the last position to the current position
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		
		if (y - m_iMousePrevY)
		{
			DRGMatrix Temp(true);

			Temp.Translate( 0,0,scaler * (y - m_iMousePrevY) );
			m_MxView.PostMultiply( Temp );
		}

		m_pWrapper->m_pD3DDevice7->SetTransform( D3DTRANSFORMSTATE_WORLD, &m_MxWorld.m_Mx );
		m_pWrapper->m_pD3DDevice7->SetTransform( D3DTRANSFORMSTATE_VIEW, &m_MxView.m_Mx );

	
		m_iMousePrevX = x;
		m_iMousePrevY = y;

	}
	else
	{
		//need to update these, or the next time someone clicks, the delta is massive
		m_iMousePrevX = LOWORD(lParam);
		m_iMousePrevY = HIWORD(lParam);	
	}
	return;
}

//Noise function as found on Hugo Elias' website
inline float CNoiseCloudTest::Noise2D(int x, int y)
{
    int n = x + y * 57;
    n = (n<<13) ^ n;
    return ( 1.0f - ( (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);    
}
 
//Noise function as found on Hugo Elias' website, modified to take a 3rd variable
inline float CNoiseCloudTest::Noise3D(int x, int y, int z)
{
    int n = x + y * 57 + z * 131;
    n = (n<<13) ^ n;
    return ( 1.0f - ( (n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) 
		* 0.000000000931322574615478515625f);    
}


void CNoiseCloudTest::InitTLVerts()
{
	//////////
	//init my rects
	for ( int oct = 0;oct<NUMOCTAVES;oct++)
	{
		
		memset( m_pCompositRectsVerts[oct], 0, sizeof( D3DTLVERTEX ) * 6 );

		float width = (float)TEXTURESIZE;
		float height = (float)TEXTURESIZE;

		m_pCompositRectsVerts[oct][0].sx = 0.0f;	m_pCompositRectsVerts[oct][0].sy = 0.0f;	
		m_pCompositRectsVerts[oct][1].sx = width;	m_pCompositRectsVerts[oct][1].sy = height;
		m_pCompositRectsVerts[oct][2].sx = 0.0f;	m_pCompositRectsVerts[oct][2].sy = height;	
		m_pCompositRectsVerts[oct][3].sx = 0.0f;	m_pCompositRectsVerts[oct][3].sy = 0.0f;	
		m_pCompositRectsVerts[oct][4].sx = width;	m_pCompositRectsVerts[oct][4].sy = 0.0f;	
		m_pCompositRectsVerts[oct][5].sx = width;	m_pCompositRectsVerts[oct][5].sy = height;	
		
		m_pCompositRectsVerts[oct][0].tu = 0.0f;	m_pCompositRectsVerts[oct][0].tv = 0.0f;	
		m_pCompositRectsVerts[oct][1].tu = 1.0f;	m_pCompositRectsVerts[oct][1].tv = 1.0f;
		m_pCompositRectsVerts[oct][2].tu = 0.0f;	m_pCompositRectsVerts[oct][2].tv = 1.0f;	
		m_pCompositRectsVerts[oct][3].tu = 0.0f;	m_pCompositRectsVerts[oct][3].tv = 0.0f;	
		m_pCompositRectsVerts[oct][4].tu = 1.0f;	m_pCompositRectsVerts[oct][4].tv = 0.0f;	
		m_pCompositRectsVerts[oct][5].tu = 1.0f;	m_pCompositRectsVerts[oct][5].tv = 1.0f;	
		
		// common elements
		for (int i=0;i<6;i++)
		{
			m_pCompositRectsVerts[oct][i].sz = 0.5f;
			m_pCompositRectsVerts[oct][i].rhw = 1.0f;
			m_pCompositRectsVerts[oct][i].dcSpecular = 0;
			//m_pCompositRectsVerts[oct][i].dcColor = D3DRGB( 0.5f, 0.5f, 0.5f );
			m_pCompositRectsVerts[oct][i].dcColor = D3DRGB( 0.75f, 0.75f, 0.75f ); //Why 0.75 vs 0.5? This is a bit of a fudging to			
																					// make up for the washed out texture due to averaging
		}
	}

	//now the smoothing rects
	for (oct = 0;oct<NUMOCTAVES;oct++)
	{
		
		memset( m_pSmoothingRectsVerts[oct], 0, sizeof( D3DTLVERTEX ) * 6 );

		float div = 1.0f/(float)pow(2,(NUMOCTAVES-1-oct));
		float width = (float)TEXTURESIZE * div;
		float height = (float)TEXTURESIZE * div;

		m_pSmoothingRectsVerts[oct][0].sx = 0.0f;	m_pSmoothingRectsVerts[oct][0].sy = 0.0f;	
		m_pSmoothingRectsVerts[oct][1].sx = width;	m_pSmoothingRectsVerts[oct][1].sy = height;
		m_pSmoothingRectsVerts[oct][2].sx = 0.0f;	m_pSmoothingRectsVerts[oct][2].sy = height;	
		m_pSmoothingRectsVerts[oct][3].sx = 0.0f;	m_pSmoothingRectsVerts[oct][3].sy = 0.0f;	
		m_pSmoothingRectsVerts[oct][4].sx = width;	m_pSmoothingRectsVerts[oct][4].sy = 0.0f;	
		m_pSmoothingRectsVerts[oct][5].sx = width;	m_pSmoothingRectsVerts[oct][5].sy = height;	
		
		m_pSmoothingRectsVerts[oct][0].tu = 0.0f;	m_pSmoothingRectsVerts[oct][0].tv = 0.0f;	
		m_pSmoothingRectsVerts[oct][1].tu = 1.0f;	m_pSmoothingRectsVerts[oct][1].tv = 1.0f;
		m_pSmoothingRectsVerts[oct][2].tu = 0.0f;	m_pSmoothingRectsVerts[oct][2].tv = 1.0f;	
		m_pSmoothingRectsVerts[oct][3].tu = 0.0f;	m_pSmoothingRectsVerts[oct][3].tv = 0.0f;	
		m_pSmoothingRectsVerts[oct][4].tu = 1.0f;	m_pSmoothingRectsVerts[oct][4].tv = 0.0f;	
		m_pSmoothingRectsVerts[oct][5].tu = 1.0f;	m_pSmoothingRectsVerts[oct][5].tv = 1.0f;	
		
		// common elements
		for (int i=0;i<6;i++)
		{
			m_pSmoothingRectsVerts[oct][i].sz = 0.5f;
			m_pSmoothingRectsVerts[oct][i].rhw = 1.0f;
			m_pSmoothingRectsVerts[oct][i].dcSpecular = 0;
			m_pSmoothingRectsVerts[oct][i].dcColor = D3DRGB( 1.0f, 1.0f, 1.0f );
		}
	}

	memset( m_pFinalCompositeRectVerts, 0, sizeof( D3DTLVERTEX ) * 6 );

	float width = (float)TEXTURESIZE;
	float height = (float)TEXTURESIZE;
	
	m_pFinalCompositeRectVerts[0].sx = 0.0f;	m_pFinalCompositeRectVerts[0].sy = 0.0f;	
	m_pFinalCompositeRectVerts[1].sx = width;	m_pFinalCompositeRectVerts[1].sy = height;
	m_pFinalCompositeRectVerts[2].sx = 0.0f;	m_pFinalCompositeRectVerts[2].sy = height;	
	m_pFinalCompositeRectVerts[3].sx = 0.0f;	m_pFinalCompositeRectVerts[3].sy = 0.0f;	
	m_pFinalCompositeRectVerts[4].sx = width;	m_pFinalCompositeRectVerts[4].sy = 0.0f;	
	m_pFinalCompositeRectVerts[5].sx = width;	m_pFinalCompositeRectVerts[5].sy = height;	
	
	m_pFinalCompositeRectVerts[0].tu = 0.0f;	m_pFinalCompositeRectVerts[0].tv = 0.0f;	
	m_pFinalCompositeRectVerts[1].tu = 1.0f;	m_pFinalCompositeRectVerts[1].tv = 1.0f;
	m_pFinalCompositeRectVerts[2].tu = 0.0f;	m_pFinalCompositeRectVerts[2].tv = 1.0f;	
	m_pFinalCompositeRectVerts[3].tu = 0.0f;	m_pFinalCompositeRectVerts[3].tv = 0.0f;	
	m_pFinalCompositeRectVerts[4].tu = 1.0f;	m_pFinalCompositeRectVerts[4].tv = 0.0f;	
	m_pFinalCompositeRectVerts[5].tu = 1.0f;	m_pFinalCompositeRectVerts[5].tv = 1.0f;	
	
	// common elements
	for (int i=0;i<6;i++)
	{
		m_pFinalCompositeRectVerts[i].sz = 0.5f;
		m_pFinalCompositeRectVerts[i].rhw = 1.0f;
		m_pFinalCompositeRectVerts[i].dcSpecular = 0x00000000;
		m_pFinalCompositeRectVerts[i].dcColor = D3DRGB( 1.0f, 1.0f, 1.0f ); //since were changing it anyway
	}
	
	height = width = 128.0f;
	
	////////////////
	m_pSunglareRectVerts[0].sx = 0.0f;	m_pSunglareRectVerts[0].sy = 0.0f;	
	m_pSunglareRectVerts[1].sx = width;	m_pSunglareRectVerts[1].sy = height;
	m_pSunglareRectVerts[2].sx = 0.0f;	m_pSunglareRectVerts[2].sy = height;	
	m_pSunglareRectVerts[3].sx = 0.0f;	m_pSunglareRectVerts[3].sy = 0.0f;	
	m_pSunglareRectVerts[4].sx = width;	m_pSunglareRectVerts[4].sy = 0.0f;	
	m_pSunglareRectVerts[5].sx = width;	m_pSunglareRectVerts[5].sy = height;	
	
	m_pSunglareRectVerts[0].tu = 0.0f;	m_pSunglareRectVerts[0].tv = 0.0f;	
	m_pSunglareRectVerts[1].tu = 1.0f;	m_pSunglareRectVerts[1].tv = 1.0f;
	m_pSunglareRectVerts[2].tu = 0.0f;	m_pSunglareRectVerts[2].tv = 1.0f;	
	m_pSunglareRectVerts[3].tu = 0.0f;	m_pSunglareRectVerts[3].tv = 0.0f;	
	m_pSunglareRectVerts[4].tu = 1.0f;	m_pSunglareRectVerts[4].tv = 0.0f;	
	m_pSunglareRectVerts[5].tu = 1.0f;	m_pSunglareRectVerts[5].tv = 1.0f;	

	m_pSunglareRectVerts[0].tu2 = 0.0f;	m_pSunglareRectVerts[0].tv2 = 0.0f;	
	m_pSunglareRectVerts[1].tu2 = 1.0f;	m_pSunglareRectVerts[1].tv2 = 1.0f;
	m_pSunglareRectVerts[2].tu2 = 0.0f;	m_pSunglareRectVerts[2].tv2 = 1.0f;	
	m_pSunglareRectVerts[3].tu2 = 0.0f;	m_pSunglareRectVerts[3].tv2 = 0.0f;	
	m_pSunglareRectVerts[4].tu2 = 1.0f;	m_pSunglareRectVerts[4].tv2 = 0.0f;	
	m_pSunglareRectVerts[5].tu2 = 1.0f;	m_pSunglareRectVerts[5].tv2 = 1.0f;	
	
	
	// common elements
	for ( i=0;i<6;i++)
	{
		m_pSunglareRectVerts[i].sz = 0.01f;
		m_pSunglareRectVerts[i].rhw = 1.0f;
		m_pSunglareRectVerts[i].dcSpecular = 0x00000000;
		m_pSunglareRectVerts[i].dcColor = D3DRGB( 1.0f, 1.0f, 1.0f ); //since were changing it anyway
	}	

	////////////////
	m_pSunglowRectVerts[0].sx = 0.0f;	m_pSunglowRectVerts[0].sy = 0.0f;	
	m_pSunglowRectVerts[1].sx = width;	m_pSunglowRectVerts[1].sy = height;
	m_pSunglowRectVerts[2].sx = 0.0f;	m_pSunglowRectVerts[2].sy = height;	
	m_pSunglowRectVerts[3].sx = 0.0f;	m_pSunglowRectVerts[3].sy = 0.0f;	
	m_pSunglowRectVerts[4].sx = width;	m_pSunglowRectVerts[4].sy = 0.0f;	
	m_pSunglowRectVerts[5].sx = width;	m_pSunglowRectVerts[5].sy = height;	
	
	m_pSunglowRectVerts[0].tu = 0.0f;	m_pSunglowRectVerts[0].tv = 0.0f;	
	m_pSunglowRectVerts[1].tu = 1.0f;	m_pSunglowRectVerts[1].tv = 1.0f;
	m_pSunglowRectVerts[2].tu = 0.0f;	m_pSunglowRectVerts[2].tv = 1.0f;	
	m_pSunglowRectVerts[3].tu = 0.0f;	m_pSunglowRectVerts[3].tv = 0.0f;	
	m_pSunglowRectVerts[4].tu = 1.0f;	m_pSunglowRectVerts[4].tv = 0.0f;	
	m_pSunglowRectVerts[5].tu = 1.0f;	m_pSunglowRectVerts[5].tv = 1.0f;	

	// common elements
	for ( i=0;i<6;i++)
	{
		m_pSunglowRectVerts[i].sz = 0.99999f;
		m_pSunglowRectVerts[i].rhw = 1.0f;
		m_pSunglowRectVerts[i].dcSpecular = 0x00000000;
		m_pSunglowRectVerts[i].dcColor = D3DRGB( 1.0f, 1.0f, 1.0f ); //since were changing it anyway
	}
}	


void CheckResult(HRESULT hr)
{
	switch (hr){
	case DDERR_INVALIDOBJECT:
		OutputDebugString("DDERR_INVALIDOBJECT\n ");;
		break;
	case DDERR_INVALIDPARAMS:
		OutputDebugString("DDERR_INVALIDPARAMS\n ");;
		break;
	case D3DERR_CONFLICTINGTEXTUREFILTER:
		OutputDebugString("D3DERR_CONFLICTINGTEXTUREFILTER\n ");;
		break;
	case D3DERR_CONFLICTINGTEXTUREPALETTE:
		OutputDebugString("D3DERR_CONFLICTINGTEXTUREPALETTE\n ");;
		break;
	case D3DERR_TOOMANYOPERATIONS:
		OutputDebugString("D3DERR_TOOMANYOPERATIONS\n ");;
		break;
	case D3DERR_UNSUPPORTEDALPHAARG:
		OutputDebugString("D3DERR_UNSUPPORTEDALPHAARG\n ");;
		break;
	case D3DERR_UNSUPPORTEDALPHAOPERATION:
		OutputDebugString("D3DERR_UNSUPPORTEDALPHAOPERATION\n ");;
		break;
	case D3DERR_UNSUPPORTEDCOLORARG:
		OutputDebugString("D3DERR_UNSUPPORTEDCOLORARG\n ");;
		break;
	case D3DERR_UNSUPPORTEDCOLOROPERATION:
		OutputDebugString("D3DERR_UNSUPPORTEDCOLOROPERATION\n ");;
		break;
	case D3DERR_UNSUPPORTEDFACTORVALUE:
		OutputDebugString("D3DERR_UNSUPPORTEDFACTORVALUE\n ");;
		break;
	case D3DERR_UNSUPPORTEDTEXTUREFILTER:
		OutputDebugString("D3DERR_UNSUPPORTEDTEXTUREFILTER\n ");;
		break;
	case D3DERR_WRONGTEXTUREFORMAT:
		OutputDebugString("D3DERR_WRONGTEXTUREFORMAT\n ");;
		break;
	default:
		OutputDebugString("other\n ");;
	}
}