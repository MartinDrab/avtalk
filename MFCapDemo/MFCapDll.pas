Unit MFCapDll;

{$Z4}
{$MINENUMSIZE 4}

Interface

Uses
  Windows;


Const
  MFCAP_CALLBACK_STREAMTICK			= 1;
  MFCAP_CALLBACK_ENDOFSTREAM		= 2;
  MFCAP_CALLBACK_ERROR				  = 4;

  DEVICE_STATE_ACTIVE = $1;
  DEVICE_STATE_DISABLED = $2;
  DEVICE_STATE_NOTPRESENT = $4;
  DEVICE_STATE_UNPLUGGED = $8;
  DEVICE_STATEMASK_ALL = (DEVICE_STATE_ACTIVE Or DEVICE_STATE_DISABLED Or DEVICE_STATE_NOTPRESENT Or DEVICE_STATE_UNPLUGGED);


Type
  _EMFCapFormatType = (
    mcftUnknown,
    mcftVideo,
    mcftAudio,
    mcftMax);
  EMFCapFormatType = _EMFCapFormatType;
  PEMFCapFormatType = ^EMFCapFormatType;

  _MFGEN_FORMAT = Record
    TypeGuid : TGuid;
    SubtypeGuid : TGuid;
    MediaType : Pointer;
    StreamIndex : Cardinal;
    Index : Cardinal;
    Selected : LongBool;
    FriendlyName : Packed Array [0..7] Of WideChar;
    Case fType : EMFCapFormatType Of
      mcftVideo : (
        Width : Cardinal;
        Height : Cardinal;
        BitRate : Cardinal;
        Framerate : Cardinal;
      );
      mcftAudio : (
        ChannelCount : Cardinal;
        SamplesPerSecond : Cardinal;
        BitsPerSample : Cardinal;
      );
    end;
  MFGEN_FORMAT = _MFGEN_FORMAT;
  PMFGEN_FORMAT = ^MFGEN_FORMAT;

  _EMCAP_DEVICE_CHARACTERISTICS = (
    mcdcLive,
    mcdcSeek,
    mcdcPause,
    mcdcSlowSeek,
    mcdcMultiplePresentations,
    mcdcSkipForward,
    mcdcSkipBackward,
    mcdcNetwork);
  MFCAP_DEVICE_CHARACTERISTICS = Set Of _EMCAP_DEVICE_CHARACTERISTICS;

  MFCAP_SAMPLE_CALLBACK = Function(ADevice:Pointer; AStreamIndex:Cardinal; AData:Pointer; ALength:Cardinal; ATimestamp:UInt64; AFlags:Cardinal; AContext:Pointer):LongBool; Cdecl;

  _MFCAP_DEVICE_INFO = Record
    FriendlyName : PWideChar;
    SymbolicLink : PWideChar;
    Index : Cardinal;
    DeviceType : EMFCapFormatType;
    end;
  MFCAP_DEVICE_INFO = _MFCAP_DEVICE_INFO;
  PMFCAP_DEVICE_INFO = ^MFCAP_DEVICE_INFO;

  _MFGEN_STREAM_INFO = Record
    MajorType : TGUid;
    StreamType : EMFCapFormatType;
    Index : Cardinal;
    Id : Cardinal;
    Node : Pointer;
    end;
  MFGEN_STREAM_INFO = _MFGEN_STREAM_INFO;
  PMFGEN_STREAM_INFO = ^MFGEN_STREAM_INFO;

  _MFPLAY_DEVICE_INFO = Record
	  Name : PWideChar;
	  Description : PWideChar;
	  EndpointId: PWideChar;
	  State : Cardinal;
	  Characteristics : Cardinal;
    end;
  MFPLAY_DEVICE_INFO = _MFPLAY_DEVICE_INFO;
  PMFPLAY_DEVICE_INFO = ^MFPLAY_DEVICE_INFO;

