#include <wx/wxprec.h>
#ifndef WX_PRECOMP
  #include <wx/wx.h>
#endif

#include "wxwin_selectpaths.h"
#include "wxwin_functions.h"
#include "wxwin_main.h"

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

  m_staticText6 = new wxStaticText( this, wxID_ANY, wxT(SELECTPATHS_LABEL_GAMES), wxDefaultPosition, wxDefaultSize, 0 );
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

  m_staticText1 = new wxStaticText( this, wxID_ANY, wxT(SELECTPATHS_LABEL_PCSX2_EXE), wxDefaultPosition, wxDefaultSize, 0 );
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

  m_staticText2 = new wxStaticText( this, wxID_ANY, wxT(SELECTPATHS_LABEL_PCSX2_CFG), wxDefaultPosition, wxDefaultSize, 0 );
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

  m_staticText3 = new wxStaticText( this, wxID_ANY, wxT(SELECTPATHS_LABEL_USER_CFG), wxDefaultPosition, wxDefaultSize, 0 );
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
  layoutVertical->Fit( this );

  this->Centre( wxBOTH );
}

bool wxWinPathConfigurator::PathConfigIsValid(void)
{
  return(checkPCSX2CFGPathValid(pcsx2Tool_m->GetPCSX2CFGPath()) &&
         checkUserCFGPathValid(pcsx2Tool_m->GetUserCFGPath()));
}

// event handlers

bool wxWinPathConfigurator::Show(bool show,
                                 bool bFirstRun)
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(show)
  {
    this->bPathsChanged=bFirstRun;
    this->txtPCSX2GamesPath->SetValue(pcsx2Tool_m->GetPCSX2GamesPath());
    this->txtPCSX2ExePath->SetValue(pcsx2Tool_m->GetPCSX2ExePath());
    this->txtPCSX2CFGPath->SetValue(pcsx2Tool_m->GetPCSX2CFGPath());
    this->txtUserCFGPath->SetValue(pcsx2Tool_m->GetUserCFGPath());
  }
  wxWindow::Fit();
  return(wxFrame::Show(show));
}

void wxWinPathConfigurator::OnBrowsePCSX2Games(wxCommandEvent& WXUNUSED(event))
{
  wxString strPath;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(!browseForFolder("Select PCSX2 Games Path (.iso containing folder)",strPath,true,this,this->txtPCSX2GamesPath->GetValue()))
    return;
  this->bPathsChanged=true;
  strPath.append((wxFILE_SEP_PATH));
  this->txtPCSX2GamesPath->SetValue(strPath);
}

void wxWinPathConfigurator::OnBrowsePCSX2EXE(wxCommandEvent& WXUNUSED(event))
{
  wxString strPath;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);

  if(!browseForFile(true,
                    "Select PCSX2-Executable",
                    strPath,
#ifdef _WIN32
                    "Exe files (*.exe)|*.exe",
#elif __linux__
                    "All files (*.*)|*",
#endif
                    this,
                    this->txtPCSX2ExePath->GetValue()))
    return;
  this->bPathsChanged=true;
  this->txtPCSX2ExePath->SetValue(strPath);
}

void wxWinPathConfigurator::OnBrowsePCSX2CFG(wxCommandEvent& WXUNUSED(event))
{
  wxString strPath;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(!browseForFolder("Select PCSX2 Config (inis-folder)",strPath,true,this,this->txtPCSX2CFGPath->GetValue()))
    return;
  if(!checkPCSX2CFGPathValid(strPath))
  {
    wxMessageBox("Selected path not valid, it should contain .ini files. Select another folder.",
                 "Path not valid",
                 wxOK | wxCENTER | wxICON_EXCLAMATION);
    return;
  }
  this->bPathsChanged=true;
  strPath.append((wxFILE_SEP_PATH));
  this->txtPCSX2CFGPath->SetValue(strPath);
}

void wxWinPathConfigurator::OnBrowsePUserCFG(wxCommandEvent& WXUNUSED(event))
{
  wxString strPath;
  DEBUG_WXPUTS("Browse for User-CFG...");
  if(!browseForFolder("Choose your location to store the diffrent PCSX2-Configurations per Game",strPath,false,this,this->txtUserCFGPath->GetValue()))
    return;
  if(!checkUserCFGPathValid(strPath))
  {
    wxMessageBox("Selected path not valid, it should be empty, or a previous created by this tool. Select another folder.",
                 "Path not valid",
                 wxOK | wxCENTER | wxICON_EXCLAMATION);
    return;
  }
  this->bPathsChanged=true;
  strPath.append((wxFILE_SEP_PATH));
  this->txtUserCFGPath->SetValue(strPath);
}

void wxWinPathConfigurator::OnClose(wxCloseEvent& WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(this->bPathsChanged)
  {
    switch(wxMessageBox("Do you want to Save Changes?",
                        wxMessageBoxCaptionStr,
                        wxYES_NO|wxCANCEL,
                        this))
    {
      case wxCANCEL:
        return;
      case wxNO:
        break;
      case wxYES:
        if(!checkPCSX2CFGPathValid(this->txtPCSX2CFGPath->GetValue()))
        {
          if(wxMessageBox("Warning: PCSX2-Config Path is not valid. Do you really want to exit Configuration?",
                          wxMessageBoxCaptionStr,
                          wxYES_NO|wxCENTER|wxICON_EXCLAMATION,
                          this) == wxNO)
            return;
        }
        if(checkUserCFGPathValid(this->txtUserCFGPath->GetValue()))
        {
            DEBUG_WXPUTS("Config Path okay, trying to create file indicator...");
            if(!createUserConfigIndicator(this->txtUserCFGPath->GetValue()))
            {
              wxMessageBox("Cannot create new User config file @" + this->txtUserCFGPath->GetValue() + ", try another path",
                           wxMessageBoxCaptionStr,
                           wxOK|wxCENTER|wxICON_EXCLAMATION,
                           this);
              return;
            }
            /* Okay, already existed or successfully created file indicator */
        }
        else if(wxMessageBox("Warning: User Config Path is not valid. Do you really want to exit Configuration?",
                wxMessageBoxCaptionStr,
                wxYES_NO|wxCENTER|wxICON_EXCLAMATION,
                this) == wxNO)
        {
          return;
        }
        if((pcsx2Tool_m->SetPCSX2GamesPath(this->txtPCSX2GamesPath->GetValue().ToStdString())) &&
           (pcsx2Tool_m->SetPCSX2ExePath(this->txtPCSX2ExePath->GetValue().ToStdString())) &&
           (pcsx2Tool_m->SetPCSX2CFGPath(this->txtPCSX2CFGPath->GetValue().ToStdString())) &&
           (pcsx2Tool_m->SetUserCFGPath(this->txtUserCFGPath->GetValue().ToStdString())))
        {
          break;
        }
        wxMessageBox("Cannot set all paths, might be too long",
                     wxMessageBoxCaptionStr,
                     wxOK|wxCENTER|wxICON_EXCLAMATION,
                     this);
        break;
      default: /* Should not happen */
        DEBUG_WXPUTS("****Unhandled case in switch (should not happen!)");
        return;
    }
  }
  wxGetApp().GetTopWindow()->Update(); /* Update winMain to show contents, if needed */
  this->Show(false);
}
