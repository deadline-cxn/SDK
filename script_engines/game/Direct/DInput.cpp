// DInput.cpp: implementation of the CDirectInput class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DInput.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDirectInput::CDirectInput()
{

}

CDirectInput::~CDirectInput()
{
   Close();
}

long CDirectInput::Init(HWND hwnd)
{
    HRESULT hr; 
 
    // Create the DirectInput object. 
    hr = DirectInputCreate(GetModuleHandle(NULL), DIRECTINPUT_VERSION,&m_lpDI, NULL); 
    if FAILED(hr) return hr; 
 
    // Retrieve a pointer to an IDirectInputDevice interface 
    hr = m_lpDI->CreateDevice(GUID_SysKeyboard, &m_lpDIDevice, NULL); 
    if FAILED(hr) {
        Close(); 
        return hr; 
    } 
 
   // Now that you have an IDirectInputDevice interface, get 
   // it ready to use. 
 
    // Set the data format using the predefined keyboard data 
    // format provided by the DirectInput object for keyboards. 
    hr = m_lpDIDevice->SetDataFormat(&c_dfDIKeyboard); 
    if FAILED(hr) { 
        Close(); 
        return hr; 
    } 
 
    // Set the cooperative level 
    hr = m_lpDIDevice->SetCooperativeLevel(hwnd,DISCL_FOREGROUND | DISCL_NONEXCLUSIVE); 
    if FAILED(hr) { 
        Close(); 
        return hr; 
    } 
 
    // Get access to the input device. 
    hr = m_lpDIDevice->Acquire(); 
    if FAILED(hr) { 
//       Close(); 
//       return FALSE; 
    } 
 
    return 0; 
}

long CDirectInput::Close()
{
    if (m_lpDI) { 
        if (m_lpDIDevice) { 
             // Always unacquire the device before calling Release(). 
             m_lpDIDevice->Unacquire(); 
             m_lpDIDevice->Release();
             m_lpDIDevice = NULL; 
        } ;
        m_lpDI->Release();
        m_lpDI = NULL; 
    };
    return 0;
}

void CDirectInput::Idle()
{
   HRESULT hr;
 
again:;
   hr = m_lpDIDevice->GetDeviceState(sizeof(KBState),(LPVOID)&KBState); 
   if FAILED(hr) { 
      if (hr == DIERR_INPUTLOST) {
         hr = m_lpDIDevice->Acquire();
         if (SUCCEEDED(hr)) goto again;
      }
      return; 
   }  
};
