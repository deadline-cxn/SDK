VERSION 5.00
Object = "{831FDD16-0C5C-11D2-A9FC-0000F8754DA1}#2.0#0"; "MSCOMCTL.OCX"
Begin VB.Form Form1 
   Caption         =   "3D Example"
   ClientHeight    =   4860
   ClientLeft      =   60
   ClientTop       =   375
   ClientWidth     =   4200
   LinkTopic       =   "Form1"
   ScaleHeight     =   4860
   ScaleWidth      =   4200
   StartUpPosition =   3  'Windows Default
   Begin VB.Timer Timer 
      Interval        =   49
      Left            =   3240
      Top             =   4080
   End
   Begin VB.CommandButton cmdExit 
      Caption         =   "Exit"
      Height          =   375
      Left            =   1680
      TabIndex        =   7
      Top             =   4080
      Width           =   975
   End
   Begin MSComctlLib.Slider PositionSlider 
      Height          =   375
      Left            =   360
      TabIndex        =   5
      Top             =   3240
      Width           =   3495
      _ExtentX        =   6165
      _ExtentY        =   661
      _Version        =   393216
      Min             =   -35
      Max             =   30
   End
   Begin VB.CommandButton cmdMovement 
      Caption         =   "Stop/Start listener automatic movement"
      Height          =   495
      Left            =   240
      TabIndex        =   4
      Top             =   2400
      Width           =   3735
   End
   Begin VB.CommandButton cmdPlayStereo 
      Caption         =   "Play 16bit STEREO 2D sound at any time"
      Height          =   495
      Left            =   240
      TabIndex        =   2
      Top             =   1800
      Width           =   3735
   End
   Begin VB.CommandButton cmdPause8bit 
      Caption         =   "Pause/Unpause 8bit 3D sound at any time"
      Height          =   495
      Left            =   240
      TabIndex        =   1
      Top             =   1200
      Width           =   3735
   End
   Begin VB.CommandButton cmdPause16bit 
      Caption         =   "Pause/Unpause 16bit 3D sound at any time"
      Height          =   495
      Left            =   240
      TabIndex        =   0
      Top             =   600
      Width           =   3735
   End
   Begin MSComctlLib.StatusBar StatusBar 
      Align           =   2  'Align Bottom
      Height          =   375
      Left            =   0
      TabIndex        =   8
      Top             =   4485
      Width           =   4200
      _ExtentX        =   7408
      _ExtentY        =   661
      Style           =   1
      _Version        =   393216
      BeginProperty Panels {8E3867A5-8586-11D1-B16A-00C0F0283628} 
         NumPanels       =   1
         BeginProperty Panel1 {8E3867AB-8586-11D1-B16A-00C0F0283628} 
         EndProperty
      EndProperty
   End
   Begin VB.Frame Frame1 
      Caption         =   "Listener Position"
      Height          =   975
      Left            =   240
      TabIndex        =   6
      Top             =   3000
      Width           =   3735
      Begin VB.Label Label2 
         Caption         =   "<2>"
         Height          =   255
         Left            =   2520
         TabIndex        =   10
         Top             =   600
         Width           =   375
      End
      Begin VB.Label Label1 
         Caption         =   "<1>"
         Height          =   255
         Left            =   1320
         TabIndex        =   9
         Top             =   600
         Width           =   375
      End
   End
   Begin VB.Label copyright 
      Alignment       =   2  'Center
      Caption         =   "Copyright (c) Firelight Technologies 2004-2005"
      Height          =   375
      Left            =   120
      TabIndex        =   3
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
' 3d Example
' Copyright (c), Firelight Technologies Pty, Ltd 2004-2005.
'
' This example shows how to basic 3d positioning
' ===============================================================================================
Option Explicit

Private Const INTERFACE_UPDATETIME As Long = 50
Private Const distancefactor As Single = 1#   'Units per meter.  I.e feet would = 3.28.  centimeters would = 100.

Dim system As Long
Dim sound1 As Long
Dim sound2 As Long
Dim sound3 As Long
Dim channel1 As Long
Dim channel2 As Long
Dim channel3 As Long
Dim lastpos As FMOD_VECTOR
Dim listenerpos As FMOD_VECTOR
Dim listenerflag As Boolean
Dim t As Single

