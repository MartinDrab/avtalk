Unit MFDevice;

Interface

Uses
  Windows, Generics.Collections, MFCAPDll, MFGenStream;

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
    Public
      Constructor Create(AUniqueName:WideString); Reintroduce;
      Destructor Destroy; Override;
      Function Open:Cardinal; Virtual; Abstract;
      Procedure Close; Virtual; Abstract;
      Function EnumStreams(AList:TObjectList<TMFGenStream>):Cardinal; Virtual; Abstract;

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


End.
