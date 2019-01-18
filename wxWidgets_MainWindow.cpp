#include <wx/wxprec.h>
#ifndef WX_PRECOMP
  #include <wx/wx.h>
  #include <wx/stdpaths.h>
  #include <wx/filename.h>
#endif

#include "wxWidgets_MainWindow.h"
#include "wxWidgets_functions.h"

enum
{
  // menu items
  MainWindow_EditCFGs           = wxID_EDIT,
  MainWindow_About              = wxID_ABOUT,
  MainWindow_CreateGameShortcut = 1,
  MainWindow_OpenPathConfig     = 2,
};

enum /* Buttons */
{
  BTN_NewConfig,
  BTN_NewConfigSave,
  BTN_NewConfigCancel,
  BTN_ConfigDelete,
  BTN_ConfigStartPCSX2,
  BTN_CreateGameShortcut
};

wxBEGIN_EVENT_TABLE(wxWinMain,wxFrame)
EVT_MENU(MainWindow_About,wxWinMain::OnAbout)
EVT_MENU(MainWindow_OpenPathConfig,wxWinMain::OnPathSelect)
EVT_MENU(MainWindow_CreateGameShortcut,wxWinMain::OnShortcutCreate)

EVT_BUTTON(BTN_NewConfig,wxWinMain::OnNewConfig)
EVT_BUTTON(BTN_NewConfigSave,wxWinMain::OnNewConfigSave)
EVT_BUTTON(BTN_NewConfigCancel,wxWinMain::OnNewConfigCancel)
EVT_BUTTON(BTN_ConfigDelete,wxWinMain::OnConfigDelete)
EVT_BUTTON(BTN_ConfigStartPCSX2,wxWinMain::OnEditConfig)

EVT_CLOSE(wxWinMain::OnClose)
wxEND_EVENT_TABLE()

IMPLEMENT_APP(PCSX2Tool_GUI)

PCSX2Tool *pcsx2Tool_m;

bool PCSX2Tool_GUI::OnInit()
{
  bool bFirstStart=false;
  wxString path=::wxStandardPaths::Get().GetUserConfigDir();
  wxFileName newFolder= wxFileName::DirName(path);
  path+= wxFILE_SEP_PATH + pcsx2Tool_m->toolGetAppName();
  if(!wxApp::OnInit())
    return(false);

  if(!newFolder.DirExists())
  {
    if(!newFolder.Mkdir())
      return(false);
  }
  pcsx2Tool_m= new PCSX2Tool();

  switch(pcsx2Tool_m->LoadToolConfig(path.ToStdString()))
  {
    case PCSX2_TOOL_CFG_ERROR:
      wxMessageBox("Failed to load config from file " + path + ", data is corrupt. Delete config.ini and restart.",
                   wxMessageBoxCaptionStr,
                   wxOK|wxICON_WARNING|wxCENTER);
      return(false);
    case PCSX2_TOOL_CFG_NEW:
      bFirstStart=true;
    case PCSX2_TOOL_CFG_LOAD:
      break;
  }
  wxWinMain *winMain = new wxWinMain(NULL,wxID_ANY,pcsx2Tool_m->toolGetAppName());
  winMain->wxPathCfg = new wxWinPathConfigurator(winMain,wxID_ANY,"Select Paths");
  winMain->Show(true);
  if(bFirstStart)
  {
    if(wxMessageBox("This seems to be the First Launch of the " + pcsx2Tool_m->toolGetAppName() + ", do you want to Setup Paths now?",
                 wxMessageBoxCaptionStr,
                 wxYES_NO,
                 winMain)==wxYES)
    {
      //Show Path config
      winMain->wxPathCfg->Show(true);
    }
  }
  return(true);
}

