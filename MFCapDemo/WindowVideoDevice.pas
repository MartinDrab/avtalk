Unit WindowVideoDevice;

Interface

Uses
  Windows, MFCapDll, MFPlayDevices, MFGenStream;

Type
  TWindowVideoDevice = Class (TMFPlayDevice)
  Private
    FVideoStream : TMFGenStream;
  Public
    Class Function NewInstance(AWindow:HWND; Var ADevice:TWindowVideoDevice):Cardinal;

    Destructor Destroy; Override;

    Property VideoStream : TMFGenStream Read FVideoStream;
  end;


Implementation

Uses
  SysUtils, Generics.Collections;

(** TWindowVideoDevice **)

Destructor TWindowVideoDevice.Destroy;
begin
If Assigned(FVideoStream) Then
  FVideoStream.Free;

Inherited Destroy;
end;

Class Function TWindowVideoDevice.NewInstance(AWindow:HWND; Var ADevice:TWindowVideoDevice):Cardinal;
Var
  h : Pointer;
  r : MFPLAY_DEVICE_INFO;
  streamList : TObjectList<TMFGenStream>;
begin
Result := MFPlay_NewInstanceForWIndow(AWindow, h);
If Result = 0 Then
  begin
  r.State := DEVICE_STATE_ACTIVE;
  r.Name := 'Window';
  r.Description := 'Window-based video output';
  r.EndpointId := '0';
  r.Characteristics := 0;
  ADevice := TWindowVideoDevice.Create(r);
  ADevice.Handle := h;
  streamList := TObjectList<TMFGenStream>.Create;
  Result := ADevice.EnumStreams(streamList);
  If (Result = 0) And (streamList.Count > 0) Then
    begin
    ADevice.FVideoStream := streamList[0];
    streamList.OwnsObjects := False;
    streamList.Delete(0);
    streamList.OwnsObjects := True;
    end;

  streamList.Free;
  If Result <> 0 Then
    begin
    ADevice.Free;
    ADevice := Nil;
    end;
  end;
end;


End.
