#include <wx/wxprec.h>
#ifndef WX_PRECOMP
  #include <wx/wx.h>
  #include <wx/filename.h>
  #include <wx/aboutdlg.h>
#endif

#include "wxwin_main.h"
#include "wxwin_functions.h"

enum
{
  // menu items
  MainWindow_OptionsStartNoGUI = 1,
  MainWindow_OptionsStartEnableFullscreen,
  MainWindow_OptionsStartEnableFullBoot,
  MainWindow_CreateGameShortcut,
  MainWindow_OpenPathConfig,
  MainWindow_About = wxID_ABOUT,

  // Buttons
  BTN_NewConfig=1,
  BTN_RenameConfig,
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

EVT_MENU(MainWindow_OptionsStartNoGUI,wxWinMain::OnOptionChangeStartNoGUI)
EVT_MENU(MainWindow_OptionsStartEnableFullscreen,wxWinMain::OnOptionChangeStartFullscreen)
EVT_MENU(MainWindow_OptionsStartEnableFullBoot,wxWinMain::OnOptionChangeStartFullBoot)

EVT_BUTTON(BTN_NewConfig,wxWinMain::OnNewConfig)
EVT_BUTTON(BTN_RenameConfig,wxWinMain::OnRenameConfig)
EVT_BUTTON(BTN_NewConfigSave,wxWinMain::OnNewConfigSave)
EVT_BUTTON(BTN_NewConfigCancel,wxWinMain::OnNewConfigCancel)
EVT_BUTTON(BTN_ConfigDelete,wxWinMain::OnConfigDelete)
EVT_BUTTON(BTN_ConfigStartPCSX2,wxWinMain::OnEditConfigStartPCSX2)

EVT_CLOSE(wxWinMain::OnClose)
wxEND_EVENT_TABLE()

IMPLEMENT_APP(PCSX2Tool_GUI)

bool PCSX2Tool_GUI::OnInit(void)
{
  bool bFirstStart=false;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(!wxApp::OnInit())
    return(false);

  pcsx2Tool_m= new PCSX2Tool();

  switch(pcsx2Tool_m->LoadToolConfig())
  {
    case PCSX2_TOOL_CFG_ERROR:
      wxMessageBox("Failed to load config from file, try to delete config.ini and restart.",
                   wxMessageBoxCaptionStr,
                   wxOK | wxICON_ERROR | wxCENTER);
      return(false);
    case PCSX2_TOOL_CFG_NEW:
      bFirstStart=true;
    case PCSX2_TOOL_CFG_EXISTING:
      break;
  }
  wxWinMain *winMain = new wxWinMain(NULL,wxID_ANY,APP_DISPLAY_NAME);
  winMain->wxPathCfg = new wxWinPathConfigurator(winMain,wxID_ANY,"Select Paths");
  winMain->Show(true);
  if(bFirstStart)
  {
    if(wxMessageBox("This seems to be the First Launch of the " APP_DISPLAY_NAME ", do you want to Setup Paths now?",
                    wxMessageBoxCaptionStr,
                    wxYES_NO,
                    winMain) == wxYES)
    {
      //Show Path config
      winMain->wxPathCfg->Show(true,true);
    }
  }
  return(true);
}

wxWinMain::wxWinMain(wxWindow *parent,
                     wxWindowID id,
                     const wxString &title,
                     const wxPoint &pos,
                     const wxSize &size,
                     long style)
          :wxFrame(parent,id,title,pos,size,style)
{
  this->SetSizeHints( wxDefaultSize, wxDefaultSize );

  layCFGEdit = new wxFlexGridSizer( 0, 2, 0, 0 );
  layCFGEdit->AddGrowableCol( 0 );
  layCFGEdit->AddGrowableCol( 1 );
  layCFGEdit->SetFlexibleDirection( wxBOTH );
  layCFGEdit->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_SPECIFIED );

  wxStaticText* m_staticText7;
  m_staticText7 = new wxStaticText( this, wxID_ANY, wxT("Manage your Configs here"), wxDefaultPosition, wxDefaultSize, 0 );
  m_staticText7->Wrap( -1 );
  layCFGEdit->Add( m_staticText7, 0, wxALL, 5 );


  layCFGEdit->Add( 0, 0, 1, wxEXPAND, 5 );

  wxStaticText* m_staticText8;
  m_staticText8 = new wxStaticText( this, wxID_ANY, wxT("The <Default> Entry is your default PCSX2 Config"), wxDefaultPosition, wxDefaultSize, 0 );
  m_staticText8->Wrap( -1 );
  layCFGEdit->Add( m_staticText8, 0, wxALL, 5 );


