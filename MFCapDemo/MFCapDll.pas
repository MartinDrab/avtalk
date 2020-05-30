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


Type
  _EMFCapFormatType = (
    mcftUnknown,
    mcftVideo,
    mcftAudio,
    mcftMax);
  EMFCapFormatType = _EMFCapFormatType;
  PEMFCapFormatType = ^EMFCapFormatType;

  _MFCAP_FORMAT = Record
    TypeGuid : TGuid;
    SubtypeGuid : TGuid;
    MediaType : Pointer;
    StreamIndex : Cardinal;
    Index : Cardinal;
    Selected : LongBool;
    FriendlyName : Packed Array [0..7] Of WideChar;
//    fType : EMFCapFormatType;
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
  MFCAP_FORMAT = _MFCAP_FORMAT;
  PMFCAP_FORMAT = ^MFCAP_FORMAT;

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

  _MFCAP_DEVICE_ATTRIBUTES = Record
    LowLatencyDelay : Cardinal;
    DisableConverters : LongBool;
    HardwareTransforms : LongBool;
    ExtraSoftwareProcessing : LongBool;
    end;
  MFCAP_DEVICE_ATTRIBUTES = _MFCAP_DEVICE_ATTRIBUTES;
  PMFCAP_DEVICE_ATTRIBUTES = ^MFCAP_DEVICE_ATTRIBUTES;

  _MFCAP_DEVICE_INFO = Record
    FriendlyName : PWideChar;
    SymbolicLink : PWideChar;
    Index : Cardinal;
    DeviceType : EMFCapFormatType;
    end;
  MFCAP_DEVICE_INFO = _MFCAP_DEVICE_INFO;
  PMFCAP_DEVICE_INFO = ^MFCAP_DEVICE_INFO;



Function MFCap_EnumDevices(Var ADevices:PMFCAP_DEVICE_INFO; Var ACount:Cardinal):Cardinal; Cdecl;
Procedure MFCap_FreeDeviceEnumeration(ADevices:PMFCAP_DEVICE_INFO; ACount:Cardinal); Cdecl;
Function MFCap_EnumMediaTypes(ADevice:Pointer; Var AFormats:PMFCAP_FORMAT; Var ACount:Cardinal; Var AStreamCount:Cardinal):Cardinal; Cdecl;
Procedure MFCap_FreeMediaTypes(AFormats:PMFCAP_FORMAT; ACount:Cardinal); Cdecl;
Function MFCap_NewInstance(AType:EMFCapFormatType; AIndex:Cardinal; Var AAttributes:MFCAP_DEVICE_ATTRIBUTES; Var ADevice:Pointer):Cardinal; Cdecl;
Procedure MFCap_FreeInstance(ADevice:Pointer); Cdecl;
Function MFCap_SetFormat(ADevice:Pointer; AStream:Cardinal; AMediaType:Pointer):Cardinal; Cdecl;
Function MFCap_SelectStream(ADevice:Pointer; AStream:Cardinal; ASelect:LongBool):Cardinal; Cdecl;
Function MFCap_Start(ADevice:Pointer; ACallback:MFCAP_SAMPLE_CALLBACK; AContext:Pointer):Cardinal; Cdecl;
Procedure MFCap_Stop(ADevice:Pointer); Cdecl;
Procedure MFCap_QueryStreamSelection(ADevice:Pointer; Var AMask:Cardinal); Cdecl;
Procedure MFCap_QueryCharacteristics(ADevice:Pointer; Var ACharacteristics:MFCAP_DEVICE_CHARACTERISTICS); Cdecl;
Function MFCap_Init:Cardinal; Cdecl;
Procedure MFCap_Finit; Cdecl;

Implementation

Const
  LibraryName = 'mfcap-dll.dll';


Function MFCap_EnumDevices(Var ADevices:PMFCAP_DEVICE_INFO; Var ACount:Cardinal):Cardinal; Cdecl; External LibraryName;
Procedure MFCap_FreeDeviceEnumeration(ADevices:PMFCAP_DEVICE_INFO; ACount:Cardinal); Cdecl; External LibraryName;
Function MFCap_EnumMediaTypes(ADevice:Pointer; Var AFormats:PMFCAP_FORMAT; Var ACount:Cardinal; Var AStreamCount:Cardinal):Cardinal; Cdecl; External LibraryName;
Procedure MFCap_FreeMediaTypes(AFormats:PMFCAP_FORMAT; ACount:Cardinal); Cdecl; External LibraryName;
Function MFCap_NewInstance(AType:EMFCapFormatType; AIndex:Cardinal; Var AAttributes:MFCAP_DEVICE_ATTRIBUTES; Var ADevice:Pointer):Cardinal; Cdecl; External LibraryName;
Procedure MFCap_FreeInstance(ADevice:Pointer); Cdecl; External LibraryName;
Function MFCap_SetFormat(ADevice:Pointer; AStream:Cardinal; AMediaType:Pointer):Cardinal; Cdecl; External LibraryName;
Function MFCap_SelectStream(ADevice:Pointer; AStream:Cardinal; ASelect:LongBool):Cardinal; Cdecl; External LibraryName;
Function MFCap_Start(ADevice:Pointer; ACallback:MFCAP_SAMPLE_CALLBACK; AContext:Pointer):Cardinal; Cdecl; External LibraryName;
Procedure MFCap_Stop(ADevice:Pointer); Cdecl; External LibraryName;
Procedure MFCap_QueryStreamSelection(ADevice:Pointer; Var AMask:Cardinal); Cdecl; External LibraryName;
Procedure MFCap_QueryCharacteristics(ADevice:Pointer; Var ACharacteristics:MFCAP_DEVICE_CHARACTERISTICS); Cdecl; External LibraryName;
Function MFCap_Init:Cardinal; Cdecl; External LibraryName;
Procedure MFCap_Finit; Cdecl; External LibraryName;

End.
