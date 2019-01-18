#include <iostream>
#include <string>
#include <cstdio>

#include "PCSX2_Tool.h"
#include "inifile.h"
#include "wxWidgets_functions.h"

#define APP_VERSION_MINOR 1
#define APP_VERSION_MAJOR 0
#define APP_VERSION_STAGE "Beta"

#define PCSX2_CMD_CFGPATH     "--cfgpath="
#define PCSX2_CMD_NOGUI       "--nogui"
#define PCSX2_CMD_FULLSCREEN  "--fullscreen"

#define DEBUG_TRACE_PRINTF(txt,...) printf(txt,__VA_ARGS__)
#define DEBUG_TRACE_TXT(txt) fputs(txt,stdout)

static const string strAppName_m="PCSX2_MultiConfigTool";

inline bool file_check_exists(const string& name);

PCSX2Tool::PCSX2Tool()
{
  this->pcSectionPaths      ="Paths";
  this->pcKeyPCSX2GamesPath ="PCSX2_Games";
  this->pcKeyPCSX2EXE       ="PCSX2_Executable";
  this->pcKeyPCSX2CFG       ="PCSX2_Config";
  this->pcKeyUserCFG        ="User_Config";
}

PCSX2Tool::~PCSX2Tool()
{
  IniFile_Delete(this->appCFGFile);
}

string PCSX2Tool::toolGetAppName(void)
{
  return(strAppName_m);
}

string PCSX2Tool::toolGetVersionString(void)
{
  string strTmp;
  char caTmp[20];
  sprintf(caTmp,"%d.%d (%s)",APP_VERSION_MAJOR,APP_VERSION_MINOR,APP_VERSION_STAGE);
  strTmp=caTmp;
  return(strTmp);
}

int PCSX2Tool::LoadToolConfig(const string &userConfigDir)
{
  this->strToolCFGPath=userConfigDir;
#ifdef _WIN32
  this->strToolCFGPath.append("\\config.ini");
#elif __linux__
  this->strToolCFGPath.append("/config.ini");
#endif
  if(file_check_exists(this->strToolCFGPath))
  {
    char caBuffer[200];
    DEBUG_TRACE_PRINTF("File %s exists, trying to read...\n",this->strToolCFGPath.data());
    if(!(this->appCFGFile=IniFile_Read(this->strToolCFGPath.data())))
      return(PCSX2_TOOL_CFG_ERROR);

    if(IniFile_FindEntry_GetValue(this->appCFGFile,this->pcSectionPaths,this->pcKeyPCSX2GamesPath,eIniDataType_String,caBuffer,sizeof(caBuffer)))
    {
      DEBUG_TRACE_PRINTF("Can't find [%s]->%s in inifile\n",this->pcSectionPaths,this->pcKeyPCSX2GamesPath);
      return(PCSX2_TOOL_CFG_ERROR);
    }
    this->strPCSX2GamesPath=caBuffer;
    if(IniFile_FindEntry_GetValue(this->appCFGFile,this->pcSectionPaths,this->pcKeyPCSX2EXE,eIniDataType_String,caBuffer,sizeof(caBuffer)))
    {
      DEBUG_TRACE_PRINTF("Can't find [%s]->%s in inifile\n",this->pcSectionPaths,this->pcKeyPCSX2EXE);
      return(PCSX2_TOOL_CFG_ERROR);
    }
    this->strPCSX2EXEPath=caBuffer;
    if(IniFile_FindEntry_GetValue(this->appCFGFile,this->pcSectionPaths,this->pcKeyPCSX2CFG,eIniDataType_String,caBuffer,sizeof(caBuffer)))
    {
      DEBUG_TRACE_PRINTF("Can't find [%s]->%s in inifile\n",this->pcSectionPaths,this->pcKeyPCSX2CFG);
      return(PCSX2_TOOL_CFG_ERROR);
    }
    this->strPCSX2CFGPath=caBuffer;
    if(IniFile_FindEntry_GetValue(this->appCFGFile,this->pcSectionPaths,this->pcKeyUserCFG,eIniDataType_String,caBuffer,sizeof(caBuffer)))
    {
      DEBUG_TRACE_PRINTF("Can't find [%s]->%s in inifile\n",this->pcSectionPaths,this->pcKeyUserCFG);
      return(PCSX2_TOOL_CFG_ERROR);
    }
    this->strUserCFGPath=caBuffer;
    return(PCSX2_TOOL_CFG_LOAD);
  }
  /* Use defaults... */
  DEBUG_TRACE_PRINTF("creating new File %s...\n",this->strToolCFGPath.data());
  if(!(this->appCFGFile=IniFile_New()))
    return(PCSX2_TOOL_CFG_ERROR);

#ifdef _WIN32
  this->strPCSX2GamesPath ="C:\\Users\\";
  this->strPCSX2EXEPath   ="C:\\Program Files (x86)\\";
  this->strPCSX2CFGPath   ="C:\\Users\\";
  this->strUserCFGPath    ="C:\\Users\\";
#elif __linux__
  this->strPCSX2GamesPath ="/home/";
  this->strPCSX2EXEPath   ="/bin/games/";
  this->strPCSX2CFGPath   ="/home/";
  this->strUserCFGPath    ="/home/";
#endif
  return(PCSX2_TOOL_CFG_NEW);
}