Function MFCap_EnumDevices(AType:EMFCapFormatType; Var ADevices:PMFCAP_DEVICE_INFO; Var ACount:Cardinal):Cardinal; Cdecl;
Procedure MFCap_FreeDeviceEnumeration(ADevices:PMFCAP_DEVICE_INFO; ACount:Cardinal); Cdecl;
Function MFCap_EnumMediaTypes(ADevice:Pointer; Var AFormats:PMFGEN_FORMAT; Var ACount:Cardinal; Var AStreamCount:Cardinal):Cardinal; Cdecl;
Function MFCap_NewInstance(AType:EMFCapFormatType; AIndex:Cardinal; Var ADevice:Pointer):Cardinal; Cdecl;
Procedure MFCap_FreeInstance(ADevice:Pointer); Cdecl;
Function MFCap_SetFormat(ADevice:Pointer; AStream:Cardinal; AMediaType:Pointer):Cardinal; Cdecl;
Function MFCap_SelectStream(ADevice:Pointer; AStream:Cardinal; ASelect:LongBool):Cardinal; Cdecl;
Function MFCap_Start(ADevice:Pointer; ACallback:MFCAP_SAMPLE_CALLBACK; AContext:Pointer):Cardinal; Cdecl;
Procedure MFCap_Stop(ADevice:Pointer); Cdecl;
Procedure MFCap_QueryStreamSelection(ADevice:Pointer; Var AMask:Cardinal); Cdecl;
Procedure MFCap_QueryCharacteristics(ADevice:Pointer; Var ACharacteristics:MFCAP_DEVICE_CHARACTERISTICS); Cdecl;
Function MFCap_CreateStreamNodes(ADevice:Pointer; Var ANodes:PMFGEN_STREAM_INFO; Var ACount:Cardinal):Cardinal; Cdecl;

Function MFPlay_EnumDevices(AStateMask:Cardinal; Var ADevices:PMFPLAY_DEVICE_INFO; Var ACount:Cardinal):Cardinal; Cdecl;
Procedure MFPlay_FreeDeviceEnum(ADevices:PMFPLAY_DEVICE_INFO; ACount:Cardinal); Cdecl;
Function MFPlay_EnumFormats(ADevice:Pointer; Var AFormats:PMFGEN_FORMAT; Var ACount:Cardinal; Var AStreamCount:Cardinal):Cardinal; Cdecl;
Function MFPlay_NewInstance(Var ADeviceInfo:MFPLAY_DEVICE_INFO; Var ADevice:Pointer):Cardinal; Cdecl;
Function MFPlay_NewInstanceForWindow(AWindow:HWND; Var ADevice:Pointer):Cardinal; Cdecl;
Procedure MFPlay_FreeInstance(ADevice:Pointer); Cdecl;
Function MFPlay_CreateStreamNodes(ADevice:Pointer; Var ANodes:PMFGEN_STREAM_INFO; Var ACount:Cardinal):Cardinal; Cdecl;

Function MFSession_NewInstance(Var ASession:Pointer):Cardinal; Cdecl;
Procedure MFSession_FreeInstance(ASession:Pointer); Cdecl;
Function MFSession_Start(ASession:Pointer):Cardinal; Cdecl;
Function MFSession_Stop(ASession:Pointer):Cardinal; Cdecl;
Function MFSession_ConnectNodes(ASession:Pointer; Var ASource:MFGEN_STREAM_INFO; Var ATarget:MFGEN_STREAM_INFO):Cardinal; Cdecl;

Function MFGen_MediaTypeToFormat(AMediaType:Pointer; Var AFormat:MFGEN_FORMAT):Cardinal; Cdecl;
Function MFGen_GetFormatProperties(Var AFormat:MFGEN_FORMAT; Var AGuids:Pointer; Var AValues:Pointer; Var ACount:Cardinal):Cardinal; Cdecl;
Function MFGen_GetProperties(AAttributes:Pointer; Var AGuids:PGuid; Var AValues:Pointer; Var ACount:Cardinal):Cardinal; Cdecl;
Procedure MFGen_FreeProperties(AGuids:PGuid; AValues:Pointer; ACount:Cardinal); Cdecl;
Procedure MFGen_FreeFormats(AFormats:PMFGEN_FORMAT; ACount:Cardinal); Cdecl;
Procedure MFGen_FreeStreamNodes(ANodes:PMFGEN_STREAM_INFO; ACount:Cardinal); Cdecl;

Function MFGen_Init:Cardinal; Cdecl;
Procedure MFGen_Finit; Cdecl;

Implementation

Const
  GENLibraryName = 'mfgen-dll.dll';
  CAPLibraryName = 'mfcap-dll.dll';
  PLAYLibraryName = 'mfplay-dll.dll';
  SESSIONLibraryName = 'mfsession-dll.dll';


