Unit MFDevice;

Interface

Uses
  Windows, MFCAPDll, MFGenStream,
  Generics.Collections;

Type
  EMFDeviceEnumerateOption = (
    mdeoOpen,
    mdeoCompare
  );
  TMFDeviceEnumerateOptions = Set Of EMFDeviceEnumerateOption;

  EMFDeviceEnumerationStatus = (
    mdesNew,
    mdesDeleted,
    mdesPresent
  );

  TMFDevice = Class
    Private
      FUniqueName : WideString;
    Protected
      FHandle : Pointer;
      Class Function _Enumerate<T:TMFDevice>(APointer:Pointer; ARecordSize:Cardinal; ACount:Cardinal; AList:TObjectList<T>; AOptions:TMFDeviceEnumerateOptions = []):Cardinal;
    Public
      Class Function CreateInstance(ARecord:Pointer):TMFDevice; Virtual; Abstract;

      Constructor Create(AUniqueName:WideString); Reintroduce;
      Destructor Destroy; Override;
      Function Open:Cardinal; Virtual; Abstract;
      Procedure Close; Virtual; Abstract;
      Function EnumStreams(AList:TObjectList<TMFGenStream>):Cardinal; Virtual; Abstract;
      Function SelectStream(AIndex:Cardinal; ASelect:Boolean):Cardinal; Virtual; Abstract;

      Property Handle : Pointer Read FHandle;
      Property UniqueName : WideString Read FUniqueName;
    end;


Implementation

(** TMFDevice **)

Constructor TMFDevice.Create(AUniqueName:WideString);
begin
Inherited Create;
FUniqueName := AUniqueName;
end;

Destructor TMFDevice.Destroy;
begin
If Assigned(FHandle) Then
  Close;

Inherited Destroy;
end;

Class Function TMFDevice._Enumerate<T>(APointer:Pointer; ARecordSize:Cardinal; ACount:Cardinal; AList:TObjectList<T>; AOptions:TMFDeviceEnumerateOptions = []):Cardinal;
Var
  I : Integer;
  d : T;
  old : T;
  p : TPair<EMFDeviceEnumerationStatus, T>;
  prevailing : TDictionary<WideString, TPair<EMFDeviceEnumerationStatus, T>>;
begin
prevailing := TDictionary<WideString, TPair<EMFDeviceEnumerationStatus, T>>.Create;
If (mdeoCompare In AOptions) Then
  begin
  For d In  AList Do
    begin
    p.Key := mdesDeleted;
    p.Value := d;
    prevailing.AddOrSetValue(d.UniqueName, p);
    end;
  end;

For I := 0 To ACount - 1 Do
  begin
  d := T.CreateInstance(APointer) As T;
  If prevailing.TryGetValue(d.UniqueName, p) Then
    p.Key := mdesPresent
  Else p.Key := mdesNew;

  p.Value := d;
  prevailing.AddOrSetValue(d.UniqueName, p);
  APointer := Pointer(NativeUInt(APointer) + ARecordSize);
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

      AList.Add(p.Value);
      end;
    mdesDeleted: AList.Delete(AList.IndexOf(p.Value));
    mdesPresent: p.Value.Free;
    end;
  end;

prevailing.Free;
end;



End.

