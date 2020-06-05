object MainFrm: TMainFrm
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  Caption = 'MFRaptor'
  ClientHeight = 388
  ClientWidth = 610
  Color = clBtnFace
  CustomTitleBar.CaptionAlignment = taCenter
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnClose = FormClose
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object MainPageControl: TPageControl
    Left = 0
    Top = 0
    Width = 610
    Height = 388
    ActivePage = DevicesTabSheet
    Align = alClient
    TabOrder = 0
    ExplicitWidth = 508
    object DevicesTabSheet: TTabSheet
      Caption = 'Devices'
      ExplicitWidth = 500
      object AudioInputGroupBox: TGroupBox
        Left = 0
        Top = 0
        Width = 602
        Height = 122
        Align = alTop
        Caption = 'Audio input'
        TabOrder = 0
        object AUdioInputPanel: TPanel
          Left = 2
          Top = 15
          Width = 120
          Height = 105
          Align = alLeft
          TabOrder = 0
          ExplicitHeight = 72
          object TestAudioInputButton: TButton
            Left = 0
            Top = 0
            Width = 57
            Height = 28
            Caption = 'Test'
            TabOrder = 0
          end
          object RefreshAudioInputButton: TButton
            Left = 0
            Top = 34
            Width = 57
            Height = 28
            Caption = 'Refresh'
            TabOrder = 1
            OnClick = RefreshAudioInputButtonClick
          end
        end
        object AudioInputListView: TListView
          Left = 122
          Top = 15
          Width = 478
          Height = 105
          Align = alClient
          Columns = <
            item
              AutoSize = True
              Caption = 'Name'
            end
            item
              AutoSize = True
              Caption = 'Link'
            end
            item
              Caption = 'Steram ID'
              Width = 75
            end>
          OwnerData = True
          ReadOnly = True
          RowSelect = True
          TabOrder = 1
          ViewStyle = vsReport
          OnData = AudioInputListViewData
          ExplicitLeft = 152
          ExplicitWidth = 448
        end
      end
      object AudioOutputGroupBox: TGroupBox
        Left = 0
        Top = 122
        Width = 602
        Height = 73
        Align = alTop
        Caption = 'AUdio output'
        TabOrder = 1
        ExplicitTop = 89
        ExplicitWidth = 500
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
        Top = 195
        Width = 602
        Height = 165
        Align = alClient
        Caption = 'Video input'
        TabOrder = 2
        ExplicitTop = 162
        ExplicitWidth = 500
        ExplicitHeight = 198
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
          Height = 148
          Align = alLeft
          Color = clBlack
          ParentBackground = False
          TabOrder = 0
          ExplicitHeight = 181
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
