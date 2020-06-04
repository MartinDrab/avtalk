Unit MFPlayDevices;

Interface

Uses
  Windows, MFCapDll, Generics.Collections,
  MFGenStream;

Type
  TMFPlayDevice = Class
  Private
    FName : WideString;
    FDescription : WideString;
    FEndpointId : WideString;
    FState : Cardinal;
    FCharacteristics : Cardinal;
    FHandle : Pointer;
    Constructor Create(Var ARecord:MFPLAY_DEVICE_INFO); Reintroduce;
  Public
    Class Function Enumerate(AList:TList<TMFPlayDevice>):Cardinal;

    Destructor Destroy; Override;
    Function Open:Cardinal;
    Procedure Close;
    Function EnumStreams(AList:TList<TMFGenStream>):Cardinal;

    Property Name : WideString Read FName;
    Property Description : WideString Read FDescription;
    Property EndpointId : WideString Read FEndpointId;
    Property State : Cardinal Read FState;
    Property Characteristics : Cardinal Read FCharacteristics;
    Property Handle : Pointer Read FHandle;
  end;

Implementation

Uses
  SysUtils;

(** TMFCapDevice **)

Class Function TMFPlayDevice.Enumerate(AList:TList<TMFPlayDevice>):Cardinal;
Var
  I : Integer;
  d : PMFPLAY_DEVICE_INFO;
  tmp : PMFPLAY_DEVICE_INFO;
  count : Cardinal;
begin
Result := MFPlay_EnumDevices(DEVICE_STATEMASK_ALL, d, count);
If Result = 0 Then
  begin
  tmp := d;
  For I := 0 To count - 1 Do
    begin
    AList.Add(TMFPlayDevice.Create(tmp^));
    Inc(tmp);
    end;

  MFPlay_FreeDeviceEnum(d, count);
  end;
end;

Constructor TMFPlayDevice.Create(Var ARecord:MFPLAY_DEVICE_INFO);
begin
Inherited Create;
FName := WideCharToString(ARecord.Name);
FDescription := WideCharToString(ARecord.Description);
FEndpointId := WideCharToString(ARecord.EndpointId);
FState := ARecord.State;
FCharacteristics := ARecord.Characteristics;
end;

Destructor TMFPlayDevice.Destroy;
begin
If Assigned(FHandle) Then
  Close;

Inherited Destroy;
end;

Function TMFPlayDevice.Open:Cardinal;
Var
  d : MFPLAY_DEVICE_INFO;
begin
FillChar(d, SizeOf(d), 0);
d.EndpointId := PWideChar(FEndpointId);
Result := MFPlay_NewInstance(d, FHandle);
end;

Procedure TMFPlayDevice.Close;
begin
MFPlay_FreeInstance(FHandle);
FHandle := Nil;
end;

Function TMFPlayDevice.EnumStreams(AList:TList<TMFGenStream>):Cardinal;
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
    AList.Add(TMFGenStream.Create(tmp^));
    Inc(tmp);
    end;

  MFGen_FreeStreamNodes(streams, count);
  end;
end;



End.
