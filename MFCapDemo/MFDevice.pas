Unit MFDevice;

Interface

Uses
  Windows, Generics.Collections, MFCAPDll, MFGenStream;

Type
  TMFDevice = Class
    Protected
      FHandle : Pointer;
    Public
      Destructor Destroy; Override;
      Function Open:Cardinal; Virtual; Abstract;
      Procedure Close; Virtual; Abstract;
      Function EnumStreams(AList:TObjectList<TMFGenStream>):Cardinal; Virtual; Abstract;

      Property Handle : Pointer Read FHandle;
    end;


Implementation

(** TMFDevice **)

Destructor TMFDevice.Destroy;
begin
If Assigned(FHandle) Then
  Close;

Inherited Destroy;
end;


End.
