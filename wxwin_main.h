#ifndef MAINWINDOW_H_INCLUDED
  #define MAINWINDOW_H_INCLUDED

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
  #include <wx/wx.h>
#endif

#include "pcsx2_multiconfigtool.h"
#include "wxwin_selectpaths.h"

class PCSX2Tool_GUI : public wxApp
{
public:
  virtual bool OnInit(void);
private:
  PCSX2Tool *ptagPCSX2Tool;
};

class wxWinMain : public wxFrame
{
public:
  wxWinMain(wxWindow* parent,
            wxWindowID id = wxID_ANY,
            const wxString& title = wxEmptyString,
            const wxPoint& pos = wxDefaultPosition,
            const wxSize& size = wxSize( 515,280 ),
            long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL);

  void UpdateConfigList();

  bool Show(bool show);

  void OnPathSelect(wxCommandEvent& event);
  void OnShortcutCreate(wxCommandEvent& event);
  void OnOptionChangeStartNoGUI(wxCommandEvent& event);
  void OnOptionChangeStartFullscreen(wxCommandEvent& event);
  void OnOptionChangeStartFullBoot(wxCommandEvent& event);
  void OnAbout(wxCommandEvent& event);

  void OnNewConfig(wxCommandEvent& event);
  void OnRenameConfig(wxCommandEvent& event);
  void OnNewConfigSave(wxCommandEvent& event);
  void OnNewConfigCancel(wxCommandEvent& event);
  void OnConfigDelete(wxCommandEvent& event);
  void OnEditConfigStartPCSX2(wxCommandEvent& event);

  void OnClose(wxCloseEvent& event);

  wxWinPathConfigurator *wxPathCfg;
private:
  bool bRenameConfig;
  int iCurrSelection;

  void ShowCfgNameEdit(const wxString &name,
                       bool configExists);
  void HideCfgNameEdit();
  wxDECLARE_EVENT_TABLE();
protected:
    wxFlexGridSizer* layCFGEdit;
    wxListBox* listGameConfigs;
    wxButton* btnConfigCreateNew;
    wxButton* btnConfigRename;
    wxButton* btnConfigDelete;
    wxTextCtrl* txtNewCFGName;
    wxBoxSizer* laySaveCancelBtn;
    wxButton* btnConfigSave;
    wxButton* btnConfigCancel;
    wxButton* btnConfigStartPCSX2;
    wxMenuBar* m_menubar2;
    wxMenu* menuFile;
    wxMenuItem* menuItemCreateGameShortcut;
    wxMenu* menuOptions;
    wxMenu* menuSubGameShortcuts;
    wxMenuItem* menuItemEnableStartNoGUI;
    wxMenuItem* menuItemEnableStartFullscreen;
    wxMenuItem* menuItemEnableStartFullBoot;
    wxMenu* menuHelp;
};

#endif //MAINWINDOW_H_INCLUDED
