#include <wx/wxprec.h>
#ifndef WX_PRECOMP
  #include <wx/wx.h>
  #include <wx/filename.h>
  #include <wx/aboutdlg.h>
#endif

#include "wxwin_main.h"
#include "wxwin_functions.h"

/**
 * Build-System will generate license.h-file on build
 */
#include "license.h"

#ifdef __linux__
  #include "pcsx2tool_logo.xpm"
#endif /* __linux__ */

enum
{
  // menu items
  MainWindow_OptionsStartNoGUI = 1,
  MainWindow_OptionsStartFullscreen,
  MainWindow_OptionsStartFullBoot,
  MainWindow_OptionsStartForceConsole,
  MainWindow_OptionsStartNoHacks,
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
  BTN_CreateGameShortcut,

  //Timer
  Timer_WinMain=1,
};

wxBEGIN_EVENT_TABLE(wxWinMain,wxFrame)
EVT_MENU(MainWindow_About,wxWinMain::OnAbout)
EVT_MENU(MainWindow_OpenPathConfig,wxWinMain::OnPathSelect)
EVT_MENU(MainWindow_CreateGameShortcut,wxWinMain::OnShortcutCreate)

EVT_MENU(MainWindow_OptionsStartNoGUI,wxWinMain::OnOptionChangeStartNoGUI)
EVT_MENU(MainWindow_OptionsStartFullscreen,wxWinMain::OnOptionChangeStartFullscreen)
EVT_MENU(MainWindow_OptionsStartFullBoot,wxWinMain::OnOptionChangeStartFullBoot)
EVT_MENU(MainWindow_OptionsStartForceConsole,wxWinMain::OnOptionChangeStartForceConsole)
EVT_MENU(MainWindow_OptionsStartNoHacks,wxWinMain::OnOptionChangeStartNoHacks)

EVT_BUTTON(BTN_NewConfig,wxWinMain::OnNewConfig)
EVT_BUTTON(BTN_RenameConfig,wxWinMain::OnRenameConfig)
EVT_BUTTON(BTN_NewConfigSave,wxWinMain::OnNewConfigSave)
EVT_BUTTON(BTN_NewConfigCancel,wxWinMain::OnNewConfigCancel)
EVT_BUTTON(BTN_ConfigDelete,wxWinMain::OnConfigDelete)
EVT_BUTTON(BTN_ConfigStartPCSX2,wxWinMain::OnStartPCSX2WithConfig)

