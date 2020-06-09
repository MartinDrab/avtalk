object MainFrm: TMainFrm
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  Caption = 'MFRaptor'
  ClientHeight = 388
  ClientWidth = 637
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
    Width = 637
    Height = 388
    ActivePage = DevicesTabSheet
    Align = alClient
    TabOrder = 0
    ExplicitWidth = 610
    object DevicesTabSheet: TTabSheet
      Caption = 'Devices'
      ExplicitWidth = 602
      object AudioInputGroupBox: TGroupBox
        Left = 0
        Top = 0
        Width = 629
        Height = 97
        Align = alTop
        Caption = 'Audio input'
        TabOrder = 0
        ExplicitWidth = 602
        object AUdioInputPanel: TPanel
          Left = 2
          Top = 15
          Width = 120
          Height = 80
          Align = alLeft
          TabOrder = 0
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
          Width = 505
          Height = 80
          Align = alClient
          Checkboxes = True
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
              Caption = 'Type'
              Width = 60
            end
            item
              Caption = 'Steram index'
              Width = 75
            end>
          ReadOnly = True
          RowSelect = True
          ShowWorkAreas = True
          TabOrder = 1
          ViewStyle = vsReport
          OnData = AudioInputListViewData
          OnItemChecked = AudioInputListViewItemChecked
          ExplicitWidth = 478
        end
      end
      object AudioOutputGroupBox: TGroupBox
        Left = 0
        Top = 97
        Width = 629
        Height = 96
        Align = alTop
        Caption = 'AUdio output'
        TabOrder = 1
        ExplicitWidth = 602
        object AudioOutputPanel: TPanel
          Left = 2
          Top = 15
          Width = 120
          Height = 79
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
            OnClick = RefreshAudioInputButtonClick
          end
        end
        object AudioOutputListView: TListView
          Left = 122
          Top = 15
          Width = 505
          Height = 79
          Align = alClient
          Checkboxes = True
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
              Caption = 'Type'
              Width = 60
            end
            item
              Caption = 'Steram index'
              Width = 75
            end>
          ReadOnly = True
          RowSelect = True
          ShowWorkAreas = True
          TabOrder = 1
          ViewStyle = vsReport
          OnData = AudioInputListViewData
          OnItemChecked = AudioInputListViewItemChecked
          ExplicitWidth = 478
        end
      end
      object VideoInputGroupBox: TGroupBox
        Left = 0
        Top = 193
        Width = 629
        Height = 167
        Align = alClient
        Caption = 'Video input'
        TabOrder = 2
        OnClick = RefreshAudioInputButtonClick
        ExplicitWidth = 602
        object VideoTestOutputPanel: TPanel
          Left = 2
          Top = 15
          Width = 151
          Height = 150
          Align = alLeft
          Color = clBlack
          ParentBackground = False
          TabOrder = 0
        end
        object Panel1: TPanel
          Left = 153
          Top = 15
          Width = 79
          Height = 150
          Align = alLeft
          TabOrder = 1
          ExplicitLeft = 122
          object RefreshVideoButton: TButton
            Left = 6
            Top = -9
            Width = 57
            Height = 28
            Caption = 'Refresh'
            TabOrder = 0
            OnClick = RefreshAudioInputButtonClick
          end
          object TestVideoOutputButton: TButton
            Left = 6
            Top = 25
            Width = 57
            Height = 28
            Caption = 'Test'
            TabOrder = 1
            OnClick = TestVideoOutputButtonClick
          end
          object RecordVideoButton: TButton
            Left = 6
            Top = 59
            Width = 59
            Height = 30
            Caption = 'Record'
            TabOrder = 2
            OnClick = RecordVideoButtonClick
          end
        end
        object VideoInputListView: TListView
          Left = 232
          Top = 15
          Width = 395
          Height = 150
          Align = alClient
          Checkboxes = True
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
              Caption = 'Type'
              Width = 60
            end
            item
              Caption = 'Steram index'
              Width = 75
            end>
          ReadOnly = True
          RowSelect = True
          ShowWorkAreas = True
          TabOrder = 2
          ViewStyle = vsReport
          OnData = AudioInputListViewData
          OnItemChecked = AudioInputListViewItemChecked
          ExplicitLeft = 201
          ExplicitWidth = 399
        end
      end
    end
  end
end
