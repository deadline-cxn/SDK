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
// NoiseCloudTest.h: declaration of the NoiseCloudTest class.
// This class is the "blank" class created by the DRG 3D Appwizard, where you will put your stuff
//
// ------------------------------------------------------------------------------------
//
// Author: Kim Pallister - Intel Developer Relations & Engineering Group
//
////////////////////////////////////////////////////////////////////////////////////////


#if !defined(AFX_DRGAPP_H__CFF24425_47D6_11D2_B1A5_00A0C90D2C1B__INCLUDED_)
#define AFX_DRGAPP_H__CFF24425_47D6_11D2_B1A5_00A0C90D2C1B__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define NUMOCTAVES	4

struct MyDualD3DTLVertex {
	float sx; 
    float sy;
	float sz;
	float rhw;
	DWORD dcColor;
	DWORD dcSpecular;
	float tu;
	float tv;
	float tu2;
	float tv2;
};


class CNoiseCloudTest  
{
	// some general vars used by the scene mgr
	CDRGWrapper *m_pWrapper;

	DRGMatrix m_MxWorld;
	DRGMatrix m_MxProjection;
	DRGMatrix m_MxView;

	D3DMATERIAL7	*m_pDefaultMaterial;
	bool m_bWireframe;

	// variables specific to the cloud demo
	float m_fCloudCover;			//how much cloud
	float m_fCloudDarkeningFactor;	//how much does the cloud block light (delta from above)
	bool  m_bDarkenClouds;			//toggle extra pass for the darkening step

	DWORD m_dwSkyColor;				//color the horizon gets colored to
	DWORD m_dwFogColor;				//color the horizon gets colored to

	DRGVector m_vSunDirection;		//vector saying where the sun is
		
	int m_iCurrentDemoShader;		//tracks which shader we are currently demoing

	//bunch 'o shaders (this class just holds a stateblock number, a string 
	// description, and a ptr to the next shader for multipass rendering)
	CDRGShader	*m_pShader0_DemoOctave0;
	CDRGShader	*m_pShader1_DemoOctave1;
	CDRGShader	*m_pShader2_DemoOctave2;
	CDRGShader	*m_pShader3_DemoOctave3;

	CDRGShader	*m_pShader4_InterpolateOctaves;
	CDRGShader	*m_pShader5_CompileOctaves;
	CDRGShader	*m_pShader6_DemoCompiledOctaves; 
	CDRGShader	*m_pShader7_CompiledOctavesSquared; 
	CDRGShader	*m_pShader8_FactorMinusNoiseSquared; 
	CDRGShader	*m_pShader9_GrayFactorMinusNoiseSquared; 
	
	CDRGShader	*m_pShader10_BlendCloudsWithFB; 
	CDRGShader	*m_pShader11_BlendDarkenedCloudsWithFB; 

	CDRGShader	*m_pShader30_Ground; 
	CDRGShader	*m_pShader31_SunGlow; 
	CDRGShader	*m_pShader32_SunGlare; 

	//Here are all the arrays of textures we are going to need
	// there are times T0 and T1, and there are 4 different octaves
	CDRGTexture *m_pBasicNoiseTexture[NUMOCTAVES];				
	CDRGTexture *m_pBasicNoiseTextureVidmem[NUMOCTAVES];		
	CDRGTexture *m_pOnceSmoothedNoiseTexture[NUMOCTAVES];			
	CDRGTexture *m_pTwiceSmoothedNoiseTexture[NUMOCTAVES];			
	CDRGTexture *m_pPreviousSmoothedNoiseTexture[NUMOCTAVES];			
	
	CDRGTexture *m_pCurrentInterpolatedNoiseTexture[NUMOCTAVES];
	
	CDRGTexture *m_pCurrentCompiledOctavesNoiseTexture;
	CDRGTexture *m_pCompiledOctavesSquaredTexture;
	CDRGTexture *m_pFactorMinusCompiledNoiseTexture;
	CDRGTexture *m_pLightFactorMinusCompiledNoiseTexture;

	CDRGTexture *m_pGlareTexture;
	CDRGTexture *m_pGlowTexture;

	CDRGTexture *m_pGroundTexture;
	
	float	m_fUpdateInterval[NUMOCTAVES];
	float	m_fElapsedTime[NUMOCTAVES];
	float	m_fTotalElapsedTime;

	float m_fCloudSpeed[NUMOCTAVES];

	bool m_bAccelerateTime;

	D3DTLVERTEX	m_pSmoothingRectsVerts[NUMOCTAVES][6];
	D3DTLVERTEX	m_pCompositRectsVerts[NUMOCTAVES][6];
	D3DTLVERTEX	m_pFinalCompositeRectVerts[6]; 

	MyDualD3DTLVertex m_pSunglareRectVerts[6];
	D3DTLVERTEX	m_pSunglowRectVerts[6]; 
	

	CDRGObject *m_pSampleRootObj;
	CDRGObjectXYZAxis *m_pSampleXYZAxisObj;
	CDRGObjectPlane *m_pSampleFloorPlaneObj;
	CDRGObjectSkyPlane *m_pSkyObj;
	CDRGObject *m_pTerrainObj;
	
	int  m_iNumSmoothingSteps;
	
	int  m_iMousePrevX;
	int  m_iMousePrevY;


public:
    CNoiseCloudTest();
    virtual ~CNoiseCloudTest();
	
    virtual HRESULT InitWorld( CDRGWrapper *pWrapper );
	virtual HRESULT SetupDeviceDependantItems(void);
	virtual HRESULT SetupShaders(void);
	virtual HRESULT DestroyWorld( void );

	virtual HRESULT UpdateWorld();
	virtual HRESULT RenderWorld( void );

	virtual HRESULT TargetChanged( void );

	void KeyDown(WPARAM wParam);
	void KeyUp(WPARAM wParam);

	void MouseLeftUp(WPARAM wParam, LPARAM lParam);
	void MouseLeftDown(WPARAM wParam, LPARAM lParam);
	void MouseRightUp(WPARAM wParam, LPARAM lParam);
	void MouseRightDown(WPARAM wParam, LPARAM lParam);
	void MouseMove(WPARAM wParam, LPARAM lParam);

private:

	void InitTLVerts();

	HRESULT InitBasicNoiseTexture(float time, int octave);
	HRESULT SmoothNoiseTexture(int octave);
	HRESULT InterpolateOctaves();
	HRESULT CompileFinalNoiseTexture();

	float Noise2D(int x, int y);
 	float Noise3D(int x, int y, int z);
};

void CheckResult(HRESULT hr);


#endif // !defined(AFX_DRGAPP_H__CFF24425_47D6_11D2_B1A5_00A0C90D2C1B__INCLUDED_)
