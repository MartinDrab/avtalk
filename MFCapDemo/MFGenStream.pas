Unit MFGenStream;

Interface

Uses
  Windows, MFCapDll;

Type
  TMFGenStream = Class
    Private
      FIndex : Cardinal;
      FId : Cardinal;
      FMajorTypeGuid : TGuid;
      FStreamType : EMFCapFormatType;
      FMFDevice : Pointer;
    Public
      Constructor Create(ADevice:Pointer; Var ARecord:MFGEN_STREAM_INFO); Reintroduce;

      Property Index : Cardinal Read FIndex;
      Property Id : Cardinal Read FId;
      Property MajorTypeGuid : TGuid Read FMajorTypeGuid;
      Property StreamType : EMFCapFormatType Read FStreamType;
      Property MFDevice : Pointer Read FMFDevice;
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
End;

End.
