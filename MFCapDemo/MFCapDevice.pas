Unit MFCapDevice;

Interface

Uses
  Windows, MFCapDll, Generics.Collections,
  MFGenStream;

Type
  TMFCapDevice = Class
  Private
    FName : WideString;
    FSymbolicLink : WideString;
    FIndex : Cardinal;
    FDeviceType : EMFCapFormatType;
    FHandle : Pointer;
    Constructor Create(Var ARecord:MFCAP_DEVICE_INFO); Reintroduce;
  Public
    Class Function Enumerate(ADeviceType:EMFCapFormatType; AList:TList<TMFCapDevice>):Cardinal;

    Destructor Destroy; Override;
    Function Open:Cardinal;
    Procedure Close;
    Function EnumStreams(AList:TList<TMFGenStream>):Cardinal;

    Property Name : WideString Read FName;
    Property SymbolicLink : WideString Read FSymbolicLink;
    Property Index : Cardinal Read FIndex;
    Property DeviceType : EMFCapFormatType Read FDeviceType;
    Property Handle : Pointer Read FHandle;
  end;

Implementation

Uses
  SysUtils;

(** TMFCapDevice **)

Class Function TMFCapDevice.Enumerate(ADeviceType:EMFCapFormatType; AList:TList<TMFCapDevice>):Cardinal;
Var
  I : Integer;
  d : PMFCAP_DEVICE_INFO;
  tmp : PMFCAP_DEVICE_INFO;
  count : Cardinal;
begin
Result := MFCap_EnumDevices(ADeviceType, d, count);
If Result = 0 Then
  begin
  tmp := d;
  For I := 0 To count - 1 Do
    begin
    AList.Add(TMFCapDevice.Create(tmp^));
    Inc(tmp);
    end;

  MFCap_FreeDeviceEnumeration(d, count);
  end;
end;

Constructor TMFCapDevice.Create(Var ARecord:MFCAP_DEVICE_INFO);
begin
Inherited Create;
FDeviceType := ARecord.DeviceType;
FIndex := ARecord.Index;
FName := WideCharToString(ARecord.FriendlyName);
FSymbolicLink := WideCharToString(ARecord.SymbolicLink);
end;

Destructor TMFCapDevice.Destroy;
begin
If Assigned(FHandle) Then
  Close;

Inherited Destroy;
end;

Function TMFCapDevice.Open:Cardinal;
begin
Result := MFCap_NewInstance(FDeviceType, FIndex, FHandle);
end;

Procedure TMFCapDevice.Close;
begin
MFCap_FreeInstance(FHandle);
FHandle := Nil;
end;

Function TMFCapDevice.EnumStreams(AList:TList<TMFGenStream>):Cardinal;
Var
  I : Integer;
  count : Cardinal;
  streams : PMFGEN_STREAM_INFO;
  tmp : PMFGEN_STREAM_INFO;
begin
Result := MFCap_CreateStreamNodes(FHandle, streams, count);
If Result = 0THen
  begin
  tmp := streams;
  FOr I := 0 To count - 1 Do
    begin
    AList.Add(TMFGenStream.Create(tmp^));
    Inc(tmp);
    end;

  MFGen_FreeStreamNodes(streams, count);
  end;
end;



End.
