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
      FSelected : Boolean;
    Public
      Constructor Create(ADevice:Pointer; Var ARecord:MFGEN_STREAM_INFO); Reintroduce;

      Property Index : Cardinal Read FIndex;
      Property Id : Cardinal Read FId;
      Property MajorTypeGuid : TGuid Read FMajorTypeGuid;
      Property StreamType : EMFCapFormatType Read FStreamType;
      Property MFDevice : Pointer Read FMFDevice;
      Property Selected : Boolean Read FSelected;
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
End;



End.
