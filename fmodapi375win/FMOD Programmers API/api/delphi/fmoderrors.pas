(* ========================================================================================== *)
(* FMOD Ex - Error string header file. Copyright (c), Firelight Technologies Pty, Ltd. 2004.  *)
(*                                                                                            *)
(* Use this header if you want to store or display a string version / english explanation of  *)
(* the FMOD error codes.                                                                      *)
(*                                                                                            *)
(* ========================================================================================== *)

unit fmoderrors;

{$I fmod.inc}

interface

uses
  fmodtypes;

function FMOD_ErrorString(errcode: FMOD_RESULT): PChar;

implementation

function FMOD_ErrorString(errcode: FMOD_RESULT): PChar;
begin
  case errcode of
    FMOD_OK:                         FMOD_ErrorString := 'No errors.';
    FMOD_ERR_ALREADYLOCKED:          FMOD_ErrorString := 'Tried to call lock a second time before unlock was called.';
    FMOD_ERR_BADCOMMAND:             FMOD_ErrorString := 'Tried to call a function on a data type that does not allow this type of functionality (ie calling Sound::lock / FMOD_Sound_Lock on a streaming sound).';
    FMOD_ERR_CDDA_DRIVERS:           FMOD_ErrorString := 'Neither NTSCSI nor ASPI could be initialised.';
    FMOD_ERR_CDDA_INIT:              FMOD_ErrorString := 'An error occurred while initialising the CDDA subsystem.';
    FMOD_ERR_CDDA_INVALID_DEVICE:    FMOD_ErrorString := 'Couldn''t find the specified device.';
    FMOD_ERR_CDDA_NOAUDIO:           FMOD_ErrorString := 'No audio tracks on the specified disc.';
    FMOD_ERR_CDDA_NODEVICES:         FMOD_ErrorString := 'No CD/DVD devices were found.';
    FMOD_ERR_CDDA_NODISC:            FMOD_ErrorString := 'No disc present in the specified drive.';
    FMOD_ERR_CDDA_READ:              FMOD_ErrorString := 'A CDDA read error occurred.';
    FMOD_ERR_CHANNEL_ALLOC:          FMOD_ErrorString := 'Error trying to allocate a channel.';
    FMOD_ERR_CHANNEL_STOLEN:         FMOD_ErrorString := 'The specified channel has been reused to play another sound.';
    FMOD_ERR_COM:                    FMOD_ErrorString := 'A Win32 COM related error occured. COM failed to initialize or a QueryInterface failed meaning a Windows codec or driver was not installed properly.';
    FMOD_ERR_DMA:                    FMOD_ErrorString := 'DMA Failure.  See debug output for more information.';
    FMOD_ERR_DSP_CONNECTION:         FMOD_ErrorString := 'DSP connection error.  Connection possibly caused a cyclic dependancy.';
    FMOD_ERR_DSP_FORMAT:             FMOD_ErrorString := 'DSP Format error.  A DSP unit may have attempted to connect to this network with the wrong format.  IE a floating point unit on a PocketPC system.';
    FMOD_ERR_DSP_NOTFOUND:           FMOD_ErrorString := 'DSP connection error.  Couldn''t find the DSP unit specified.';
    FMOD_ERR_DSP_RUNNING:            FMOD_ErrorString := 'DSP error.  Cannot perform this operation while the network is in the middle of running.  This will most likely happen if a connection or disconnection is attempted in a DSP callback.';
    FMOD_ERR_DSP_TOOMANYCONNECTIONS: FMOD_ErrorString := 'DSP connection error.  The unit being connected to or disconnected should only have 1 input or output.';
    FMOD_ERR_FILE_BAD:               FMOD_ErrorString := 'Error loading file.';
    FMOD_ERR_FILE_COULDNOTSEEK:      FMOD_ErrorString := 'Couldn''t perform seek operation.';
    FMOD_ERR_FILE_EOF:               FMOD_ErrorString := 'End of file unexpectedly reached while trying to read essential data (truncated data?).';
    FMOD_ERR_FILE_NOTFOUND:          FMOD_ErrorString := 'File not found.';
    FMOD_ERR_FORMAT:                 FMOD_ErrorString := 'Unsupported file or audio format.';
    FMOD_ERR_HTTP:                   FMOD_ErrorString := 'A HTTP error occurred. This is a catch-all for HTTP errors not listed elsewhere.';
    FMOD_ERR_HTTP_ACCESS:            FMOD_ErrorString := 'The specified resource requires authentication or is forbidden.';
    FMOD_ERR_HTTP_PROXY_AUTH:        FMOD_ErrorString := 'Proxy authentication is required to access the specified resource.';
    FMOD_ERR_HTTP_SERVER_ERROR:      FMOD_ErrorString := 'A HTTP server error occurred.';
    FMOD_ERR_HTTP_TIMEOUT:           FMOD_ErrorString := 'The HTTP request timed out.';
    FMOD_ERR_INITIALIZATION:         FMOD_ErrorString := 'FMOD was not initialized correctly to support this function.';
    FMOD_ERR_INITIALIZED:            FMOD_ErrorString := 'Cannot call this command after FMOD_System_Init.';
    FMOD_ERR_INTERNAL:               FMOD_ErrorString := 'An error occured that wasnt supposed to.  Contact support.';
    FMOD_ERR_INVALID_HANDLE:         FMOD_ErrorString := 'An invalid object handle was used.';
    FMOD_ERR_INVALID_PARAM:          FMOD_ErrorString := 'An invalid parameter was passed to this function.';
    FMOD_ERR_IRX:                    FMOD_ErrorString := 'PS2 only.  fmodex.irx failed to initialize.  This is most likely because you forgot to load it.';
    FMOD_ERR_MEMORY:                 FMOD_ErrorString := 'Not enough memory or resources.';
    FMOD_ERR_MEMORY_IOP:             FMOD_ErrorString := 'PS2 only.  Not enough memory or resources on PlayStation 2 IOP ram.';
    FMOD_ERR_MEMORY_SRAM:            FMOD_ErrorString := 'Not enough memory or resources on console sound ram.';
    FMOD_ERR_NEEDSOFTWARE:           FMOD_ErrorString := 'Tried to use a feature that requires the software engine but the software engine has been turned off.';
    FMOD_ERR_NET_CONNECT:            FMOD_ErrorString := 'Couldn''t connect to the specified host.';
    FMOD_ERR_NET_SOCKET_ERROR:       FMOD_ErrorString := 'A socket error occurred.  This is a catch-all for socket-related errors not listed elsewhere.';
    FMOD_ERR_NET_URL:                FMOD_ErrorString := 'The specified URL couldn''t be resolved.';
    FMOD_ERR_NOTREADY:               FMOD_ErrorString := 'Operation could not be performed because specified sound is not ready.';
    FMOD_ERR_OUTPUT_ALLOCATED:       FMOD_ErrorString := 'Error initializing output device, but more specifically, the output device is already in use and cannot be reused.';
    FMOD_ERR_OUTPUT_CREATEBUFFER:    FMOD_ErrorString := 'Error creating hardware sound buffer.';
    FMOD_ERR_OUTPUT_DRIVERCALL:      FMOD_ErrorString := 'A call to a standard soundcard driver failed, which could possibly mean a bug in the driver.';
    FMOD_ERR_OUTPUT_FORMAT:          FMOD_ErrorString := 'Soundcard does not support the minimum features needed for this soundsystem (16bit stereo output).';
    FMOD_ERR_OUTPUT_INIT:            FMOD_ErrorString := 'Error initializing output device.';
    FMOD_ERR_OUTPUT_NOHARDWARE:      FMOD_ErrorString := 'FMOD_HARDWARE was specified but the sound card does not have the resources nescessary to play it.';
    FMOD_ERR_OUTPUT_NOSOFTWARE:      FMOD_ErrorString := 'Attempted to create a software sound but no software channels were specified in System::init.';
    FMOD_ERR_PAN:                    FMOD_ErrorString := 'Panning only works with mono or stereo sound sources.';
    FMOD_ERR_PLUGIN:                 FMOD_ErrorString := 'An unspecified error has been returned from a 3rd party plugin.';
    FMOD_ERR_PLUGIN_MISSING:         FMOD_ErrorString := 'A requested output, dsp unit type or codec was not available.';
    FMOD_ERR_PLUGIN_RESOURCE:        FMOD_ErrorString := 'A resource that the plugin requires cannot be found.';
    FMOD_ERR_RECORD:                 FMOD_ErrorString := 'An error occured trying to initialize the recording device.';
    FMOD_ERR_REVERB_INSTANCE:        FMOD_ErrorString := 'Specified Instance in FMOD_REVERB_PROPERTIES couldn''t be set. Most likely because another application has locked the EAX4 FX slot.';
    FMOD_ERR_SUBSOUND_ALLOCATED:     FMOD_ErrorString := 'This subsound is already being used by another sound, you cannot have more than one parent to a sound.  Null out the other parent''s entry first.';
    FMOD_ERR_TAGNOTFOUND:            FMOD_ErrorString := 'The specified tag could not be found or there are no tags.';
    FMOD_ERR_TOOMANYCHANNELS:        FMOD_ErrorString := 'The sound created exceeds the allowable input channel count.  This can be increased with System::setMaxInputChannels.';
    FMOD_ERR_UNIMPLEMENTED:          FMOD_ErrorString := 'Something in FMOD hasn''t been implemented when it should be! contact support!';
    FMOD_ERR_UNINITIALIZED:          FMOD_ErrorString := 'This command failed because FMOD_System_Init or FMOD_System_SetDriver was not called.';
    FMOD_ERR_UNSUPPORTED:            FMOD_ErrorString := 'A commmand issued was not supported by this object.  Possibly a plugin without certain callbacks specified.';
    FMOD_ERR_VERSION:                FMOD_ErrorString := 'The version number of this file format is not supported.';
  else
    FMOD_ErrorString := 'Unknown error.';
  end;
end;

end.
