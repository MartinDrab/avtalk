Unit MFGenStream;

Interface

Uses
  Windows, MFCapDll;

Type
  TMFGenStream = Class
    Private
      FInfo: MFGEN_STREAM_INFO;
      FIndex : Cardinal;
      FId : Cardinal;
      FMajorTypeGuid : TGuid;
      FStreamType : EMFCapFormatType;
      FMFDevice : Pointer;
      FSelected : Boolean;
      FNode : IUnknown;
    Public
      Constructor Create(ADevice:Pointer; Var ARecord:MFGEN_STREAM_INFO); Reintroduce;
      Destructor Destroy; Override;

      Property Index : Cardinal Read FIndex;
      Property Id : Cardinal Read FId;
      Property MajorTypeGuid : TGuid Read FMajorTypeGuid;
      Property StreamType : EMFCapFormatType Read FStreamType;
      Property MFDevice : Pointer Read FMFDevice;
      Property Selected : Boolean Read FSelected;
      Property Node : IUnknown Read FNode;
      Property Info : MFGEN_STREAM_INFO Read FInfo;
    end;

Implementation

(** TMFGenStream **)

Constructor TMFGenStream.Create(ADevice:Pointer; Var ARecord:MFGEN_STREAM_INFO);
Begin
Inherited Create;
FIndex := ARecord.Id;
FIndex := ARecord.Index;
FStreamType := ARecord.StreamType;
FMajorTypeGuid := ARecord.MajorType;
FMFDevice := ADevice;
FSelected := ARecord.Selected;
FNode := ARecord.Node;
FNode._AddRef;
FInfo := ARecord;
End;

Destructor TMFGenStream.Destroy;
begin
FNode._Release;
Inherited Destroy;
end;


End.

