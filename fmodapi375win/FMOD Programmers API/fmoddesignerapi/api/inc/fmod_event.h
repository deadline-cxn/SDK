/*
    fmod_event.h - Data-driven event classes
    Copyright (c), Firelight Technologies Pty, Ltd. 2004-2005.
*/

#ifndef __FMOD_EVENT_H__
#define __FMOD_EVENT_H__

#ifndef _FMOD_HPP
#include "fmod.hpp"
#endif


namespace FMOD
{
    class EventSystem;
    class EventGroup;
    class Event;
    class EventParameter;

    FMOD_RESULT F_API EventSystem_Create(EventSystem **eventsystem);

    class EventSystem
    {
        public :

        virtual FMOD_RESULT F_API init                   (int maxchannels, FMOD_INITFLAGS flags, void *extradriverdata) = 0;
        virtual FMOD_RESULT F_API release                () = 0;
        virtual FMOD_RESULT F_API update                 () = 0;
        virtual FMOD_RESULT F_API setMediaPath           (char *path) = 0;
        virtual FMOD_RESULT F_API setPluginPath          (char *path) = 0;
        virtual FMOD_RESULT F_API load                   (char *filename, char *encryptionkey) = 0;
        virtual FMOD_RESULT F_API unload                 () = 0;
        virtual FMOD_RESULT F_API getGroup               (char *name, EventGroup **group) = 0;
        virtual FMOD_RESULT F_API getGroupByIndex        (int index, EventGroup **group) = 0;
        virtual FMOD_RESULT F_API getSystemObject        (System **system) = 0;

        virtual FMOD_RESULT F_API set3DSettings          (float dopplerscale, float distancefactor, float rolloffscale) = 0;
        virtual FMOD_RESULT F_API get3DSettings          (float *dopplerscale, float *distancefactor, float *rolloffscale) = 0;
        virtual FMOD_RESULT F_API set3DNumListeners      (int numlisteners) = 0;
        virtual FMOD_RESULT F_API get3DNumListeners      (int *numlisteners) = 0;
        virtual FMOD_RESULT F_API set3DListenerAttributes(int listener, const FMOD_VECTOR *pos, const FMOD_VECTOR *vel, const FMOD_VECTOR *forward, const FMOD_VECTOR *up) = 0;
        virtual FMOD_RESULT F_API get3DListenerAttributes(int listener, FMOD_VECTOR *pos, FMOD_VECTOR *vel, FMOD_VECTOR *forward, FMOD_VECTOR *up) = 0;
    };


    class EventGroup
    {
        public :

        virtual FMOD_RESULT F_API release() = 0;
        virtual FMOD_RESULT F_API getName(char **name) = 0;
        virtual FMOD_RESULT F_API getGroup(char *name, EventGroup **group) = 0;
        virtual FMOD_RESULT F_API getGroupByIndex(int index, EventGroup **group) = 0;
        virtual FMOD_RESULT F_API getEvent(char *name, Event **event) = 0;
        virtual FMOD_RESULT F_API getEventByIndex(int index, Event **event) = 0;
    };


    class Event
    {
        public :

        virtual FMOD_RESULT F_API release() = 0;
        virtual FMOD_RESULT F_API getName(char **name) = 0;
        virtual FMOD_RESULT F_API start() = 0;
        virtual FMOD_RESULT F_API stop() = 0;
        virtual FMOD_RESULT F_API getParameter(char *name, EventParameter **parameter) = 0;
        virtual FMOD_RESULT F_API getParameterByIndex(int index, EventParameter **parameter) = 0;
        virtual FMOD_RESULT F_API preload(bool sound, bool dsp) = 0;

        virtual FMOD_RESULT F_API setVolume              (float volume) = 0;
        virtual FMOD_RESULT F_API getVolume              (float *volume) = 0;
        virtual FMOD_RESULT F_API setPaused              (bool paused) = 0;
        virtual FMOD_RESULT F_API getPaused              (bool *paused) = 0;
        virtual FMOD_RESULT F_API set3DAttributes        (const FMOD_VECTOR *pos, const FMOD_VECTOR *vel) = 0;
        virtual FMOD_RESULT F_API get3DAttributes        (FMOD_VECTOR *pos, FMOD_VECTOR *vel) = 0;
        virtual FMOD_RESULT F_API isPlaying              (bool *isplaying) = 0;
    };


    class EventParameter
    {
        public :

        virtual FMOD_RESULT F_API getName(char **name) = 0;
        virtual FMOD_RESULT F_API getRange(float *rangemin, float *rangemax) = 0;
        virtual FMOD_RESULT F_API setValue(float value) = 0;
        virtual FMOD_RESULT F_API getValue(float *value) = 0;
    };
}


#endif
