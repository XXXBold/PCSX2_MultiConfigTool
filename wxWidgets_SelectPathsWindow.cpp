#include <wx/wxprec.h>
#ifndef WX_PRECOMP
  #include <wx/wx.h>
#endif

#include "wxWidgets_SelectPathsWindow.h"
#include "wxWidgets_functions.h"

enum
{
  // Buttons
  BTN_Browse_PCSX2Games,
  BTN_Browse_PCSX2EXE,
  BTN_Browse_PCSX2CFGs,
  BTN_Browse_UserCFGs
};

wxBEGIN_EVENT_TABLE(wxWinPathConfigurator,wxFrame)
EVT_BUTTON(BTN_Browse_PCSX2Games,wxWinPathConfigurator::OnBrowsePCSX2Games)
EVT_BUTTON(BTN_Browse_PCSX2EXE,wxWinPathConfigurator::OnBrowsePCSX2EXE)
EVT_BUTTON(BTN_Browse_PCSX2CFGs,wxWinPathConfigurator::OnBrowsePCSX2CFG)
EVT_BUTTON(BTN_Browse_UserCFGs,wxWinPathConfigurator::OnBrowsePUserCFG)
EVT_CLOSE(wxWinPathConfigurator::OnClose)
wxEND_EVENT_TABLE()