EVT_TIMER(Timer_WinMain,wxWinMain::OnTimerTick)

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

  switch(pcsx2Tool_m->iLoadConfig())
  {
    case PCSX2_TOOL_CFG_ERR_DATA_CORRUPT:
      if(wxMessageBox("Failed to load config, Data is corrupted\n"
                      "Do you want to delete the configuration? Will be created new if you restart this Tool.",
                      "Configuration data corrupted",
                      wxYES_NO | wxICON_EXCLAMATION | wxCENTER) == wxYES)
      {
        if(!pcsx2Tool_m->bDeleteConfig())
        {
          wxMessageBox("Can't delete the configuration, this should not happen.\n"
                       "Please try to launch the Tool from a console, and report any output you see there.",
                       "Failed to delete configuration",
                       wxOK | wxICON_ERROR | wxCENTER);
        }
      }
      return(false);
    case PCSX2_TOOL_CFG_ERR_INTERNAL:
      wxMessageBox("Failed to load configuration, this should not happen.\n"
                   "Please try to launch the Tool from a console, and report any output you see there.",
                   "Error loading configuration",
                   wxOK | wxICON_ERROR | wxCENTER);
      return(false);
    case PCSX2_TOOL_CFG_NEW:
      bFirstStart=true;
    case PCSX2_TOOL_CFG_EXISTING:
      break;
  }
  wxWinMain *winMain = new wxWinMain(NULL,
                                     wxID_ANY,
                                     APP_DISPLAY_NAME);
  winMain->wxPathCfg = new wxWinPathConfigurator(winMain,wxID_ANY,"Select Paths");
  winMain->Show(true);
  winMain->Update();
  if(bFirstStart)
  {
    if(wxMessageBox("This seems to be the First Launch of the " APP_DISPLAY_NAME ", do you want to Setup Paths now?",
                    "First Launch",
                    wxYES_NO | wxICON_QUESTION | wxCENTER, // wxCENTER is not working as intended here (Centers on top left corner of wxWinMain)
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

  wxStaticText* strDesc_1;
  strDesc_1 = new wxStaticText( this, wxID_ANY, wxT("Manage your Configs here"), wxDefaultPosition, wxDefaultSize, 0 );
  strDesc_1->Wrap( -1 );
  layCFGEdit->Add( strDesc_1, 0, wxALL, 5 );


  layCFGEdit->Add( 0, 0, 1, wxEXPAND, 5 );

  wxStaticText* strDesc_2;
  strDesc_2 = new wxStaticText( this, wxID_ANY, wxT("The <Default> Entry is your default PCSX2 Config"), wxDefaultPosition, wxDefaultSize, 0 );
  strDesc_2->Wrap( -1 );
  layCFGEdit->Add( strDesc_2, 0, wxALL, 5 );


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

  btnConfigStartPCSX2 = new wxButton( this, BTN_ConfigStartPCSX2, wxT("Start PCSX2 with selected Config"), wxDefaultPosition, wxDefaultSize, 0 );
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
  menuItemStartNoGUI = new wxMenuItem( menuSubGameShortcuts, MainWindow_OptionsStartNoGUI, wxString( wxT("Disable PCSX2-GUI on Startup") ) , wxEmptyString, wxITEM_CHECK );
  menuSubGameShortcuts->Append( menuItemStartNoGUI );

  menuItemStartFullscreen = new wxMenuItem( menuSubGameShortcuts, MainWindow_OptionsStartFullscreen, wxString( wxT("Start in Fullscreen") ) , wxEmptyString, wxITEM_CHECK );
  menuSubGameShortcuts->Append( menuItemStartFullscreen );

  menuItemStartFullBoot = new wxMenuItem( menuSubGameShortcuts, MainWindow_OptionsStartFullBoot, wxString( wxT("Use Full Boot on Startup") ) , wxEmptyString, wxITEM_CHECK );
  menuSubGameShortcuts->Append( menuItemStartFullBoot );

  menuItemStartForceConsole = new wxMenuItem( menuSubGameShortcuts, MainWindow_OptionsStartForceConsole, wxString( wxT("Force PCSX2-Console to be visible") ) , wxEmptyString, wxITEM_CHECK );
  menuSubGameShortcuts->Append( menuItemStartForceConsole );

  menuItemStartNoHacks = new wxMenuItem( menuSubGameShortcuts, MainWindow_OptionsStartNoHacks, wxString( wxT("Disable Speedhacks") ) , wxEmptyString, wxITEM_CHECK );
  menuSubGameShortcuts->Append( menuItemStartNoHacks );

  menuOptions->Append( menuSubGameShortcutsItem );

  m_menubar2->Append( menuOptions, wxT("&Options") );

  menuHelp = new wxMenu();
  wxMenuItem* menuItemAbout;
  menuItemAbout = new wxMenuItem( menuHelp, MainWindow_About, wxString( wxT("&About\tF1") ) , wxEmptyString, wxITEM_NORMAL );
  menuHelp->Append( menuItemAbout );

  m_menubar2->Append( menuHelp, wxT("&Help") );

  this->SetMenuBar( m_menubar2 );

  wxTimerMain.SetOwner( this, Timer_WinMain );


  /* Loaded in ressource file (.rc) on Windows, for linux: included above */
  this->appIcon=wxICON(pcsx2tool_logo);
}

bool wxWinMain::Show(bool show)
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(show)
  {
    this->SetIcon(this->appIcon);
    this->bRenameConfig=false;
    this->menuItemStartNoGUI->Check(pcsx2Tool_m->GetOptionStartNoGUI());
    this->menuItemStartFullscreen->Check(pcsx2Tool_m->GetOptionStartFullScreen());
    this->menuItemStartFullBoot->Check(pcsx2Tool_m->GetOptionStartFullBoot());
    this->menuItemStartForceConsole->Check(pcsx2Tool_m->GetOptionStartForceConsole());
    this->menuItemStartNoHacks->Check(pcsx2Tool_m->GetOptionStartNoHacks());

    this->Centre(wxBOTH);
#ifdef __WXGTK__ /* GTK has a native control for licenses */
    this->wAbout.SetLicense(ucaLicense_m);
#else /* Shorten License text for non-native controls */
    this->wAbout.SetLicense(wxString::FromAscii(ucaLicense_m).Left(250) +
                            "...\n"
                            "Check out the Homepage for full License!");
#endif

    this->wAbout.SetIcon(this->appIcon);
    this->wAbout.SetName(APP_DISPLAY_NAME);
    this->wAbout.SetVersion(wxString::Format("Version %d.%d.%d (%s)\n"
                                             "%s (%s)",
                                             APP_VERSION_MAJOR,
                                             APP_VERSION_MINOR,
                                             APP_VERSION_PATCH,
                                             APP_VERSION_DEVSTATE,
                                             APP_VERSION_BUILDTIME,
                                             APP_VERSION_ARCH));
    this->wAbout.SetDescription("Welcome to the " APP_DISPLAY_NAME "!\n"
                                "This is a Tool to manage multiple Configs for PCSX2\n"
                                "For proper functionality, make sure all Paths are set correctly.\n"
                                "\n"
                                "If you need help, found a bug, or have ideas for new features or improvements, visit the website of the Project, and let me know.\n");
    this->wAbout.SetCopyright("(C) 2018-2021 by XXXBold");

    this->wAbout.SetWebSite(APP_PROJECT_HOMEPAGE);
    this->wAbout.AddDeveloper(APP_PROJECT_DEVELOPER);
    this->btnConfigStartPCSX2->SetLabel(this->strTextBtnStartPCSX2_NotRunning);
    this->lPCSX2ProcessPid=0;
  }
  return(wxFrame::Show(show));
}

void wxWinMain::Update()
{
  if(!this->wxPathCfg->PathConfigIsValid())
  {
    DEBUG_WXPUTS("User/pcsx2 CFG path invalid!");
    this->menuItemCreateGameShortcut->Enable(false);
    this->layCFGEdit->Show(false);
  }
  else
  {
    this->UpdateConfigList();
    this->menuItemCreateGameShortcut->Enable(true);
    this->layCFGEdit->Show(true);
    this->laySaveCancelBtn->Show(false);

    this->layCFGEdit->Layout(); // Recalculate layout
    this->Fit();
    this->txtNewCFGName->Show(false); //hiding after layout fixes window beeing too small at startup
  }

  wxWindow::Update();
}

void wxWinMain::UpdateConfigList()
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
                    wxYES_NO | wxICON_QUESTION | wxCENTER,
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
                  wxYES_NO | wxICON_QUESTION | wxCENTER,
                  this) == wxYES)
  {
    iIndex|=wxPOSIX_USER_EXECUTE;
  }
