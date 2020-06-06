Unit MFSession;

Interface

Uses
  Windows, MFCapDll, MFGenStream;

Type
  TMFSession = Class
  Private
    FHandle : Pointer;
    Constructor Create(AHandle:Pointer); Reintroduce;
  Public
    Destructor Destroy; Override;

    Class Function NewSession(Var ASession:TMFSession):Cardinal;

    Function AddEdge(ASource:TMFGenStream; ADest:TMFGenStream):Cardinal;
    Function Start:Cardinal;
    Function Stoop:Cardinal;

    Property Handle : Pointer Read FHandle;
  end;

Implementation

(** TMFSession **)

Class Function TMFSession.NewSession(Var ASession:TMFSession):Cardinal;
Var
  h : Pointer;
begin
Result := MFSession_NewInstance(h);
If Result = 0 Then
  ASession := TMFSession.Create(h);
end;

Constructor TMFSession.Create(AHandle:Pointer);
begin
Inherited Create;
FHandle := AHandle;
end;

Destructor TMFSession.Destroy;
begin
If Assigned(FHandle) Then
  MFSession_FreeInstance(FHandle);

Inherited Destroy;
end;

Function TMFSession.AddEdge(ASource:TMFGenStream; ADest:TMFGenStream):Cardinal;
Var
  s : MFGEN_STREAM_INFO;
  d : MFGEN_STREAM_INFO;
begin
s := ASource.Info;
d := ADest.Info;
Result := MFSession_ConnectNodes(FHandle, s, d);
end;

Function TMFSession.Start:Cardinal;
begin
Result := MFSession_Start(FHandle);
end;

Function TMFSession.Stoop:Cardinal;
begin
Result := MFSession_Stop(FHandle);
end;


End.
