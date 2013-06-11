VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form Form1 
   Caption         =   "Rip Netstream Example"
   ClientHeight    =   3045
   ClientLeft      =   60
   ClientTop       =   375
   ClientWidth     =   4455
   LinkTopic       =   "Form1"
   ScaleHeight     =   3045
   ScaleWidth      =   4455
   StartUpPosition =   3  'Windows Default
   Begin VB.CommandButton cmdMute 
      Caption         =   "Mute"
      Height          =   495
      Left            =   1320
      TabIndex        =   6
      Top             =   1680
      Width           =   1815
   End
   Begin VB.CommandButton cmdExit 
      Caption         =   "Exit"
      Height          =   375
      Left            =   1740
      TabIndex        =   5
      Top             =   2280
      Width           =   975
   End
   Begin VB.Timer Timer 
      Interval        =   1
      Left            =   3960
      Top             =   1800
   End
   Begin VB.CommandButton cmdPauseResume 
      Caption         =   "Pause / Resume"
      Height          =   495
      Left            =   2280
      TabIndex        =   3
      Top             =   1080
      Width           =   1815
   End
   Begin VB.CommandButton cmdStart 
      Caption         =   "Start"
      Height          =   495
      Left            =   360
      TabIndex        =   2
      Top             =   1080
      Width           =   1815
   End
   Begin VB.TextBox urlText 
      Height          =   285
      Left            =   240
      TabIndex        =   1
      Top             =   600
      Width           =   3975
   End
   Begin MSComctlLib.StatusBar StatusBar 
      Align           =   2  'Align Bottom
      Height          =   375
      Left            =   0
      TabIndex        =   4
      Top             =   2670
      Width           =   4455
      _ExtentX        =   7858
      _ExtentY        =   661
      Style           =   1
      _Version        =   393216
      BeginProperty Panels {8E3867A5-8586-11D1-B16A-00C0F0283628} 
         NumPanels       =   1
         BeginProperty Panel1 {8E3867AB-8586-11D1-B16A-00C0F0283628} 
         EndProperty
      EndProperty
   End
   Begin VB.Label copyright 
      Alignment       =   2  'Center
      Caption         =   "Copyright (c) Firelight Technologies 2004-2005"
      Height          =   495
      Left            =   240
      TabIndex        =   0
      Top             =   120
      Width           =   3855
   End
End
Attribute VB_Name = "Form1"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
' ===============================================================================================
' ripnetstream example
' Copyright (c), Firelight Technologies Pty, Ltd 2004-2005.
'
' This example shows how to rip streaming audio from the internet to local files, using
' System::attachFileSystem.
'
' This example also uses tags from the net stream to name the files as they are ripped.
'
' Some internet radio station only broadcast new tag information on fixed intervals.  This
' means that the rip might not be exactly in line with the filenames that are produced.
' For example if a radio station only broadcast the track name every 10 seconds, and the
' music changed half way inbetween this period, then the first file would contain 5 seconds
' of the new song, and the second song would be missing  the first 5 seconds of the track.
' Silence detection might help this problem, but most radio stations do not offer silence
' between  tracks.
' ===============================================================================================
Option Explicit

Dim system As Long
Dim sound As Long
Dim channel As Long
Dim soundcreated As Boolean

Private Function GetFunctionPointer(fptr As Long) As Long
    GetFunctionPointer = fptr
End Function

Private Sub Form_Load()
    Dim result As FMOD_RESULT
    Dim version As Long
    
    soundcreated = False
    
    gSoundType = FMOD_SOUND_TYPE_MPEG
    gUpdateFileName = False
    gOutputFileName = "output.mp3"
    
    ' Create a System object and initialize.
    result = FMOD_System_Create(system)
    ERRCHECK (result)
    
    result = FMOD_System_GetVersion(system, version)
    ERRCHECK (result)

    If version <> FMOD_VERSION Then
        MsgBox "Error!  You are using an old version of FMOD " & Hex$(version) & ". " & _
               "This program requires " & Hex$(FMOD_VERSION)
    End If
    
    result = FMOD_System_Init(system, 1, FMOD_INIT_NORMAL, 0)
    ERRCHECK (result)
    
    ' Bump up the file buffer size a little bit for netstreams (to account for lag).  Decode buffer is left at default.
    result = FMOD_System_SetStreamBufferSize(system, 65536, FMOD_TIMEUNIT_RAWBYTES)
    ERRCHECK (result)
    
    result = FMOD_System_AttachFileSystem(system, AddressOf myopen, AddressOf myclose, AddressOf myread, 0)
    ERRCHECK (result)
End Sub

Private Sub Form_Unload(Cancel As Integer)
    Dim result As FMOD_RESULT
    
    ' Shutdown
    If sound Then
        result = FMOD_Sound_Release(sound)
        ERRCHECK (result)
    End If
    If system Then
        result = FMOD_System_Close(system)
        ERRCHECK (result)
        result = FMOD_System_Release(system)
        ERRCHECK (result)
    End If