  layCFGEdit->Add( 0, 0, 1, wxEXPAND, 5 );

  listGameConfigs = new wxListBox( this, wxID_ANY, wxDefaultPosition, wxSize( -1,100 ), 0, NULL, 0 );
  layCFGEdit->Add( listGameConfigs, 0, wxALL|wxEXPAND, 5 );

  wxBoxSizer* layConfigEditBtns;
  layConfigEditBtns = new wxBoxSizer( wxVERTICAL );

  btnConfigCreateNew = new wxButton( this, BTN_NewConfig, wxT("Create new Config..."), wxDefaultPosition, wxDefaultSize, 0 );
  layConfigEditBtns->Add( btnConfigCreateNew, 0, wxALL, 5 );

  btnConfigRename = new wxButton( this, BTN_RenameConfig, wxT("Rename Config..."), wxDefaultPosition, wxDefaultSize, 0 );
  layConfigEditBtns->Add( btnConfigRename, 0, wxALL, 5 );

  btnConfigDelete = new wxButton( this, BTN_ConfigDelete, wxT("Delete Config"), wxDefaultPosition, wxDefaultSize, 0 );
  layConfigEditBtns->Add( btnConfigDelete, 0, wxALL, 5 );


  layCFGEdit->Add( layConfigEditBtns, 1, wxEXPAND, 5 );

  txtNewCFGName = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize( -1,-1 ), 0 );
  layCFGEdit->Add( txtNewCFGName, 0, wxEXPAND|wxLEFT|wxRIGHT, 5 );

  laySaveCancelBtn = new wxBoxSizer( wxHORIZONTAL );

  btnConfigSave = new wxButton( this, BTN_NewConfigSave, wxT("Save"), wxDefaultPosition, wxSize( -1,-1 ), 0 );
  laySaveCancelBtn->Add( btnConfigSave, 0, wxALIGN_CENTER_VERTICAL, 5 );

  btnConfigCancel = new wxButton( this, BTN_NewConfigCancel, wxT("Cancel"), wxDefaultPosition, wxDefaultSize, 0 );
  laySaveCancelBtn->Add( btnConfigCancel, 0, wxALIGN_CENTER_VERTICAL|wxLEFT, 5 );


  layCFGEdit->Add( laySaveCancelBtn, 1, wxEXPAND, 0 );

  btnConfigStartPCSX2 = new wxButton( this, BTN_ConfigStartPCSX2, wxT("Start PCSX2 to Edit your Settings for the selected Config"), wxDefaultPosition, wxDefaultSize, 0 );
  layCFGEdit->Add( btnConfigStartPCSX2, 0, wxALL, 5 );


  layCFGEdit->Add( 0, 0, 1, wxEXPAND, 5 );


  this->SetSizer( layCFGEdit );
  this->Layout();
  m_menubar2 = new wxMenuBar( 0 );
  menuFile = new wxMenu();
  wxMenuItem* menuItemOpenPathsCfg;
  menuItemOpenPathsCfg = new wxMenuItem( menuFile, MainWindow_OpenPathConfig, wxString( wxT("Set Paths...") ) , wxEmptyString, wxITEM_NORMAL );
  menuFile->Append( menuItemOpenPathsCfg );

  menuItemCreateGameShortcut = new wxMenuItem( menuFile, MainWindow_CreateGameShortcut, wxString( wxT("Create Game Start Shortcut...") ) , wxEmptyString, wxITEM_NORMAL );
  menuFile->Append( menuItemCreateGameShortcut );

  m_menubar2->Append( menuFile, wxT("&File") );

  menuOptions = new wxMenu();
  menuSubGameShortcuts = new wxMenu();
  wxMenuItem* menuSubGameShortcutsItem = new wxMenuItem( menuOptions, wxID_ANY, wxT("Game Shortcuts"), wxEmptyString, wxITEM_NORMAL, menuSubGameShortcuts );
  menuItemEnableStartNoGUI = new wxMenuItem( menuSubGameShortcuts, MainWindow_OptionsStartNoGUI, wxString( wxT("Disable PCSX2-GUI on Startup") ) , wxEmptyString, wxITEM_CHECK );
  menuSubGameShortcuts->Append( menuItemEnableStartNoGUI );

  menuItemEnableStartFullscreen = new wxMenuItem( menuSubGameShortcuts, MainWindow_OptionsStartEnableFullscreen, wxString( wxT("Start in Fullscreen") ) , wxEmptyString, wxITEM_CHECK );
  menuSubGameShortcuts->Append( menuItemEnableStartFullscreen );

  menuItemEnableStartFullBoot = new wxMenuItem( menuSubGameShortcuts, MainWindow_OptionsStartEnableFullBoot, wxString( wxT("Use Full Boot on Startup") ) , wxEmptyString, wxITEM_CHECK );
  menuSubGameShortcuts->Append( menuItemEnableStartFullBoot );

  menuOptions->Append( menuSubGameShortcutsItem );

  m_menubar2->Append( menuOptions, wxT("&Options") );

  menuHelp = new wxMenu();
  wxMenuItem* menuItemAbout;
  menuItemAbout = new wxMenuItem( menuHelp, MainWindow_About, wxString( wxT("&About\tF1") ) , wxEmptyString, wxITEM_NORMAL );
  menuHelp->Append( menuItemAbout );

  m_menubar2->Append( menuHelp, wxT("&Help") );

  this->SetMenuBar( m_menubar2 );


  this->Centre( wxBOTH );
}

