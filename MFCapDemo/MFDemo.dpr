program MFDemo;

uses
  Vcl.Forms,
  MainForm in 'MainForm.pas' {MainFrm},
  MFCapDll in 'MFCapDll.pas';

{$R *.res}

begin
  Application.Initialize;
  Application.MainFormOnTaskbar := True;
  Application.CreateForm(TMainFrm, MainFrm);
  Application.Run;
end.
