#include <string>
#include <string.h>

#include "pcsx2_multiconfigtool.h"

#define PCSX2_CMD_CFGPATH       "--cfgpath="
#define PCSX2_CMD_NOGUI         "--nogui"
#define PCSX2_CMD_FULLSCREEN    "--fullscreen"
#define PCSX2_CMD_FULLBOOT      "--fullboot"
#define PCSX2_CMD_FORCECONSOLE  "--console"
#define PCSX2_CMD_NO_HACKS      "--nohacks"

PCSX2Tool *pcsx2Tool_m;

PCSX2Tool::PCSX2Tool()
{
  this->taEntries[ENTRY_INDEX_PATH_GAMES].groupName="Paths";
  this->taEntries[ENTRY_INDEX_PATH_GAMES].keyName="PCSX2_Games";
  dataType_Set_String(&this->taEntries[ENTRY_INDEX_PATH_GAMES].tagData,
                      this->caBufPathGames,
                      sizeof(this->caBufPathGames),
                      PCSX2_TOOL_PATH_DEFAULT_GAMES,
                      sizeof(PCSX2_TOOL_PATH_DEFAULT_GAMES),
                      eRepr_String_Default);

  this->taEntries[ENTRY_INDEX_PATH_EXE].groupName="Paths";
  this->taEntries[ENTRY_INDEX_PATH_EXE].keyName="PCSX2_Executable";
  dataType_Set_String(&this->taEntries[ENTRY_INDEX_PATH_EXE].tagData,
                      this->caBufPathExe,
                      sizeof(this->caBufPathExe),
                      PCSX2_TOOL_PATH_DEFAULT_EXE,
                      sizeof(PCSX2_TOOL_PATH_DEFAULT_EXE),
                      eRepr_String_Default);

  this->taEntries[ENTRY_INDEX_PATH_PCSX2CFG].groupName="Paths";
  this->taEntries[ENTRY_INDEX_PATH_PCSX2CFG].keyName="PCSX2_Config";
  dataType_Set_String(&this->taEntries[ENTRY_INDEX_PATH_PCSX2CFG].tagData,
                      this->caBufPathPCSX2Cfg,
                      sizeof(this->caBufPathPCSX2Cfg),
                      PCSX2_TOOL_PATH_DEFAULT_CFG,
                      sizeof(PCSX2_TOOL_PATH_DEFAULT_CFG),
                      eRepr_String_Default);

  this->taEntries[ENTRY_INDEX_PATH_USRCFG].groupName="Paths";
  this->taEntries[ENTRY_INDEX_PATH_USRCFG].keyName="User_Config";
  dataType_Set_String(&this->taEntries[ENTRY_INDEX_PATH_USRCFG].tagData,
                      this->caBufPathUserCfg,
                      sizeof(this->caBufPathUserCfg),
                      PCSX2_TOOL_PATH_DEFAULT_USR_CFG,
                      sizeof(PCSX2_TOOL_PATH_DEFAULT_USR_CFG),
                      eRepr_String_Default);

  this->taEntries[ENTRY_INDEX_OPT_START_NOGUI].groupName="Options";
  this->taEntries[ENTRY_INDEX_OPT_START_NOGUI].keyName="ShortcutStartNoGUI";
  dataType_Set_Boolean(&this->taEntries[ENTRY_INDEX_OPT_START_NOGUI].tagData,0,eRepr_Boolean_Default);

  this->taEntries[ENTRY_INDEX_OPT_START_FULLSCREEN].groupName="Options";
  this->taEntries[ENTRY_INDEX_OPT_START_FULLSCREEN].keyName="ShortcutStartFullscreen";
  dataType_Set_Boolean(&this->taEntries[ENTRY_INDEX_OPT_START_FULLSCREEN].tagData,0,eRepr_Boolean_Default);

  this->taEntries[ENTRY_INDEX_OPT_START_FULLBOOT].groupName="Options";
  this->taEntries[ENTRY_INDEX_OPT_START_FULLBOOT].keyName="ShortcutStartFullBoot";
  dataType_Set_Boolean(&this->taEntries[ENTRY_INDEX_OPT_START_FULLBOOT].tagData,0,eRepr_Boolean_Default);

  this->taEntries[ENTRY_INDEX_OPT_START_FORCE_CONSOLE].groupName="Options";
  this->taEntries[ENTRY_INDEX_OPT_START_FORCE_CONSOLE].keyName="ShortcutStartForceConsole";
  dataType_Set_Boolean(&this->taEntries[ENTRY_INDEX_OPT_START_FORCE_CONSOLE].tagData,0,eRepr_Boolean_Default);

  this->taEntries[ENTRY_INDEX_OPT_START_NO_HACKS].groupName="Options";
  this->taEntries[ENTRY_INDEX_OPT_START_NO_HACKS].keyName="ShortcutStartNoHacks";
  dataType_Set_Boolean(&this->taEntries[ENTRY_INDEX_OPT_START_NO_HACKS].tagData,0,eRepr_Boolean_Default);
}

