/*===============================================================================================
 DSP_GAIN.DLL
 Copyright (c), Firelight Technologies Pty, Ltd 2005.

===============================================================================================*/

#include <stdio.h>

#include "../../../api/inc/fmod.h"
#include "../../../api/inc/fmod_errors.h"

FMOD_RESULT F_CALLBACK dspcreate  (FMOD_DSP *dsp);
FMOD_RESULT F_CALLBACK dsprelease (FMOD_DSP *dsp);
FMOD_RESULT F_CALLBACK dspreset   (FMOD_DSP *dsp);
FMOD_RESULT F_CALLBACK dspread    (FMOD_DSP *dsp, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels);
FMOD_RESULT F_CALLBACK dspsetparam(FMOD_DSP *dsp, int index, float value);
FMOD_RESULT F_CALLBACK dspgetparam(FMOD_DSP *dsp, int index, float *value, char *valuestr);


/*
    DSP Parameter list.
*/
FMOD_DSP_PARAMETERDESC dspparam[1] =
{
    { 0.0f,     1.0f,   1.0, "Level",   "%",     "Gain level" },
};


FMOD_DSP_DESCRIPTION dspgaindesc =
{
    "FMOD gain example",        // name
    0x00010000,                 // version 0xAAAABBBB   A = major, B = minor.
    0,                          // 0 = we can filter whatever you throw at us.  To be most user friendly, always write a filter like this.
    dspcreate,                  //
    dsprelease,                 //
    dspreset,                   //
    dspread,                    //
    0,                          // This is for if you want to allow the plugin to seek, which doesnt really make sense in a gain filter so we'll just leave it out.
    1,                          // 1 parameter.  "level"
    dspparam,                   // pointer to the parameter list definition.
    dspsetparam,                //
    dspgetparam,                //
    0,                          // This is for if you want to pop up a dialog box to configure the plugin.  Not doing that here.
    0,                          // This is for if you want to pop up a dialog box to configure the plugin.  Not doing that here.
    0,                          // This is for if you want to pop up a dialog box to configure the plugin.  Not doing that here.
};

/*
    FMODGetDSPDescription is mandantory for every fmod plugin.  This is the symbol the registerplugin function searches for.
    Must be declared with F_API to make it export as stdcall.
    MUST BE EXTERN'ED AS C!  C++ functions will be mangled incorrectly and not load in fmod.
*/
#ifdef __cplusplus
extern "C" {
#endif

F_DECLSPEC F_DLLEXPORT FMOD_DSP_DESCRIPTION * F_API FMODGetDSPDescription()
{
    return &dspgaindesc;
}

#ifdef __cplusplus
}
#endif


float dspgain;


FMOD_RESULT F_CALLBACK dspcreate(FMOD_DSP *dsp)
{
    dspgain = dspparam[0].defaultval;

    /*
        If we were allocating memory etc, it would be done in this function.
    */

    return FMOD_OK;
}

FMOD_RESULT F_CALLBACK dsprelease(FMOD_DSP *dsp)
{
    /*
        If memory was allocated in create, it would be freed in this function.
    */
    return FMOD_OK;
}


FMOD_RESULT F_CALLBACK dspreset(FMOD_DSP *dsp)
{
    /*
        This isnt really needed here.  It is used to reset a filter back to it's default state.
    */

    dspgain = dspparam[0].defaultval;

    return FMOD_OK;
}


/*
    This callback does the work.  Modify data from inbuffer and send it to outbuffer.
*/
FMOD_RESULT F_CALLBACK dspread(FMOD_DSP *dsp, float *inbuffer, float *outbuffer, unsigned int length, int inchannels, int outchannels)
{
    unsigned int count;
    int count2;
    int channels = inchannels;  // outchannels and inchannels will always be the same because this is a flexible filter.

    for (count = 0; count < length; count++)
    {
        for (count2 = 0; count2 < channels; count2++)
        {
            outbuffer[(count * channels) + count2] = inbuffer[(count * channels) + count2] * dspgain;
        }
    }

    return FMOD_OK;
}

/*
    This callback is for when the user sets a parameter.  It is automatically clamped between 0 and 1.
*/
FMOD_RESULT F_CALLBACK dspsetparam(FMOD_DSP *dsp, int index, float value)
{
    switch (index)
    {
        case 0:
        {
            dspgain = value;
            break;
        }
    }
    return FMOD_OK;
}

/*
    This callback is for when the user gets a parameter.  The label for our only parameter is percent, 
    so when the string is requested print it out as 0 to 100.
*/
FMOD_RESULT F_CALLBACK dspgetparam(FMOD_DSP *dsp, int index, float *value, char *valuestr)
{
    switch (index)
    {
        case 0:
        {
            *value = dspgain;
            sprintf(valuestr, "%.02f", dspgain * 100.0f);    // our units are '%', so print it out as 0 to 100.
        }
    }

    return FMOD_OK;
}

