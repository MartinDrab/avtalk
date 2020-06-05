Unit MainForm;

Interface

Uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants, System.Classes, Vcl.Graphics,
  Vcl.Controls, Vcl.Forms, Vcl.Dialogs, Vcl.StdCtrls, Vcl.ComCtrls, Vcl.ExtCtrls,
  Generics.Collections,
  MFCapDevice, MFPlayDevices, MFGenStream;

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
    procedure FormCreate(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
    procedure AudioInputListViewData(Sender: TObject; Item: TListItem);
    procedure RefreshAudioInputButtonClick(Sender: TObject);
  Private
    FAudioInList : TObjectList<TMFCapDevice>;
    FAudioInStreamList : TObjectList<TMFGenStream>;
    FVideoInList : TObjectList<TMFCapDevice>;
    FVideoInStreamList : TObjectList<TMFGenStream>;
    FAudioOutList : TObjectList<TMFPlayDevice>;
    FAudioOutStreamList : TObjectList<TMFGenStream>;
  end;

Var
  MainFrm: TMainFrm;

Implementation

Uses
  Utils, MFDevice, MFCapDll;

{$R *.DFM}

Procedure TMainFrm.AudioInputListViewData(Sender: TObject; Item: TListItem);
Var
  s : TMFGenStream;
  d : TMFCapDevice;
  streamList : TObjectList<TMFGenStream>;
  streamTypeStr : WideString;
begin
s := Nil;
d := Nil;
If Sender = AudioInputListView Then
  streamList := FAudioInStreamList
Else If Sender = VideoInputListView Then
  streamList := FVideoInStreamList;

s := streamList[Item.Index];
d := s.MFDevice;
With Item  Do
  begin
  Caption := d.Name;
  SubItems.Add(d.SymbolicLink);
  streamTypeStr := '';
  Case s.StreamType Of
    mcftVideo: streamTypeStr := 'Video';
    mcftAudio: streamTypeStr := 'Audio';
    end;

  SubItems.Add(streamTypeStr);
  SubItems.Add(Format('%d', [s.Index]));
  Checked := s.Selected;
  end;
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
FAudioInList := TObjectList<TMFCapDevice>.Create;
FAudioOutList := TObjectList<TMFPlayDevice>.Create;
FVideoInList := TObjectList<TMFCapDevice>.Create;
FAudioInStreamList := TObjectList<TMFGenStream>.Create;
FAudioOutStreamList := TObjectList<TMFGenStream>.Create;
FVideoInStreamList := TObjectList<TMFGenStream>.Create;
end;

Procedure TMainFrm.RefreshAudioInputButtonClick(Sender: TObject);
Var
  err : Cardinal;
  d : TMFDevice;
  s : TMFGenStream;
  deviceList : TObjectList<TMFCapDevice>;
  streamList : TObjectList<TMFGenStream>;
  l : TListView;
  li : TListItem;
  deviceType : EMFCapFormatType;
begin
If Sender = RefreshAudioInputButton Then
  begin
  deviceList := FAudioInList;
  streamList := FAudioInStreamList;
  l := AudioInputListView;
  deviceType := mcftAudio;
  end
Else If Sender = RefreshVideoButton Then
  begin
  deviceList := FVideoInList;
  streamList := FVideoInStreamList;
  l := VideoInputListView;
  deviceType := mcftVideo;
  end;

l.Items.Clear;
streamList.Clear;
deviceList.Clear;
err := TMFCapDevice.Enumerate(deviceType, deviceList);
If err = 0 Then
  begin
  For d In deviceList Do
    begin
    err := d.Open;
    If err <> 0 Then
      begin
      Win32ErrorMessage('Unable to open the input device', err);
      Continue;
      end;

    err := d.EnumStreams(streamList);
    d.Close;
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
end;

End.