bool PCSX2Tool::SaveToolConfig()
{
  if(IniFile_CreateEntry_SetValue(this->appCFGFile,this->pcSectionPaths,this->pcKeyPCSX2GamesPath,eIniDataType_String,(void*)this->strPCSX2GamesPath.data()))
  {
    DEBUG_TRACE_PRINTF("IniFile_CreateEntry_SetValue() failed @[%s]->%s=%s",
                       this->pcSectionPaths,
                       this->pcKeyPCSX2EXE,
                       this->strPCSX2GamesPath.data());
    return(false);
  }
  if(IniFile_CreateEntry_SetValue(this->appCFGFile,this->pcSectionPaths,this->pcKeyPCSX2EXE,eIniDataType_String,(void*)this->strPCSX2EXEPath.data()))
  {
    DEBUG_TRACE_PRINTF("IniFile_CreateEntry_SetValue() failed @[%s]->%s=%s",
                       this->pcSectionPaths,
                       this->pcKeyPCSX2EXE,
                       this->strPCSX2EXEPath.data());
    return(false);
  }
  if(IniFile_CreateEntry_SetValue(this->appCFGFile,this->pcSectionPaths,this->pcKeyPCSX2CFG,eIniDataType_String,this->strPCSX2CFGPath.data()))
  {
    DEBUG_TRACE_PRINTF("IniFile_CreateEntry_SetValue() failed @[%s]->%s=%s",
                       this->pcSectionPaths,
                       this->pcKeyPCSX2CFG,
                       this->strPCSX2CFGPath.data());
    return(false);
  }
  if(IniFile_CreateEntry_SetValue(this->appCFGFile,this->pcSectionPaths,this->pcKeyUserCFG,eIniDataType_String,this->strUserCFGPath.data()))
  {
    DEBUG_TRACE_PRINTF("IniFile_CreateEntry_SetValue() failed @[%s]->%s=%s",
                       this->pcSectionPaths,
                       this->pcKeyUserCFG,
                       this->strUserCFGPath.data());
    return(false);
  }
  if(IniFile_Write(this->appCFGFile,this->strToolCFGPath.data()))
  {
    DEBUG_TRACE_PRINTF("IniFile_Write() failed @ path %s\n",this->strToolCFGPath.data());
    return(false);
  }
  DEBUG_TRACE_TXT("Saved tool config!\n");
  return(true);
}

string PCSX2Tool::GetPCSX2GamesPath()
{
  return(this->strPCSX2GamesPath);
}

string PCSX2Tool::GetPCSX2ExePath()
{
  return(this->strPCSX2EXEPath);
}

string PCSX2Tool::GetPCSX2CFGPath()
{
  return(this->strPCSX2CFGPath);
}

string PCSX2Tool::GetUserCFGPath()
{
  return(this->strUserCFGPath);
}

string PCSX2Tool::GetToolCFGPath()
{
  return(this->strToolCFGPath);
}

void PCSX2Tool::SetPCSX2GamesPath(const string &path)
{
  DEBUG_TRACE_PRINTF("Set PCSX2-Games-Path: %s\n",path.data());
  this->strPCSX2GamesPath=path;
  if(path.back()!=wxFILE_SEP_PATH)
    this->strPCSX2GamesPath.push_back(wxFILE_SEP_PATH);
}

void PCSX2Tool::SetPCSX2ExePath(const string &path)
{
  DEBUG_TRACE_PRINTF("Set PCSX2-EXE-Path: %s\n",path.data());
  this->strPCSX2EXEPath=path;
}

void PCSX2Tool::SetPCSX2CFGPath(const string &path)
{
  DEBUG_TRACE_PRINTF("Set PCSX2-CFG-Path: %s\n",path.data());
  this->strPCSX2CFGPath=path;
  if(path.back()!=wxFILE_SEP_PATH)
    this->strPCSX2CFGPath.push_back(wxFILE_SEP_PATH);
}

void PCSX2Tool::SetUserCFGPath(const string &path)
{
  DEBUG_TRACE_PRINTF("Set User-CFG-Path: %s\n",path.data());
  this->strUserCFGPath=path;
  if(path.back()!=wxFILE_SEP_PATH)
    this->strUserCFGPath.push_back(wxFILE_SEP_PATH);
}

void PCSX2Tool::CreateCommandLine_PCSX2StartWithCFG(const string &configName,
                                                    const string &gamePath,
                                                    string &cmdLine)
{
  cmdLine="\"";
  cmdLine.append(this->strPCSX2EXEPath);
  cmdLine.append("\" \"");
  cmdLine.append(gamePath);
  cmdLine.append("\"");
  if(configName!="")
  {
    cmdLine.append(" ");
    cmdLine.append(PCSX2_CMD_CFGPATH);
    cmdLine.append("\"");
    cmdLine.append(this->strUserCFGPath);
    cmdLine.append(configName);
    cmdLine.append("\"");
  }
}

void PCSX2Tool::CreateCommandLine_PCSX2StartWithCFG(const string &configName,
                                  string &cmdLine)
{
  cmdLine="\"";
  cmdLine.append(this->strPCSX2EXEPath);
  cmdLine.append("\"");
  if(configName!="")
  {
    cmdLine.append(" ");
    cmdLine.append(PCSX2_CMD_CFGPATH);
    cmdLine.append("\"");
    cmdLine.append(this->strUserCFGPath);
    cmdLine.append(configName);
    cmdLine.append("\"");
  }
}

inline bool file_check_exists(const string& name)
{
  if(FILE *file = fopen(name.c_str(), "r"))
  {
    fclose(file);
    return true;
  }
  else
  {
    return false;
  }
}
