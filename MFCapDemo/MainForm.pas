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
    VideoInputComboBox: TComboBox;
    Label1: TLabel;
    TestVideoButton: TButton;
    AudioOutputPanel: TPanel;
    TestAudioOutputButton: TButton;
    AUdioInputPanel: TPanel;
    TestAudioInputButton: TButton;
    RefreshAudioInputButton: TButton;
    RefreshAudioOutputButton: TButton;
    RefreshVideoButton: TButton;
    AudioInputListView: TListView;
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
  Utils, MFCapDll;

{$R *.DFM}

Procedure TMainFrm.AudioInputListViewData(Sender: TObject; Item: TListItem);
Var
  s : TMFGenStream;
  d : TMFCapDevice;
begin
s := FAudioInStreamList[Item.Index];
d := s.MFDevice;
With Item  Do
  begin
  Caption := d.Name;
  SubItems.Add(d.SymbolicLink);
  SubItems.Add(Format('%d', [s.Id]))
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
  d : TMFCapDevice;
begin
AudioInputListView.Items.Count := 0;
FAudioInStreamList.Clear;
FAudioInList.Clear;
err := TMFCapDevice.Enumerate(mcftAudio, FAudioInList);
If err = 0 Then
  begin
  For d In FAudioInList Do
    begin
    err := d.Open;
    If err <> 0 Then
      begin
      Win32ErrorMessage('Unable to open the audio device', err);
      Continue;
      end;

    err := d.EnumStreams(FAudioInStreamList);
    d.Close;
    If err <> 0 Then
      begin
      Win32ErrorMessage('Unable to enumerate audio device streams', err);
      Break;
      end;
    end;

  If err <> 0 Then
    begin
    FAudioInStreamList.Clear;
    FAudioInList.Clear;
    end;

  AudioInputListView.Items.Count := FAudioInStreamList.Count;
  end
Else Win32ErrorMessage('Unable to enumerate audio input devices', err);
end;

End.