void wxWinMain::UpdateConfigList(void)
{
  wxArrayString astrFolders;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  astrFolders.Add(DEFAULT_CONFIG_STRING);

  if(!getSubFoldersFromPath(pcsx2Tool_m->GetUserCFGPath(),&astrFolders))
  {
    DEBUG_WXPUTS("No folders found in usercfg, adding default only");
  }
  for(unsigned int uiIndex=0;uiIndex < astrFolders.GetCount();++uiIndex)
  {
    DEBUG_WXPUTS("Add Folder(config) to list: "+astrFolders[uiIndex]);
  }
  this->listGameConfigs->Clear();
  this->listGameConfigs->InsertItems(astrFolders,0);
  this->listGameConfigs->SetSelection(0); // Select default entry
}

bool wxWinMain::Show(bool show)
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(show)
  {
    this->bRenameConfig=false;
    this->menuItemEnableStartNoGUI->Check(pcsx2Tool_m->GetOptionStartNoGUI());
    this->menuItemEnableStartFullscreen->Check(pcsx2Tool_m->GetOptionStartFullScreen());
    this->menuItemEnableStartFullBoot->Check(pcsx2Tool_m->GetOptionStartFullBoot());
    if(!this->wxPathCfg->PathConfigIsValid())
    {
      DEBUG_WXPUTS("User/pcsx2 CFG path invalid!");
      this->menuItemCreateGameShortcut->Enable(false);
      this->layCFGEdit->Show(false);
    }
    else
    {
      this->menuItemCreateGameShortcut->Enable(true);
      this->layCFGEdit->Show(true);
      this->txtNewCFGName->Show(false);
      this->laySaveCancelBtn->Show(false);
      this->UpdateConfigList();
      this->layCFGEdit->Layout(); // Recalculate layout
    }
  }
  return(wxFrame::Show(show));
}

void wxWinMain::OnPathSelect(wxCommandEvent &WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  this->wxPathCfg->Show(true);
}

