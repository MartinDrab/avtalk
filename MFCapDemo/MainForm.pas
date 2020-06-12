Unit MainForm;

Interface

Uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes, Vcl.Graphics,
  Vcl.Controls, Vcl.Forms, Vcl.Dialogs, Vcl.StdCtrls, Vcl.ComCtrls, Vcl.ExtCtrls,
  Generics.Collections,
  MFCapDevice, MFPlayDevices, MFGenStream, MFDevice, MFSession;

Type
  TMainFrm = Class (TForm)
    MainPageControl: TPageControl;
    DevicesTabSheet: TTabSheet;
    AudioInputGroupBox: TGroupBox;
    AudioOutputGroupBox: TGroupBox;
    VideoInputGroupBox: TGroupBox;
    VideoTestOutputPanel: TPanel;
    AudioOutputPanel: TPanel;
    TestAudioOutputButton: TButton;
    AUdioInputPanel: TPanel;
    TestAudioInputButton: TButton;
    RefreshAudioInputButton: TButton;
    RefreshAudioOutputButton: TButton;
    AudioInputListView: TListView;
    Panel1: TPanel;
    RefreshVideoButton: TButton;
    VideoInputListView: TListView;
    TestVideoOutputButton: TButton;
    AudioOutputListView: TListView;
    RecordVideoButton: TButton;
    RecordAudioButton: TButton;
    InputSelectionTabSheet: TTabSheet;
    InputUpperPanel: TPanel;
    InputSelectionListView: TListView;
    RefreshInputSelectionButton: TButton;
    procedure FormCreate(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure RefreshAudioInputButtonClick(Sender: TObject);
    procedure AudioInputListViewItemChecked(Sender: TObject; Item: TListItem);
    procedure TestVideoOutputButtonClick(Sender: TObject);
    procedure RecordVideoButtonClick(Sender: TObject);
    procedure RefreshInputSelectionButtonClick(Sender: TObject);
  Private
    FAudioInList : TObjectList<TMFDevice>;
    FAudioInStreamList : TObjectList<TMFGenStream>;
    FVideoInList : TObjectList<TMFDevice>;
    FVideoInStreamList : TObjectList<TMFGenStream>;
    FAudioOutList : TObjectList<TMFDevice>;
    FAudioOutStreamList : TObjectList<TMFGenStream>;
    FInputDevices : TObjectList<TMFDevice>;
    Procedure ListViewProcessItem(AListView:TListView; AStream:TMFGenStream);
  end;

Var
  MainFrm: TMainFrm;

Implementation

Uses
  Utils, MFCapDll, WindowVideoDevice;

{$R *.DFM}

Procedure TMainFrm.ListViewProcessItem(AListView:TListView; AStream:TMFGenStream);
Var
  d : TMFDevice;
  streamTypeStr : WideString;
  tmp : TLVCheckedItemEvent;
begin
tmp := AListView.OnItemChecked;
AListView.OnItemChecked := Nil;
d := AStream.MFDevice;
With AListView.Items.Add  Do
  begin
  Caption := d.Name;
  SubItems.Add(d.UniqueName);
  streamTypeStr := '';
  Case AStream.StreamType Of
    mcftVideo: streamTypeStr := 'Video';
    mcftAudio: streamTypeStr := 'Audio';
    end;

  SubItems.Add(streamTypeStr);
  SubItems.Add(Format('%d:%d', [AStream.Id, AStream.Index]));
  Checked := AStream.Selected;
  end;

AListView.OnItemChecked := tmp;
end;

Procedure TMainFrm.AudioInputListViewItemChecked(Sender: TObject;
  Item: TListItem);
Var
  d : TMFDevice;
  s : TMFGenStream;
  streamList : TObjectList<TMFGenStream>;
  b : TButton;
  err : Cardinal;
begin
If Sender = AudioInputListView Then
  begin
  streamList := FAudioInStreamList;
  b := RefreshAudioInputButton;
  end
Else If Sender = VideoInputListView Then
  begin
  streamList := FVideoInStreamList;
  b := RefreshVideoButton;
  end
Else If Sender = AudioOutputListView Then
  begin
  streamList := FAudioOutStreamList;
  b := RefreshAudioOutputButton;
  end
Else If Sender = InputSelectionListView Then
  begin
  streamList := TObjectList<TMFGenStream>(InputSelectionListView.Tag);
  b := RefreshInputSelectionButton;
  end;

s := streamList[Item.Index];
d := s.MFDevice;
err := d.SelectStream(s.Index, Item.Checked);
If err <> 0 Then
  Win32ErrorMessage('Failed to (de)select stream', err);

b.OnClick(b);
end;

Procedure TMainFrm.FormClose(Sender: TObject; var Action: TCloseAction);
begin
FVideoInStreamList.Free;
FAudioOutStreamList.Free;
FAudioInStreamList.Free;
FVideoInList.Free;
FAudioOutList.Free;
FAudioInList.Free;
FInputDevices.Free;
end;

Procedure TMainFrm.FormCreate(Sender: TObject);
begin
FInputDevices := TObjectList<TMFDevice>.Create;
FAudioInList := TObjectList<TMFDevice>.Create;
FAudioOutList := TObjectList<TMFDevice>.Create;
FVideoInList := TObjectList<TMFDevice>.Create;
FAudioInStreamList := TObjectList<TMFGenStream>.Create;
FAudioOutStreamList := TObjectList<TMFGenStream>.Create;
FVideoInStreamList := TObjectList<TMFGenStream>.Create;
end;


Function _WriteCallback(APosition:UInt64; ABuffer:Pointer; ALength:Cardinal; Var AWritten:Cardinal; AContext:Pointer):Cardinal; Cdecl;
begin
AWritten := ALength;
Result := 0;
end;

Procedure TMainFrm.RecordVideoButtonClick(Sender: TObject);
Var
  b : TButton;
  d : TMFCapDevice;
  l : TListView;
  err : Cardinal;
  s : TMFSession;
  I : Integer;
  asfDevice : TMFPlayDevice;
  streamList : TObjectList<TMFGenStream>;
  inList : TObjectList<TMFGenStream>;
  outList : TObjectList<TMFGenStream>;
  outIndex : Integer;
begin
b := Sender As TButton;
If Sender = RecordVideoButton Then
  begin
  l := VideoInputListView;
  streamList := FVideoInStreamList;
  end
Else If Sender = RecordAudioButton Then
  begin
  l := AudioInputListView;
  streamList := FAudioInStreamList;
  end;

If b.Tag = 0 Then
  begin
  If Assigned(l.Selected) Then
    begin
    d := streamList[l.Selected.Index].MFDevice;
    err := d.CreateASFDevice(_WriteCallback, Nil, asfDevice);
    If err <> 0 Then
      begin
      Win32ErrorMessage('Failed to create ASF device', err);
      Exit;
      end;

    err := d.EnumStreamsCreateList(inList);
    If err <> 0 Then
      begin
      Win32ErrorMessage('Unable to enumerate input streams', err);
      asfDevice.Free;
      Exit;
      end;

    err := asfDevice.EnumStreamsCreateList(outList);
    If err <> 0 Then
      begin
      Win32ErrorMessage('Unable to enumerate output streams', err);
      inList.Free;
      asfDevice.Free;
      Exit;
      end;

    err := TMFSession.NewSession(s);
    If err <> 0 Then
      begin
      Win32ErrorMessage('Unable to create session', err);
      outList.Free;
      inList.Free;
      asfDevice.Free;
      Exit;
      end;

    outIndex := 0;
    For I := 0 To inList.Count - 1 Do
      begin
      If inList[I].Selected Then
        begin
        err := s.AddEdge(inList[I], outList[outIndex]);
        Inc(outIndex);
        end;

      If err <> 0 Then
        Break;
      end;

    If err <> 0 Then
      begin
      Win32ErrorMessage('Unable to add edges', err);
      s.Free;
      outList.Free;
      inList.Free;
      asfDevice.Free;
      Exit;
      end;

    err := s.Start;
    If err <> 0 Then
      begin
      Win32ErrorMessage('Unable start the session', err);
      s.Free;
      outList.Free;
      inList.Free;
      asfDevice.Free;
      Exit;
      end;

    b.Caption := 'Stop';
    b.Tag := NativeUInt(s);
    end
  Else WarningMessage('No device selected');
  end
Else begin
  s := TMFSession(Pointer(b.Tag));
  b.Tag := 0;
  b.Caption := 'Record';
  s.Stoop;
  s.Free;
  end;
end;

Procedure TMainFrm.RefreshAudioInputButtonClick(Sender: TObject);
Var
  err : Cardinal;
  d : TMFDevice;
  s : TMFGenStream;
  deviceList : TObjectList<TMFDevice>;
  streamList : TObjectList<TMFGenStream>;
  l : TListView;
  li : TListItem;
  deviceType : EMFCapFormatType;
  tmp : TLVCheckedItemEvent;
begin
If Sender = RefreshAudioInputButton Then
  begin
  deviceList := FAudioInList;
  streamList := FAudioInStreamList;
  l := AudioInputListView;
  deviceType := mcftAudio;
  err := TMFCapDevice.Enumerate(deviceType, deviceList, [mdeoOpen, mdeoCompare]);
  end
Else If Sender = RefreshVideoButton Then
  begin
  deviceList := FVideoInList;
  streamList := FVideoInStreamList;
  l := VideoInputListView;
  deviceType := mcftVideo;
  err := TMFCapDevice.Enumerate(deviceType, deviceList, [mdeoOpen, mdeoCompare]);
  end
Else If Sender = RefreshAudioOutputButton Then
  begin
  deviceList := FAudioOutList;
  streamList := FAudioOutStreamList;
  l := AudioOutputListView;
  deviceType := mcftAudio;
  err := TMFPlayDevice.Enumerate(deviceType, deviceList, [mdeoOpen, mdeoCompare]);
  end;

tmp := l.OnItemChecked;
l.OnItemChecked := Nil;
l.Clear;
streamList.Clear;
If err = 0 Then
  begin
  For d In deviceList Do
    begin
    err := d.EnumStreams(streamList);
    If err <> 0 Then
      begin
      Win32ErrorMessage('Unable to enumerate input device streams', err);
      Break;
      end;
    end;

  If err = 0 Then
    begin
    For s In streamList Do
      ListViewProcessItem(l, s);
    end;

  If err <> 0 Then
    begin
    l.Clear;
    streamList.Clear;
    deviceList.Clear;
    end;
  end
Else Win32ErrorMessage('Unable to enumerate audio input devices', err);

l.OnItemChecked := tmp;
end;

Procedure TMainFrm.RefreshInputSelectionButtonClick(Sender: TObject);
Var
  I : Integer;
  s : TMFGenStream;
  err : Cardinal;
  genD : TMFDevice;
  sl : TObjectList<TMFGenStream>;
  devRefs : TDictionary<TMFDevice, Cardinal>;
  p : TPair<TMFDevice, Cardinal>;
begin
If InputSelectionListView.Tag <> 0 Then
  begin
  sl := TObjectList<TMFGenStream>(InputSelectionListView.Tag);
  InputSelectionListView.Tag := 0;
  sl.Free;
  end;

devRefs := TDictionary<TMFDevice, Cardinal>.Create;
InputSelectionListView.Clear;
err := TMFCapDevice.Enumerate(mcftVideo, FInputDevices, [mdeoOpen, mdeoCompare]);
If err = 0 Then
  begin
  err := TMFCapDevice.Enumerate(mcftAudio, FInputDevices, [mdeoOpen]);
  If err = 0 THen
    begin
    sl := TObjectList<TMFGenStream>.Create;
    For genD In FInputDevices Do
      begin
      devRefs.Add(genD, 0);
      err := genD.EnumStreams(sl);
      If err <> 0 Then
        begin
        Win32ErrorMessage('Failed to enumerate streams for' + genD.Name, err);
        Break;
        end;
      end;

    If err = 0 Then
      begin
      I := 0;
      While I < sl.Count Do
        begin
        If (sl[I].Id < 1) Or (sl[I].Id > 127) THen
          begin
          sl.Delete(I);
          Continue;
          end;

        Inc(I);
        end;
      end;

    If err = 0 Then
      begin
      For s In sl Do
        begin
        p.Key := s.MFDevice;
        If Not devRefs.TryGetValue(p.Key, p.Value) Then
          begin
          err := $FFFFFFFF;
          ErrorMessage('Unknown device');
          Break;
          end;

        Inc(p.Value);
        devRefs.AddOrSetValue(p.Key, p.Value);
        ListViewProcessItem(InputSelectionListView, s);
        end;

      InputSelectionListView.Tag := NativeUInt(sl);
      For p In devRefs Do
        begin
        If p.Value = 0 Then
          FInputDevices.Delete(FInputDevices.IndexOf(p.Key));
        end;
      end;
    end
  Else Win32ErrorMessage('Failed to enumerate audio devices', err);
  end
Else Win32ErrorMessage('Failed to enumerate video devices', err);

devRefs.Free;
end;

Procedure TMainFrm.TestVideoOutputButtonClick(Sender: TObject);
Var
  b : TButton;
  err : Cardinal;
  s : TMFSession;
  o : TWindowVideoDevice;
begin
b := Sender As TButton;
s := TMFSession(Pointer(b.Tag));
If Not Assigned(s) Then
  begin
  err := TMFSession.NewSession(s);
  If err <> 0 Then
    begin
    Win32ErrorMessage('Unable to create video session', err);
    Exit;
    end;

  err := TWindowVideoDevice.NewInstance(VideoTestOutputPanel.Handle, o);
  If err <> 0 Then
    begin
    s.Free;
    Win32ErrorMessage('Unable to create video output', err);
    Exit;
    end;

  If Not Assigned(o.VideoStream) Then
    begin
    s.Free;
    ErrorMessage('No video output streams');
    Exit;
    end;

  If Not Assigned(VideoInputListView.Selected) Then
    begin
    s.Free;
    ErrorMessage('No video input device selected');
    Exit;
    end;

  err := s.AddEdge(FVideoInStreamList[VideoInputListView.Selected.Index], o.VideoStream);
  If err <> 0 Then
    begin
    s.Free;
    Win32ErrorMessage('Unable to add streams into the session', err);
    Exit;
    end;

  err := s.Start;
  If err <> 0 Then
    begin
    s.Free;
    Win32ErrorMessage('Unable to start the session', err);
    Exit;
    end;

  b.Tag := NativeUInt(s);
  b.Caption := 'Stop';
  end
Else begin
  s.Stoop;
  s.Free;
  b.Caption := 'Test';
  b.Tag := 0;
  end;
end;

End.

