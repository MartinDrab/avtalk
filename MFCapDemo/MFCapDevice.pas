Unit MFCapDevice;

Interface

Uses
  Windows, MFCapDll, Generics.Collections,
  MFGenStream, MFDevice;

Type
  TMFCapDevice = Class (TMFDevice)
  Private
    FName : WideString;
    FSymbolicLink : WideString;
    FIndex : Cardinal;
    FDeviceType : EMFCapFormatType;
    Constructor Create(Var ARecord:MFCAP_DEVICE_INFO); Reintroduce;
  Public
    Class Function Enumerate(ADeviceType:EMFCapFormatType; AList:TObjectList<TMFCapDevice>):Cardinal;

    Function Open:Cardinal; Override;
    Procedure Close; Override;
    Function EnumStreams(AList:TObjectList<TMFGenStream>):Cardinal; Override;

    Property Name : WideString Read FName;
    Property SymbolicLink : WideString Read FSymbolicLink;
    Property Index : Cardinal Read FIndex;
    Property DeviceType : EMFCapFormatType Read FDeviceType;
  end;

Implementation

Uses
  SysUtils;

(** TMFCapDevice **)

Class Function TMFCapDevice.Enumerate(ADeviceType:EMFCapFormatType; AList:TObjectList<TMFCapDevice>):Cardinal;
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

Function TMFCapDevice.Open:Cardinal;
begin
Result := MFCap_NewInstance(FDeviceType, FIndex, FHandle);
end;

Procedure TMFCapDevice.Close;
begin
MFCap_FreeInstance(FHandle);
FHandle := Nil;
end;

Function TMFCapDevice.EnumStreams(AList:TObjectList<TMFGenStream>):Cardinal;
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
    AList.Add(TMFGenStream.Create(Self, tmp^));
    Inc(tmp);
    end;

  MFGen_FreeStreamNodes(streams, count);
  end;
end;



End.

