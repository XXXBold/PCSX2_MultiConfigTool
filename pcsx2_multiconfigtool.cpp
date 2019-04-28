#include <string>
#include <string.h>

#include "pcsx2_multiconfigtool.h"

#define PCSX2_CMD_CFGPATH     "--cfgpath="
#define PCSX2_CMD_NOGUI       "--nogui"
#define PCSX2_CMD_FULLSCREEN  "--fullscreen"
#define PCSX2_CMD_FULLBOOT    "--fullboot"

PCSX2Tool *pcsx2Tool_m;

PCSX2Tool::PCSX2Tool(void)
{
  strcpy(this->caBufPathGames,PCSX2_TOOL_PATH_DEFAULT_GAMES);
  strcpy(this->caBufPathExe,PCSX2_TOOL_PATH_DEFAULT_EXE);
  strcpy(this->caBufPathPCSX2Cfg,PCSX2_TOOL_PATH_DEFAULT_CFG);
  strcpy(this->caBufPathUserCfg,PCSX2_TOOL_PATH_DEFAULT_USR_CFG);

  this->taEntries[ENTRY_INDEX_PATH_GAMES].groupName="Paths";
  this->taEntries[ENTRY_INDEX_PATH_GAMES].keyName="PCSX2_Games";
  DATA_SET_STRING(this->taEntries[ENTRY_INDEX_PATH_GAMES].tagData,this->caBufPathGames,sizeof(this->caBufPathGames));

  this->taEntries[ENTRY_INDEX_PATH_EXE].groupName="Paths";
  this->taEntries[ENTRY_INDEX_PATH_EXE].keyName="PCSX2_Executable";
  DATA_SET_STRING(this->taEntries[ENTRY_INDEX_PATH_EXE].tagData,this->caBufPathExe,sizeof(this->caBufPathExe));

  this->taEntries[ENTRY_INDEX_PATH_PCSX2CFG].groupName="Paths";
  this->taEntries[ENTRY_INDEX_PATH_PCSX2CFG].keyName="PCSX2_Config";
  DATA_SET_STRING(this->taEntries[ENTRY_INDEX_PATH_PCSX2CFG].tagData,this->caBufPathPCSX2Cfg,sizeof(this->caBufPathPCSX2Cfg));

  this->taEntries[ENTRY_INDEX_PATH_USRCFG].groupName="Paths";
  this->taEntries[ENTRY_INDEX_PATH_USRCFG].keyName="User_Config";
  DATA_SET_STRING(this->taEntries[ENTRY_INDEX_PATH_USRCFG].tagData,this->caBufPathUserCfg,sizeof(this->caBufPathUserCfg));

  this->taEntries[ENTRY_INDEX_OPT_START_NOGUI].groupName="Options";
  this->taEntries[ENTRY_INDEX_OPT_START_NOGUI].keyName="ShortcutStartNoGUI";
  DATA_SET_BOOL(this->taEntries[ENTRY_INDEX_OPT_START_NOGUI].tagData,0);

  this->taEntries[ENTRY_INDEX_OPT_START_FULLSCREEN].groupName="Options";
  this->taEntries[ENTRY_INDEX_OPT_START_FULLSCREEN].keyName="ShortcutStartFullscreen";
  DATA_SET_BOOL(this->taEntries[ENTRY_INDEX_OPT_START_FULLSCREEN].tagData,0);

  this->taEntries[ENTRY_INDEX_OPT_START_FULLBOOT].groupName="Options";
  this->taEntries[ENTRY_INDEX_OPT_START_FULLBOOT].keyName="ShortcutStartFullBoot";
  DATA_SET_BOOL(this->taEntries[ENTRY_INDEX_OPT_START_FULLBOOT].tagData,0);
}

PCSX2Tool::~PCSX2Tool(void)
{
  appConfig_Close(this->tagAppConfig);
}

