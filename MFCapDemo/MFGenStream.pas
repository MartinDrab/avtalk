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
    Public
      Constructor Create(Var ARecord:MFGEN_STREAM_INFO); Reintroduce;

      Property Index : Cardinal Read FIndex;
      Property Id : Cardinal Read FId;
      Property MajorTypeGuid : TGuid Read FMajorTypeGuid;
      Property StreamType : EMFCapFormatType Read FStreamType;
    end;

Implementation

(** TMFGenStream **)

Constructor TMFGenStream.Create(Var ARecord:MFGEN_STREAM_INFO);
Begin
Inherited Create;
FIndex := ARecord.Id;
FIndex := ARecord.Index;
FStreamType := ARecord.StreamType;
FMajorTypeGuid := ARecord.MajorType;
End;

End.
