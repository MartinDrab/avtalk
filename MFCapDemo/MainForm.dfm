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
  object MainPageControl: TPageControl
    Left = 0
    Top = 0
    Width = 454
    Height = 327
    ActivePage = DevicesTabSheet
    Align = alClient
    TabOrder = 0
    object DevicesTabSheet: TTabSheet
      Caption = 'Devices'
      ExplicitLeft = 8
      ExplicitTop = 28
      object AudioInputGroupBox: TGroupBox
        Left = 0
        Top = 0
        Width = 446
        Height = 89
        Align = alTop
        Caption = 'Audio input'
        TabOrder = 0
        ExplicitLeft = 40
        ExplicitTop = 40
        ExplicitWidth = 121
        object AUdioInputPanel: TPanel
          Left = 2
          Top = 15
          Width = 120
          Height = 72
          Align = alLeft
          TabOrder = 0
          ExplicitTop = 14
          object TestAudioInputButton: TButton
            Left = 0
            Top = 0
            Width = 57
            Height = 28
            Caption = 'Test'
            TabOrder = 0
          end
          object RefreshAudioInputBUtton: TButton
            Left = 0
            Top = 34
            Width = 57
            Height = 28
            Caption = 'Refresh'
            TabOrder = 1
          end
        end
      end
      object AudioOutputGroupBox: TGroupBox
        Left = 0
        Top = 89
        Width = 446
        Height = 73
        Align = alTop
        Caption = 'AUdio output'
        TabOrder = 1
        ExplicitTop = 83
        object AudioOutputPanel: TPanel
          Left = 2
          Top = 15
          Width = 120
          Height = 56
          Align = alLeft
          TabOrder = 0
          object TestAudioOutputButton: TButton
            Left = -2
            Top = -9
            Width = 57
            Height = 28
            Caption = 'Test'
            TabOrder = 0
          end
          object RefreshAudioOutputButton: TButton
            Left = 0
            Top = 24
            Width = 57
            Height = 28
            Caption = 'Refresh'
            TabOrder = 1
          end
        end
      end
      object VideoInputGroupBox: TGroupBox
        Left = 0
        Top = 162
        Width = 446
        Height = 137
        Align = alClient
        Caption = 'Video input'
        TabOrder = 2
        ExplicitLeft = 56
        ExplicitTop = 200
        ExplicitWidth = 81
        ExplicitHeight = 65
        object Label1: TLabel
          Left = 128
          Top = 21
          Width = 60
          Height = 13
          Caption = 'Input device'
        end
        object VideoTestOutputPanel: TPanel
          Left = 2
          Top = 15
          Width = 120
          Height = 120
          Align = alLeft
          Color = clBlack
          ParentBackground = False
          TabOrder = 0
        end
        object VideoInputComboBox: TComboBox
          Left = 128
          Top = 40
          Width = 154
          Height = 21
          Style = csDropDownList
          TabOrder = 1
        end
        object TestVideoButton: TButton
          Left = 128
          Top = 78
          Width = 89
          Height = 25
          Caption = 'Test'
          TabOrder = 2
        end
        object RefreshVideoButton: TButton
          Left = 128
          Top = 109
          Width = 89
          Height = 25
          Caption = 'Refresh'
          TabOrder = 3
        end
      end
    end
  end
end
