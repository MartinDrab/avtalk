Unit MFCapDevice;

Interface

Uses
  Windows, MFCapDll, Generics.Collections,
  MFGenStream, MFDevice;

Type
  TMFCapDevice = Class (TMFDevice)
  Private
    FIndex : Cardinal;
    FDeviceType : EMFCapFormatType;
    Constructor Create(Var ARecord:MFCAP_DEVICE_INFO); Reintroduce;
  Protected
  Public
    Class Function Enumerate(ADeviceType:EMFCapFormatType; AList:TObjectList<TMFDevice>; AOptions:TMFDeviceEnumerateOptions = []):Cardinal; Override;
    Class Function CreateInstance(ARecord:Pointer):TMFDevice; Override;

    Constructor CreateFromFile(AFileName:WideString); Reintroduce;

    Function SelectStream(AIndex:Cardinal; ASelect:Boolean):Cardinal; Override;
    Function Open:Cardinal; Override;
    Procedure Close; Override;
    Function EnumStreams(AList:TObjectList<TMFGenStream>):Cardinal; Override;

    Property Index : Cardinal Read FIndex;
    Property DeviceType : EMFCapFormatType Read FDeviceType;
  end;

Implementation

Uses
  SysUtils, Utils;

(** TMFCapDevice **)

Class Function TMFCapDevice.Enumerate(ADeviceType:EMFCapFormatType; AList:TObjectList<TMFDevice>; AOptions:TMFDeviceEnumerateOptions = []):Cardinal;
Var
  di : PMFCAP_DEVICE_INFO;
  count : Cardinal;
begin
Result := MFCap_EnumDevices(ADeviceType, di, count);
If Result = 0 Then
  begin
  Result := _Enumerate<TMFCapDevice>(di, SizeOf(MFCAP_DEVICE_INFO), count, AList, AOptions);
  MFCap_FreeDeviceEnumeration(di, count);
  end;
end;

Constructor TMFCapDevice.Create(Var ARecord:MFCAP_DEVICE_INFO);
begin
Inherited Create(WideCharToString(ARecord.SymbolicLink), WideCharToString(ARecord.FriendlyName));
FDeviceType := ARecord.DeviceType;
FIndex := ARecord.Index;
end;

Constructor TMFCapDevice.CreateFromFile(AFileName:WideString);
Var
  err : Cardinal;
begin
Inherited Create(AFileName, ExtractFileName(AFileName));
FIndex := $FFFFFFFF;
FDeviceType := mcftUnknown;
FHandle := Nil;
err := MFCapDll.MFCap_NewInstanceFromURL(PWideChar(AFileName), FHandle);
If err <> 0 Then
  begin
  Win32ErrorMessage('Cannot create a device from file', err);
  Raise Exception.Create('Cannot create a device from file');
  end;
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

Function TMFCapDevice.SelectStream(AIndex:Cardinal; ASelect:Boolean):Cardinal;
begin
Result := MFCap_SelectStream(FHandle, AIndex, ASelect);
end;

Class Function TMFCapDevice.CreateInstance(ARecord:Pointer):TMFDevice;
begin
Result := TMFCapDevice.Create(PMFCAP_DEVICE_INFO(ARecord)^);
end;


End.