void wxWinMain::OnShortcutCreate(wxCommandEvent &WXUNUSED(event))
{
  wxString strPath;
  wxString strConfigName;
  string strCMDLine;
  wxFile newFile;
  int iIndex;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if((iIndex=this->listGameConfigs->GetSelection()) == wxNOT_FOUND)
    return;
  if(!browseForFile(true,
                    "Select .iso file of the game",
                    strPath,
#ifdef _WIN32
                    ".iso-Files (*.iso)|*.iso|All files (*.*)|*.*",
#elif __linux__
                    ".iso-Files (*.iso)|*.iso|All files (*.*)|*",
#endif /* _WIN32 */
                    this,
                    pcsx2Tool_m->GetPCSX2GamesPath()))
  {
    return;
  }
  if(iIndex)
    strConfigName=this->listGameConfigs->GetString(iIndex);
  else
    strConfigName="";
  pcsx2Tool_m->CreateCommandLine_PCSX2StartWithCFG(strConfigName.ToStdString(),strPath.ToStdString(),strCMDLine);
  DEBUG_WXPUTS("Got cmdline: "+strCMDLine);
  if(!browseForFile(false,
                    "Save Shortcut for "+strPath,
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
  if(wxFile::Exists(strPath))
  {
    if(wxMessageBox("File \""+strPath+"\" already exists. Overwrite it?",
                    "File already exists",
                    wxYES_NO | wxCENTER | wxICON_QUESTION,
                    this) != wxYES)
    {
      return;
    }
  }
  iIndex=wxS_DEFAULT;
#ifdef __linux__
  /* On Linux, ask to set executable bit for new shortcut */
  if(wxMessageBox("Enable Execute bit for this user on Shortcut?",
                  "Set Executable bit",
                  wxYES_NO | wxCENTER | wxICON_QUESTION,
                  this) == wxYES)
  {
    iIndex|=wxPOSIX_USER_EXECUTE;
  }
#endif /* __linux__ */
  if(!newFile.Create(strPath,true,iIndex))
  {
    wxMessageBox("Failed to Create file "+strPath,
                 wxMessageBoxCaptionStr,
                 wxOK | wxCENTER | wxICON_EXCLAMATION,
                 this);
    return;
  }
  newFile.Write(strCMDLine);
  newFile.Close();
}

void wxWinMain::OnOptionChangeStartNoGUI(wxCommandEvent &WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  pcsx2Tool_m->SetOptionStartNoGUI(this->menuItemEnableStartNoGUI->IsChecked());
}

void wxWinMain::OnOptionChangeStartFullscreen(wxCommandEvent &WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  pcsx2Tool_m->SetOptionStartFullScreen(this->menuItemEnableStartFullscreen->IsChecked());
}

void wxWinMain::OnOptionChangeStartFullBoot(wxCommandEvent &WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  pcsx2Tool_m->SetOptionStartFullBoot(this->menuItemEnableStartFullBoot->IsChecked());
}

void wxWinMain::OnAbout(wxCommandEvent &WXUNUSED(event))
{
  wxAboutDialogInfo wAbout;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);

  wAbout.SetName(APP_DISPLAY_NAME);
  wAbout.SetVersion(wxString::Format("Version %d.%d.%d (%s)\n"
                                     "%s",
                                     APP_VERSION_MAJOR,
                                     APP_VERSION_MINOR,
                                     APP_VERSION_PATCH,
                                     APP_VERSION_DEVSTATE,
                                     APP_VERSION_BUILDTIME));
  wAbout.SetDescription("Welcome to the " APP_DISPLAY_NAME "!\n"
                        "This is a Tool to manage multiple Configs for PCSX2\n"
                        "For proper functionality, make sure all Paths are set correctly.\n"
                        "If you need help, or found a bug, visit the website of the Project.\n"
                        "\n"
                        "Credits to the wxWidgets GUI-Library, for making this possible.\n");
  wAbout.SetCopyright("(C) 2018-2019 by XXXBold");

  wAbout.SetWebSite(APP_PROJECT_HOMEPAGE);
  wAbout.AddDeveloper(APP_PROJECT_DEVELOPER);

  wxAboutBox(wAbout,this);
}

void wxWinMain::OnNewConfig(wxCommandEvent &WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if((this->iCurrSelection=this->listGameConfigs->GetSelection()) == wxNOT_FOUND)
    return;

  if(wxMessageBox("Do you want to create a new Config based on \""+this->listGameConfigs->GetString(this->iCurrSelection)+"\"?",
                  wxMessageBoxCaptionStr,
                  wxYES_NO,
                  this) != wxYES)
  {
    return;
  }
  this->ShowCfgNameEdit("NewConfigName",false);
}

void wxWinMain::OnRenameConfig(wxCommandEvent &WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(((this->iCurrSelection=this->listGameConfigs->GetSelection()) == wxNOT_FOUND) || (this->iCurrSelection == 0))
    return;

  this->ShowCfgNameEdit(this->listGameConfigs->GetString(this->iCurrSelection),true);
}

void wxWinMain::ShowCfgNameEdit(const wxString &name,
                                bool configExists)
{
  this->bRenameConfig=configExists;
  /* Enable elements for editing */
  this->txtNewCFGName->Show(true);
  this->laySaveCancelBtn->Show(true);
  /* Disable other elements */
  this->btnConfigStartPCSX2->Show(false);
  this->listGameConfigs->Deselect(this->iCurrSelection); /* Mint theme visible bug fix */
  this->listGameConfigs->Enable(false);
  this->btnConfigCreateNew->Enable(false);
  this->btnConfigRename->Enable(false);
  this->btnConfigDelete->Enable(false);

  this->txtNewCFGName->SetValue(name);
  this->txtNewCFGName->SetFocus();
  this->txtNewCFGName->SetSelection(0,this->txtNewCFGName->GetLineLength(0));
}

void wxWinMain::HideCfgNameEdit(void)
{
  this->layCFGEdit->Show(true);
  this->txtNewCFGName->Show(false);
  this->laySaveCancelBtn->Show(false);
  this->btnConfigStartPCSX2->Show(true);

  this->btnConfigCreateNew->Enable(true);
  this->btnConfigRename->Enable(true);
  this->btnConfigDelete->Enable(true);
  this->listGameConfigs->Enable(true);
  this->listGameConfigs->Select(this->iCurrSelection); /* Mint theme visible bug fix */
}

void wxWinMain::OnNewConfigSave(wxCommandEvent &WXUNUSED(event))
{
  wxString strSrcPath;
  wxString strDestPath;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);

  if(!this->iCurrSelection) /* Default, means use PCSX2 CFG path*/
    strSrcPath=pcsx2Tool_m->GetPCSX2CFGPath();
  else
    strSrcPath=pcsx2Tool_m->GetUserCFGPath()+this->listGameConfigs->GetString(this->iCurrSelection);

  this->btnConfigSave->SetFocus(); /* to let wxWidgets take our typed in name into ->txtNewCFGName */

  strDestPath=pcsx2Tool_m->GetUserCFGPath()+this->txtNewCFGName->GetLineText(0);
  if(this->bRenameConfig)
  {
    DEBUG_WXPUTS("Rename "+strSrcPath+" to "+strDestPath);
    if(!renameFolder(strSrcPath,strDestPath))
    {
      wxMessageBox("Rename "+strSrcPath+" to "+strDestPath+" failed.\n"
                   "Make sure the File doesn't exist already, or check permissions",
                   wxMessageBoxCaptionStr,
                   wxOK | wxCENTER | wxICON_EXCLAMATION,this);
      return;
    }
    this->bRenameConfig=false;
    this->HideCfgNameEdit();
    this->UpdateConfigList();
    return;
  }
  DEBUG_WXPUTS("Copy "+strSrcPath+" to "+strDestPath);
  if(!copyFolderContents(strSrcPath,strDestPath))
  {
    wxMessageBox("Copy from "+strSrcPath+" to "+strDestPath+" failed.\n"
                 "Make sure the File doesn't exist already, or check permissions",
                 wxMessageBoxCaptionStr,
                 wxOK | wxCENTER | wxICON_EXCLAMATION,this);
    return;
  }
  this->HideCfgNameEdit();
  this->UpdateConfigList();
}

