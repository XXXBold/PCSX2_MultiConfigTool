#ifndef MAINWINDOW_H_INCLUDED
  #define MAINWINDOW_H_INCLUDED

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
  #include <wx/wx.h>
#endif

#include "wxWidgets_SelectPathsWindow.h"

class PCSX2Tool_GUI : public wxApp
{
public:
  virtual bool OnInit(void);
private:
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
  void OnClose(wxCloseEvent& event);
  void OnAbout(wxCommandEvent& event);

  void OnNewConfig(wxCommandEvent& event);
  void OnNewConfigSave(wxCommandEvent& event);
  void OnNewConfigCancel(wxCommandEvent& event);
  void OnConfigDelete(wxCommandEvent& event);
  void OnEditConfig(wxCommandEvent& event);

  wxWinPathConfigurator *wxPathCfg;
private:
  wxDECLARE_EVENT_TABLE();
protected:
  wxFlexGridSizer* layCFGEdit;
  wxStaticText* m_staticText7;
  wxStaticText* m_staticText8;
  wxListBox* listGameConfigs;
  wxButton* btnNewConfig;
  wxButton* btnConfigDelete;
  wxTextCtrl* txtNewCFGName;
  wxButton* btnNewConfigSave;
  wxButton* btnNewConfigCancel;
  wxButton* btnConfigStartPCSX2;
  wxMenuBar* m_menubar2;
  wxMenu* m_menu3;
  wxMenuItem* menuItemCreateGameShortcut;
  wxMenu* menuHelp;
};

#endif //MAINWINDOW_H_INCLUDED
