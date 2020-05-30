object MainFrm: TMainFrm
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  Caption = 'MFRaptor'
  ClientHeight = 327
  ClientWidth = 454
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  PixelsPerInch = 96
  TextHeight = 13
  object DeviceGroupBox: TGroupBox
    Left = 0
    Top = 0
    Width = 454
    Height = 113
    Align = alTop
    Caption = 'Devices'
    TabOrder = 0
    object DeviceListView: TListView
      Left = 2
      Top = 15
      Width = 450
      Height = 96
      Align = alClient
      Columns = <
        item
          Caption = 'Type'
        end
        item
          AutoSize = True
          Caption = 'Name'
        end
        item
          AutoSize = True
          Caption = 'Link'
        end>
      ReadOnly = True
      RowSelect = True
      TabOrder = 0
      ViewStyle = vsReport
      ExplicitLeft = 32
      ExplicitWidth = 420
      ExplicitHeight = 64
    end
  end
  object StreamGroupBox: TGroupBox
    Left = 0
    Top = 113
    Width = 454
    Height = 72
    Align = alTop
    Caption = 'Streams'
    TabOrder = 1
  end
end