wxWinMain::wxWinMain(wxWindow* parent,
                     wxWindowID id,
                     const wxString& title,
                     const wxPoint& pos,
                     const wxSize& size,
                     long style )
          : wxFrame( parent, id, title, pos, size, style )
{
  this->SetSizeHints( wxDefaultSize, wxDefaultSize );

  layCFGEdit = new wxFlexGridSizer( 0, 2, 0, 0 );
  layCFGEdit->AddGrowableCol( 0 );
  layCFGEdit->AddGrowableCol( 1 );
  layCFGEdit->SetFlexibleDirection( wxBOTH );
  layCFGEdit->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

  m_staticText7 = new wxStaticText( this, wxID_ANY, wxT("Manage your Configs here"), wxDefaultPosition, wxDefaultSize, 0 );
  m_staticText7->Wrap( -1 );
  layCFGEdit->Add( m_staticText7, 0, wxALL, 5 );


  layCFGEdit->Add( 0, 0, 1, wxEXPAND, 5 );

  m_staticText8 = new wxStaticText( this, wxID_ANY, wxT("The <Default> Entry is your default PCSX2 Config"), wxDefaultPosition, wxDefaultSize, 0 );
  m_staticText8->Wrap( -1 );
  layCFGEdit->Add( m_staticText8, 0, wxALL, 5 );


  layCFGEdit->Add( 0, 0, 1, wxEXPAND, 5 );

  listGameConfigs = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxSize( -1,100 ), 0, NULL, 0 );
  layCFGEdit->Add( listGameConfigs, 0, wxALL|wxEXPAND, 5 );

  wxBoxSizer* bSizer5;
  bSizer5 = new wxBoxSizer( wxVERTICAL );

  btnNewConfig = new wxButton( this, BTN_NewConfig, wxT("Create new Config..."), wxDefaultPosition, wxDefaultSize, 0 );
  bSizer5->Add( btnNewConfig, 0, wxALL, 5 );

  btnConfigDelete = new wxButton( this, BTN_ConfigDelete, wxT("Delete Config"), wxDefaultPosition, wxDefaultSize, 0 );
  bSizer5->Add( btnConfigDelete, 0, wxALL, 5 );


  layCFGEdit->Add( bSizer5, 1, wxEXPAND, 5 );

  txtNewCFGName = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), 0 );
  layCFGEdit->Add( txtNewCFGName, 0, wxEXPAND|wxLEFT|wxRIGHT, 5 );

  wxBoxSizer* bSizer4;
  bSizer4 = new wxBoxSizer( wxHORIZONTAL );

  btnNewConfigSave = new wxButton( this, BTN_NewConfigSave, wxT("Save"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
  bSizer4->Add( btnNewConfigSave, 0, wxALIGN_CENTER_VERTICAL, 5 );

  btnNewConfigCancel = new wxButton( this, BTN_NewConfigCancel, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  bSizer4->Add( btnNewConfigCancel, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 5 );


  layCFGEdit->Add( bSizer4, 1, wxEXPAND, 0 );

  btnConfigStartPCSX2 = new wxButton( this, BTN_ConfigStartPCSX2, wxT("Start PCSX2 to Edit your Settings for the selected Config"), wxDefaultPosition, wxDefaultSize, 0 );
  layCFGEdit->Add( btnConfigStartPCSX2, 0, wxALL, 5 );


  layCFGEdit->Add( 0, 0, 1, wxEXPAND, 5 );


  this->SetSizer( layCFGEdit );
  this->Layout();
  m_menubar2 = new wxMenuBar( 0 );
  m_menu3 = new wxMenu();
  wxMenuItem* menuItemOpenPathsCfg;
  menuItemOpenPathsCfg = new wxMenuItem( m_menu3, MainWindow_OpenPathConfig, wxString( wxT("Set Paths...") ) , wxEmptyString, wxITEM_NORMAL );
  m_menu3->Append( menuItemOpenPathsCfg );

  menuItemCreateGameShortcut = new wxMenuItem( m_menu3, MainWindow_CreateGameShortcut, wxString( wxT("Create Game Start Shortcut...") ) , wxEmptyString, wxITEM_NORMAL );
  m_menu3->Append( menuItemCreateGameShortcut );

  m_menubar2->Append( m_menu3, wxT("&File") );

  menuHelp = new wxMenu();
  wxMenuItem* menuItemAbout;
  menuItemAbout = new wxMenuItem( menuHelp, MainWindow_About, wxString( wxT("&About\tF1") ) , wxEmptyString, wxITEM_NORMAL );
  menuHelp->Append( menuItemAbout );

  m_menubar2->Append( menuHelp, wxT("&Help") );

  this->SetMenuBar( m_menubar2 );


  this->Centre( wxBOTH );
}

bool wxWinMain::Show(bool show)
{
  if(show)
  {
    if((!this->wxPathCfg->bUserCFGValid) || (!this->wxPathCfg->bPCSX2CFGValid))
    {
      wxPuts("User/pcsx2 CFG path invalid!");
      this->menuItemCreateGameShortcut->Enable(false);
      this->layCFGEdit->Show(false);
    }
    else
    {
      this->menuItemCreateGameShortcut->Enable(true);
      this->layCFGEdit->Show(true);
      this->txtNewCFGName->Show(false);
      this->btnNewConfigSave->Show(false);
      this->btnNewConfigCancel->Show(false);
      this->UpdateConfigList();
    }
  }
  return(wxFrame::Show(show));
}

void wxWinMain::OnPathSelect(wxCommandEvent& WXUNUSED(event))
{
  this->wxPathCfg->Show(true);
}

void wxWinMain::OnShortcutCreate(wxCommandEvent& WXUNUSED(event))
{
  wxString strPath;
  wxString strConfigName;
  string strCMDLine;
  wxFile newFile;
  int iIndex;
  if((iIndex=this->listGameConfigs->GetSelection())==wxNOT_FOUND)
    return;
  if(!browseForFile(true,"Select .iso file of the game",strPath,".iso-Files (*.iso)|*.iso",this,pcsx2Tool_m->GetPCSX2GamesPath()))
  {
    return;
  }
  if(iIndex)
    strConfigName=this->listGameConfigs->GetString(iIndex);
  else
    strConfigName="";
  pcsx2Tool_m->CreateCommandLine_PCSX2StartWithCFG(strConfigName.ToStdString(),strPath.ToStdString(),strCMDLine);
  wxPuts("Get cmdline: " + strCMDLine);
  if(!browseForFile(false,
                    "Save Shortcut",
                    strPath,
#ifdef _WIN32
                    ".bat Files (*.bat)|*.bat",
#elif __linux__
                    ".sh Files (*.sh)|*.sh",
#endif
                    this,
                    pcsx2Tool_m->GetPCSX2GamesPath()))
  {
    return;
  }
  if(!newFile.Create(strPath))
  {
    wxMessageBox("Failed to Create file " + strPath,
                 wxMessageBoxCaptionStr,
                 wxOK|wxCENTER|wxICON_EXCLAMATION,
                 this);
    return;
  }
  newFile.Write(strCMDLine);
  newFile.Close();
}

void wxWinMain::OnClose(wxCloseEvent& WXUNUSED(event))
{
  if(!pcsx2Tool_m->SaveToolConfig())
  {
    wxMessageDialog confirmMe(this,
                              wxString::Format("Failed to save tool configuration to %s, Quit anyway?",pcsx2Tool_m->GetToolCFGPath()),
                              wxMessageBoxCaptionStr,
                              wxYES_NO);
    switch(confirmMe.ShowModal())
    {
      case wxID_NO:
        return;
      case wxID_YES:
        break;
    }
  }
  this->Destroy();
}

void wxWinMain::OnAbout(wxCommandEvent& WXUNUSED(event))
{
  wxMessageBox("Welcome to the " + pcsx2Tool_m->toolGetAppName() + " Version " + pcsx2Tool_m->toolGetVersionString() + "!\n"
               "\n"
               "This is a Tool to manage multiple Configs for PCSX2\n"
               "For more Information visit https://github.com/XXXBold/PCSX2_MultiConfigTool\n"
               "Credits to wxWidgets GUI-Library, for making this possible.\n"
               "https://www.wxwidgets.org/ for more Infos\n"
               "There might be some more Information here later...\n",
               "About",
               wxOK|wxICON_INFORMATION|wxCENTER,
               this);
}

void wxWinMain::OnNewConfig(wxCommandEvent& WXUNUSED(event))
{
  int iIndex;
  if((iIndex=this->listGameConfigs->GetSelection())==wxNOT_FOUND)
    return;

  if(wxMessageBox("Do you want to create a new Config based on \"" + this->listGameConfigs->GetString(iIndex) + "\"?",
                  wxMessageBoxCaptionStr,
                  wxYES_NO,
                  this)!=wxYES)
  {
    return;
  }
  this->txtNewCFGName->Show(true);
  this->btnNewConfigSave->Show(true);
  this->btnNewConfigCancel->Show(true);
  this->txtNewCFGName->SetLabelText("NewConfigName");
  this->txtNewCFGName->SetFocus();
  this->txtNewCFGName->SetSelection(0,this->txtNewCFGName->GetLineLength(0));
}

void wxWinMain::UpdateConfigList()
{
  wxArrayString astrFolders;
  astrFolders.Add(DEFAULT_CONFIG_STRING);
  if(!getSubFoldersFromPath(pcsx2Tool_m->GetUserCFGPath(),&astrFolders))
  {
    wxPuts("No folders found in usercfg");
  }
  for(unsigned int uiIndex=0;uiIndex<astrFolders.GetCount();++uiIndex)
  {
    wxPuts("Folder: " + astrFolders[uiIndex]);

  }
  this->listGameConfigs->Clear();
  this->listGameConfigs->InsertItems(astrFolders,0);
  this->listGameConfigs->SetSelection(0);
}

void wxWinMain::OnNewConfigSave(wxCommandEvent& WXUNUSED(event))
{
  int iIndex;
  wxString strSrcPath;
  wxString strDestPath;
  if((iIndex=this->listGameConfigs->GetSelection())==wxNOT_FOUND)
    return;

  if(!iIndex) /* Default, means use PCSX2 CFG path*/
    strSrcPath=pcsx2Tool_m->GetPCSX2CFGPath();
  else
    strSrcPath=pcsx2Tool_m->GetUserCFGPath() + this->listGameConfigs->GetString(iIndex);

  this->btnNewConfigSave->SetFocus(); /* to let wxWidgets take our typed in name into ->txtNewCFGName */

  strDestPath=pcsx2Tool_m->GetUserCFGPath() + this->txtNewCFGName->GetLineText(0);
  wxPuts("Would copy " + strSrcPath + " to " + strDestPath);
  if(!copyFolderContents(strSrcPath,strDestPath))
  {
    wxMessageBox("Copy from " + strSrcPath + " to " + strDestPath + " failed, maybe check permissions?",wxMessageBoxCaptionStr,wxOK | wxCENTER,this);
    return;
  }
  this->txtNewCFGName->Show(false);
  this->btnNewConfigSave->Show(false);
  this->btnNewConfigCancel->Show(false);
  this->UpdateConfigList();
}

void wxWinMain::OnNewConfigCancel(wxCommandEvent& WXUNUSED(event))
{
  this->txtNewCFGName->Show(false);
  this->btnNewConfigSave->Show(false);
  this->btnNewConfigCancel->Show(false);
}

void wxWinMain::OnConfigDelete(wxCommandEvent& WXUNUSED(event))
{
  int iIndex;
  wxString strPath;
  if(((iIndex=this->listGameConfigs->GetSelection())==wxNOT_FOUND) || (iIndex==0)) /* Don't allow default to be deleted */
    return;

  strPath=pcsx2Tool_m->GetUserCFGPath() + this->listGameConfigs->GetString(iIndex);
  if(wxMessageBox("Do you really want to delete Config \"" + this->listGameConfigs->GetString(iIndex) + "\"?",
                  wxMessageBoxCaptionStr,
                  wxYES_NO | wxCENTER,
                  this)==wxYES)
  {
    if(!deleteFolderRecursive(strPath))
    {
      wxMessageBox("Failed to delete " + strPath,wxMessageBoxCaptionStr,
                   wxOK | wxCENTER | wxICON_EXCLAMATION,this);
    }
  }
  this->UpdateConfigList();
}

void wxWinMain::OnEditConfig(wxCommandEvent& WXUNUSED(event))
{
  string strCmd;
  int iIndex;
  if((iIndex=this->listGameConfigs->GetSelection())==wxNOT_FOUND)
    return;

  pcsx2Tool_m->CreateCommandLine_PCSX2StartWithCFG((iIndex)?this->listGameConfigs->GetString(iIndex).ToStdString():"",
                                 strCmd);

  wxPuts("Starting: " + strCmd);

  if(wxExecute(strCmd,wxEXEC_ASYNC)<0)
  {
    wxMessageBox("Failed to start " + strCmd,
                 wxMessageBoxCaptionStr,
                 wxOK|wxCENTER|wxICON_EXCLAMATION,
                 this);
  }
}


