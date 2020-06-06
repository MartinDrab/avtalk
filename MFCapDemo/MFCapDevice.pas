Unit MFCapDevice;

Interface

Uses
  Windows, MFCapDll, Generics.Collections,
  MFGenStream, MFDevice;

Type
  TMFCapDevice = Class (TMFDevice)
  Private
    FName : WideString;
    FIndex : Cardinal;
    FDeviceType : EMFCapFormatType;
    Constructor Create(Var ARecord:MFCAP_DEVICE_INFO); Reintroduce;
  Public
    Class Function Enumerate(ADeviceType:EMFCapFormatType; AList:TObjectList<TMFCapDevice>; AOptions:TMFDeviceEnumerateOptions = []):Cardinal;

    Constructor CreateFromFile(AFileName:WideString); Reintroduce;

    Function SelectStream(AIndex:Cardinal; ASelect:Boolean):Cardinal;
    Function Open:Cardinal; Override;
    Procedure Close; Override;
    Function EnumStreams(AList:TObjectList<TMFGenStream>):Cardinal; Override;

    Property Name : WideString Read FName;
    Property Index : Cardinal Read FIndex;
    Property DeviceType : EMFCapFormatType Read FDeviceType;
  end;

Implementation

Uses
  SysUtils, Utils;

(** TMFCapDevice **)

Class Function TMFCapDevice.Enumerate(ADeviceType:EMFCapFormatType; AList:TObjectList<TMFCapDevice>; AOptions:TMFDeviceEnumerateOptions = []):Cardinal;
Var
  I : Integer;
  di : PMFCAP_DEVICE_INFO;
  tmp : PMFCAP_DEVICE_INFO;
  count : Cardinal;
  d : TMFDevice;
  old : TMFDevice;
  p : TPair<EMFDeviceEnumerationStatus, TMFDevice>;
  prevailing : TDictionary<WideString, TPair<EMFDeviceEnumerationStatus, TMFDevice>>;
begin
Result := MFCap_EnumDevices(ADeviceType, di, count);
If Result = 0 Then
  begin
  prevailing := TDictionary<WideString, TPair<EMFDeviceEnumerationStatus, TMFDevice>>.Create;
  If (mdeoCompare In AOptions) Then
    begin
    For d In  AList Do
      begin
      p.Key := mdesDeleted;
      p.Value := d;
      prevailing.AddOrSetValue(d.UniqueName, p);
      end;
    end;

  tmp := di;
  For I := 0 To count - 1 Do
    begin
    d := TMFCapDevice.Create(tmp^);
    If prevailing.TryGetValue(d.UniqueName, p) Then
      p.Key := mdesPresent
    Else p.Key := mdesNew;

    p.Value := d;
    prevailing.AddOrSetValue(d.UniqueName, p);
    Inc(tmp);
    end;

  For p In prevailing.Values Do
    begin
    Case p.Key Of
      mdesNew: begin
        If (mdeoOpen In AOptions) Then
          begin
          Result := p.Value.Open;
          If Result <> 0 Then
            begin
            p.Value.Free;
            Continue;
            end;
          end;

        AList.Add(p.Value As TMFCapDevice);
        end;
      mdesDeleted: AList.Delete(AList.IndexOf(p.Value As TMFCapDevice));
      mdesPresent: p.Value.Free;
      end;
    end;

  prevailing.Free;
  MFCap_FreeDeviceEnumeration(di, count);
  end;
end;

Constructor TMFCapDevice.Create(Var ARecord:MFCAP_DEVICE_INFO);
begin
Inherited Create(WideCharToString(ARecord.SymbolicLink));
FDeviceType := ARecord.DeviceType;
FIndex := ARecord.Index;
FName := WideCharToString(ARecord.FriendlyName);
end;

Constructor TMFCapDevice.CreateFromFile(AFileName:WideString);
Var
  err : Cardinal;
begin
Inherited Create(AFileName);
FName := ExtractFileName(AFileName);
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


End.