wxWinPathConfigurator::wxWinPathConfigurator(wxWindow* parent,
                                             wxWindowID id,
                                             const wxString& title,
                                             const wxPoint& pos,
                                             const wxSize& size,
                                             long style )
                      : wxFrame( parent, id, title, pos, size, style )
{
  this->SetSizeHints( wxDefaultSize, wxDefaultSize );

  wxBoxSizer* layoutVertical;
  layoutVertical = new wxBoxSizer( wxVERTICAL );

  wxFlexGridSizer* fgSizer5;
  fgSizer5 = new wxFlexGridSizer( 2, 2, 0, 5 );
  fgSizer5->AddGrowableCol( 0 );
  fgSizer5->SetFlexibleDirection( wxHORIZONTAL );
  fgSizer5->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

  m_staticText6 = new wxStaticText( this, wxID_ANY, wxT("Path to your folder containg your Games (the .iso-Files)"), wxDefaultPosition, wxDefaultSize, 0 );
  m_staticText6->Wrap( -1 );
  fgSizer5->Add( m_staticText6, 0, wxALL, 5 );


  fgSizer5->Add( 0, 0, 1, wxEXPAND, 5 );

  txtPCSX2GamesPath = new wxTextCtrl( this, wxID_ANY, wxT("/home/myps2games"), wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
  fgSizer5->Add( txtPCSX2GamesPath, 0, wxEXPAND, 5 );

  btnBrowsePCSX2Games = new wxButton( this, BTN_Browse_PCSX2Games, wxT("Browse..."), wxDefaultPosition, wxDefaultSize, 0 );
  fgSizer5->Add( btnBrowsePCSX2Games, 0, 0, 5 );


  layoutVertical->Add( fgSizer5, 1, wxALL|wxEXPAND, 5 );

  wxFlexGridSizer* fgSizer1;
  fgSizer1 = new wxFlexGridSizer( 2, 2, 0, 5 );
  fgSizer1->AddGrowableCol( 0 );
  fgSizer1->SetFlexibleDirection( wxHORIZONTAL );
  fgSizer1->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

  m_staticText1 = new wxStaticText( this, wxID_ANY, wxT("Path to PCSX2-Executable, e.g. C:\\Program Files (x86)\\PCSX2_1.4.0\\pcsx2.exe"), wxDefaultPosition, wxDefaultSize, 0 );
  m_staticText1->Wrap( -1 );
  fgSizer1->Add( m_staticText1, 0, wxALL, 5 );


  fgSizer1->Add( 0, 0, 1, wxEXPAND, 5 );

  txtPCSX2ExePath = new wxTextCtrl( this, wxID_ANY, wxT("/usr/games"), wxDefaultPosition, wxSize( -1,-1 ), wxTE_READONLY );
  fgSizer1->Add( txtPCSX2ExePath, 0, wxEXPAND, 5 );

  btnBrowseExePath = new wxButton( this, BTN_Browse_PCSX2EXE, wxT("Browse..."), wxDefaultPosition, wxDefaultSize, 0 );
  fgSizer1->Add( btnBrowseExePath, 0, 0, 5 );


  layoutVertical->Add( fgSizer1, 1, wxALL|wxEXPAND, 5 );

  wxFlexGridSizer* fgSizer2;
  fgSizer2 = new wxFlexGridSizer( 2, 2, 0, 5 );
  fgSizer2->AddGrowableCol( 0 );
  fgSizer2->SetFlexibleDirection( wxHORIZONTAL );
  fgSizer2->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

  m_staticText2 = new wxStaticText( this, wxID_ANY, wxT("Path to Configs (inis-Folder), e.g. C:\\Users\\<user>\\Documents\\PCSX2\\inis"), wxDefaultPosition, wxDefaultSize, 0 );
  m_staticText2->Wrap( -1 );
  fgSizer2->Add( m_staticText2, 0, wxALL, 5 );


  fgSizer2->Add( 0, 0, 1, wxEXPAND, 5 );

  txtPCSX2CFGPath = new wxTextCtrl( this, wxID_ANY, wxT("/usr/cfgs"), wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
  fgSizer2->Add( txtPCSX2CFGPath, 0, wxEXPAND, 5 );

  btnBrowseCfgs = new wxButton( this, BTN_Browse_PCSX2CFGs, wxT("Browse..."), wxDefaultPosition, wxDefaultSize, 0 );
  fgSizer2->Add( btnBrowseCfgs, 0, 0, 5 );


  layoutVertical->Add( fgSizer2, 1, wxALL|wxEXPAND, 5 );

  wxFlexGridSizer* fgSizer3;
  fgSizer3 = new wxFlexGridSizer( 2, 2, 0, 5 );
  fgSizer3->AddGrowableCol( 0 );
  fgSizer3->SetFlexibleDirection( wxHORIZONTAL );
  fgSizer3->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

  m_staticText3 = new wxStaticText( this, wxID_ANY, wxT("Your Path to Create the diffrent Configs in e.g. C:\\Users\\<user>\\Documents\\MyPCSX2Cfgs"), wxDefaultPosition, wxDefaultSize, 0 );
  m_staticText3->Wrap( -1 );
  fgSizer3->Add( m_staticText3, 0, wxALL, 5 );


  fgSizer3->Add( 0, 0, 1, wxEXPAND, 5 );

  txtUserCFGPath = new wxTextCtrl( this, wxID_ANY, wxT("/home/user"), wxDefaultPosition, wxDefaultSize, wxTE_READONLY );
  fgSizer3->Add( txtUserCFGPath, 0, wxEXPAND, 5 );

  btnBrowseUserCfgs = new wxButton( this, BTN_Browse_UserCFGs, wxT("Browse..."), wxDefaultPosition, wxDefaultSize, 0 );
  fgSizer3->Add( btnBrowseUserCfgs, 0, 0, 5 );


  layoutVertical->Add( fgSizer3, 1, wxALL|wxEXPAND, 5 );


  this->SetSizer( layoutVertical );
  this->Layout();

  this->Centre( wxBOTH );

  this->bPCSX2CFGValid=checkPCSX2CFGPathValid(pcsx2Tool_m->GetPCSX2CFGPath());
  this->bUserCFGValid=(checkUserCFGPathValid(pcsx2Tool_m->GetUserCFGPath())<0)?false:true;
}

// event handlers

bool wxWinPathConfigurator::Show(bool show)
{
  wxPuts("Show/hide Path cfg");
  if(show)
  {
    this->bPathsChanged=false;
    this->txtPCSX2GamesPath->SetLabelText(pcsx2Tool_m->GetPCSX2GamesPath());
    this->txtPCSX2ExePath->SetLabelText(pcsx2Tool_m->GetPCSX2ExePath());
    this->txtPCSX2CFGPath->SetLabelText(pcsx2Tool_m->GetPCSX2CFGPath());
    this->txtUserCFGPath->SetLabelText(pcsx2Tool_m->GetUserCFGPath());
  }
  return(wxFrame::Show(show));
}

void wxWinPathConfigurator::OnBrowsePCSX2Games(wxCommandEvent& WXUNUSED(event))
{
  wxString strPath;
  if(!browseForFolder("Select PCSX2 Games Path (.iso containing folder)",strPath,this,this->txtPCSX2GamesPath->GetLabelText()))
    return;
  this->bPathsChanged=true;
  this->txtPCSX2GamesPath->SetLabelText(strPath);
}

void wxWinPathConfigurator::OnBrowsePCSX2EXE(wxCommandEvent& WXUNUSED(event))
{
  wxString strPath;
  wxPuts("Browse for PCSX2-Exe...");
  if(!browseForFile(true,
                    "Select PCSX2-Executable",
                    strPath,
#ifdef _WIN32
                    "Exe files (*.exe)|*.exe",
#elif __linux__
                    "All files (*.*)|*.*",
#endif
                    this,
                    this->txtPCSX2ExePath->GetLabelText()))
    return;
  this->bPathsChanged=true;
  this->txtPCSX2ExePath->SetLabelText(strPath);
}

void wxWinPathConfigurator::OnBrowsePCSX2CFG(wxCommandEvent& WXUNUSED(event))
{
  wxString strPath;
  wxPuts("Browse for PCSX2-CFG...");
  if(!browseForFolder("Select PCSX2 Config (inis-folder)",strPath,this,this->txtPCSX2CFGPath->GetLabelText()))
    return;
  if(!checkPCSX2CFGPathValid(strPath))
  {
    wxMessageBox("Selected path not valid, it should contain .ini files. Select another folder.",
                 "Path not valid",
                 wxOK | wxCENTER | wxICON_EXCLAMATION);
    return;
  }
  this->bPathsChanged=true;
  this->txtPCSX2CFGPath->SetLabelText(strPath);
}

void wxWinPathConfigurator::OnBrowsePUserCFG(wxCommandEvent& WXUNUSED(event))
{
  wxString strPath;
  wxPuts("Browse for User-CFG...");
  if(!browseForFolder("Choose your location to store the diffrent PCSX2-Configurations per Game",strPath,this,this->txtUserCFGPath->GetLabelText()))
    return;
  if(checkUserCFGPathValid(strPath)<0)
  {
    wxMessageBox("Selected path not valid, it should be empty, or a previous created by this tool. Select another folder.",
                 "Path not valid",
                 wxOK | wxCENTER | wxICON_EXCLAMATION);
    return;
  }
  this->bPathsChanged=true;
  this->txtUserCFGPath->SetLabelText(strPath);
}

void wxWinPathConfigurator::OnClose(wxCloseEvent& WXUNUSED(event))
{
  if(this->bPathsChanged)
  {
    switch(wxMessageBox("Do you want to Save Changes?",
                        wxMessageBoxCaptionStr,
                        wxYES_NO|wxCANCEL,
                        this))
    {
      case wxCANCEL:
        return;
      case wxYES:
        if(!(this->bPCSX2CFGValid=checkPCSX2CFGPathValid(this->txtPCSX2CFGPath->GetLabelText())))
        {
          if(wxMessageBox("Warning: User Config Path is not valid. Do you really want to exit Configuration?",
                          wxMessageBoxCaptionStr,
                          wxYES_NO|wxCENTER|wxICON_EXCLAMATION,
                          this)==wxNO)
            return;
          break;
        }
        switch(checkUserCFGPathValid(this->txtUserCFGPath->GetLabelText()))
        {
          case 0: /* Valid, but empty */
            wxPuts("Empty folder, creating file indicator...");
            if(!createUserConfigIndicator(this->txtUserCFGPath->GetLabelText()))
            {
              wxMessageBox("Cannot create new User config file @" + this->txtUserCFGPath->GetLabelText() + ", try another path",
                           wxMessageBoxCaptionStr,
                           wxOK|wxCENTER|wxICON_EXCLAMATION,
                           this);
              return;
            }
            /* Fallthrough */
          case 1: /* Already checked & some data there */
            wxPuts("Already cfgs there");
            this->bUserCFGValid=true;
            break;
          default:
            if(wxMessageBox("Warning: User Config Path is not valid. Do you really want to exit Configuration?",
                            wxMessageBoxCaptionStr,
                            wxYES_NO|wxCENTER|wxICON_EXCLAMATION,
                            this)==wxNO)
              return;
            this->bUserCFGValid=false;
            break;
        }
        pcsx2Tool_m->SetPCSX2GamesPath(this->txtPCSX2GamesPath->GetLabelText().ToStdString());
        pcsx2Tool_m->SetPCSX2ExePath(this->txtPCSX2ExePath->GetLabelText().ToStdString());
        pcsx2Tool_m->SetPCSX2CFGPath(this->txtPCSX2CFGPath->GetLabelText().ToStdString());
        pcsx2Tool_m->SetUserCFGPath(this->txtUserCFGPath->GetLabelText().ToStdString());
        break;
      case wxNO:
        break;
      default:
        return;
    }
  }
  this->GetParent()->Show(true); /* Update winMain to show contents, if needed */
  this->Show(false);
}