#endif /* __linux__ */
  if(!newFile.Create(strPath,true,iIndex))
  {
    wxMessageBox("Failed to Create file "+strPath,
                 wxMessageBoxCaptionStr,
                 wxOK | wxICON_EXCLAMATION | wxCENTER,
                 this);
    return;
  }
  newFile.Write(strCMDLine);
  newFile.Close();
}

void wxWinMain::OnOptionChangeStartNoGUI(wxCommandEvent &WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  pcsx2Tool_m->SetOptionStartNoGUI(this->menuItemStartNoGUI->IsChecked());
}

void wxWinMain::OnOptionChangeStartFullscreen(wxCommandEvent &WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  pcsx2Tool_m->SetOptionStartFullScreen(this->menuItemStartFullscreen->IsChecked());
}

void wxWinMain::OnOptionChangeStartFullBoot(wxCommandEvent &WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  pcsx2Tool_m->SetOptionStartFullBoot(this->menuItemStartFullBoot->IsChecked());
}

void wxWinMain::OnOptionChangeStartForceConsole(wxCommandEvent &WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  pcsx2Tool_m->SetOptionStartForceConsole(this->menuItemStartForceConsole->IsChecked());
}

void wxWinMain::OnOptionChangeStartNoHacks(wxCommandEvent &WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  pcsx2Tool_m->SetOptionStartNoHacks(this->menuItemStartNoHacks->IsChecked());
}

void wxWinMain::OnAbout(wxCommandEvent &WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  wxAboutBox(this->wAbout,this);
}

void wxWinMain::EnableEditControls(bool enable)
{
  this->btnConfigCreateNew->Enable(enable);
  this->btnConfigRename->Enable(enable);
  this->btnConfigDelete->Enable(enable);
  this->listGameConfigs->Enable(enable);
}

