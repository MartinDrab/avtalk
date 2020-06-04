Unit Utils;

Interface

Procedure ErrorMessage(AMsg:WideString);
Procedure WarningMessage(AMsg:WideString);
Procedure InformationMessage(AMsg:WideString);
Procedure Win32ErrorMessage(AMsg:WideString; AErrorCode:Cardinal);

Implementation

Uses
  WIndows, SysUtils;

Procedure _MessageBox(ACaption:WideString; AText:WideString; AType:Cardinal);
begin
MessageBoxW(0, PWideChar(AText), PWideChar(ACaption), AType Or MB_Ok);
end;

Procedure ErrorMessage(AMsg:WideString);
begin
_MessageBox('Error', AMsg, MB_ICONERROR);
end;

Procedure WarningMessage(AMsg:WideString);
begin
_MessageBox('Warning', AMsg, MB_ICONWARNING);
end;

Procedure InformationMessage(AMsg:WideString);
begin
_MessageBox('Information', AMsg, MB_ICONINFORMATION);
end;

Procedure Win32ErrorMessage(AMsg:WideString; AErrorCode:Cardinal);
begin
ErrorMessage(Format('%s'#13#10'Error: %u (0x%x)'#13#10'Description: %s', [AMsg, AErrorCode, AErrorCode, SysErrorMessage(AErrorCode)]));
end;


End.