int PCSX2Tool::LoadToolConfig(void)
{
  switch(appConfig_Load(&this->tagAppConfig,
                        APP_DISPLAY_NAME,
                        this->taEntries,
                        sizeof(this->taEntries)/sizeof(AppConfigEntry),
                        NULL,
                        NULL))
  {
    case APPCONFIG_LOAD_EXIST:
      return(PCSX2_TOOL_CFG_EXISTING);
    case APPCONFIG_LOAD_NEW:
      return(PCSX2_TOOL_CFG_NEW);
    case APPCONFIG_LOAD_ERROR:
    default:
      break;
  }
  return(PCSX2_TOOL_CFG_ERROR);
}

bool PCSX2Tool::SaveToolConfig(void)
{
  return((appConfig_Save(this->tagAppConfig)?false:true));
}

string PCSX2Tool::GetPCSX2GamesPath(void)
{
  return(string(this->caBufPathGames));
}

string PCSX2Tool::GetPCSX2ExePath(void)
{
  return(string(this->caBufPathExe));
}

string PCSX2Tool::GetPCSX2CFGPath(void)
{
  return(string(this->caBufPathPCSX2Cfg));
}

string PCSX2Tool::GetUserCFGPath(void)
{
  return(string(this->caBufPathUserCfg));
}

string PCSX2Tool::GetToolCFGPath(void)
{
  return(string(appConfig_GetPath(this->tagAppConfig)));
}

bool PCSX2Tool::GetOptionStartNoGUI(void)
{
  return((DATA_GET_BOOL(this->taEntries[ENTRY_INDEX_OPT_START_NOGUI].tagData)?true:false));
}

bool PCSX2Tool::GetOptionStartFullScreen(void)
{
  return((DATA_GET_BOOL(this->taEntries[ENTRY_INDEX_OPT_START_FULLSCREEN].tagData)?true:false));
}

bool PCSX2Tool::GetOptionStartFullBoot(void)
{
  return((DATA_GET_BOOL(this->taEntries[ENTRY_INDEX_OPT_START_FULLBOOT].tagData)?true:false));
}

bool PCSX2Tool::SetPCSX2GamesPath(const string &path)
{
  if(path.length() < MAX_PATHLEN-1)
  {
    memcpy(this->caBufPathGames,path.c_str(),path.length()+1);
    return(true);
  }
  return(false);
}

bool PCSX2Tool::SetPCSX2ExePath(const string &path)
{
  if(path.length() < MAX_PATHLEN-1)
  {
    memcpy(this->caBufPathExe,path.c_str(),path.length()+1);
    return(true);
  }
  return(false);
}

bool PCSX2Tool::SetPCSX2CFGPath(const string &path)
{
  if(path.length() < MAX_PATHLEN-1)
  {
    memcpy(this->caBufPathPCSX2Cfg,path.c_str(),path.length()+1);
    return(true);
  }
  return(false);
}

bool PCSX2Tool::SetUserCFGPath(const string &path)
{
  if(path.length() < MAX_PATHLEN-1)
  {
    memcpy(this->caBufPathUserCfg,path.c_str(),path.length()+1);
    return(true);
  }
  return(false);
}

void PCSX2Tool::SetOptionStartNoGUI(bool noGUI)
{
  DATA_SET_BOOL(this->taEntries[ENTRY_INDEX_OPT_START_NOGUI].tagData,(noGUI)?1:0);
}

void PCSX2Tool::SetOptionStartFullScreen(bool fullscreen)
{
  DATA_SET_BOOL(this->taEntries[ENTRY_INDEX_OPT_START_FULLSCREEN].tagData,(fullscreen)?1:0);
}

void PCSX2Tool::SetOptionStartFullBoot(bool fullboot)
{
  DATA_SET_BOOL(this->taEntries[ENTRY_INDEX_OPT_START_FULLBOOT].tagData,(fullboot)?1:0);
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
#ifdef _WIN32
#elif __linux__
#endif /* _WIN32 */
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