void wxWinMain::OnNewConfig(wxCommandEvent &WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if((this->iCurrSelection=this->listGameConfigs->GetSelection()) == wxNOT_FOUND)
    return;

  if(wxMessageBox("Do you want to create a new Config based on \""+this->listGameConfigs->GetString(this->iCurrSelection)+"\"?",
                  wxMessageBoxCaptionStr,
                  wxYES_NO | wxICON_QUESTION | wxCENTER,
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
  this->EnableEditControls(false);
  this->Fit();

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

  this->Fit();
  this->EnableEditControls(true);
}

void wxWinMain::TimerEnable(bool enable)
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(enable)
  {
    this->wxTimerMain.Start(300);
  }
  else
    this->wxTimerMain.Stop();
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
                   wxOK | wxICON_EXCLAMATION | wxCENTER,
                   this);
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
                 wxOK | wxICON_EXCLAMATION | wxCENTER,
                 this);
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
                  wxYES_NO | wxICON_QUESTION | wxCENTER,
                  this) == wxYES)
  {
    if(!deleteFolderRecursive(strPath))
    {
      wxMessageBox("Failed to delete "+strPath,
                   wxMessageBoxCaptionStr,
                   wxOK | wxICON_EXCLAMATION | wxCENTER,
                   this);
    }
  }
  this->UpdateConfigList();
}

void wxWinMain::OnStartPCSX2WithConfig(wxCommandEvent &WXUNUSED(event))
{
  string strCmd;
  int iIndex;
  if((iIndex=this->listGameConfigs->GetSelection()) == wxNOT_FOUND)
    return;

  DEBUG_WXPUTS(__PRETTY_FUNCTION__);

  strCmd=pcsx2Tool_m->GetPCSX2ExePath();
  if(!wxFileExists(wxString(strCmd)))
  {/* Cannot start if executable doesn't exist */
    wxMessageBox(wxString("PCSX2 Executable \"" + strCmd + "\" not found, can't launch.\n"
                 "Please setup paths correctly (File->Set Paths...)"),
                 wxMessageBoxCaptionStr,
                 wxOK | wxICON_EXCLAMATION | wxCENTER,
                 this);
    return;
  }

  pcsx2Tool_m->CreateCommandLine_PCSX2StartWithCFG((iIndex)?this->listGameConfigs->GetString(iIndex).ToStdString():"",
                                                   strCmd);

  DEBUG_WXPUTS("Starting: " + strCmd);

  if(!runCommandAsync(strCmd,
                      this->lPCSX2ProcessPid))
  {
    wxMessageBox("Failed to start " + strCmd,
                 wxMessageBoxCaptionStr,
                 wxOK | wxICON_EXCLAMATION | wxCENTER,
                 this);
    this->lPCSX2ProcessPid=0;
    return;
  }
  this->btnConfigStartPCSX2->SetLabel(this->strTextBtnStartPCSX2_Running + "\"" +this->listGameConfigs->GetString(iIndex) + "\"...");
  this->btnConfigStartPCSX2->Enable(false);
  this->btnConfigStartPCSX2->Fit(); // Fit button to text size
  this->TimerEnable(true);
  this->EnableEditControls(false); // Disable edit controls, if PCSX2 is started
  DEBUG_WXPUTS(wxString::Format("Started process with PID %ld",
                                this->lPCSX2ProcessPid));
}

void wxWinMain::OnClose(wxCloseEvent &WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(!pcsx2Tool_m->bSaveToolConfig())
  {
    if(wxMessageBox("Failed to save tool configuration file to " + pcsx2Tool_m->GetToolCFGPath() + ", Quit anyway?",
                    "Saving Configuration failed",
                    wxYES_NO | wxICON_EXCLAMATION | wxCENTER,
                    this) == wxNO)
    {
      return;
    }
  }
  this->Destroy();
}

void wxWinMain::OnTimerTick(wxTimerEvent &WXUNUSED(event))
{
  DEBUG_WXPUTS(__PRETTY_FUNCTION__);
  if(checkPIDActive(this->lPCSX2ProcessPid))
  {/* Still running */
    return;
  }
  this->lPCSX2ProcessPid=0;
  this->btnConfigStartPCSX2->Enable(true);
  this->EnableEditControls(true); // reenable edit controls, if PCSX2 is closed
  this->TimerEnable(false);
  this->btnConfigStartPCSX2->SetLabel(this->strTextBtnStartPCSX2_NotRunning);
  this->btnConfigStartPCSX2->Fit(); // Fit button to text size
}

