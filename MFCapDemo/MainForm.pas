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
    procedure FormCreate(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure AudioInputListViewData(Sender: TObject; Item: TListItem);
    procedure RefreshAudioInputButtonClick(Sender: TObject);
    procedure AudioInputListViewItemChecked(Sender: TObject; Item: TListItem);
    procedure TestVideoOutputButtonClick(Sender: TObject);
  Private
    FAudioInList : TObjectList<TMFDevice>;
    FAudioInStreamList : TObjectList<TMFGenStream>;
    FVideoInList : TObjectList<TMFDevice>;
    FVideoInStreamList : TObjectList<TMFGenStream>;
    FAudioOutList : TObjectList<TMFDevice>;
    FAudioOutStreamList : TObjectList<TMFGenStream>;
  end;

Var
  MainFrm: TMainFrm;

Implementation

Uses
  Utils, MFCapDll;

{$R *.DFM}

Procedure TMainFrm.AudioInputListViewData(Sender: TObject; Item: TListItem);
Var
  s : TMFGenStream;
  d : TMFDevice;
  l : TListView;
  streamList : TObjectList<TMFGenStream>;
  streamTypeStr : WideString;
  tmp : TLVCheckedItemEvent;
begin
l := Sender As TListView;
s := Nil;
d := Nil;
If Sender = AudioInputListView Then
  streamList := FAudioInStreamList
Else If Sender = VideoInputListView Then
  streamList := FVideoInStreamList
Else If Sender = AudioOutputListView Then
  streamList := FAudioOutStreamList;

s := streamList[Item.Index];
d := s.MFDevice;
With Item  Do
  begin
  Caption := d.Name;
  SubItems.Add(d.UniqueName);
  streamTypeStr := '';
  Case s.StreamType Of
    mcftVideo: streamTypeStr := 'Video';
    mcftAudio: streamTypeStr := 'Audio';
    end;

  SubItems.Add(streamTypeStr);
  SubItems.Add(Format('%d', [s.Index]));
  tmp := l.OnItemChecked;
  l.OnItemChecked := Nil;
  Checked := s.Selected;
  l.OnItemChecked := tmp;
  end;
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
  end;

s := streamList[Item.Index];
d := s.MFDevice;
err := d.SelectStream(s.Index, Item.Checked);
If err <> 0 Then
  Win32ErrorMessage('Failed to (de)select stream', err);

Self.RefreshAudioInputButtonClick(b);
end;

Procedure TMainFrm.FormClose(Sender: TObject; var Action: TCloseAction);
begin
FVideoInStreamList.Free;
FAudioOutStreamList.Free;
FAudioInStreamList.Free;
FVideoInList.Free;
FAudioOutList.Free;
FAudioInList.Free;
end;

Procedure TMainFrm.FormCreate(Sender: TObject);
begin
FAudioInList := TObjectList<TMFDevice>.Create;
FAudioOutList := TObjectList<TMFDevice>.Create;
FVideoInList := TObjectList<TMFDevice>.Create;
FAudioInStreamList := TObjectList<TMFGenStream>.Create;
FAudioOutStreamList := TObjectList<TMFGenStream>.Create;
FVideoInStreamList := TObjectList<TMFGenStream>.Create;
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
l.Items.Clear;
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
      begin
      li := l.Items.Add;
      AudioInputListViewData(l, li);
      end;
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

Procedure TMainFrm.TestVideoOutputButtonClick(Sender: TObject);
Var
  b : TButton;
  err : Cardinal;
  s : TMFSession;
  o : TMFPlayDevice;
  sl : TObjectList<TMFGenStream>;
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

  err := TMFPlayDevice.NewInstanceForWindow(VideoTestOutputPanel.Handle, o);
  If err <> 0 Then
    begin
    s.Free;
    Win32ErrorMessage('Unable to create video output', err);
    Exit;
    end;

  sl := TObjectList<TMFGenStream>.Create;
  err := o.EnumStreams(sl);
  If err <> 0 Then
    begin
    sl.Free;
    s.Free;
    Win32ErrorMessage('Unable to enumerate video output streams', err);
    Exit;
    end;

  If sl.Count = 0 Then
    begin
    sl.Free;
    s.Free;
    ErrorMessage('No video output streams');
    Exit;
    end;

  If Not Assigned(VideoInputListView.Selected) Then
    begin
    sl.Free;
    s.Free;
    ErrorMessage('No video input device selected');
    Exit;
    end;

  err := s.AddEdge(FVideoInStreamList[VideoInputListView.Selected.Index], sl[0]);
  If err <> 0 Then
    begin
    sl.Free;
    s.Free;
    Win32ErrorMessage('Unable to add streams into the session', err);
    Exit;
    end;

  err := s.Start;
  If err <> 0 Then
    begin
    sl.Free;
    s.Free;
    Win32ErrorMessage('Unable to start the session', err);
    Exit;
    end;

  sl.Free;
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