void wxWinMain::OnNewConfigCancel(wxCommandEvent &WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  this->HideCfgNameEdit();
}

void wxWinMain::OnConfigDelete(wxCommandEvent &WXUNUSED(event))
{
  int iIndex;
  wxString strPath;
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(((iIndex=this->listGameConfigs->GetSelection()) == wxNOT_FOUND) || (iIndex == 0)) /* Don't allow default to be deleted */
    return;

  strPath=pcsx2Tool_m->GetUserCFGPath()+this->listGameConfigs->GetString(iIndex);
  if(wxMessageBox("Do you really want to delete Config \""+this->listGameConfigs->GetString(iIndex)+"\"?",
                  wxMessageBoxCaptionStr,
                  wxYES_NO | wxCENTER,
                  this) == wxYES)
  {
    if(!deleteFolderRecursive(strPath))
    {
      wxMessageBox("Failed to delete "+strPath,wxMessageBoxCaptionStr,
                   wxOK | wxCENTER | wxICON_EXCLAMATION,this);
    }
  }
  this->UpdateConfigList();
}

void wxWinMain::OnEditConfigStartPCSX2(wxCommandEvent &WXUNUSED(event))
{
  string strCmd;
  int iIndex;
  if((iIndex=this->listGameConfigs->GetSelection()) == wxNOT_FOUND)
    return;

  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  pcsx2Tool_m->CreateCommandLine_PCSX2StartWithCFG((iIndex)?this->listGameConfigs->GetString(iIndex).ToStdString():"",
                                                   strCmd);

  DEBUG_WXPUTS("Starting: "+strCmd);

  if(wxExecute(strCmd,wxEXEC_ASYNC) < 0)
  {
    wxMessageBox("Failed to start "+strCmd,
                 wxMessageBoxCaptionStr,
                 wxOK | wxCENTER | wxICON_EXCLAMATION,
                 this);
  }
}

void wxWinMain::OnClose(wxCloseEvent &WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(!pcsx2Tool_m->SaveToolConfig())
  {
    if(wxMessageBox("Failed to save tool configuration file to " + pcsx2Tool_m->GetToolCFGPath() + ", Quit anyway?",
                    "Saving Configuration failed",
                    wxYES_NO | wxCENTER | wxICON_EXCLAMATION,
                    this) == wxNO)
    {
      return;
    }
  }
  this->Destroy();
}