PCSX2Tool::~PCSX2Tool()
{
  appConfig_Close(this->appCfg);
}

int PCSX2Tool::iLoadConfig()
{
  int iRc;
  switch((iRc=appConfig_Open(&this->appCfg,
                             this->taEntries,
                             sizeof(this->taEntries)/sizeof(AppConfigEntry),
                             0,
                             APP_DISPLAY_NAME,
                             NULL,
                             NULL)))
  {
    case APPCFG_ERR_NONE:
      break;
    default:
      fprintf(stderr,"appConfig_New() failed (%d): %s",iRc,appConfig_GetErrorString(iRc));
      return(PCSX2_TOOL_CFG_ERR_INTERNAL);
  }
  switch((iRc=appConfig_DataLoad(this->appCfg)))
  {
    case APPCFG_LOAD_NEW:
      return(PCSX2_TOOL_CFG_NEW);
    case APPCFG_LOAD_EXISTING:
      return(PCSX2_TOOL_CFG_EXISTING);
    case APPCFG_ERR_DATA_MALFORMED:
      return(PCSX2_TOOL_CFG_ERR_DATA_CORRUPT);
    default:
      fprintf(stderr,"appConfig_DataLoad() failed (%d): %s",iRc,appConfig_GetErrorString(iRc));
  }
  return(PCSX2_TOOL_CFG_ERR_INTERNAL);
}

bool PCSX2Tool::bSaveToolConfig()
{
  int iRc;
  switch((iRc=appConfig_DataSave(this->appCfg)))
  {
    case APPCFG_ERR_NONE:
      break;
    default:
      fprintf(stderr,"appConfig_DataSave() failed (%d): %s",iRc,appConfig_GetErrorString(iRc));
      return(false);
  }
  return(true);
}

bool PCSX2Tool::bDeleteConfig()
{
  int iRc;
  switch((iRc=appConfig_DataDelete(this->appCfg)))
  {
    case APPCFG_ERR_NONE:
      break;
    default:
      fprintf(stderr,"appConfig_DataDelete() failed (%d): %s",iRc,appConfig_GetErrorString(iRc));
      return(false);
  }
  return(true);
}

string PCSX2Tool::GetPCSX2GamesPath()
{
  return(string(this->caBufPathGames));
}

string PCSX2Tool::GetPCSX2ExePath()
{
  return(string(this->caBufPathExe));
}

string PCSX2Tool::GetPCSX2CFGPath()
{
  return(string(this->caBufPathPCSX2Cfg));
}

string PCSX2Tool::GetUserCFGPath()
{
  return(string(this->caBufPathUserCfg));
}

string PCSX2Tool::GetToolCFGPath()
{
  return(string(appConfig_GetPath(this->appCfg)));
}

bool PCSX2Tool::GetOptionStartNoGUI()
{
  return((DATA_GET_BOOL(this->taEntries[ENTRY_INDEX_OPT_START_NOGUI].tagData)?true:false));
}

bool PCSX2Tool::GetOptionStartFullScreen()
{
  return((DATA_GET_BOOL(this->taEntries[ENTRY_INDEX_OPT_START_FULLSCREEN].tagData)?true:false));
}

bool PCSX2Tool::GetOptionStartFullBoot()
{
  return((DATA_GET_BOOL(this->taEntries[ENTRY_INDEX_OPT_START_FULLBOOT].tagData)?true:false));
}

bool PCSX2Tool::GetOptionStartForceConsole()
{
  return((DATA_GET_BOOL(this->taEntries[ENTRY_INDEX_OPT_START_FORCE_CONSOLE].tagData)?true:false));
}

bool PCSX2Tool::GetOptionStartNoHacks()
{
  return((DATA_GET_BOOL(this->taEntries[ENTRY_INDEX_OPT_START_NO_HACKS].tagData)?true:false));
}

bool PCSX2Tool::SetPCSX2GamesPath(const string &path)
{
  return((dataType_Update_String(&this->taEntries[ENTRY_INDEX_PATH_GAMES].tagData,path.c_str(),path.length()+1))?false:true);
}

