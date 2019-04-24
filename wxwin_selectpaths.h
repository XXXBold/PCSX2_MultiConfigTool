#ifndef SELECTPATHSWINDOW_H_INCLUDED
  #define SELECTPATHSWINDOW_H_INCLUDED

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
  #include <wx/wx.h>
#endif

#include "pcsx2_multiconfigtool.h"

#ifdef _WIN32
  #define SELECTPATHS_LABEL_PCSX2_EXE "Path to PCSX2-Executable, e.g. C:\\Program Files (x86)\\PCSX2_1.4.0\\pcsx2.exe"
  #define SELECTPATHS_LABEL_PCSX2_CFG "Path to Configs (inis-Folder), e.g. C:\\Users\\<user>\\Documents\\PCSX2\\inis"
  #define SELECTPATHS_LABEL_USER_CFG  "Your Path to Create the diffrent Configs in e.g. C:\\Users\\<user>\\Documents\\MyPCSX2Cfgs"
  #define SELECTPATHS_LABEL_GAMES     "Path to your folder containg your Games (the .iso-Files)"
#elif __linux__
  #define SELECTPATHS_LABEL_PCSX2_EXE "Path to PCSX2-Executable, e.g. /usr/games/PCSX2-linux.sh"
  #define SELECTPATHS_LABEL_PCSX2_CFG "Path to Configs (inis-Folder), e.g. /home/<user>/.config/PCSX2/inis"
  #define SELECTPATHS_LABEL_USER_CFG  "Your Path to Create the diffrent Configs in e.g. /home/<user>/.config/MyPCSX2Cfgs"
  #define SELECTPATHS_LABEL_GAMES     "Path to your folder containg your Games (the .iso-Files)"
#else
  #error not implemented
#endif /* _WIN32 */

class wxWinPathConfigurator : public wxFrame
{
  public:
    wxWinPathConfigurator(wxWindow* parent,
                          wxWindowID id = wxID_ANY,
                          const wxString& title = wxEmptyString,
                          const wxPoint& pos = wxDefaultPosition,
                          const wxSize& size = wxSize( 640,330 ),
                          long style = wxDEFAULT_FRAME_STYLE|wxTAB_TRAVERSAL);

    bool PathConfigIsValid(void);

    bool Show(bool);
    void OnBrowsePCSX2Games(wxCommandEvent& event);
    void OnBrowsePCSX2EXE(wxCommandEvent& event);
    void OnBrowsePCSX2CFG(wxCommandEvent& event);
    void OnBrowsePUserCFG(wxCommandEvent& event);
    void OnClose(wxCloseEvent &event);

  private:
    bool bPathsChanged;
    bool bUserCFGValid;
    bool bPCSX2CFGValid;
    wxDECLARE_EVENT_TABLE();
  protected:
    wxStaticText* m_staticText6;
    wxTextCtrl* txtPCSX2GamesPath;
    wxButton* btnBrowsePCSX2Games;
    wxStaticText* m_staticText1;
    wxTextCtrl* txtPCSX2ExePath;
    wxButton* btnBrowseExePath;
    wxStaticText* m_staticText2;
    wxTextCtrl* txtPCSX2CFGPath;
    wxButton* btnBrowseCfgs;
    wxStaticText* m_staticText3;
    wxTextCtrl* txtUserCFGPath;
    wxButton* btnBrowseUserCfgs;
};


#endif //SELECTPATHSWINDOW_H_INCLUDED
