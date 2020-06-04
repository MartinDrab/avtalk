program MFDemo;

uses
  Vcl.Forms,
  MainForm in 'MainForm.pas' {MainFrm},
  MFCapDll in 'MFCapDll.pas',
  MFCapDevice in 'MFCapDevice.pas',
  MFGenStream in 'MFGenStream.pas';

{$R *.res}

Var
  err : Cardinal;
begin
Application.Initialize;
err := MFGen_Init;
If err = 0 Then
  begin
  Application.MainFormOnTaskbar := True;
  Application.CreateForm(TMainFrm, MainFrm);
  Application.Run;
  MFGen_Finit;
  end;
end.

