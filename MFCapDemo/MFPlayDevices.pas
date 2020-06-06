Unit MFPlayDevices;

Interface

Uses
  Windows, MFCapDll, Generics.Collections,
  MFDevice, MFGenStream;

Type
  TMFPlayDevice = Class (TMFDevice)
  Private
    FName : WideString;
    FDescription : WideString;
    FState : Cardinal;
    FCharacteristics : Cardinal;
    FDeviceType : EMFCapFormatType;
    Constructor Create(Var ARecord:MFPLAY_DEVICE_INFO); Reintroduce;
  Public
    Class Function CreateInstance(ARecord:Pointer):TMFDevice; Override;
    Class Function Enumerate(ADeviceType:EMFCapFormatType; AList:TObjectList<TMFDevice>; AOptions:TMFDeviceEnumerateOptions = []):Cardinal; Override;

    Function Open:Cardinal; Override;
    Procedure Close; Override;
    Function EnumStreams(AList:TObjectList<TMFGenStream>):Cardinal; Override;

    Property Name : WideString Read FName;
    Property Description : WideString Read FDescription;
    Property State : Cardinal Read FState;
    Property Characteristics : Cardinal Read FCharacteristics;
    Property DeviceType : EMFCapFormatType Read FDeviceType;
  end;

Implementation

Uses
  SysUtils;

(** TMFPlayDevice **)

Class Function TMFPlayDevice.Enumerate(ADeviceType:EMFCapFormatType; AList:TObjectList<TMFDevice>; AOptions:TMFDeviceEnumerateOptions = []):Cardinal;
Var
  d : PMFPLAY_DEVICE_INFO;
  count : Cardinal;
begin
Result := MFPlay_EnumDevices(DEVICE_STATEMASK_ALL, d, count);
If Result = 0 Then
  begin
  Result := _Enumerate<TMFPlayDevice>(d, SizeOf(MFPLAY_DEVICE_INFO), count, AList, AOptions);
  MFPlay_FreeDeviceEnum(d, count);
  end;
end;

Constructor TMFPlayDevice.Create(Var ARecord:MFPLAY_DEVICE_INFO);
begin
Inherited Create(WideCharToString(ARecord.EndpointId), WideCharToString(ARecord.Name));
FDescription := WideCharToString(ARecord.Description);
FState := ARecord.State;
FCharacteristics := ARecord.Characteristics;
FDeviceType := mcftAudio;
end;

Function TMFPlayDevice.Open:Cardinal;
Var
  d : MFPLAY_DEVICE_INFO;
begin
FillChar(d, SizeOf(d), 0);
d.EndpointId := PWideChar(UniqueName);
Result := MFPlay_NewInstance(d, FHandle);
end;

Procedure TMFPlayDevice.Close;
begin
MFPlay_FreeInstance(FHandle);
FHandle := Nil;
end;

Function TMFPlayDevice.EnumStreams(AList:TObjectList<TMFGenStream>):Cardinal;
Var
  I : Integer;
  count : Cardinal;
  streams : PMFGEN_STREAM_INFO;
  tmp : PMFGEN_STREAM_INFO;
begin
Result := MFPlay_CreateStreamNodes(FHandle, streams, count);
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

Class Function TMFPlayDevice.CreateInstance(ARecord:Pointer):TMFDevice;
begin
Result := TMFPlayDevice.Create(PMFPLAY_DEVICE_INFO(ARecord)^);
end;


End.