Function MFCap_EnumDevices(AType:EMFCapFormatType; Var ADevices:PMFCAP_DEVICE_INFO; Var ACount:Cardinal):Cardinal; Cdecl; External CAPLibraryName;
Procedure MFCap_FreeDeviceEnumeration(ADevices:PMFCAP_DEVICE_INFO; ACount:Cardinal); Cdecl; External CAPLibraryName;
Function MFCap_EnumMediaTypes(ADevice:Pointer; Var AFormats:PMFGEN_FORMAT; Var ACount:Cardinal; Var AStreamCount:Cardinal):Cardinal; Cdecl; External CAPLibraryName;
Function MFCap_NewInstance(AType:EMFCapFormatType; AIndex:Cardinal; Var ADevice:Pointer):Cardinal; Cdecl; External CAPLibraryName;
Procedure MFCap_FreeInstance(ADevice:Pointer); Cdecl; External CAPLibraryName;
Function MFCap_SetFormat(ADevice:Pointer; AStream:Cardinal; AMediaType:Pointer):Cardinal; Cdecl; External CAPLibraryName;
Function MFCap_SelectStream(ADevice:Pointer; AStream:Cardinal; ASelect:LongBool):Cardinal; Cdecl; External CAPLibraryName;
Function MFCap_Start(ADevice:Pointer; ACallback:MFCAP_SAMPLE_CALLBACK; AContext:Pointer):Cardinal; Cdecl; External CAPLibraryName;
Procedure MFCap_Stop(ADevice:Pointer); Cdecl; External CAPLibraryName;
Procedure MFCap_QueryStreamSelection(ADevice:Pointer; Var AMask:Cardinal); Cdecl; External CAPLibraryName;
Procedure MFCap_QueryCharacteristics(ADevice:Pointer; Var ACharacteristics:MFCAP_DEVICE_CHARACTERISTICS); Cdecl; External CAPLibraryName;
Function MFCap_CreateStreamNodes(ADevice:Pointer; Var ANodes:PMFGEN_STREAM_INFO; Var ACount:Cardinal):Cardinal; Cdecl; External CAPLibraryName;

Function MFPlay_EnumDevices(AStateMask:Cardinal; Var ADevices:PMFPLAY_DEVICE_INFO; Var ACount:Cardinal):Cardinal; Cdecl; External PLAYLibraryName;
Procedure MFPlay_FreeDeviceEnum(ADevices:PMFPLAY_DEVICE_INFO; ACount:Cardinal); Cdecl; External PLAYLibraryName;
Function MFPlay_EnumFormats(ADevice:Pointer; Var AFormats:PMFGEN_FORMAT; Var ACount:Cardinal; Var AStreamCount:Cardinal):Cardinal; Cdecl; Cdecl; External PLAYLibraryName;
Function MFPlay_NewInstance(Var ADeviceInfo:MFPLAY_DEVICE_INFO; Var ADevice:Pointer):Cardinal; Cdecl; Cdecl; External PLAYLibraryName;
Function MFPlay_NewInstanceForWindow(AWindow:HWND; Var ADevice:Pointer):Cardinal; Cdecl; Cdecl; External PLAYLibraryName;
Procedure MFPlay_FreeInstance(ADevice:Pointer); Cdecl; Cdecl; External PLAYLibraryName;
Function MFPlay_CreateStreamNodes(ADevice:Pointer; Var ANodes:PMFGEN_STREAM_INFO; Var ACount:Cardinal):Cardinal; Cdecl; External PLAYLibraryName;

Function MFSession_NewInstance(Var ASession:Pointer):Cardinal; Cdecl; External SESSIONLibraryName;
Procedure MFSession_FreeInstance(ASession:Pointer); Cdecl; External SESSIONLibraryName;
Function MFSession_Start(ASession:Pointer):Cardinal; Cdecl; External SESSIONLibraryName;
Function MFSession_Stop(ASession:Pointer):Cardinal; Cdecl; External SESSIONLibraryName;
Function MFSession_ConnectNodes(ASession:Pointer; Var ASource:MFGEN_STREAM_INFO; Var ATarget:MFGEN_STREAM_INFO):Cardinal; Cdecl; External SESSIONLibraryName;

Procedure MFGen_FreeFormats(AFormats:PMFGEN_FORMAT; ACount:Cardinal); Cdecl; External GENLibraryName;
Procedure MFGen_FreeStreamNodes(ANodes:PMFGEN_STREAM_INFO; ACount:Cardinal); Cdecl; External GENLibraryName;
Function MFGen_MediaTypeToFormat(AMediaType:Pointer; Var AFormat:MFGEN_FORMAT):Cardinal; Cdecl; External GENLibraryName;
Function MFGen_GetFormatProperties(Var AFormat:MFGEN_FORMAT; Var AGuids:Pointer; Var AValues:Pointer; Var ACount:Cardinal):Cardinal; Cdecl; External GENLibraryName;
Function MFGen_GetProperties(AAttributes:Pointer; Var AGuids:PGuid; Var AValues:Pointer; Var ACount:Cardinal):Cardinal; Cdecl; External GENLibraryName;
Procedure MFGen_FreeProperties(AGuids:PGuid; AValues:Pointer; ACount:Cardinal); Cdecl; External GENLibraryName;

Function MFGen_Init:Cardinal; Cdecl; External GENLibraryName;
Procedure MFGen_Finit; Cdecl; External GENLibraryName;

End.