bool PCSX2Tool::SetPCSX2ExePath(const string &path)
{
  return((dataType_Update_String(&this->taEntries[ENTRY_INDEX_PATH_EXE].tagData,path.c_str(),path.length()+1))?false:true);
}

bool PCSX2Tool::SetPCSX2CFGPath(const string &path)
{
  return((dataType_Update_String(&this->taEntries[ENTRY_INDEX_PATH_PCSX2CFG].tagData,path.c_str(),path.length()+1))?false:true);
}

bool PCSX2Tool::SetUserCFGPath(const string &path)
{
  return((dataType_Update_String(&this->taEntries[ENTRY_INDEX_PATH_USRCFG].tagData,path.c_str(),path.length()+1))?false:true);
}

void PCSX2Tool::SetOptionStartNoGUI(bool noGUI)
{
  dataType_Update_Boolean(&this->taEntries[ENTRY_INDEX_OPT_START_NOGUI].tagData,(noGUI)?1:0);
}

void PCSX2Tool::SetOptionStartFullScreen(bool fullscreen)
{
  dataType_Update_Boolean(&this->taEntries[ENTRY_INDEX_OPT_START_FULLSCREEN].tagData,(fullscreen)?1:0);
}

void PCSX2Tool::SetOptionStartFullBoot(bool fullboot)
{
  dataType_Update_Boolean(&this->taEntries[ENTRY_INDEX_OPT_START_FULLBOOT].tagData,(fullboot)?1:0);
}

void PCSX2Tool::SetOptionStartForceConsole(bool forceConsole)
{
  dataType_Update_Boolean(&this->taEntries[ENTRY_INDEX_OPT_START_FORCE_CONSOLE].tagData,(forceConsole)?1:0);
}

void PCSX2Tool::SetOptionStartNoHacks(bool noHacks)
{
  dataType_Update_Boolean(&this->taEntries[ENTRY_INDEX_OPT_START_NO_HACKS].tagData,(noHacks)?1:0);
}

void PCSX2Tool::CreateCommandLine_PCSX2StartWithCFG(const string &configName,
                                                    const string &gamePath,
                                                    string &cmdLine)
{
#ifdef _WIN32
  cmdLine="";
#elif __linux__
  cmdLine="#!/bin/sh\n";
#endif /* _WIN32 */
  cmdLine.append("\"");
  cmdLine.append(this->caBufPathExe);
  cmdLine.append("\" \"");
  cmdLine.append(gamePath);
  cmdLine.append("\"");
  if(configName != "")
  {
    cmdLine.append(" ");
    cmdLine.append(PCSX2_CMD_CFGPATH);
    cmdLine.append("\"");
    cmdLine.append(this->caBufPathUserCfg);
    cmdLine.append(configName);
    cmdLine.append("\"");
  }
  if(DATA_GET_BOOL(this->taEntries[ENTRY_INDEX_OPT_START_NOGUI].tagData))
  {
    cmdLine.append(" ");
    cmdLine.append(PCSX2_CMD_NOGUI);
  }
  if(DATA_GET_BOOL(this->taEntries[ENTRY_INDEX_OPT_START_FULLSCREEN].tagData))
  {
    cmdLine.append(" ");
    cmdLine.append(PCSX2_CMD_FULLSCREEN);
  }
  if(DATA_GET_BOOL(this->taEntries[ENTRY_INDEX_OPT_START_FULLBOOT].tagData))
  {
    cmdLine.append(" ");
    cmdLine.append(PCSX2_CMD_FULLBOOT);
  }
  if(DATA_GET_BOOL(this->taEntries[ENTRY_INDEX_OPT_START_FORCE_CONSOLE].tagData))
  {
    cmdLine.append(" ");
    cmdLine.append(PCSX2_CMD_FORCECONSOLE);
  }
  if(DATA_GET_BOOL(this->taEntries[ENTRY_INDEX_OPT_START_NO_HACKS].tagData))
  {
    cmdLine.append(" ");
    cmdLine.append(PCSX2_CMD_NO_HACKS);
  }
}

void PCSX2Tool::CreateCommandLine_PCSX2StartWithCFG(const string &configName,
                                                    string &cmdLine)
{
  cmdLine="\"";
  cmdLine.append(this->caBufPathExe);
  cmdLine.append("\"");
  if(configName != "")
  {
    cmdLine.append(" ");
    cmdLine.append(PCSX2_CMD_CFGPATH);
    cmdLine.append("\"");
    cmdLine.append(this->caBufPathUserCfg);
    cmdLine.append(configName);
    cmdLine.append("\"");
  }
}