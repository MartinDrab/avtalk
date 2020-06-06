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
    procedure AudioInputListViewItemChecked(Sender: TObject; Item: TListItem);
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
  streamList := FVideoInStreamList;

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
  d : TMFCapDevice;
  s : TMFGenStream;
  streamList : TObjectList<TMFGenStream>;
  deviceList : TObjectList<TMFCapDevice>;
  b : TButton;
  err : Cardinal;
begin
If Sender = AudioInputListView Then
  begin
  deviceList := FAudioInList;
  streamList := FAudioInStreamList;
  b := RefreshAudioInputButton;
  end
Else If Sender = VideoInputListView Then
  begin
  deviceList := FVideoInList;
  streamList := FVideoInStreamList;
  b := RefreshVideoButton;
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
  tmp : TLVCheckedItemEvent;
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

tmp := l.OnItemChecked;
l.OnItemChecked := Nil;
l.Items.Clear;
streamList.Clear;
err := TMFCapDevice.Enumerate(deviceType, deviceList, [mdeoOpen, mdeoCompare]);
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

End.

