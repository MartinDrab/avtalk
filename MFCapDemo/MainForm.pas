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
    RefreshAudioInputBUtton: TButton;
    RefreshAudioOutputButton: TButton;
    RefreshVideoButton: TButton;
    procedure FormCreate(Sender: TObject);
    procedure FormClose(Sender: TObject; var Action: TCloseAction);
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

{$R *.DFM}

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



End.