End Sub

Private Sub cmdStart_Click()
    Dim result As FMOD_RESULT
    
    If Not soundcreated Then
        StatusBar.SimpleText = "Buffering..."
        
        result = FMOD_System_CreateSound(system, urlText.Text, (FMOD_HARDWARE Or FMOD_2D Or FMOD_CREATESTREAM Or FMOD_NONBLOCKING), sound)
        ERRCHECK (result)
        
        soundcreated = True
    End If
End Sub

Private Sub cmdPauseResume_Click()
    Dim result As FMOD_RESULT
    Dim paused As Long
    
    If channel Then
        result = FMOD_Channel_GetPaused(channel, paused)
        ERRCHECK (result)

        If paused Then
            result = FMOD_Channel_SetPaused(channel, 0)
        Else
            result = FMOD_Channel_SetPaused(channel, 1)
        End If
        
        ERRCHECK (result)
    End If
End Sub

Private Sub cmdMute_Click()
    Dim result As FMOD_RESULT
    Dim mute As Long

    If channel Then
        result = FMOD_Channel_GetMute(channel, mute)
        ERRCHECK (result)
        
        If mute Then
            result = FMOD_Channel_SetMute(channel, 0)
        Else
            result = FMOD_Channel_SetMute(channel, 1)
        End If
    End If
End Sub

Private Sub Timer_Timer()
    Dim result As FMOD_RESULT
    
    If sound Then
        Dim openstate As FMOD_OPENSTATE
        Dim percent As Long
        Dim starving As Long
        
        result = FMOD_Sound_GetOpenState(sound, openstate, percent, starving)
        ERRCHECK (result)
        
        If (openstate = FMOD_OPENSTATE_READY) And channel = 0 Then
            result = FMOD_System_PlaySound(system, FMOD_CHANNEL_FREE, sound, 0, channel)
            ERRCHECK (result)
        End If
    End If
    
    If channel Then
        Dim paused As Long
        Dim playing As Long
        Dim ms As Long
        Dim tagsupdated As Long
        Dim numtags As Long
        Dim nullstring As String

        result = FMOD_Sound_GetNumTags(sound, numtags, tagsupdated)
        ERRCHECK (result)

        If tagsupdated <> 0 Then
        
            Do While True
                Dim tag As FMOD_TAG
                
                result = FMOD_Sound_GetTag(sound, nullstring, -1, tag)
                If result <> FMOD_OK Then
                    GoTo break
                End If
                
                If tag.datatype = FMOD_TAGDATATYPE_STRING Then
                    Dim format As FMOD_SOUND_FORMAT
                    Dim channels As Long
                    Dim bits As Long
                    Dim tagname As String
                    Dim tagdata As String
                    
                    result = FMOD_Sound_GetFormat(sound, gSoundType, format, channels, bits)
                    ERRCHECK (result)
                    
                    tagname = Space(lstrlen(tag.name))
                    Call lstrcpy(tagname, tag.name)
    
                    If tagname = "ARTIST" Then
                        tagdata = Space(tag.datalen - 1)
                        Call lstrcpy(tagdata, tag.data)
    
                        gCurrentTrackArtist = tagdata
                        gUpdateFileName = True
                    End If
                    If tagname = "TITLE" Then
                        tagdata = Space(tag.datalen - 1)
                        Call lstrcpy(tagdata, tag.data)
                        
                        gCurrentTrackTitle = tagdata
                        gUpdateFileName = True
                    End If
    
                    GoTo break
                Else
                    GoTo break
                End If
            Loop
        End If

break:
        result = FMOD_Channel_GetPaused(channel, paused)
        ERRCHECK (result)
        result = FMOD_Channel_IsPlaying(channel, playing)
        ERRCHECK (result)
        result = FMOD_Channel_GetPosition(channel, ms, FMOD_TIMEUNIT_MS)
        ERRCHECK (result)

        StatusBar.SimpleText = "Time " & CInt(ms / 1000 / 60) & ":" & CInt(ms / 1000 Mod 60) & ":" & CInt(ms / 10 Mod 100) & " : " & IIf(paused, "Paused", IIf(playing, "Playing", "Stopped"))
    End If
    
    If gMyopen Then
        Call opencallback
    End If
    If gMyclose Then
        Call closecallback
    End If
    If gMyread Then
        Call readcallback
    End If
End Sub

Private Sub cmdExit_Click()
    Unload Me
    End
End Sub

Private Sub ERRCHECK(result As FMOD_RESULT)
    Dim msgResult As VbMsgBoxResult
    
    If result <> FMOD_OK Then
        msgResult = MsgBox("FMOD error! (" & result & ") " & FMOD_ErrorString(result))
    End If
    
    If msgResult Then
        End
    End If
End Sub