Private Sub Form_Load()
    Dim result As FMOD_RESULT
    Dim version As Long
    Dim pos1 As FMOD_VECTOR
    Dim vel1 As FMOD_VECTOR
    Dim pos2 As FMOD_VECTOR
    Dim vel2 As FMOD_VECTOR
    
    PositionSlider.Enabled = False
    listenerflag = True
    
    listenerpos.x = 0#
    listenerpos.y = 0#
    listenerpos.z = -1 * distancefactor
    
    ' Create a System object and initialize.
    result = FMOD_System_Create(system)
    ERRCHECK (result)
    
    result = FMOD_System_GetVersion(system, version)
    ERRCHECK (result)

    If version <> FMOD_VERSION Then
        MsgBox "Error!  You are using an old version of FMOD " & Hex$(version) & ". " & _
               "This program requires " & Hex$(FMOD_VERSION)
    End If

    result = FMOD_System_Init(system, 100, FMOD_INIT_NORMAL, 0)
    ERRCHECK (result)
    
    'Set the distance units. (meters/feet etc).
    result = FMOD_System_Set3DSettings(system, 1#, distancefactor, 1#)
    ERRCHECK (result)

    result = FMOD_System_CreateSound(system, "../../examples/media/drumloop.wav", (FMOD_HARDWARE Or FMOD_3D), sound1)
    ERRCHECK (result)
    result = FMOD_Sound_Set3DMinMaxDistance(sound1, 2# * distancefactor, 10000# * distancefactor)
    ERRCHECK (result)
    result = FMOD_Sound_SetMode(sound1, FMOD_LOOP_NORMAL)
    ERRCHECK (result)

    result = FMOD_System_CreateSound(system, "../../examples/media/jaguar.wav", (FMOD_HARDWARE Or FMOD_3D), sound2)
    ERRCHECK (result)
    result = FMOD_Sound_Set3DMinMaxDistance(sound2, 2# * distancefactor, 10000# * distancefactor)
    ERRCHECK (result)
    result = FMOD_Sound_SetMode(sound2, FMOD_LOOP_NORMAL)
    ERRCHECK (result)
    
    result = FMOD_System_CreateSound(system, "../../examples/media/swish.wav", (FMOD_HARDWARE Or FMOD_2D), sound3)
    ERRCHECK (result)
    
    
    ' Play sounds at certain positions
    pos1.x = -10 * distancefactor
    pos1.y = 0
    pos2.z = 0
    
    result = FMOD_System_PlaySound(system, FMOD_CHANNEL_FREE, sound1, 1, channel1)
    ERRCHECK (result)
    result = FMOD_Channel_Set3DAttributes(channel1, pos1, vel1)
    ERRCHECK (result)
    result = FMOD_Channel_SetPaused(channel1, 0)
    ERRCHECK (result)
    
    pos2.x = 15 * distancefactor
    pos2.y = 0
    pos2.z = 0
    
    result = FMOD_System_PlaySound(system, FMOD_CHANNEL_FREE, sound2, 1, channel2)
    ERRCHECK (result)
    result = FMOD_Channel_Set3DAttributes(channel2, pos2, vel2)
    ERRCHECK (result)
    result = FMOD_Channel_SetPaused(channel2, 0)
    ERRCHECK (result)

End Sub

Private Sub Form_Unload(Cancel As Integer)
    Dim result As FMOD_RESULT

    ' Shutdown
    If sound1 Then
        result = FMOD_Sound_Release(sound1)
        ERRCHECK (result)
    End If
    If sound2 Then
        result = FMOD_Sound_Release(sound2)
        ERRCHECK (result)
    End If
    If sound3 Then
        result = FMOD_Sound_Release(sound3)
        ERRCHECK (result)
    End If
    If system Then
        result = FMOD_System_Close(system)
        ERRCHECK (result)
        
        result = FMOD_System_Release(system)
        ERRCHECK (result)
    End If
End Sub

Private Sub Timer_Timer()
    Dim result As FMOD_RESULT
    Dim forward As FMOD_VECTOR
    Dim up As FMOD_VECTOR
    Dim vel As FMOD_VECTOR
    Dim s As String
    Dim bytearray() As Byte
    Dim count As Long
    
    ' ==========================================================================================
    ' UPDATE THE LISTENER
    ' ==========================================================================================
    
    forward.x = 0
    forward.y = 0
    forward.z = 1
    
    up.x = 0
    up.y = 1
    up.z = 0
    
    If listenerflag Then
        listenerpos.x = Sin(t * 0.05) * 33 * distancefactor 'left right pingpong
        PositionSlider.value = CInt(listenerpos.x)
    End If
    
    ' ********* NOTE ******* READ NEXT COMMENT!!!!!
    ' vel = how far we moved last FRAME (m/f), then time compensate it to SECONDS (m/s).
    vel.x = (listenerpos.x - lastpos.x) * (1000 / INTERFACE_UPDATETIME)
    vel.y = (listenerpos.y - lastpos.y) * (1000 / INTERFACE_UPDATETIME)
    vel.z = (listenerpos.z - lastpos.z) * (1000 / INTERFACE_UPDATETIME)
    
    ' store pos for next time
    lastpos = listenerpos
    
    result = FMOD_System_Set3DListenerAttributes(system, 0, listenerpos, vel, forward, up)
    ERRCHECK (result)
    
    t = t + (30 * (1 / INTERFACE_UPDATETIME)) 't is just a time value .. it increments in 30m/s steps in this example

    If system Then
        result = FMOD_System_Update(system)
        ERRCHECK (result)
    End If
End Sub

Private Sub cmdPause16bit_Click()
    Dim result As FMOD_RESULT
    Dim paused As Long
    
    result = FMOD_Channel_GetPaused(channel1, paused)
    ERRCHECK (result)

    If paused Then
        result = FMOD_Channel_SetPaused(channel1, 0)
    Else
        result = FMOD_Channel_SetPaused(channel1, 1)
    End If
    ERRCHECK (result)
End Sub

Private Sub cmdPause8bit_Click()
    Dim result As FMOD_RESULT
    Dim paused As Long
    
    result = FMOD_Channel_GetPaused(channel2, paused)
    ERRCHECK (result)

    If paused Then
        result = FMOD_Channel_SetPaused(channel2, 0)
    Else
        result = FMOD_Channel_SetPaused(channel2, 1)
    End If
    ERRCHECK (result)
End Sub

Private Sub cmdPlayStereo_Click()
    Dim result As FMOD_RESULT
    
    result = FMOD_System_PlaySound(system, FMOD_CHANNEL_FREE, sound3, 0, channel3)
    ERRCHECK (result)
End Sub

Private Sub cmdMovement_Click()
    PositionSlider.Enabled = listenerflag
    listenerflag = IIf(listenerflag = True, False, True)
End Sub

Private Sub PositionSlider_Click()
    listenerpos.x = PositionSlider.value * distancefactor
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
