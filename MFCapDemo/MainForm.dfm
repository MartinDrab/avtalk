object MainFrm: TMainFrm
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  Caption = 'MFRaptor'
  ClientHeight = 388
  ClientWidth = 610
  Color = clBtnFace
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
    object DevicesTabSheet: TTabSheet
      Caption = 'Devices'
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
          TabOrder = 1
          ViewStyle = vsReport
          OnData = AudioInputListViewData
          OnItemChecked = AudioInputListViewItemChecked
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
            OnClick = RefreshAudioInputButtonClick
          end
        end
        object AudioOutputListView: TListView
          Left = 122
          Top = 15
          Width = 478
          Height = 56
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
          TabOrder = 1
          ViewStyle = vsReport
          OnData = AudioInputListViewData
          OnItemChecked = AudioInputListViewItemChecked
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
        OnClick = RefreshAudioInputButtonClick
        object VideoTestOutputPanel: TPanel
          Left = 2
          Top = 15
          Width = 120
          Height = 148
          Align = alLeft
          Color = clBlack
          ParentBackground = False
          TabOrder = 0
        end
        object Panel1: TPanel
          Left = 122
          Top = 15
          Width = 79
          Height = 148
          Align = alLeft
          TabOrder = 1
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
          end
        end
        object VideoInputListView: TListView
          Left = 201
          Top = 15
          Width = 399
          Height = 148
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
          TabOrder = 2
          ViewStyle = vsReport
          OnData = AudioInputListViewData
          OnItemChecked = AudioInputListViewItemChecked
        end
      end
    end
  end
end
